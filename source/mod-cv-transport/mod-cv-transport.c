#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/atom/forge.h>
#include <lv2/lv2plug.in/ns/ext/log/logger.h>
#include <lv2/lv2plug.in/ns/ext/midi/midi.h>
#include "lv2/lv2plug.in/ns/ext/time/time.h"
#include <lv2/lv2plug.in/ns/ext/urid/urid.h>

#define PLUGIN_URI "http://moddevices.com/plugins/mod-devel/mod-cv-transport"


typedef enum {
    EVENTS_IN = 0,
    OUTPUT    = 1,
	STATUS    = 2,
    ENABLE    = 3
} PortIndex;


typedef struct {
    LV2_URID atom_Blank;
    LV2_URID atom_Float;
    LV2_URID atom_Object;
    LV2_URID time_Position;
    LV2_URID time_speed;
} ClockURIs;

typedef struct {
    LV2_URID_Map*          map; // URID map feature
    LV2_Log_Log* 	       log;
    LV2_Log_Logger      logger; // Logger API
    ClockURIs             uris; // Cache of mapped URIDs

    const LV2_Atom_Sequence* events_in;

    float* input;
    float* output;
    float* status;
    float* plugin_enabled;

    float speed;
} CVTransport;


static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
        double                    rate,
        const char*               bundle_path,
        const LV2_Feature* const* features)
{
    CVTransport* self = (CVTransport*)malloc(sizeof(CVTransport));

    if (!self)
    {
        return NULL;
    }

    for (uint32_t i=0; features[i]; ++i)
    {
        if (!strcmp (features[i]->URI, LV2_URID__map))
        {
            self->map = (LV2_URID_Map*)features[i]->data;
        }
        else if (!strcmp (features[i]->URI, LV2_LOG__log))
        {
            self->log = (LV2_Log_Log*)features[i]->data;
        }
    }

    lv2_log_logger_init (&self->logger, self->map, self->log);

    if (!self->map) {
        lv2_log_error (&self->logger, "arpeggiator.lv2 error: Host does not support urid:map\n");
        free (self);
        return NULL;
    }
    // Map URIS
    ClockURIs* const    uris  = &self->uris;
    LV2_URID_Map* const map   = self->map;

    uris->atom_Blank          = map->map(map->handle, LV2_ATOM__Blank);
    uris->atom_Float          = map->map(map->handle, LV2_ATOM__Float);
    uris->atom_Object         = map->map(map->handle, LV2_ATOM__Object);
    uris->time_Position       = map->map(map->handle, LV2_TIME__Position);
    uris->time_speed          = map->map(map->handle, LV2_TIME__speed);

    return (LV2_Handle)self;
}


static void
update_position(CVTransport* self, const LV2_Atom_Object* obj)
{
    const ClockURIs* uris = &self->uris;

    // Received new transport position/speed
    LV2_Atom *speed = NULL;
    lv2_atom_object_get(obj, uris->time_speed, &speed, NULL);

    if (speed && speed->type == uris->atom_Float)
    {
        // Speed changed, e.g. 0 (stop) to 1 (play)
        self->speed = ((LV2_Atom_Float*)speed)->body;
    }
}

static void
connect_port(LV2_Handle instance,
        uint32_t   port,
        void*      data)
{
    CVTransport* self = (CVTransport*)instance;

    switch ((PortIndex)port) {
        case EVENTS_IN:
            self->events_in = (const LV2_Atom_Sequence*)data;
            break;
        case OUTPUT:
            self->output = (float*)data;
            break;
        case STATUS:
            self->status = (float*)data;
            break;
        case ENABLE:
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
    CVTransport* self = (CVTransport*) instance;
    const ClockURIs* uris = &self->uris;

    // Read incoming events
    LV2_ATOM_SEQUENCE_FOREACH(self->events_in, ev)
    {
        if (ev->body.type == uris->atom_Object ||
                ev->body.type == uris->atom_Blank) {
            const LV2_Atom_Object* obj = (const LV2_Atom_Object*)&ev->body;
            if (obj->body.otype == uris->time_Position) {
                update_position(self, obj);
            }
        }
    }

    if ((int)*self->plugin_enabled == 1) {

        const float value = self->speed * 10.0;
        *self->status = self->speed;

        for ( uint32_t i = 0; i < n_samples; i++) {
            self->output[i] = value;
        }

    } else {
        memset(self->output, 0, sizeof(float)*n_samples);
        *self->status = 0.0;
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
