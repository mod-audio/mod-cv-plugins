#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#define PLUGIN_URI "http://moddevices.com/plugins/mod-devel/mod-cv-attenuverter"


typedef enum {
    L_INPUT     = 0,
    L_OUTPUT    = 1,
    L_LEVEL     = 2,
    L_OFFSET    = 3,
    L_MODE      = 4,
    L_SMOOTHING = 5,
    L_ENABLE    = 6
} PortIndex;

typedef struct Filter {
  double a0;
  double b1;
  double z1;
}Filter;

typedef struct {
    float* input;
    float* output;
    float* level;
    float* offset;
    float* plugin_enabled;
    float* mode;
    float* smoothing;
    Filter   *lowpass;
} Attenuverter;


static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
        double                    rate,
        const char*               bundle_path,
        const LV2_Feature* const* features)
{
    Attenuverter* self = (Attenuverter*)malloc(sizeof(Attenuverter));

    self->lowpass = (Filter*)malloc(2*sizeof(Filter));


    double frequency = 440.0 / rate;

    for (unsigned channel = 0; channel < 2; channel++) {
        self->lowpass[channel].z1 = 0.0;
        self->lowpass[channel].b1 = exp(-2.0 * M_PI * frequency);
        self->lowpass[channel].a0 = 1.0 - self->lowpass[channel].b1;
    }

    return (LV2_Handle)self;
}


static void
connect_port(LV2_Handle instance,
        uint32_t   port,
        void*      data)
{
    Attenuverter* self = (Attenuverter*)instance;

    switch ((PortIndex)port) {
        case L_INPUT:
            self->input = (float*)data;
            break;
        case L_OUTPUT:
            self->output = (float*)data;
            break;
        case L_LEVEL:
            self->level = (float*)data;
            break;
        case L_OFFSET:
            self->offset = (float*)data;
            break;
        case L_MODE:
            self->mode = (float*)data;
            break;
        case L_SMOOTHING:
            self->smoothing = (float*)data;
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


static double
one_pole(Filter* lowpass, float input, int channel)
{
  return lowpass[channel].z1 = input * lowpass[channel].a0 + lowpass[channel].z1 * lowpass[channel].b1;
}


static void
run(LV2_Handle instance, uint32_t n_samples)
{
    Attenuverter* self = (Attenuverter*) instance;

    float direction = (*self->level >= 0 ) ? 1 : -1;
    float coef = pow(fabs(*self->level), 2);
    coef = (coef * direction) / 10;

    if (*self->mode == 0.0) {
        coef = *self->level;
    }

    float offset = *self->offset;

    float smooth = (float)one_pole(self->lowpass, coef, 0);
    float smooth_offset = (float)one_pole(self->lowpass, offset, 1);

    if (*self->smoothing == 1.0) {
        coef = smooth;
        offset = smooth_offset;
    }

    for ( uint32_t i = 0; i < n_samples; i++)
    {
        if ((int)*self->plugin_enabled == 1) {
        self->output[i] = (self->input[i] * coef) + offset;
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
