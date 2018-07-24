/*************************************************************************/
/*  audio_driver_pulseaudio.h                                            */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2018 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2018 Godot Engine contributors (cf. AUTHORS.md)    */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifdef PULSEAUDIO_ENABLED

#ifndef AUDIO_DRIVER_PULSEAUDIO_H
#define AUDIO_DRIVER_PULSEAUDIO_H

#include "core/os/mutex.h"
#include "core/os/thread.h"
#include "servers/audio_server.h"

#include <pulse/pulseaudio.h>

class AudioDriverPulseAudio : public AudioDriver {

	Thread *thread;
	Mutex *mutex;

	pa_mainloop *pa_ml;
	pa_context *pa_ctx;
	pa_stream *pa_str;
	pa_channel_map pa_map;

	String device_name;
	String new_device;
	String default_device;

	Vector<int16_t> samples_out;

	unsigned int mix_rate;
	unsigned int buffer_frames;
	int channels;
	int pa_ready;
	int pa_status;
	Array pa_devices;

	bool active;
	bool thread_exited;
	mutable bool exit_thread;

	float latency;

	static void pa_state_cb(pa_context *c, void *userdata);
	static void pa_sink_info_cb(pa_context *c, const pa_sink_info *l, int eol, void *userdata);
	static void pa_server_info_cb(pa_context *c, const pa_server_info *i, void *userdata);
	static void pa_sinklist_cb(pa_context *c, const pa_sink_info *l, int eol, void *userdata);

	Error init_device();
	void finish_device();

	void detect_channels();

	static void thread_func(void *p_udata);

public:
	const char *get_name() const {
		return "PulseAudio";
	};

	virtual Error init();
	virtual void start();
	virtual int get_mix_rate() const;
	virtual SpeakerMode get_speaker_mode() const;
	virtual Array get_device_list();
	virtual String get_device();
	virtual void set_device(String device);
	virtual void lock();
	virtual void unlock();
	virtual void finish();

	virtual float get_latency();

	AudioDriverPulseAudio();
	~AudioDriverPulseAudio();
};

#endif // AUDIO_DRIVER_PULSEAUDIO_H

#endif // PULSEAUDIO_ENABLED
