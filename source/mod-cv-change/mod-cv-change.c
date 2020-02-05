#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#define PLUGIN_URI "http://moddevices.com/plugins/mod-devel/mod-cv-change"


typedef enum {
    L_INPUT  = 0,
    L_OUTPUT = 1,
    L_MODE   = 2,
    L_S_HOLD = 3,
    L_L_HOLD = 4,
    L_ENABLE = 5
} PortIndex;


typedef struct {
    float* input;
    float* output;
    float* mode;
    float* s_hold;
    float* l_hold;
    float* plugin_enabled;

    bool hold;
    float hold_time;
    float previous_input_value;
    float samplerate;

} CVChange;


static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
        double                    rate,
        const char*               bundle_path,
        const LV2_Feature* const* features)
{
    CVChange* self = (CVChange*)malloc(sizeof(CVChange));
    self->samplerate = rate;

    self->previous_input_value = 0.f;
    self->hold = false;
    self->hold_time = 0.f;

    return (LV2_Handle)self;
}


static void
connect_port(LV2_Handle instance,
        uint32_t   port,
        void*      data)
{
    CVChange* self = (CVChange*)instance;

    switch ((PortIndex)port) {
        case L_INPUT:
            self->input = (float*)data;
            break;
        case L_OUTPUT:
            self->output = (float*)data;
            break;
        case L_MODE:
            self->mode = (float*)data;
            break;
        case L_S_HOLD:
            self->s_hold = (float*)data;
            break;
        case L_L_HOLD:
            self->l_hold = (float*)data;
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


static void
run(LV2_Handle instance, uint32_t n_samples)
{
    CVChange* self = (CVChange*) instance;

    for ( uint32_t i = 0; i < n_samples; i++)
    {
        if ((int)*self->plugin_enabled == 1) {

            if (self->input[i] != self->previous_input_value) {
                self->hold = true;
                self->hold_time = 1.0;
                self->previous_input_value = self->input[i];
            }

            if (self->hold) {

                self->output[i] = 10.0;

                float divider = ((int)*self->mode == 0) ? 1000.0 : 1.0;
                float decrementor = ( 1 / self->samplerate * (*self->s_hold / divider));
                self->hold_time -= decrementor;

                if (self->hold_time <= 0) {
                    self->hold = false;
                }
            } else {
                self->output[i] = 0.0;
            }
        } else {
            self->output[i] = 0.0;
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
