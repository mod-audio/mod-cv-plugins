#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/atom/forge.h>
#include <lv2/lv2plug.in/ns/ext/log/logger.h>
#include <lv2/lv2plug.in/ns/ext/midi/midi.h>
#include "lv2/lv2plug.in/ns/ext/time/time.h"
#include <lv2/lv2plug.in/ns/ext/urid/urid.h>

#ifndef DEBUG
#define DEBUG 0
#endif
#define debug_print(...) \
((void)((DEBUG) ? fprintf(stderr, __VA_ARGS__) : 0))


#ifndef M_PI
#    define M_PI 3.14159265
#endif
#define PLUGIN_URI "http://moddevices.com/plugins/mod-devel/mod-cv-clock"

static const double attack_s = 0.00005;
static const double decay_s  = 0.00005;

typedef enum {
	OUTPUT_PORT1   = 0,
	OUTPUT_PORT2   = 1,
	BPM_PORT       = 2,
	DIVISIONS_PORT = 3,
	SYNC_PORT      = 4,
	CONTROL_PORT   = 5
} PortIndex;

//During execution this plugin can be in one of 3 states:
typedef enum {
	STATE_ATTACK = 0,// Envelope rising
	STATE_DECAY  = 1,// Envelope lowering
	STATE_OFF    = 2, // Silent
	STATE_HOLD	 = 3
} State;

typedef struct {
	LV2_URID atom_Blank;
	LV2_URID atom_Float;
	LV2_URID atom_Object;
	LV2_URID atom_Path;
	LV2_URID atom_Resource;
	LV2_URID atom_Sequence;
	LV2_URID time_Position;
	LV2_URID time_barBeat;
	LV2_URID time_beatsPerMinute;
	LV2_URID time_speed;
} ClockURIs;

typedef struct {
	LV2_URID_Map*          map; // URID map feature
	LV2_Log_Log* 	       log;
	LV2_Log_Logger      logger; // Logger API
	ClockURIs             uris; // Cache of mapped URIDs

	float*         pulseOutput;
	float*           changeBpm;
	float   	     divisions;
	float*          changedDiv;
	float 	     *squareOutput;
  double          samplerate;
	LV2_Atom_Sequence* control;
	int*   		    	  sync;
  int            prevSync;
	// Variables to keep track of the tempo information sent by the host
	float        	       bpm; // Beats per minute (tempo)
	uint32_t    	       pos;
	uint32_t   	        period;
	uint32_t	  h_wavelength;

  bool printed;
	float   	         speed; // Transport speed (usually 0=stop, 1=play)
  float              beatInMeasure;

	float 	   elapsed_len; // Frames since the start of the last click
	uint32_t 	   wave_offset; // Current play offset in the wave
	State    	         state; // Current play state

	// Envelope parameters
	uint32_t	    attack_len;
	uint32_t 	     decay_len;
} Clock;


static void
connect_port(LV2_Handle instance,
             uint32_t   port,
             void*      data)
{
	Clock* self = (Clock*)instance;

	switch ((PortIndex)port) {
	case OUTPUT_PORT1:
		self->pulseOutput    = (float*)data;
		break;
	case  OUTPUT_PORT2:
		self->squareOutput   = (float*)data;
		break;
	case BPM_PORT:
		self->changeBpm = (float*)data;
		break;
	case DIVISIONS_PORT:
		self->changedDiv = (float*)data;
		break;
	case SYNC_PORT:
		self->sync = (int*)data;
		break;
	case CONTROL_PORT:
		self->control = (LV2_Atom_Sequence*)data;
		break;
	}
}

static void
activate(LV2_Handle instance)
{
	Clock* self = (Clock*)instance;

	self->bpm = *self->changeBpm;
	self->divisions =*self->changedDiv;
	self->pos = 0;
	//self->period = (uint32_t)(self->samplerate * (60.0f / (self->bpm * (self->divisions / 2.0f))));
	//self->h_wavelength = (self->period/2.0f);
}

static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
            double                    rate,
            const char*               bundle_path,
            const LV2_Feature* const* features)
{
	Clock* self = (Clock*)calloc(1, sizeof(Clock));
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
		lv2_log_error (&self->logger, "StepSeq.lv2 error: Host does not support urid:map\n");
		free (self);
		return NULL;
	}

		// Map URIS
	ClockURIs* const    uris  = &self->uris;
	LV2_URID_Map* const map   = self->map;
	uris->atom_Blank          = map->map(map->handle, LV2_ATOM__Blank);
	uris->atom_Float          = map->map(map->handle, LV2_ATOM__Float);
	uris->atom_Object         = map->map(map->handle, LV2_ATOM__Object);
	uris->atom_Path           = map->map(map->handle, LV2_ATOM__Path);
	uris->atom_Resource       = map->map(map->handle, LV2_ATOM__Resource);
	uris->atom_Sequence       = map->map(map->handle, LV2_ATOM__Sequence);
	uris->time_Position       = map->map(map->handle, LV2_TIME__Position);
	uris->time_barBeat        = map->map(map->handle, LV2_TIME__barBeat);
	uris->time_beatsPerMinute = map->map(map->handle, LV2_TIME__beatsPerMinute);
	uris->time_speed          = map->map(map->handle, LV2_TIME__speed);

  debug_print("DEBUGING");
	self->attack_len = (uint32_t)(attack_s * rate);
	self->decay_len  = (uint32_t)(decay_s * rate);
	self->state      = STATE_OFF;
  self->samplerate = rate;
  self->prevSync   = 0; 
  self->beatInMeasure = 0;
  self->printed = false;
	return (LV2_Handle)self;
}



// Update the current position based on a host message.  This is called by
// run() when a time:Position is received.

static void
update_position(Clock* self, const LV2_Atom_Object* obj)
{
	const ClockURIs* uris = &self->uris;

	// Received new transport position/speed
	LV2_Atom *beat = NULL, *bpm = NULL, *speed = NULL;
	lv2_atom_object_get(obj,
	                    uris->time_barBeat, &beat,
	                    uris->time_beatsPerMinute, &bpm,
	                    uris->time_speed, &speed,
	                    NULL);
	if (bpm && bpm->type == uris->atom_Float)
	{
		// Tempo changed, update BPM
		self->bpm = ((LV2_Atom_Float*)bpm)->body;
	}
	if (speed && speed->type == uris->atom_Float)
	{
		// Speed changed, e.g. 0 (stop) to 1 (play)
		self->speed = ((LV2_Atom_Float*)speed)->body;
	}
	if (beat && beat->type == uris->atom_Float)
	{
			// Received a beat position, synchronise
			// This hard sync may cause clicks, a real plugin would be more graceful
			const float frames_per_beat = (self->samplerate * (60.0f / (self->bpm * self->divisions)));
			const float bar_beats       = (((LV2_Atom_Float*)beat)->body * self->divisions);
			const float beat_beats      = bar_beats - floorf(bar_beats);
      self->beatInMeasure         = ((LV2_Atom_Float*)beat)->body; 
			self->elapsed_len           = beat_beats * frames_per_beat;
			//self->h_wavelength 	        = (uint32_t)(frames_per_beat/2.0f);
	}
}

static uint32_t 
resetPhase(Clock* self)
{
  //fmod?
  uint32_t pos = (uint32_t)fmod(self->samplerate * (60.0f / self->bpm) * self->beatInMeasure, (self->samplerate * (60.0f / (self->bpm * (self->divisions / 2.0f)))));
  debug_print("pos in resetPhase = %i\n", pos);
  return pos;
}


static void
run(LV2_Handle instance, uint32_t n_samples)
{
  Clock* self = (Clock*)instance;
  float bpm;

  if (!*self->sync) {
    self->bpm = *self->changeBpm;
  } else {
    self->bpm = self->bpm;
  }

  if (*self->sync != self->prevSync) {
    self->pos = resetPhase(self);
    self->prevSync = *self->sync;
  }

  self->divisions = *self->changedDiv;
  self->period = (uint32_t)(self->samplerate * (60.0f / (self->bpm * (self->divisions / 2.0f))));
  self->h_wavelength = (self->period/2.0f);

  const ClockURIs* uris = &self->uris;
  const LV2_Atom_Sequence* in     = self->control;

  for (const LV2_Atom_Event* ev = lv2_atom_sequence_begin(&in->body);
      !lv2_atom_sequence_is_end(&in->body, in->atom.size, ev);
      ev = lv2_atom_sequence_next(ev)) {

    if (ev->body.type == uris->atom_Object ||
        ev->body.type == uris->atom_Blank) {
      const LV2_Atom_Object* obj = (const LV2_Atom_Object*)&ev->body;
      if (obj->body.otype == uris->time_Position) {
        update_position(self, obj);
      }
    }
  }

  if (self->divisions != *self->changedDiv) {
    self->divisions = *self->changedDiv;
  }

  for(uint32_t i = 0; i < n_samples; i ++) {
    if(self->pos >= self->period && i < n_samples) {
      self->printed = false;
      self->squareOutput[i] = 2.0f;
      self->pulseOutput[i] = 2.0f;
      self->pos = 0;
    } else {
      if(self->pos < self->h_wavelength) {
        self->squareOutput[i] = 2.0f;
      } else {
        if (!self->printed) {
          debug_print("self->h_waveLength = %i\n", self->h_wavelength);
          debug_print("self->period = %i\n", self->period);
          debug_print("pos in switch to zero = %i\n", self->pos);
          self->printed = true;
        }
        self->squareOutput[i] = 0.0f;
      }
      self->pulseOutput[i] = 0.0f;
    }
    self->pos += 1;
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

//static void
//play(Clock* self, uint32_t begin, uint32_t end)
//{
//	float* const   output          = self->squareOutput;
//	const uint32_t frames_per_beat = (self->samplerate * (60.0f / (self->bpm * (self->divisions / 1.0f))));
//	if (self->speed == 0.0f)
//	{
//		//self->pos = 0;
//		memset(output, 0.0f, (end - begin) * sizeof(float));
//		return;
//	}
//	for (uint32_t i = begin; i < end; ++i)
//	{
//		switch (self->state)
//		{
//		case STATE_ATTACK:
//			// Amplitude increases from 0..1 until attack_len
//			output[i] = 2.0f *self->elapsed_len / (float)self->attack_len;
//			self->pulseOutput[i] = 2.0f;
//			if (self->elapsed_len >= self->attack_len)
//			{
//				self->state = STATE_HOLD;
//			}
//			break;
//		case STATE_DECAY:
//			// Amplitude decreases from 1..0 until attack_len + decay_len
//			output[i] = 2.0f * (1 - ((self->elapsed_len - self->h_wavelength) / (float)self->decay_len));
//			self->pulseOutput[i] = 0.0f;
//			if (self->elapsed_len >= self->h_wavelength + self->decay_len)
//			{
//				self->state = STATE_OFF;
//			}
//			break;
//		case STATE_OFF:
//			output[i] = 0.0f;
//			self->pulseOutput[i] = 0.0f;
//			break;
//		case STATE_HOLD:
//			output[i] = 2.0f;
//			self->pulseOutput[i] = 0.0f;
//			if(self->elapsed_len > self->h_wavelength)
//			{
//				self->state = STATE_DECAY;
//			}
//			break;
//		default:
//		break;
//		}
//
//		// Update elapsed time and start attack if necessary
//		if (++self->elapsed_len == frames_per_beat)
//		{
//			self->state       = STATE_ATTACK;
//			self->elapsed_len = 0;
//		}
//	}
//}
