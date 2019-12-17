#include <math.h>
#include <stdlib.h>

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#define PLUGIN_URI "http://moddevices.com/plugins/mod-devel/mod-cv-range"

typedef enum {
    P_INPUT     = 0,
    P_OUTPUT_1  = 1,
    P_OUTPUT_2  = 2,
	P_MIN_RANGE = 3,
	P_MAX_RANGE = 4,
	P_MODE_ZERO = 5,
} PortIndex;

typedef struct {
  float*       input;
  float*       output_1;
  float*       output_2;
  float*       min_range;
  float*       max_range;
  float*       mode_zero;
  float        output_value_1;
  float        output_value_2;
} Control;

static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
            double                    rate,
            const char*               bundle_path,
            const LV2_Feature* const* features)
{
	Control* self = (Control*)malloc(sizeof(Control));

  self->output_value_1 = 0.0;
  self->output_value_2 = 0.0;
	return (LV2_Handle)self;
}

static void
connect_port(LV2_Handle instance,
             uint32_t   port,
             void*      data)
{
	Control* self = (Control*)instance;

	switch ((PortIndex)port) {
	case P_INPUT:
		self->input = (float*)data;
		break;
	case P_OUTPUT_1:
		self->output_1 = (float*)data;
		break;
	case P_OUTPUT_2:
		self->output_2 = (float*)data;
		break;
	case P_MIN_RANGE:
		self->min_range = (float*)data;
        break;
	case P_MAX_RANGE:
		self->max_range = (float*)data;
        break;
	case P_MODE_ZERO:
		self->mode_zero = (float*)data;
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
  Control* self = (Control*) instance;

  for ( uint32_t i = 0; i < n_samples; i++)
  {
      if (*self->mode_zero == 1) {
          self->output_value_1 = 0.0;
          self->output_value_2 = 0.0;
      }

      if (*self->input > *self->min_range && *self->input < *self->max_range) {
          self->output_value_1 = *self->input;
      } else {
          self->output_value_2 = *self->input;
      }
      self->output_1[i] = self->output_value_1;
      self->output_2[i] = self->output_value_2;
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
