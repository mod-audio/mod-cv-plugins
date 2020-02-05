#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#define PLUGIN_URI "http://moddevices.com/plugins/mod-devel/mod-cv-clock-divider"

#define NUM_DIVIDERS 8

typedef enum {
    P_MODE     = 0,
    P_ROTATE   = 1,
    P_RESET    = 2,
    P_CLOCK_IN = 3,
    P_CLOCK_1  = 4,
    P_CLOCK_2  = 5,
    P_CLOCK_3  = 6,
    P_CLOCK_4  = 7,
    P_CLOCK_5  = 8,
    P_CLOCK_6  = 9,
    P_CLOCK_7  = 10,
    P_CLOCK_8  = 11,
    P_ENABLE   = 12
} PortIndex;


typedef struct {
    float* mode;
    float* rotate;
    float* reset;
    float* clock_in;
    float* clock_1;
    float* clock_2;
    float* clock_3;
    float* clock_4;
    float* clock_5;
    float* clock_6;
    float* clock_7;
    float* clock_8;
    float* plugin_enabled;

    float prev_rotate;
    float prev_reset;
    float prev_clock_in;

    int offset;
    int frames;
    int counted_frames;

    bool offset_set;
    bool counting;
    bool clock_started;
    bool first_clock;
    bool tick;

    int frame_counter[NUM_DIVIDERS];

    float **clocks;

} ClockDivider;


static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
        double                    rate,
        const char*               bundle_path,
        const LV2_Feature* const* features)
{
    ClockDivider* self = (ClockDivider*)malloc(sizeof(ClockDivider));
    self->clocks = (float**)malloc(1448000*sizeof(float*)); //TODO take more sensible malloc size

    self->frames = 0;
    self->offset = 0;
    self->prev_rotate = 0.0;
    self->prev_reset = 0.0;
    self->prev_clock_in = 0.0;
    self->counted_frames = 0;
    self->counting = false;
    self->first_clock = false;
    self->clock_started = false;
    self->offset_set = false;
    self->tick = false;

    for (unsigned i = 0; i < NUM_DIVIDERS; i++) {
        self->frame_counter[i] = 0;
    }


    return (LV2_Handle)self;
}


static void
connect_port(LV2_Handle instance,
        uint32_t   port,
        void*      data)
{
    ClockDivider* self = (ClockDivider*)instance;

    switch ((PortIndex)port) {
        case P_MODE:
            self->mode = (float*)data;
            break;
        case P_ROTATE:
            self->rotate = (float*)data;
            break;
        case P_RESET:
            self->reset = (float*)data;
            break;
        case P_CLOCK_IN:
            self->clock_in = (float*)data;
            break;
        case P_CLOCK_1:
            self->clock_1 = (float*)data;
            break;
        case P_CLOCK_2:
            self->clock_2 = (float*)data;
            break;
        case P_CLOCK_3:
            self->clock_3 = (float*)data;
            break;
        case P_CLOCK_4:
            self->clock_4 = (float*)data;
            break;
        case P_CLOCK_5:
            self->clock_5 = (float*)data;
            break;
        case P_CLOCK_6:
            self->clock_6 = (float*)data;
            break;
        case P_CLOCK_7:
            self->clock_7 = (float*)data;
            break;
        case P_CLOCK_8:
            self->clock_8 = (float*)data;
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
    ClockDivider* self = (ClockDivider*) instance;

    self->clocks[0] = self->clock_1;
    self->clocks[1] = self->clock_2;
    self->clocks[2] = self->clock_3;
    self->clocks[3] = self->clock_4;
    self->clocks[4] = self->clock_5;
    self->clocks[5] = self->clock_6;
    self->clocks[6] = self->clock_7;
    self->clocks[7] = self->clock_8;

    for ( uint32_t i = 0; i < n_samples; i++)
    {
        if (self->reset[i] > 0.0) {
            self->offset = 0;
        }

        if (self->rotate[i] > 0.0 && !self->offset_set) {
            self->offset = (self->offset + 1) % NUM_DIVIDERS;
            self->offset_set = true;
        } else if (self->rotate[i] <= 0.0 && self->offset_set) {
            self->offset_set = false;
        }

        if (self->clock_in[i] > 0.0 && !self->tick) {

            self->counting = true;
            self->counted_frames = self->frames;
            self->frames = 0;

            if (self->first_clock)
                self->clock_started = true;

            self->first_clock = true;
            self->tick = true;

            if (*self->mode > 0.0) {
                for (unsigned t = 0; t < NUM_DIVIDERS; t++) {
                    //self->frame_counter[t] = 0;
                }
            }

        } else if (self->clock_in[i] == 0.0 && self->tick) {
            self->tick = false;
        }

        if (self->counting)
            self->frames++;

        if ((int)*self->plugin_enabled == 1)
        {

            for (unsigned d = 0; d < NUM_DIVIDERS; d++) {

                bool multiply = true;
                float trigger_time;

                if (*self->mode == 0.0) {
                    trigger_time = self->counted_frames * (d + 1);
                } else {
                    trigger_time = self->counted_frames / (d + 1);
                }

                if (self->frame_counter[d] > trigger_time && self->clock_started) {
                    self->clocks[(d + self->offset) % NUM_DIVIDERS][i] = 10.0;
                    self->frame_counter[d] = 0;
                } else {
                    self->clocks[(d + self->offset) % NUM_DIVIDERS][i] = 0.0;
                    self->frame_counter[d]++;
                }
            }
        } else { // if plugin is not active;
            for (unsigned d = 0; d < NUM_DIVIDERS; d++) {
                self->clocks[d][i] = 0.0;
            }
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
