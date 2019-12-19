#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#define PLUGIN_URI "http://moddevices.com/plugins/mod-devel/cv-logic-or"


typedef enum {
    L_INPUT1  = 0,
    L_INPUT2  = 1,
    L_OUTPUT  = 2,
    L_LEVEL   = 3,
    L_ENABLE  = 4
} PortIndex;


typedef struct {
    float* input1;
    float* input2;
    float* output;
    float* compare_value;
    float* plugin_enabled;
} LogicOperator;


static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
        double                    rate,
        const char*               bundle_path,
        const LV2_Feature* const* features)
{
    LogicOperator* self = (LogicOperator*)malloc(sizeof(LogicOperator));


    return (LV2_Handle)self;
}


static void
connect_port(LV2_Handle instance,
        uint32_t   port,
        void*      data)
{
    LogicOperator* self = (LogicOperator*)instance;

    switch ((PortIndex)port) {
        case L_INPUT1:
            self->input1 = (float*)data;
            break;
        case L_INPUT2:
            self->input2 = (float*)data;
            break;
        case L_OUTPUT:
            self->output = (float*)data;
            break;
        case L_LEVEL:
            self->compare_value = (float*)data;
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
    LogicOperator* self = (LogicOperator*) instance;

    for ( uint32_t i = 0; i < n_samples; i++)
    {
        if ((int)*self->plugin_enabled == 1) {

            if (self->input2[i] != 0.0) {
                if ((self->input1[i] > *self->compare_value) || (self->input2[i] > *self->compare_value)) {
                    self->output[i] = 1.0;
                } else {
                    self->output[i] = 0.0;
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
