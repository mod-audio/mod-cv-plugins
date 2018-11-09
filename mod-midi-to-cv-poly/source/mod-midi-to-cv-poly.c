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
#define PLUGIN_URI "http://moddevices.com/plugins/mod-devel/midi-to-cv-poly"
#define MAX_NOTES 3

typedef enum {IN, CV1, CV2, CV3, CV4, VELOCITY, TRIGGER1, TRIGGER2, TRIGGER3, TRIGGER4, OCTAVE, PITCH, SEMITONE}PortIndex;

typedef struct
{
    // keep track of active notes
    uint8_t activeNotes[4];
    uint8_t activeVelocity;
    bool activePorts[4];

    LV2_URID urid_midiEvent;    
    //ports
    const LV2_Atom_Sequence* port_events_in;
    float *cv1;
    float *cv2;
    float *cv3;
    float *cv4;
    float *velocity;
    float *trigger1;
    float *trigger2;
    float *trigger3;
    float *trigger4;
    float *octaveC;
    float *pitchC;
    float *semitoneC;
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

    // use for arrays
    //memset(self->activeNotes, 0, sizeof(uint8_t)*4);
    //memset(self->activePorts, 0, sizeof(bool)*4);
    self->activeNotes[0] = 0;
    self->activeNotes[1] = 0;  
    self->activeNotes[2] = 0;  
    self->activeNotes[3] = 0;
    self->activePorts[0] = false;
    self->activePorts[1] = false;
    self->activePorts[2] = false;
    self->activePorts[3] = false;      
    self->activeVelocity = 0;

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
            self->cv1 = (float*) data;
            break;
        case CV2:
            self->cv2 = (float*) data;
            break;
        case CV3:
            self->cv3 = (float*) data;
            break;
        case CV4:
            self->cv4 = (float*) data;
            break;
        case VELOCITY:
            self->velocity = (float*) data;
            break;
        case TRIGGER1:
            self->trigger1 = (float*) data;
            break;
        case TRIGGER2:
            self->trigger2 = (float*) data;
            break;
        case TRIGGER3:
            self->trigger3 = (float*) data;
            break;
        case TRIGGER4:
            self->trigger4 = (float*) data;
            break;
        case OCTAVE:
            self->octaveC = (float*) data;
        break;
        case PITCH:
            self->pitchC = (float*) data;
        break;
        case SEMITONE:
            self->semitoneC = (float*) data;
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

    float *cv1 = self->cv1;
    float *cv2 = self->cv2;
    float *cv3 = self->cv3;
    float *cv4 = self->cv4;
    float *velocity = self->velocity;
    float *trigger1 = self->trigger1;
    float *trigger2 = self->trigger2;
    float *trigger3 = self->trigger3;
    float *trigger4 = self->trigger4;
    float cA = *self->octaveC;
    float cB = *self->pitchC;
    float cC = *self->semitoneC;

    for(uint32_t i=0;i<n_samples;i++)
    {
        // Read incoming events
        LV2_ATOM_SEQUENCE_FOREACH(self->port_events_in, ev)
        {
            if (ev->body.type == self->urid_midiEvent)
            {
                    const uint8_t* const msg = (const uint8_t*)(ev + 1);
                    //const uint8_t channel = msg[0] & 0x0F;
                    const uint8_t status  = msg[0] & 0xF0;
                    switch (status)
                    {
                    case LV2_MIDI_MSG_NOTE_ON:
                        if (self->activePorts[0] != true && self->activeNotes[1] != msg[1] && self->activeNotes[2] != msg[1] && self->activeNotes[3] != msg[1])
                        {
                            self->activeNotes[0] = msg[1];
                            self->activeVelocity = msg[2];
                            self->activePorts[0] = true;
                        }
                        else if (self->activePorts[1] != true && self->activeNotes[0] != msg[1]&& self->activeNotes[2] != msg[1] && self->activeNotes[3] != msg[1])
                        {
                            self->activeNotes[1] = msg[1];
                            self->activeVelocity = msg[2];
                            self->activePorts[1] = true;
                        }
                        else if (self->activePorts[2] != true && self->activeNotes[0] != msg[1]&& self->activeNotes[1] != msg[1] && self->activeNotes[3] != msg[1])
                        {
                            self->activeNotes[2] = msg[1];
                            self->activeVelocity = msg[2];
                            self->activePorts[2] = true;
                        }
                        else if (self->activePorts[3] != true && self->activeNotes[0] != msg[1]&& self->activeNotes[1] != msg[1] && self->activeNotes[2] != msg[1])
                        {
                            self->activeNotes[3] = msg[1];
                            self->activeVelocity = msg[2];
                            self->activePorts[3] = true;
                        }
                    break;
                    case LV2_MIDI_MSG_NOTE_OFF:  
                        if (self->activeNotes[0] == msg[1])
                        {
                            self->activeNotes[0] = 0;
                            self->activePorts[0] = false;
                        }
                        else if (self->activeNotes[1] == msg[1])
                        {
                            self->activeNotes[1] = 0;
                            self->activePorts[1] = false;
                        }
                        else if (self->activeNotes[2] == msg[1])
                        {
                            self->activeNotes[2] = 0;
                            self->activePorts[2] = false;
                        }
                        else if (self->activeNotes[3] == msg[1])
                        {
                            self->activeNotes[3] = 0;
                            self->activePorts[3] = false;
                        }
                    break;
                    default:
                    break;
                    }
            }
        }
    }
        if(self->activePorts[0] == false)
        {
            if(self->activePorts[1] == false)
            {
                if(self->activePorts[2] == false)
                {
                    if(self->activePorts[3] == false)
                    {
                        self->activeVelocity = 0;
                    }
                }
            }
        }
    for(uint32_t z = 0; z < n_samples; z++)
    {
        cv1[z] = 0.0f + (cA + (cB * 1/12.0f) + (cC * 1/1200.0f) + (float)(self->activeNotes[0] * 1/12.0f));
        cv2[z] = 0.0f + (cA + (cB * 1/12.0f) + (cC * 1/1200.0f) + (float)(self->activeNotes[1] * 1/12.0f));
        cv3[z] = 0.0f + (cA + (cB * 1/12.0f) + (cC * 1/1200.0f) + (float)(self->activeNotes[2] * 1/12.0f));  
        cv4[z] = 0.0f + (cA + (cB * 1/12.0f) + (cC * 1/1200.0f) + (float)(self->activeNotes[3] * 1/12.0f));    
        trigger1[z] = ((self->activePorts[0] == true) ? 10.0f : 0.0f);
        trigger2[z] = ((self->activePorts[1] == true) ? 10.0f : 0.0f);
        trigger3[z] = ((self->activePorts[2] == true) ? 10.0f : 0.0f);
        trigger4[z] = ((self->activePorts[3] == true) ? 10.0f : 0.0f);
        velocity[z] = (float)(0.0f + ((float)self->activeVelocity * 1/12.0f));
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