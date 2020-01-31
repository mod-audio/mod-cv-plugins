#include <math.h>
#include <stdlib.h>

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#define PLUGIN_URI "http://moddevices.com/plugins/mod-devel/mod-cv-to-audio"

typedef enum {
	CV_IN     = 0,
	AUDIO_OUT = 1,
    LEVEL     = 2,
    DC_BLOCK  = 3,
    ENABLED   = 4
} PortIndex;

typedef struct {
    float* input;
    float* output;
    float* level;
    float* dc_block;
    float* plugin_enabled;

    float xn1;
    float yn1;
    double a0;
    double b1;
    double z1;

} Convert;

static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
            double                    rate,
            const char*               bundle_path,
            const LV2_Feature* const* features)
{
    Convert* self = (Convert*)malloc(sizeof(Convert));

    self->xn1 = 0.0;
    self->yn1 = 0.0;

    self->z1 = 0.0;
    double frequency = 20.0 / rate;
    self->b1 = exp(-2.0 * M_PI * frequency);
    self->a0 = 1.0 - self->b1;

    return (LV2_Handle)self;
}

static void
connect_port(LV2_Handle instance,
             uint32_t   port,
             void*      data)
{
    Convert* self = (Convert*)instance;

    switch ((PortIndex)port) {
        case CV_IN:
            self->input = (float*)data;
            break;
        case AUDIO_OUT:
            self->output = (float*)data;
            break;
        case LEVEL:
            self->level = (float*)data;
            break;
        case DC_BLOCK:
            self->dc_block = (float*)data;
            break;
        case ENABLED:
            self->plugin_enabled = (float*)data;
            break;
    }
}

static void
activate(LV2_Handle instance)
{
}


static double
lowPassProcess(Convert* self, float input)
{
  return self->z1 = input * self->a0 + self->z1 * self->b1;
}


static void
run(LV2_Handle instance, uint32_t n_samples)
{
    Convert* self = (Convert*)instance;

    for ( uint32_t i = 0; i < n_samples; i++)
    {
        float x = self->input[i];
        float y = x - self->xn1 + 0.995 * self->yn1;
        self->xn1 = x;
        self->yn1 = y;

        float smooth_level = lowPassProcess(self, *self->level);

        if ((int)*self->plugin_enabled == 1) {
            if ((int)*self->dc_block == 1) {
                self->output[i] = y * smooth_level;
            } else {
                self->output[i] = self->input[i] * smooth_level;
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
