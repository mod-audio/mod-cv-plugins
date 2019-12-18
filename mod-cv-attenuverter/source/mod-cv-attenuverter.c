#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#define PLUGIN_URI "http://moddevices.com/plugins/mod-devel/mod-cv-attenuverter"


typedef enum {
    L_INPUT     = 0,
    L_OUTPUT    = 1,
    L_LEVEL     = 2,
    L_MODE      = 3,
    L_SMOOTHING = 4,
    L_ENABLE    = 5
} PortIndex;


typedef struct {
    float* input;
    float* output;
    float* level;
    float* plugin_enabled;
    float* mode;
    float* smoothing;
    double a0;
    double b1;
    double z1;
} Attenuverter;


static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
        double                    rate,
        const char*               bundle_path,
        const LV2_Feature* const* features)
{
    Attenuverter* self = (Attenuverter*)malloc(sizeof(Attenuverter));

    self->z1 = 0.0;
    double frequency = 440.0 / rate;
    self->b1 = exp(-2.0 * M_PI * frequency);
    self->a0 = 1.0 - self->b1;

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
lowPassProcess(Attenuverter* self, float input)
{
    return self->z1 = input * self->a0 + self->z1 * self->b1;
}


static void
run(LV2_Handle instance, uint32_t n_samples)
{
    Attenuverter* self = (Attenuverter*) instance;

    float direction = (*self->level >= 0 ) ? 1 : -1;
    float coef = pow(fabs(*self->level), 2);
    coef = (coef * direction) / 10;

    if (*self->mode == 1) {
        coef = *self->level;
    }

    float smooth = lowPassProcess(self, coef);

    if (*self->smoothing == 1) {
        coef = smooth;
    }

    for ( uint32_t i = 0; i < n_samples; i++)
    {
        if ((int)*self->plugin_enabled == 1) {
        self->output[i] = (self->input[i] * coef);
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
