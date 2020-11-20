#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#define PLUGIN_URI "http://moddevices.com/plugins/mod-devel/mod-cv-scale"

#define CHROMATIC -1

typedef enum {
    L_INPUT   = 0,
    L_OUTPUT  = 1,
    P_NOTE    = 2,
    P_SCALE   = 3,
    L_ENABLE  = 4
} PortIndex;

typedef struct {
    float* input;
    float* output;
    float* root_note_parameter;
    float* scale_parameter;
    float* plugin_enabled;
} CvScale;

static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
        double                    rate,
        const char*               bundle_path,
        const LV2_Feature* const* features)
{
    CvScale* self = (CvScale*)malloc(sizeof(CvScale));

    return (LV2_Handle)self;
}


static void
connect_port(LV2_Handle instance,
        uint32_t   port,
        void*      data)
{
    CvScale* self = (CvScale*)instance;

    switch ((PortIndex)port) {
        case L_INPUT:
            self->input = (float*)data;
            break;
        case L_OUTPUT:
            self->output = (float*)data;
            break;
        case P_NOTE:
            self->root_note_parameter = (float*)data;
            break;
        case P_SCALE:
            self->scale_parameter = (float*)data;
            break;
        case L_ENABLE:
            self->plugin_enabled = (float*)data;
            break;
    }
}


static void
activate(LV2_Handle instance)
{
}


static float
roundUp(float numToRound, float multiple)
{
    if (multiple == 0.0)
        return numToRound;

    float remainder = fmod(numToRound, multiple);
    if (remainder == 0.0)
        return numToRound;

    return numToRound + multiple - remainder;
}


static void
run(LV2_Handle instance, uint32_t n_samples)
{
    CvScale* self = (CvScale*) instance;

    int scale[12] = {1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1}; // base scale
    int scale_offset = (int)*self->scale_parameter;

    float note_value = roundUp(self->input[0], 1.0/12.0);
    int note_index = round(fmod(note_value, 1.0) * 12);
    note_index = note_index - (int)*self->root_note_parameter;
    note_index = (note_index < 0) ? note_index + 12 : note_index;

    if (*self->scale_parameter != CHROMATIC) {
        if (scale[(note_index + scale_offset) % 12] == 0) {
            note_value += 1.0/12.0;
        }
    }

    for ( uint32_t i = 0; i < n_samples; i++)
    {
        if ((int)*self->plugin_enabled == 1) {
            self->output[i] = note_value;
        } else {
            self->output[i] = self->input[i];
        }
    }
}


static void
deactivate(LV2_Handle instance)
{
}


static void
cleanup(LV2_Handle instance)
{
    free(instance);
}


static const void*
extension_data(const char* uri)
{
    return NULL;
}


static const LV2_Descriptor descriptor = {
    PLUGIN_URI,
    instantiate,
    connect_port,
    activate,
    run,
    deactivate,
    cleanup,
    extension_data
};


LV2_SYMBOL_EXPORT
    const LV2_Descriptor*
lv2_descriptor(uint32_t index)
{
    switch (index) {
        case 0:  return &descriptor;
        default: return NULL;
    }
}
