#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#ifndef DEBUG
#define DEBUG 0
#endif
#define debug_print(...) \
((void)((DEBUG) ? fprintf(stderr, __VA_ARGS__) : 0))


#define PLUGIN_URI "http://moddevices.com/plugins/mod-devel/mod-cv-meter"

typedef enum {
    INPUT     = 0,
    OUTPUT    = 1,
    MIN_LEVEL = 2,
    MAX_LEVEL = 3,
    LEVEL     = 4,
    RESET     = 5
} PortIndex;

typedef struct {
    float* input;
    float* output;
    float* reset;
    float* min_level;
    float* max_level;
    float* level;
    float  current_value;
    float  min_value;
    float  max_value;
    bool   calibrated;
} Meter;

static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
        double                    rate,
        const char*               bundle_path,
        const LV2_Feature* const* features)
{
    Meter* self = (Meter*)malloc(sizeof(Meter));

    self->current_value = 0.2f;
    self->min_value = 0.2f;
    self->max_value = 0.2f;
    self->calibrated = false;


    return (LV2_Handle)self;
}

static void
connect_port(LV2_Handle instance,
        uint32_t   port,
        void*      data)
{
    Meter* self = (Meter*)instance;

    switch ((PortIndex)port) {
        case INPUT:
            self->input = (float*)data;
            break;
        case OUTPUT:
            self->output = (float*)data;
            break;
        case MIN_LEVEL:
            self->min_level = (float*)data;
            break;
        case MAX_LEVEL:
            self->max_level = (float*)data;
            break;
        case LEVEL:
            self->level = (float*)data;
            break;
        case RESET:
            self->reset = (float*)data;
            break;
    }
}

static void
activate(LV2_Handle instance)
{
}


static void
calibrate(Meter* self)
{
    self->max_value = *self->input;
    self->min_value = *self->input;

}

static void
run(LV2_Handle instance, uint32_t n_samples)
{
    Meter* self = (Meter*) instance;

    if (self->calibrated)
        self->current_value = *self->input;

    *self->min_level = self->min_value;
    *self->max_level = self->max_value;
    *self->level = self->current_value;

    for ( uint32_t i = 0; i < n_samples; i++)
    {
        if (*self->input != 0 && !self->calibrated) {
            calibrate(self);
            self->calibrated = true;
        }
        if (self->calibrated) {
            if (*self->reset == 1) {
                self->max_value = *self->input;
                self->min_value = *self->input;
            }
            if (*self->input > self->max_value) {
                self->max_value = *self->input;
            }
            else if (*self->input < self->min_value) {
                self->min_value = *self->input;
            }
        }
        self->output[i] = self->input[i];
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
