#include <math.h>
#include <stdlib.h>

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#define PLUGIN_URI "http://moddevices.com/plugins/mod-devel/mod-cv-switch3"

typedef enum {
	SWITCH_IN   = 0,
	SWITCH_OUT1 = 1,
	SWITCH_OUT2 = 2,
  	SWITCH_OUT3 = 3,
  	SWITCH_SEL  = 4
} PortIndex;

typedef struct {
	const float* select;
	float*       input;
	float*       output1;
  float*       output2;
  float*       output3;
} Switch;

static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
            double                    rate,
            const char*               bundle_path,
            const LV2_Feature* const* features)
{
	Switch* self = (Switch*)malloc(sizeof(Switch));

	return (LV2_Handle)self;
}

static void
connect_port(LV2_Handle instance,
             uint32_t   port,
             void*      data)
{
	Switch* self = (Switch*)instance;

	switch ((PortIndex)port) {
	case SWITCH_IN:
		self->input = (float*)data;
		break;
	case SWITCH_OUT1:
		self->output1 = (float*)data;
		break;
	case SWITCH_OUT2:
		self->output2 = (float*)data;
		break;
  case SWITCH_OUT3:
    self->output3 = (float*)data;
    break;
  case SWITCH_SEL:
    self->select = (float*)data;
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
  int sel;
  Switch* self = (Switch*) instance;
  sel = (int)(*(self->select));
  float *input = self->input;
  float *output1 = self->output1;
  float *output2 = self->output2;
  float *output3 = self->output3;

  switch (sel)
 {
    case 0:
    for ( uint32_t i = 0; i < n_samples; i++)
    {
      *output1 = *input;
      *output2 = 0.0f;
      *output3 = 0.0f;
      input++;
      output1++;
      output2++;
      output3++;
    }
    break;
    case 1:
    for (uint32_t i = 0; i < n_samples; i++)
    {
      *output1 = 0.0f;
      *output2 = *input;
      *output3 = 0.0f;
      input++;
      output1++;
      output2++;
      output3++;
    }
    break;
    case 2:
    for (uint32_t i = 0; i < n_samples; i++)
    {
      *output1 = 0.0f;
      *output2 = 0.0f;
      *output3 = *input;
      input++;
      output1++;
      output2++;
      output3++;
    }
    default:
    break;
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
