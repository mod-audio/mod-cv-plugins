/*
    LV2 Parameter code from:
    https://github.com/lv2/lv2/tree/master/plugins/eg-params.lv2

    LV2 Parameter Example Plugin
    Copyright 2014-2016 David Robillard <d@drobilla.net>

    Permission to use, copy, modify, and/or distribute this software for any
    purpose with or without fee is hereby granted, provided that the above
    copyright notice and this permission notice appear in all copies.

    THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdatomic.h>

#include "state_map.h"

#include "lv2/atom/atom.h"
#include "lv2/atom/forge.h"
#include "lv2/atom/util.h"
#include "lv2/core/lv2.h"
#include "lv2/core/lv2_util.h"
#include "lv2/log/log.h"
#include "lv2/log/logger.h"
#include "lv2/midi/midi.h"
#include "lv2/patch/patch.h"
#include "lv2/state/state.h"
#include "lv2/urid/urid.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lv2-hmi.h"

#define PLUGIN_URI "http://moddevices.com/plugins/mod-devel/mod-button-to-cv"

#define N_PROPS             3
#define MAX_STRING          1024

#define SINGLE_PRESS_STRING_URI PLUGIN_URI "#port1string"
#define LONG_PRESS_STRING_URI   PLUGIN_URI "#port2string"
#define DOUBLE_PRESS_STRING_URI PLUGIN_URI "#port3string"

#define SPECIAL_PORT_RESET  UINT8_MAX

#define SINGLE_PRESS_TXT    "SINGLE PRESS"
#define LONG_PRESS_TXT      "LONG PRESS"
#define DOUBLE_PRESS_TXT    "DOUBLE PRESS"

//timing configs
#define CHANGE_COUNTER      1400

//status bitmasks
#define SINGLE_PRESS_ON     0x01
#define DOUBLE_PRESS_ON     0x02
#define LONG_PRESS_ON       0x04

typedef struct {
    LV2_URID plugin;
    LV2_URID atom_Path;
    LV2_URID atom_Sequence;
    LV2_URID atom_URID;
    LV2_URID atom_eventTransfer;
    LV2_URID atom_String;
    LV2_URID midi_Event;
    LV2_URID patch_Get;
    LV2_URID patch_Set;
    LV2_URID patch_Put;
    LV2_URID patch_body;
    LV2_URID patch_subject;
    LV2_URID patch_property;
    LV2_URID patch_value;
    LV2_URID state_StateChanged;
    LV2_URID port1_string;
    LV2_URID port2_string;
    LV2_URID port3_string;
} URIs;

typedef struct {
    LV2_Atom        port1string;
    char            port1string_data[MAX_STRING];
    LV2_Atom        port2string;
    char            port2string_data[MAX_STRING];
    LV2_Atom        port3string;
    char            port3string_data[MAX_STRING];
} State;

typedef struct {
    uint32_t size;
    void *body;
} restore_value;

static inline void
map_uris(LV2_URID_Map* map, URIs* uris)
{
    uris->plugin = map->map(map->handle, PLUGIN_URI);

    uris->atom_Path          = map->map(map->handle, LV2_ATOM__Path);
    uris->atom_Sequence      = map->map(map->handle, LV2_ATOM__Sequence);
    uris->atom_URID          = map->map(map->handle, LV2_ATOM__URID);
    uris->atom_eventTransfer = map->map(map->handle, LV2_ATOM__eventTransfer);
    uris->atom_String        = map->map(map->handle, LV2_ATOM__String);
    uris->midi_Event         = map->map(map->handle, LV2_MIDI__MidiEvent);
    uris->patch_Get          = map->map(map->handle, LV2_PATCH__Get);
    uris->patch_Set          = map->map(map->handle, LV2_PATCH__Set);
    uris->patch_Put          = map->map(map->handle, LV2_PATCH__Put);
    uris->patch_body         = map->map(map->handle, LV2_PATCH__body);
    uris->patch_subject      = map->map(map->handle, LV2_PATCH__subject);
    uris->patch_property     = map->map(map->handle, LV2_PATCH__property);
    uris->patch_value        = map->map(map->handle, LV2_PATCH__value);
    uris->state_StateChanged = map->map(map->handle, LV2_STATE__StateChanged);

    uris->port1_string       = map->map(map->handle, SINGLE_PRESS_STRING_URI);
    uris->port2_string       = map->map(map->handle, LONG_PRESS_STRING_URI);
    uris->port3_string       = map->map(map->handle, DOUBLE_PRESS_STRING_URI);
}

typedef enum {
    BUTTON = 0,
    CV_SINGLE_PRESS,
    CV_LONG_PRESS,
    CV_DOUBLE_PRESS,
    LONG_PRESS_TIME_MS,
    DOUBLE_PRESS_DEBOUNCE_MS,
    LONG_PRESS_MODE,
    BUTTON_STATUS_MASK,
    PARAMS_IN,
    PARAMS_OUT
} PortIndex;

enum LP_MODES{
    LATCHING = 0,
    MOMENTARY_ON,
    MOMENTARY_OFF
};

typedef struct {
    
    //main button
    const float* button;

    // CV signals
    float* cv_single_press;
    float* cv_long_press;
    float* cv_double_press;

    //controls
    const float* long_press_time;
    const float* double_press_debounce;

    const float* long_press_mode;

    //control output mask
    float* button_mask;

    float prev_button_value;

    float cv_single_value;
    float cv_long_value;
    float cv_double_value;

    uint32_t long_press_counter;
    uint32_t double_press_counter;
    uint32_t change_counter;

    bool state_changed;
    double sample_rate;

    // Features
    LV2_URID_Map*  map;
    LV2_Log_Logger logger;
    LV2_HMI_WidgetControl* hmi;
    
    LV2_Atom_Forge forge;
    LV2_Atom_Forge_Ref ref;

    // Ports
    const LV2_Atom_Sequence* in_port;
    LV2_Atom_Sequence*       out_port;
    LV2_Atom_Forge_Frame notify_frame;

    // URIs
    URIs uris;

    // Plugin state
    StateMapItem props[N_PROPS];
    State        state;

    // HMI Widgets stuff
    LV2_HMI_Addressing toggle_addressing;
    LV2_HMI_LED_Colour colour;
} Control;

//MOD products only support ascii 32 to 126
void check_popup_string(char *text)
{
    int char_lenght = strlen(text);
    int ascii = 0;
    for (int i = 0; i < char_lenght; i++) {
        ascii = (int)text[i];

        //replace chars with -
        if (ascii < 32)
            text[i] = '-';

        //dont do quotation marks as they are tricky
        if (ascii == 34)
            text[i] = '-';

        if (ascii > 126)
            text[i] = '-';
    }
}

void trigger_widget_change(Control* self, uint8_t port_index)
{
    //if we dont reset, set counter
    if (port_index != SPECIAL_PORT_RESET)
        self->change_counter = (CHANGE_COUNTER * self->sample_rate) / 1000;

    if (!self->hmi)
        return;

    self->hmi->set_led_with_brightness(self->hmi->handle, self->toggle_addressing, LV2_HMI_LED_Colour_Off, LV2_HMI_LED_Brightness_High);

    //update label
    char status_string[9] = {" - - - "};
    if (self->cv_single_value)
        status_string[1] = '*';

    if (self->cv_long_value)
        status_string[3] = '*';

    if (self->cv_double_value)
        status_string[5] = '*';

    //send to HMI
    self->hmi->set_label(self->hmi->handle, self->toggle_addressing, status_string);

    switch (port_index) {
        //reset state
        case SPECIAL_PORT_RESET:
        {
            //reset LED
            self->hmi->set_led_with_brightness(self->hmi->handle, self->toggle_addressing, LV2_HMI_LED_Colour_White, LV2_HMI_LED_Brightness_Low);
        }
        break;

        case CV_SINGLE_PRESS:
        {
            //send to HMI
            char *label = self->state.port1string_data;
            if (label[0] == '\0')
                strcpy(label, SINGLE_PRESS_TXT);

            //sanity check for the chars we want to display
            check_popup_string(label);

            if (self->hmi->size >= LV2_HMI_WIDGETCONTROL_SIZE_POPUP_MESSAGE) {
                if (self->cv_single_value)
                    self->hmi->popup_message(self->hmi->handle, self->toggle_addressing, LV2_HMI_Popup_Style_Inverted, SINGLE_PRESS_TXT, label);
                else
                    self->hmi->popup_message(self->hmi->handle, self->toggle_addressing, LV2_HMI_Popup_Style_Normal, SINGLE_PRESS_TXT, label);
            }
            else {
                self->hmi->set_label(self->hmi->handle, self->toggle_addressing, label);
            }

            //reset LED
            self->hmi->set_led_with_brightness(self->hmi->handle, self->toggle_addressing, LV2_HMI_LED_Colour_Red, LV2_HMI_LED_Brightness_High);
        }
        break;

        case CV_LONG_PRESS:
        {
            //send to HMI
            char *label = self->state.port2string_data;
            if (label[0] == '\0')
                strcpy(label, LONG_PRESS_TXT);

            //sanity check for the chars we want to display
            check_popup_string(label);

            if (self->hmi->size >= LV2_HMI_WIDGETCONTROL_SIZE_POPUP_MESSAGE) {
                if (self->cv_long_value)
                    self->hmi->popup_message(self->hmi->handle, self->toggle_addressing, LV2_HMI_Popup_Style_Inverted, LONG_PRESS_TXT, label);
                else
                    self->hmi->popup_message(self->hmi->handle, self->toggle_addressing, LV2_HMI_Popup_Style_Normal, LONG_PRESS_TXT, label);
            }
            else {
                self->hmi->set_label(self->hmi->handle, self->toggle_addressing, label);
            }

            //reset LED
            self->hmi->set_led_with_brightness(self->hmi->handle, self->toggle_addressing, LV2_HMI_LED_Colour_Blue, LV2_HMI_LED_Brightness_High);
        }
        break;

        case CV_DOUBLE_PRESS:
        {
            //send to HMI
            char *label = self->state.port3string_data;
            if (label[0] == '\0')
                strcpy(label, DOUBLE_PRESS_TXT);

            //sanity check for the chars we want to display
            check_popup_string(label);

            if (self->hmi->size >= LV2_HMI_WIDGETCONTROL_SIZE_POPUP_MESSAGE) {
                if (self->cv_double_value)
                    self->hmi->popup_message(self->hmi->handle, self->toggle_addressing, LV2_HMI_Popup_Style_Inverted, DOUBLE_PRESS_TXT, label);
                else
                    self->hmi->popup_message(self->hmi->handle, self->toggle_addressing, LV2_HMI_Popup_Style_Normal, DOUBLE_PRESS_TXT, label);
            }
            else {
                self->hmi->set_label(self->hmi->handle, self->toggle_addressing, label);
            }

            //reset LED
            self->hmi->set_led_with_brightness(self->hmi->handle, self->toggle_addressing, LV2_HMI_LED_Colour_Yellow, LV2_HMI_LED_Brightness_High);
        }
        break;
    }
}

static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
            double                    rate,
            const char*               bundle_path,
            const LV2_Feature* const* features)
{
    Control* self = (Control*)calloc(sizeof(Control), 1);

    // Get host features
    // clang-format off
    const char* missing = lv2_features_query(
            features,
            LV2_LOG__log,           &self->logger.log,  false,
            LV2_URID__map,          &self->map,         true,
            LV2_HMI__WidgetControl, &self->hmi,         false,
            NULL);
    // clang-format on

    lv2_log_logger_set_map(&self->logger, self->map);

    if (missing) {
        lv2_log_error(&self->logger, "Missing feature <%s>\n", missing);
        free(self);
        return NULL;
    }

    // Map URIs and initialise forge
    map_uris(self->map, &self->uris);
    lv2_atom_forge_init(&self->forge, self->map);

    // Initialise state dictionary
    // clang-format off
    State* state = &self->state;
    state_map_init(
        self->props, self->map, self->map->handle,
        SINGLE_PRESS_STRING_URI, STATE_MAP_INIT(String, &state->port1string),
        LONG_PRESS_STRING_URI, STATE_MAP_INIT(String, &state->port2string),
        DOUBLE_PRESS_STRING_URI, STATE_MAP_INIT(String, &state->port3string),
        NULL);
    // clang-format on

    self->colour = LV2_HMI_LED_Colour_Off;
    self->sample_rate = rate;

    return (LV2_Handle)self;
}

static LV2_State_Status
set_parameter(Control*     self,
              LV2_URID    key,
              uint32_t    size,
              LV2_URID    type,
              const void* body,
              bool        from_state)
{
    // Look up property in state dictionary
    const StateMapItem* entry = state_map_find(self->props, N_PROPS, key);

    // Set property value in state dictionary
    lv2_log_trace(&self->logger, "Set <%s>\n", entry->uri);
    memcpy(entry->value + 1, body, size);
    entry->value->size = size;
    return LV2_STATE_SUCCESS;
}

static const LV2_Atom*
get_parameter(Control* self, LV2_URID key)
{
    const StateMapItem* entry = state_map_find(self->props, N_PROPS, key);
    if (entry) {
        lv2_log_trace(&self->logger, "Get <%s>\n", entry->uri);
        return entry->value;
    }

    return NULL;
}

static LV2_State_Status
write_param_to_forge(LV2_State_Handle handle,
                     uint32_t         key,
                     const void*      value,
                     size_t           size,
                     uint32_t         type,
                     uint32_t         flags)
{
    LV2_Atom_Forge* forge = (LV2_Atom_Forge*)handle;

    if (!lv2_atom_forge_key(forge, key) ||
        !lv2_atom_forge_atom(forge, size, type) ||
        !lv2_atom_forge_write(forge, value, size)) {
        return LV2_STATE_ERR_UNKNOWN;
    }

    return LV2_STATE_SUCCESS;
}

static void
store_prop(Control*                 self,
           LV2_State_Map_Path*      map_path,
           LV2_State_Status*        save_status,
           LV2_State_Store_Function store,
           LV2_State_Handle         handle,
           LV2_URID                 key,
           const LV2_Atom*          value)
{
    LV2_State_Status st = LV2_STATE_SUCCESS;
    
    // Store simple property
    st = store(handle,
               key,
               value + 1,
               value->size,
               value->type,
               LV2_STATE_IS_POD | LV2_STATE_IS_PORTABLE);
  

    if (save_status && !*save_status) {
        *save_status = st;
    }
}

/**
   State save method.
   This is used in the usual way when called by the host to save plugin state,
   but also internally for writing messages in the audio thread by passing a
   "store" function which actually writes the description to the forge.
*/
static LV2_State_Status
save(LV2_Handle                instance,
     LV2_State_Store_Function  store,
     LV2_State_Handle          handle,
     uint32_t                  flags,
     const LV2_Feature* const* features)
{
    Control*             self = (Control*)instance;
    LV2_State_Map_Path* map_path =
        (LV2_State_Map_Path*)lv2_features_data(features, LV2_STATE__mapPath);

    LV2_State_Status st = LV2_STATE_SUCCESS;
    for (unsigned i = 0; i < N_PROPS; ++i) {
        StateMapItem* prop = &self->props[i];
        store_prop(self, map_path, &st, store, handle, prop->urid, prop->value);
    }

    return st;
}

static void
retrieve_prop(Control*                     self,
              LV2_State_Status*           restore_status,
              LV2_State_Retrieve_Function retrieve,
              LV2_State_Handle            handle,
              LV2_URID                    key,
              const LV2_Feature* const*   features)
{
    // Retrieve value from saved state
    size_t      vsize  = 0;
    uint32_t    vtype  = 0;
    uint32_t    vflags = 0;
    const void* value  = retrieve(handle, key, &vsize, &vtype, &vflags);

    // Set plugin instance state
    const LV2_State_Status st =
        value ? set_parameter(self, key, vsize, vtype, value, true)
                : LV2_STATE_ERR_NO_PROPERTY;

    if (!*restore_status) {
        *restore_status = st; // Set status if there has been no error yet
    }
}

/** State restore method. */
static LV2_State_Status
restore(LV2_Handle                  instance,
        LV2_State_Retrieve_Function retrieve,
        LV2_State_Handle            handle,
        uint32_t                    flags,
        const LV2_Feature* const*   features)
{
  Control*         self = (Control*)instance;
  LV2_State_Status st   = LV2_STATE_SUCCESS;

  for (unsigned i = 0; i < N_PROPS; ++i) {
    retrieve_prop(self, &st, retrieve, handle, self->props[i].urid, features);
  }

  self->state_changed = true;

  return st;
}

static inline bool
subject_is_plugin(Control* self, const LV2_Atom_URID* subject)
{
    // This simple plugin only supports one subject: itself
    return (!subject || (subject->atom.type == self->uris.atom_URID &&
                         subject->body == self->uris.plugin));
}

static void
connect_port(LV2_Handle instance,
             uint32_t   port,
             void*      data)
{
    Control* self = (Control*)instance;

    switch ((PortIndex)port) {
        case BUTTON:
            self->button = (const float*)data;
            break;
        case CV_SINGLE_PRESS:
            self->cv_single_press = (float*)data;
            break;
        case CV_LONG_PRESS:
            self->cv_long_press = (float*)data;
            break;
        case CV_DOUBLE_PRESS:
            self->cv_double_press = (float*)data;
            break;
        case LONG_PRESS_TIME_MS:
            self->long_press_time = (const float*)data;
            break;
        case DOUBLE_PRESS_DEBOUNCE_MS:
            self->double_press_debounce = (const float*)data;
            break;
        case LONG_PRESS_MODE:
            self->long_press_mode = (const float*)data;
            break;
        case BUTTON_STATUS_MASK:
            self->button_mask = (float*)data;
            break;
        case PARAMS_IN:
            self->in_port = (const LV2_Atom_Sequence*)data;
            break;
        case PARAMS_OUT:
            self->out_port = (LV2_Atom_Sequence*)data;
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
    URIs*    uris = &self->uris;
    LV2_Atom_Forge* forge = &self->forge;

    // Initially, self->out_port contains a Chunk with size set to capacity
    // Set up forge to write directly to output port
    const uint32_t out_capacity = self->out_port->atom.size;
    lv2_atom_forge_set_buffer(forge, (uint8_t*)self->out_port, out_capacity);
    lv2_atom_forge_sequence_head(forge, &self->notify_frame, 0);

    // Start a sequence in the output port
    LV2_Atom_Forge_Frame out_frame;
    lv2_atom_forge_sequence_head(forge, &out_frame, 0);

    // Read incoming events
    LV2_ATOM_SEQUENCE_FOREACH (self->in_port, ev) {
        const LV2_Atom_Object* obj = (const LV2_Atom_Object*)&ev->body;
        if (obj->body.otype == uris->patch_Set) {
            // Get the property and value of the set message
            const LV2_Atom_URID* subject  = NULL;
            const LV2_Atom_URID* property = NULL;
            const LV2_Atom*      value    = NULL;

            // clang-format off
            lv2_atom_object_get(obj,
                                uris->patch_subject,  (const LV2_Atom**)&subject,
                                uris->patch_property, (const LV2_Atom**)&property,
                                uris->patch_value,    &value,
                                0);
            // clang-format on

            if (!subject_is_plugin(self, subject)) {
                lv2_log_error(&self->logger, "Set for unknown subject\n");
            }
            else if (!property) {
                lv2_log_error(&self->logger, "Set with no property\n");
            }
            else if (property->atom.type != uris->atom_URID) {
                lv2_log_error(&self->logger, "Set property is not a URID\n");
            }
            else {
                // Set property to the given value
                const LV2_URID key = property->body;
                set_parameter(self, key, value->size, value->type, value + 1, false);
            }
        }
        else if (obj->body.otype == uris->patch_Get) {
            // Get the property of the get message
            const LV2_Atom_URID* subject  = NULL;
            const LV2_Atom_URID* property = NULL;

            // clang-format off
            lv2_atom_object_get(obj,
                                uris->patch_subject,  (const LV2_Atom**)&subject,
                                uris->patch_property, (const LV2_Atom**)&property,
                                0);
            // clang-format on

            if (!subject_is_plugin(self, subject)) {
                lv2_log_error(&self->logger, "Get with unknown subject\n");
            }
            else if (!property) {
                // Get with no property, emit complete state
                lv2_atom_forge_frame_time(forge, ev->time.frames);
                LV2_Atom_Forge_Frame pframe;
                lv2_atom_forge_object(forge, &pframe, 0, uris->patch_Put);
                lv2_atom_forge_key(forge, uris->patch_body);

                LV2_Atom_Forge_Frame bframe;
                lv2_atom_forge_object(forge, &bframe, 0, 0);
                save(self, write_param_to_forge, forge, 0, NULL);

                lv2_atom_forge_pop(forge, &bframe);
                lv2_atom_forge_pop(forge, &pframe);
            }
            else if (property->atom.type != uris->atom_URID) {
                lv2_log_error(&self->logger, "Get property is not a URID\n");
            }
            else {
                // Get for a specific property
                const LV2_URID  key   = property->body;
                const LV2_Atom* value = get_parameter(self, key);
                if (value) {
                    lv2_atom_forge_frame_time(forge, ev->time.frames);
                    LV2_Atom_Forge_Frame frame;
                    lv2_atom_forge_object(forge, &frame, 0, uris->patch_Set);
                    lv2_atom_forge_key(forge, uris->patch_property);
                    lv2_atom_forge_urid(forge, property->body);
                    store_prop(self,
                               NULL,
                               NULL,
                               write_param_to_forge,
                               forge,
                               uris->patch_value,
                               value);
                    lv2_atom_forge_pop(forge, &frame);
                }
            }
        }
    }

    // notify of state change
    if (self->state_changed) {
        const uint32_t last_frame = n_samples-1;

        // string 1
        {
            lv2_atom_forge_frame_time(forge, last_frame);
            LV2_Atom_Forge_Frame frame;
            lv2_atom_forge_object(forge, &frame, 0, uris->patch_Set);
            lv2_atom_forge_key(forge, uris->patch_property);
            lv2_atom_forge_urid(forge, uris->port1_string);
            lv2_atom_forge_key(forge, uris->patch_value);
            lv2_atom_forge_string(forge, self->state.port1string_data, strlen(self->state.port1string_data)+1);
            lv2_atom_forge_pop(forge, &frame);
        }

        // string 2
        {
            lv2_atom_forge_frame_time(forge, last_frame);
            LV2_Atom_Forge_Frame frame;
            lv2_atom_forge_object(forge, &frame, 0, uris->patch_Set);
            lv2_atom_forge_key(forge, uris->patch_property);
            lv2_atom_forge_urid(forge, uris->port2_string);
            lv2_atom_forge_key(forge, uris->patch_value);
            lv2_atom_forge_string(forge, self->state.port2string_data, strlen(self->state.port2string_data)+1);
            lv2_atom_forge_pop(forge, &frame);
        }

        // string 3
        {
            lv2_atom_forge_frame_time(forge, last_frame);
            LV2_Atom_Forge_Frame frame;
            lv2_atom_forge_object(forge, &frame, 0, uris->patch_Set);
            lv2_atom_forge_key(forge, uris->patch_property);
            lv2_atom_forge_urid(forge, uris->port3_string);
            lv2_atom_forge_key(forge, uris->patch_value);
            lv2_atom_forge_string(forge, self->state.port3string_data, strlen(self->state.port3string_data)+1);
            lv2_atom_forge_pop(forge, &frame);
        }

        self->state_changed = false;
    }

    float button_value = (float)*self->button;

    float LP_time = ((float)*self->long_press_time * self->sample_rate) / 1000;
    float DP_debounce = ((float)*self->double_press_debounce * self->sample_rate) / 1000;

    uint8_t LP_mode = (int)*self->long_press_mode;

    if (self->prev_button_value != button_value) {
        //button pressed
        if (button_value > 0) {
            //double press
            if (self->double_press_counter > 0) {
                //we have a double press trigger cv port double
                self->cv_double_value = 10.f - self->cv_double_value;
                self->double_press_counter = 0;
                trigger_widget_change(self, CV_DOUBLE_PRESS);
            }
            else {
                //no double press, start counting long press
                self->long_press_counter = LP_time;
            }
        }
        //button released
        else {
            //button released before long press, trigger cv port single
            if (self->long_press_counter > 0) {
               //also start double press counter
               self->double_press_counter = DP_debounce;
               self->long_press_counter = 0;
            }

            //long press is in momentary mode and on, so turn off
            if (LP_mode) {
                if (LP_mode == MOMENTARY_OFF)
                    self->cv_long_value = 10.f;
                else
                    self->cv_long_value = 0.f;

                trigger_widget_change(self, CV_LONG_PRESS);
            }
        }

        self->prev_button_value = button_value;
    }

    for(uint32_t i = 0; i < n_samples; i++) {
        if (self->long_press_counter > 0) {
            self->long_press_counter--;

            //reached longpress, toggle  long port
            if (self->long_press_counter == 0) {
                switch (LP_mode) {
                    case LATCHING:
                        self->cv_long_value = 10.f - self->cv_long_value;
                    break;

                    case MOMENTARY_ON:
                        self->cv_long_value = 10.f;
                    break;

                    case MOMENTARY_OFF:
                        self->cv_long_value = 0.f;
                    break;
                }

                trigger_widget_change(self, CV_LONG_PRESS);
            }
        }

        if (self->double_press_counter > 0) {
            self->double_press_counter--;

            //reached debounce, toggle single port
            if (self->double_press_counter == 0) {
                self->cv_single_value = 10.f - self->cv_single_value;
                trigger_widget_change(self, CV_SINGLE_PRESS);
            }
        }

        if (self->change_counter > 0) {
            self->change_counter--;

            if (self->change_counter == 0) {
                trigger_widget_change(self, SPECIAL_PORT_RESET);
            }
        }

        self->cv_single_press[i] = self->cv_single_value;
        self->cv_long_press[i] = self->cv_long_value;
        self->cv_double_press[i] = self->cv_double_value;
    }

    int bitmask = 0;
    if (self->cv_single_value)
        bitmask |= SINGLE_PRESS_ON;
    if (self->cv_long_value)
        bitmask |= LONG_PRESS_ON;
    if (self->cv_double_value)
        bitmask |= DOUBLE_PRESS_ON; 

    *self->button_mask = bitmask;

    lv2_atom_forge_pop(forge, &out_frame);
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

static void
addressed(LV2_Handle handle, uint32_t index, LV2_HMI_Addressing addressing, const LV2_HMI_AddressingInfo* info)
{
    Control* self = (Control*) handle;

    if (index == 0) {
        self->toggle_addressing = addressing;
        trigger_widget_change(self, SPECIAL_PORT_RESET);
    }
}

static void
unaddressed(LV2_Handle handle, uint32_t index)
{
    Control* self = (Control*) handle;

    if (index == BUTTON)
        self->toggle_addressing = NULL;
}

static const void*
extension_data(const char* uri)
{
    static const LV2_HMI_PluginNotification hmiNotif = {
        addressed,
        unaddressed,
    };
    if (!strcmp(uri, LV2_HMI__PluginNotification))
        return &hmiNotif;

    static const LV2_State_Interface state = {save, restore};
    if (!strcmp(uri, LV2_STATE__interface)) {
        return &state;
    }

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
