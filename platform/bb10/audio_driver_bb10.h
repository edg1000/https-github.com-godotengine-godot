/*************************************************************************/
/*  audio_driver_bb10.h                                                  */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2017 Juan Linietsky, Ariel Manzur.                 */
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
#include "servers/audio_server.h"

#include "core/os/thread.h"
#include "core/os/mutex.h"

#include <sys/asoundlib.h>

class AudioDriverBB10 : public AudioDriver {

	Thread* thread;
	Mutex* mutex;

	snd_pcm_t* pcm_handle;

	int32_t* samples_in;
	int16_t* samples_out;
	int sample_buf_count;

	static void thread_func(void* p_udata);

	int mix_rate;
	SpeakerMode speaker_mode;

	int pcm_frag_size;
	int pcm_max_frags;

	bool active;
	bool thread_exited;
	mutable bool exit_thread;
	bool pcm_open;

public:

	const char* get_name() const {
		return "BB10";
	};

	virtual Error init();
	virtual Error init(const char* p_name);
	virtual void start();
	virtual int get_mix_rate() const;
	virtual SpeakerMode get_speaker_mode() const;
	virtual void lock();
	virtual void unlock();
	virtual void finish();

	AudioDriverBB10();
	~AudioDriverBB10();
};

