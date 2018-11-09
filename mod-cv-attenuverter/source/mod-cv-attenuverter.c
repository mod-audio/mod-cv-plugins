#include <math.h>
#include <stdlib.h>

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#define PLUGIN_URI "http://moddevices.com/plugins/mod-devel/mod-cv-attenuverter"

typedef enum {
	L_INPUT   = 0,
	L_OUTPUT  = 1,
	L_LEVEL   = 2
} PortIndex;

typedef struct {
	float*       input;
	float*       output;
    float*       level;
} Attenuverter;

static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
            double                    rate,
            const char*               bundle_path,
            const LV2_Feature* const* features)
{
	Attenuverter* self = (Attenuverter*)malloc(sizeof(Attenuverter));

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
	}
}

static void
activate(LV2_Handle instance)
{
}

static void
run(LV2_Handle instance, uint32_t n_samples)
{
  Attenuverter* self = (Attenuverter*) instance;
  float coef = *self->level;
    for ( uint32_t i = 0; i < n_samples; i++)
    {
      self->output[i] = (self->input[i] * coef); 
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
