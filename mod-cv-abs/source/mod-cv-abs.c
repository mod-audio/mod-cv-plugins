#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#define PLUGIN_URI "http://moddevices.com/plugins/mod-devel/mod-cv-abs"


typedef enum {
    L_INPUT   = 0,
    L_OUTPUT  = 1,
    L_ENABLE  = 2
} PortIndex;


typedef struct {
    float* input;
    float* output;
    float* plugin_enabled;
} CvAbs;


static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
        double                    rate,
        const char*               bundle_path,
        const LV2_Feature* const* features)
{
    CvAbs* self = (CvAbs*)malloc(sizeof(CvAbs));

    return (LV2_Handle)self;
}


static void
connect_port(LV2_Handle instance,
        uint32_t   port,
        void*      data)
{
    CvAbs* self = (CvAbs*)instance;

    switch ((PortIndex)port) {
        case L_INPUT:
            self->input = (float*)data;
            break;
        case L_OUTPUT:
            self->output = (float*)data;
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
    CvAbs* self = (CvAbs*) instance;

    for ( uint32_t i = 0; i < n_samples; i++)
    {
        if ((int)*self->plugin_enabled == 1) {
            self->output[i] = fabs(self->input[i]);
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
