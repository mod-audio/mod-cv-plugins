#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#define PLUGIN_URI "http://moddevices.com/plugins/mod-devel/mod-cv-to-parameter"

#ifndef DEBUG
#define DEBUG 0
#endif
#define debug_print(...) \
((void)((DEBUG) ? fprintf(stderr, __VA_ARGS__) : 0))

typedef enum {
    AUDIO_IN     = 0,
    CV_OUT       = 1,
    P_OFFSET	 = 2,
    P_MODULATION = 3,
    P_ENABLE     = 4
} PortIndex;

typedef struct {
    float* input;
    float* output;
    float* offset;
    float* modulation;
    float* plugin_enabled;
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
	case P_OFFSET:
		self->offset = (float*)data;
		break;
	case P_MODULATION:
		self->modulation = (float*)data;
		break;
  case P_ENABLE:
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
    Convert* self = (Convert*)instance;
    float offset_param = *self->offset;
    float mod_param = *self->modulation;
    float out;
    for ( uint32_t i = 0; i < n_samples; i++)
    {
      if(*self->plugin_enabled == 1) {
        out = ((self->input[i] * (mod_param * 0.01)) + (offset_param * 0.1));
        out = (out > 10.0) ? 10.0 : out;
        out = (out < 0.0) ? 0.0 : out;
        self->output[i] = out;
      } else {
        self->output[i] = 0.0f;
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
