/*
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/atom/forge.h>
#include <lv2/lv2plug.in/ns/ext/atom/util.h>
#include <lv2/lv2plug.in/ns/ext/log/logger.h>
#include <lv2/lv2plug.in/ns/ext/midi/midi.h>
#include "lv2/lv2plug.in/ns/ext/time/time.h"
#include <lv2/lv2plug.in/ns/ext/urid/urid.h>
#include <lv2/lv2plug.in/ns/lv2core/lv2.h>



typedef enum {
  IN = 0, 
  CV1, 
  VELOCITY, 
  TRIGGER,
  OCTAVE,
  SEMITONE,
  CENT,
  RETRIGGER
} PortEnum;



typedef struct {

    // URIDs
    LV2_URID urid_midiEvent;

    // atom ports
    const LV2_Atom_Sequence* port_events_in;
    int notesPressed;

//===========================================

    // keep track of active notes
    uint8_t activeNotesList[8];
    uint8_t triggerIndex;
    uint8_t activeNotes;
    uint8_t newNotes;
    uint8_t holdNote;
    uint8_t activeVelocity;
    uint8_t newVelocity;
    uint8_t prevMsg[2];
    uint8_t reTriggered;
    size_t  notesIndex;
    uint8_t reTriggerBuffer[8];
    bool activePorts;

    
    float *pitch;
    float *velocity;
    float *trigger;
    float *octave;
    float *semitone;
    float *cent;
    float *reTrigger;

    bool triggerState;

//===========================================
} Data;

// Struct for a 3 byte MIDI event
typedef struct {
    LV2_Atom_Event event;
    uint8_t        msg[3];
} LV2_Atom_MIDI;

static LV2_Handle instantiate(const LV2_Descriptor*     descriptor,
                              double                    rate,
                              const char*               path,
                              const LV2_Feature* const* features)
{
    Data* self = (Data*)calloc(1, sizeof(Data));

    // Get host features
    const LV2_URID_Map* map = NULL;

    for (int i = 0; features[i]; ++i) {
        if (!strcmp(features[i]->URI, LV2_URID__map)) {
            map = (const LV2_URID_Map*)features[i]->data;
            break;
        }
    }
    if (!map) {
        free(self);
        return NULL;
    }

    // Map URIs
    self->urid_midiEvent = map->map(map->handle, LV2_MIDI__MidiEvent);
    self->notesPressed = 0;

//===========================================
    memset(self->activeNotesList, 200, sizeof(uint8_t)*8);
    memset(self->reTriggerBuffer, 0, sizeof(uint8_t)*8);
    self->triggerIndex = 0;
    self->reTriggered = 200;
    self->activeNotes = 0;
    self->activeVelocity = 0;
    self->activePorts = false;
    self->newNotes = 0;
    self->newVelocity = 0;    
    self->notesPressed = 0;
    self->notesIndex = 0;
    self->triggerState = false;
//===========================================
    return self;
}

static void connect_port(LV2_Handle instance, uint32_t port, void* data)
{
    Data* self = (Data*)instance;

    switch (port)
    {
      case IN:
        self->port_events_in = (const LV2_Atom_Sequence*)data;
        break;
      case CV1:
        self->pitch = (float*) data;
        break;
      case VELOCITY:
        self->velocity = (float*) data;
        break;
      case TRIGGER:
        self->trigger = (float*) data;
        break;
      case OCTAVE:
        self->octave = (float*) data;
        break;
      case SEMITONE:
        self->semitone = (float*) data;
        break;
      case CENT:
        self->cent = (float*) data;
        break;
      case RETRIGGER:
        self->reTrigger = (float*) data;
        break;
    }
}

static void activate(LV2_Handle instance)
{
}

static void
setPortState(Data* self, int status)
{
  self->activePorts = status;
  self->triggerState = status;
}

static void run(LV2_Handle instance, uint32_t sample_count)
{
  Data* self = (Data*)instance;
  float *pitch = self->pitch;
  float *velocity = self->velocity;
  float *trigger = self->trigger;
  float  oC = *self->octave;
  float  sC = *self->semitone;
  float  cC = *self->cent;
  int storedNotes = 8;
  bool retrigger = true;

  // Read incoming events
  LV2_ATOM_SEQUENCE_FOREACH(self->port_events_in, ev)
  {
    if (ev->body.type == self->urid_midiEvent)
    {
      const uint8_t* const msg = (const uint8_t*)(ev + 1);
      const uint8_t status  = msg[0] & 0xF0;

      int storeN = 0;
      bool emptySlot = false;
      int notesIndex = storedNotes - 1;
      bool noteFound = false;

      switch (status)
      {
        case LV2_MIDI_MSG_NOTE_ON:
          while (!emptySlot && storeN < storedNotes) {
            if (self->activeNotesList[storeN] == 200) {
              self->activeNotesList[storeN] = msg[1];
              emptySlot = true;
            }
            storeN++;
          }
          self->activeNotes = msg[1];
          self->activeVelocity = msg[2];
          self->triggerIndex = (self->triggerIndex + 1) % 8;
          self->reTriggerBuffer[self->triggerIndex] = 1;
          self->reTriggered = msg[1];
          break;
        case LV2_MIDI_MSG_NOTE_OFF:
          self->notesPressed--;
          for (int notesIndex = 0; notesIndex < storedNotes; notesIndex++) {
              if (msg[1] == self->activeNotesList[notesIndex]) {
                  self->activeNotesList[notesIndex] = 200;
              }
          }
          while (!noteFound && notesIndex >= 0) {
              if (self->activeNotesList[notesIndex] < 200){
                  self->activeNotes = self->activeNotesList[notesIndex];
                  if(retrigger && self->activeNotes != self->reTriggered){
                      self->reTriggered = msg[1];
                  }
                  noteFound = true;
              }
              notesIndex--;
          }
          break;
        default:
          break;
      }
    }
  }
  int checked_note = 0;
  bool active_notes_found = false;
  while(!active_notes_found && checked_note < storedNotes) {
      if (self->activeNotesList[checked_note] != 200)
          active_notes_found = true;
      checked_note++;
  }

  if (active_notes_found) {
      setPortState(self, 1);
  } else {
      setPortState(self, 0);
      self->activeVelocity = 0;
  }

  for(uint32_t i=0;i<sample_count;i++)
  {
    pitch[i] = (0.0f + (float)((oC) + (sC/12.0f)+(cC/1200.0f)) + ((float)self->activeNotes * 1/12.0f));
    velocity[i] = (0.0f + ((float)self->activeVelocity * 1/12.0f));
    trigger[i] = ((self->triggerState == true) ? 10.0f : 0.0f);
    if (self->reTriggerBuffer[self->triggerIndex] == 1 && *self->reTrigger == 1.0) {
      self->reTriggerBuffer[self->triggerIndex] = 0;
      trigger[i] = 0.0f;
    }
  }
}

static void cleanup(LV2_Handle instance)
{
    free(instance);
}

static const LV2_Descriptor descriptor = {
    .URI = "http://moddevices.com/plugins/mod-devel/midi-to-cv-mono",
    .instantiate = instantiate,
    .connect_port = connect_port,
    .activate = activate,
    .run = run,
    .deactivate = NULL,
    .cleanup = cleanup,
    .extension_data = NULL
};

LV2_SYMBOL_EXPORT
const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
    return (index == 0) ? &descriptor : NULL;
}
