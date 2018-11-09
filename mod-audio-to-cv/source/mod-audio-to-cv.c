#include <math.h>
#include <stdlib.h>

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#define PLUGIN_URI "http://moddevices.com/plugins/mod-devel/mod-audio-to-cv"

typedef enum {
	AUDIO_IN   = 0,
	CV_OUT     = 1,
	P_LEVEL	   = 2,
	P_OFFSET   = 3
} PortIndex;

typedef struct {
	float*       input;
	float*       output;
	float*		 level;
	float*		 offset;
} Convert;

static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
            double                    rate,
            const char*               bundle_path,
            const LV2_Feature* const* features)
{
	Convert* self = (Convert*)malloc(sizeof(Convert));

	return (LV2_Handle)self;
}

static void
connect_port(LV2_Handle instance,
             uint32_t   port,
             void*      data)
{
	Convert* self = (Convert*)instance;

	switch ((PortIndex)port) {
	case AUDIO_IN:
		self->input = (float*)data;
		break;
	case CV_OUT:
		self->output = (float*)data;
		break;
	case P_LEVEL:
		self->level = (float*)data;
		break;
	case P_OFFSET:
		self->offset = (float*)data;
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
    Convert* self = (Convert*)instance;
    float lC = *self->level;
    float oC = *self->offset;
    for ( uint32_t i = 0; i < n_samples; i++)
    {
      self->output[i] = ((self->input[i] * lC) + oC);
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
