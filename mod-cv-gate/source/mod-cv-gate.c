#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#define PLUGIN_URI "http://moddevices.com/plugins/mod-devel/mod-cv-gate"


typedef enum {
    L_GATE              = 0,
    L_INPUT             = 1,
    L_OUTPUT            = 2,
    L_OPENING_THRESHOLD = 3,
    L_CLOSING_THRESHOLD = 4,
    L_ENABLE            = 5
} PortIndex;


typedef struct {
    float* gate;
    float* input;
    float* output;
    float* opening_threshold;
    float* closing_threshold;
    float* plugin_enabled;

    bool   gate_open;
} Gate;


static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
        double                    rate,
        const char*               bundle_path,
        const LV2_Feature* const* features)
{
    Gate* self = (Gate*)malloc(sizeof(Gate));

    self->gate_open = false;

    return (LV2_Handle)self;
}


static void
connect_port(LV2_Handle instance,
        uint32_t   port,
        void*      data)
{
    Gate* self = (Gate*)instance;

    switch ((PortIndex)port) {
        case L_GATE:
            self->gate = (float*)data;
            break;
        case L_INPUT:
            self->input = (float*)data;
            break;
        case L_OUTPUT:
            self->output = (float*)data;
            break;
        case L_OPENING_THRESHOLD:
            self->opening_threshold = (float*)data;
            break;
        case L_CLOSING_THRESHOLD:
            self->closing_threshold = (float*)data;
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
    Gate* self = (Gate*) instance;

    for ( uint32_t i = 0; i < n_samples; i++)
    {
        if ((int)*self->plugin_enabled == 1) {
            if (*self->gate >= *self->opening_threshold) {
                self->gate_open = true;
            }
            if (*self->gate <= *self->closing_threshold) {
                self->gate_open = false;
            }

            if (self->gate_open) {
                self->output[i] = self->input[i];
            } else {
                self->output[i] = 0.0;
            }
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
