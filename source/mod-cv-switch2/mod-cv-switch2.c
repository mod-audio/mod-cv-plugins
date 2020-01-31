#include <math.h>
#include <stdlib.h>

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#define PLUGIN_URI "http://moddevices.com/plugins/mod-devel/mod-cv-switch2"

typedef enum {
	SWITCH_IN1  = 0,
	SWITCH_IN2  = 1,
	SWITCH_OUT  = 2,
  SWITCH_SEL  = 3
} PortIndex;

typedef struct {
	const float* select;
	float*       input1;
	float*       input2;
  float*       output;
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
	case SWITCH_IN1:
		self->input1 = (float*)data;
		break;
	case SWITCH_IN2:
		self->input2 = (float*)data;
		break;
	case SWITCH_OUT:
		self->output = (float*)data;
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
  float *input1 = self->input1;
  float *input2 = self->input2;
  float *output = self->output;

  switch (sel)
 {
    case 0:
    for ( uint32_t i = 0; i < n_samples; i++)
    {
      *output = *input1;
      input1++;
      input2++;
      output++;
    }
    break;
    case 1:
    for (uint32_t i = 0; i < n_samples; i++)
    {
      *output = *input2;
      input1++;
      input2++;
      output++;
    }
    break;
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
