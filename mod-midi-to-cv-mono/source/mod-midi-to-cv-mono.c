#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/atom/forge.h>
#include <lv2/lv2plug.in/ns/ext/log/logger.h>
#include <lv2/lv2plug.in/ns/ext/midi/midi.h>
#include "lv2/lv2plug.in/ns/ext/time/time.h"
#include <lv2/lv2plug.in/ns/ext/urid/urid.h>

/**********************************************************************************************************************************************************/
#define PLUGIN_URI "http://moddevices.com/plugins/mod-devel/midi-to-cv-mono"

typedef enum {IN, CV1, VELOCITY, TRIGGER,OCTAVE,SEMITONE,CENT}PortIndex;

typedef struct
{
    // keep track of active notes
    uint8_t activeNotesList[4];
    uint8_t activeNotes;
    uint8_t newNotes;
    uint8_t activeVelocity;
    uint8_t newVelocity;
    uint8_t notesPressed;
    uint8_t prevMsg[2];
    size_t  notesIndex;
    bool activePorts;

    LV2_URID urid_midiEvent;    
    //ports
    const LV2_Atom_Sequence* port_events_in;
    
    float *pitch;
    float *velocity;
    float *trigger;
    float *octave;
    float *semitone;
    float *cent;

    bool triggerState;
}Midicv;

// Struct for a 3 byte MIDI event
typedef struct
{
    LV2_Atom_Event event;
    uint8_t msg[3];
}LV2_Atom_MIDI;

/**********************************************************************************************************************************************************/
//functions


/**********************************************************************************************************************************************************/
static LV2_Handle
instantiate(const LV2_Descriptor*   descriptor,
double                              samplerate,
const char*                         bundle_path,
const LV2_Feature* const* features)
{
    Midicv* self = (Midicv*)malloc(sizeof(Midicv));

    //get host features
    const LV2_URID_Map* map = NULL;

    for (int i = 0; features[i]; ++i)
    {
        if (!strcmp(features[i]->URI, LV2_URID__map))
        {
            map = (const LV2_URID_Map*)features[i]->data;
            break;
        }
    }
    if (!map)
    {
        free(self);
        return NULL;
    }

    // Map URIs
    self->urid_midiEvent = map->map(map->handle, LV2_MIDI__MidiEvent);

    memset(self->activeNotesList, 200, sizeof(uint8_t)*4);
    self->activeNotes = 0;
    self->activeVelocity = 0;
    self->activePorts = false;
    self->newNotes = 0;
    self->newVelocity = 0;    
    self->notesPressed = 0;
    self->notesIndex = 0;
    self->prevMsg[0] = 200;
    self->prevMsg[1] = 200;
    
    return self; 
}
/**********************************************************************************************************************************************************/
static void connect_port(LV2_Handle instance, uint32_t port, void *data)
{
    Midicv* self = (Midicv*)instance;

    switch ((PortIndex)port)
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
    }
}
/**********************************************************************************************************************************************************/
void activate(LV2_Handle instance)
{
}

/**********************************************************************************************************************************************************/
void run(LV2_Handle instance, uint32_t n_samples)
{
  Midicv* self = (Midicv*)instance;

  float *pitch = self->pitch;
  float *velocity = self->velocity;
  float *trigger = self->trigger;
  float  oC = *self->octave;
  float  sC = *self->semitone;
  float  cC = *self->cent;

  // Read incoming events
  LV2_ATOM_SEQUENCE_FOREACH(self->port_events_in, ev)
  {
    if (ev->body.type == self->urid_midiEvent)
    {
      const uint8_t* const msg = (const uint8_t*)(ev + 1);
      //const uint8_t channel = msg[0] & 0x0F;
      const uint8_t status  = msg[0] & 0xF0;
      if (msg[1] != self->prevMsg[0] || (msg[1] == self->prevMsg[0] && msg[2] != self->prevMsg[1])) {
        switch (status)
        {
          case LV2_MIDI_MSG_NOTE_ON:
            self->notesPressed++;
            self->activeNotesList[self->notesIndex++ % 4] = msg[1];
            if (self->notesPressed > 0)
            {
              self->activeNotes = msg[1];
              self->activeVelocity = msg[2];
              self->activePorts = true;
              self->triggerState = true;
            }
            break;
          case LV2_MIDI_MSG_NOTE_OFF:
            self->notesPressed--;
            for (size_t notesIndex = 0; notesIndex < 4; notesIndex++) {
              if (msg[1] == self->activeNotesList[notesIndex]) {
                self->activeNotesList[notesIndex] = 200;
              }
            }
            if (self->notesPressed <= 0)
            {
              self->activeNotes = 0;
              self->activePorts = false;
              self->activeVelocity = 0;
              self->triggerState = false;
            } else {
              size_t notesIndex = 0;
              bool noteFound = false;
              while (!noteFound && notesIndex < 4) {
                if (self->activeNotesList[notesIndex] < 200){
                  noteFound = true;
                  self->activeNotes = self->activeNotesList[notesIndex];
                }
                notesIndex++;
              }
            }
            break;
          default:
            break;
        }
        self->prevMsg[0] = msg[1];
        self->prevMsg[1] = msg[2];
      }
    }
  }
  for(uint32_t i=0;i<n_samples;i++)
  {
    pitch[i] = (0.0f + (float)((oC) + (sC/12.0f)+(cC/1200.0f)) + ((float)self->activeNotes * 1/12.0f));
    velocity[i] = (0.0f + ((float)self->activeVelocity * 1/12.0f));
    trigger[i] = ((self->triggerState == true) ? 10.0f : 0.0f);
  }
}

/**********************************************************************************************************************************************************/
void deactivate(LV2_Handle instance)
{
    // TODO: include the deactivate function code here
}
/**********************************************************************************************************************************************************/
void cleanup(LV2_Handle instance)
{
  free(instance);   
}
/**********************************************************************************************************************************************************/
const void* extension_data(const char* uri)
{
    return NULL;
}
/**********************************************************************************************************************************************************/
static const LV2_Descriptor Descriptor = {
    PLUGIN_URI,
    instantiate,
    connect_port,
    activate,
    run,
    deactivate,
    cleanup,
    extension_data
};
/**********************************************************************************************************************************************************/
LV2_SYMBOL_EXPORT
const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
    if (index == 0) return &Descriptor;
    else return NULL;
}
/**********************************************************************************************************************************************************/
