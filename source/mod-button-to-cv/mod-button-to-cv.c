#include <math.h>
#include <stdlib.h>
#include <stdio.h>

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

#define N_PROPS 3
#define MAX_STRING 1024

#define PORT_1_DEF_NAME     "PORT 1"
#define PORT_2_DEF_NAME     "PORT 2"
#define PORT_3_DEF_NAME     "PORT 3"

#define SINGLE_PRESS_TXT     "SINGLE PRESS"
#define LONG_PRESS_TXT       "LONG PRESS"
#define DOUBLE_PRESS_TXT     "DOUBLE PRESS"

#define PLUGIN_URI "http://moddevices.com/plugins/mod-devel/mod-button-to-cv"

//timing configs
#define CHANGE_COUNTER                 1000

typedef struct {
  LV2_URID plugin;
  LV2_URID atom_Path;
  LV2_URID atom_Sequence;
  LV2_URID atom_URID;
  LV2_URID atom_eventTransfer;
  LV2_URID eg_spring;
  LV2_URID midi_Event;
  LV2_URID patch_Get;
  LV2_URID patch_Set;
  LV2_URID patch_Put;
  LV2_URID patch_body;
  LV2_URID patch_subject;
  LV2_URID patch_property;
  LV2_URID patch_value;
} URIs;

typedef struct {
  LV2_Atom        port1string;
  char            port1string_data[MAX_STRING];
  LV2_Atom        port2string;
  char            port2string_data[MAX_STRING];
  LV2_Atom        port3string;
  char            port3string_data[MAX_STRING];
} State;

static inline void
map_uris(LV2_URID_Map* map, URIs* uris)
{
  uris->plugin = map->map(map->handle, PLUGIN_URI);

  uris->atom_Path          = map->map(map->handle, LV2_ATOM__Path);
  uris->atom_Sequence      = map->map(map->handle, LV2_ATOM__Sequence);
  uris->atom_URID          = map->map(map->handle, LV2_ATOM__URID);
  uris->atom_eventTransfer = map->map(map->handle, LV2_ATOM__eventTransfer);
  uris->midi_Event         = map->map(map->handle, LV2_MIDI__MidiEvent);
  uris->patch_Get          = map->map(map->handle, LV2_PATCH__Get);
  uris->patch_Set          = map->map(map->handle, LV2_PATCH__Set);
  uris->patch_Put          = map->map(map->handle, LV2_PATCH__Put);
  uris->patch_body         = map->map(map->handle, LV2_PATCH__body);
  uris->patch_subject      = map->map(map->handle, LV2_PATCH__subject);
  uris->patch_property     = map->map(map->handle, LV2_PATCH__property);
  uris->patch_value        = map->map(map->handle, LV2_PATCH__value);
}

typedef enum {
    BUTTON = 0,
    CV_SINGLE_PRESS,
    CV_LONG_PRESS,
    CV_DOUBLE_PRESS,
    LONG_PRESS_TIME_MS,
    DOUBLE_PRESS_DEBOUNCE_MS,
    PARAMS_IN,
    PARAMS_OUT
} PortIndex;

typedef struct {
    
    //main button
    float* button;

    // CV signals
    float* cv_single_press;
    float* cv_long_press;
    float* cv_double_press;

    //controls
    float* long_press_time;
    float* double_press_debounce;

    float prev_button_value;

    float cv_single_value;
    float cv_long_value;
    float cv_double_value;

    uint32_t long_press_counter;
    uint32_t double_press_counter;
    uint32_t change_counter;

    char single_press_string[9];
    char long_press_string[9];
    char double_press_string[9];

    // Features
    LV2_URID_Map*  map;
    LV2_URID_Unmap* unmap;
    LV2_Log_Logger logger;
    LV2_HMI_WidgetControl* hmi;
    
    LV2_Atom_Forge forge;
    LV2_Atom_Forge_Ref ref;

    // Ports
    const LV2_Atom_Sequence* in_port;
    LV2_Atom_Sequence*       out_port;

    // URIs
    URIs uris;

    // Plugin state
    StateMapItem props[N_PROPS];
    State        state;

    // Buffer for making strings from URIDs if unmap is not provided
    char urid_buf[1024];

    // HMI Widgets stuff
    LV2_HMI_Addressing toggle_addressing;
    LV2_HMI_LED_Colour colour;
} Control;

void trigger_widget_change(Control* self, uint8_t port_index)
{
    //if we dont reset, set counter
    if (port_index > 0)
        self->change_counter = (CHANGE_COUNTER * 48000) / 1000;

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
        case 0:
        {
            //reset LED
            self->hmi->set_led_with_brightness(self->hmi->handle, self->toggle_addressing, LV2_HMI_LED_Colour_White, LV2_HMI_LED_Brightness_Low);
        }
        break;

        case CV_SINGLE_PRESS:
        {
            //send to HMI
            char *label = self->state.port1string_data;
            if ((label != NULL) && (label[0] == '\0')) 
                label = PORT_1_DEF_NAME;

            if (self->hmi->size >= LV2_HMI_WIDGETCONTROL_SIZE_POPUP_MESSAGE) {
                if (self->cv_single_value)
                    self->hmi->popup_message(self->hmi->handle, self->toggle_addressing, LV2_HMI_Popup_Style_Inverted, SINGLE_PRESS_TXT, label);
                else
                    self->hmi->popup_message(self->hmi->handle, self->toggle_addressing, LV2_HMI_Popup_Style_Normal, SINGLE_PRESS_TXT, label);
            } else {
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
            if ((label != NULL) && (label[0] == '\0'))
                label = PORT_2_DEF_NAME;

            if (self->hmi->size >= LV2_HMI_WIDGETCONTROL_SIZE_POPUP_MESSAGE) {
                if (self->cv_long_value)
                    self->hmi->popup_message(self->hmi->handle, self->toggle_addressing, LV2_HMI_Popup_Style_Inverted, LONG_PRESS_TXT, label);
                else
                    self->hmi->popup_message(self->hmi->handle, self->toggle_addressing, LV2_HMI_Popup_Style_Normal, LONG_PRESS_TXT, label);
            } else {
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
            if ((label != NULL) && (label[0] == '\0'))
                label = PORT_3_DEF_NAME;

            if (self->hmi->size >= LV2_HMI_WIDGETCONTROL_SIZE_POPUP_MESSAGE) {
                if (self->cv_double_value)
                    self->hmi->popup_message(self->hmi->handle, self->toggle_addressing, LV2_HMI_Popup_Style_Inverted, DOUBLE_PRESS_TXT, label);
                else
                    self->hmi->popup_message(self->hmi->handle, self->toggle_addressing, LV2_HMI_Popup_Style_Normal, DOUBLE_PRESS_TXT, label);
            } else {
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
            LV2_URID__unmap,        &self->unmap,       false,
            LV2_HMI__WidgetControl, &self->hmi,         true,
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
        PLUGIN_URI "#port1string", STATE_MAP_INIT(String, &state->port1string),
        PLUGIN_URI "#port2string", STATE_MAP_INIT(String, &state->port2string),
        PLUGIN_URI "#port3string", STATE_MAP_INIT(String, &state->port3string),
        NULL);
    // clang-format on

    self->colour = LV2_HMI_LED_Colour_Off;

    return (LV2_Handle)self;
}

/** Helper function to unmap a URID if possible. */
static const char*
unmap(Control* self, LV2_URID urid)
{
  if (self->unmap) {
    return self->unmap->unmap(self->unmap->handle, urid);
  } else {
    snprintf(self->urid_buf, sizeof(self->urid_buf), "%u", urid);
    return self->urid_buf;
  }
}

static LV2_State_Status
check_type(Control* self, LV2_URID key, LV2_URID type, LV2_URID required_type)
{
  if (type != required_type) {
    lv2_log_trace(&self->logger,
                  "Bad type <%s> for <%s> (needs <%s>)\n",
                  unmap(self, type),
                  unmap(self, key),
                  unmap(self, required_type));
    return LV2_STATE_ERR_BAD_TYPE;
  }
  return LV2_STATE_SUCCESS;
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
  if (!entry) {
    lv2_log_trace(&self->logger, "Unknown parameter <%s>\n", unmap(self, key));
    return LV2_STATE_ERR_NO_PROPERTY;
  }

  // Ensure given type matches property's type
  if (check_type(self, key, type, entry->value->type)) {
    return LV2_STATE_ERR_BAD_TYPE;
  }

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

  lv2_log_trace(&self->logger, "Unknown parameter <%s>\n", unmap(self, key));
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
store_prop(Control*                  self,
           LV2_State_Map_Path*      map_path,
           LV2_State_Status*        save_status,
           LV2_State_Store_Function store,
           LV2_State_Handle         handle,
           LV2_URID                 key,
           const LV2_Atom*          value)
{
  LV2_State_Status st = LV2_STATE_SUCCESS;
  if (map_path && value->type == self->uris.atom_Path) {
    // Map path to abstract path for portable storage
    const char* path  = (const char*)(value + 1);
    char*       apath = map_path->abstract_path(map_path->handle, path);
    st                = store(handle,
               key,
               apath,
               strlen(apath) + 1,
               self->uris.atom_Path,
               LV2_STATE_IS_POD | LV2_STATE_IS_PORTABLE);
    free(apath);
  } else {
    // Store simple property
    st = store(handle,
               key,
               value + 1,
               value->size,
               value->type,
               LV2_STATE_IS_POD | LV2_STATE_IS_PORTABLE);
  }

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
              LV2_URID                    key)
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
  Control*          self = (Control*)instance;
  LV2_State_Status st   = LV2_STATE_SUCCESS;

  for (unsigned i = 0; i < N_PROPS; ++i) {
    retrieve_prop(self, &st, retrieve, handle, self->props[i].urid);
  }

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
            self->button = (float*)data;
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
            self->long_press_time = (float*)data;
            break;
        case DOUBLE_PRESS_DEBOUNCE_MS:
            self->double_press_debounce = (float*)data;
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
    URIs*   uris = &self->uris;

    // Initially, self->out_port contains a Chunk with size set to capacity
    // Set up forge to write directly to output port
    const uint32_t out_capacity = self->out_port->atom.size;
    lv2_atom_forge_set_buffer(
      &self->forge, (uint8_t*)self->out_port, out_capacity);

    // Start a sequence in the output port
    LV2_Atom_Forge_Frame out_frame;
    lv2_atom_forge_sequence_head(&self->forge, &out_frame, 0);

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
            } else if (!property) {
              lv2_log_error(&self->logger, "Set with no property\n");
            } else if (property->atom.type != uris->atom_URID) {
              lv2_log_error(&self->logger, "Set property is not a URID\n");
            } else {
              // Set property to the given value
              const LV2_URID key = property->body;
              set_parameter(self, key, value->size, value->type, value + 1, false);
            }
        } else if (obj->body.otype == uris->patch_Get) {
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
      } else if (!property) {
        // Get with no property, emit complete state
        lv2_atom_forge_frame_time(&self->forge, ev->time.frames);
        LV2_Atom_Forge_Frame pframe;
        lv2_atom_forge_object(&self->forge, &pframe, 0, uris->patch_Put);
        lv2_atom_forge_key(&self->forge, uris->patch_body);

        LV2_Atom_Forge_Frame bframe;
        lv2_atom_forge_object(&self->forge, &bframe, 0, 0);
        save(self, write_param_to_forge, &self->forge, 0, NULL);

        lv2_atom_forge_pop(&self->forge, &bframe);
        lv2_atom_forge_pop(&self->forge, &pframe);
      } else if (property->atom.type != uris->atom_URID) {
        lv2_log_error(&self->logger, "Get property is not a URID\n");
      } else {
        // Get for a specific property
        const LV2_URID  key   = property->body;
        const LV2_Atom* value = get_parameter(self, key);
        if (value) {
          lv2_atom_forge_frame_time(&self->forge, ev->time.frames);
          LV2_Atom_Forge_Frame frame;
          lv2_atom_forge_object(&self->forge, &frame, 0, uris->patch_Set);
          lv2_atom_forge_key(&self->forge, uris->patch_property);
          lv2_atom_forge_urid(&self->forge, property->body);
          store_prop(self,
                     NULL,
                     NULL,
                     write_param_to_forge,
                     &self->forge,
                     uris->patch_value,
                     value);
          lv2_atom_forge_pop(&self->forge, &frame);
        }
      }
    } else {
      lv2_log_trace(
        &self->logger, "Unknown object type <%s>\n", unmap(self, obj->body.otype));
    }
  } 

    float button_value = (float)*self->button;

    float LP_time = ((float)*self->long_press_time * 48000) / 1000;
    float DP_debounce = ((float)*self->double_press_debounce * 48000) / 1000;

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
        }

        self->prev_button_value = button_value;
    }

    for(uint32_t i = 0; i < n_samples; i++) {
        if (self->long_press_counter > 0) {
            self->long_press_counter--;

            //reached longpress, toggle  long port
            if (self->long_press_counter == 0) {
                self->cv_long_value = 10.f - self->cv_long_value;
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
                trigger_widget_change(self, 0);
            }
        }

        self->cv_single_press[i] = self->cv_single_value;
        self->cv_long_press[i] = self->cv_long_value;
        self->cv_double_press[i] = self->cv_double_value;
    }

    lv2_atom_forge_pop(&self->forge, &out_frame);
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
        trigger_widget_change(self, 0);
    }
}

static void
unaddressed(LV2_Handle handle, uint32_t index)
{
    Control* self = (Control*) handle;

    if (index == 0)
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
