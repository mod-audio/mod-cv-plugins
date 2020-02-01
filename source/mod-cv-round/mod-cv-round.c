#include <math.h>
#include <stdlib.h>

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#define PLUGIN_URI "http://moddevices.com/plugins/mod-devel/mod-cv-round"


typedef enum {
    L_INPUT    = 0,
    L_ROUND    = 1,
    L_CEIL     = 2,
    L_FLOOR    = 3,
    L_FRACTION = 4,
    L_ENABLE   = 5
} PortIndex;


typedef struct {
    float* input;
    float* round;
    float* ceil;
    float* floor;
    float* fraction;
    float* mode;
    float* plugin_enabled;
} CvRound;


static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
        double                    rate,
        const char*               bundle_path,
        const LV2_Feature* const* features)
{
    CvRound* self = (CvRound*)malloc(sizeof(CvRound));

    return (LV2_Handle)self;
}


static void
connect_port(LV2_Handle instance,
        uint32_t   port,
        void*      data)
{
    CvRound* self = (CvRound*)instance;

    switch ((PortIndex)port) {
        case L_INPUT:
            self->input = (float*)data;
            break;
        case L_ROUND:
            self->round = (float*)data;
            break;
        case L_CEIL:
            self->ceil = (float*)data;
            break;
        case L_FLOOR:
            self->floor = (float*)data;
            break;
        case L_FRACTION:
            self->fraction = (float*)data;
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
    CvRound* self = (CvRound*) instance;

    for ( uint32_t i = 0; i < n_samples; i++)
    {
        if ((int)*self->plugin_enabled == 1) {
            float input_sample = self->input[i];
            self->round[i] = floor(input_sample + 0.5);
            self->ceil[i] = ceil(input_sample);
            self->floor[i] = floor(input_sample);
            self->fraction[i] = self->input[i] - (int)input_sample;
        } else {
            self->round[i] = 0.0;
            self->ceil[i] = 0.0;
            self->floor[i] = 0.0;
            self->fraction[i] = 0.0;
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
