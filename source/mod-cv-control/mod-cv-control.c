#include <math.h>
#include <stdlib.h>

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#define PLUGIN_URI "http://moddevices.com/plugins/mod-devel/mod-cv-control"

typedef enum {
	L_OUTPUT    = 0,
	L_LEVEL     = 1,
	L_SMOOTHING = 2
} PortIndex;

typedef struct {
    float* output;
    float* level;
    float* smoothing;
    double a0;
    double b1;
    double z1;
} Control;

static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
            double                    rate,
            const char*               bundle_path,
            const LV2_Feature* const* features)
{
    Control* self = (Control*)malloc(sizeof(Control));

    self->z1 = 0.0;
    double frequency = 550.0 / rate;
    self->b1 = exp(-2.0 * M_PI * frequency);
    self->a0 = 1.0 - self->b1;

	return (LV2_Handle)self;
}

static void
connect_port(LV2_Handle instance,
             uint32_t   port,
             void*      data)
{
	Control* self = (Control*)instance;

	switch ((PortIndex)port) {
	case L_OUTPUT:
		self->output = (float*)data;
		break;
	case L_LEVEL:
		self->level = (float*)data;
        break;
	case L_SMOOTHING:
		self->smoothing = (float*)data;
        break;
	}
}

static void
activate(LV2_Handle instance)
{
}

static double
lowPassProcess(Control* self, float input)
{
    return self->z1 = input * self->a0 + self->z1 * self->b1;
}

static void
run(LV2_Handle instance, uint32_t n_samples)
{
    Control* self = (Control*) instance;
    float coef = *self->level;

    for ( uint32_t i = 0; i < n_samples; i++)
    {
        float smooth = lowPassProcess(self, coef);

        if ((int)*self->smoothing == 1) {
            coef = smooth;
        }

        self->output[i] = coef;
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
