/*************************************************************************/
/*  audio_rb_resampler.h                                                 */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2017 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2017 Godot Engine contributors (cf. AUTHORS.md)    */
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
#ifndef AUDIO_RB_RESAMPLER_H
#define AUDIO_RB_RESAMPLER_H

#include "os/memory.h"
#include "typedefs.h"
#include "servers/audio_server.h"

struct AudioRBResampler {

	uint32_t rb_bits;
	uint32_t rb_len;
	uint32_t rb_mask;
	uint32_t read_buff_len;
	uint32_t channels;
	uint32_t src_mix_rate;
	uint32_t target_mix_rate;

	volatile int rb_read_pos;
	volatile int rb_write_pos;

	int32_t offset; //contains the fractional remainder of the resampler
	enum {
		MIX_FRAC_BITS = 13,
		MIX_FRAC_LEN = (1 << MIX_FRAC_BITS),
		MIX_FRAC_MASK = MIX_FRAC_LEN - 1,
	};

	float *read_buf;
	float *rb;


	uint32_t copy_stereo(AudioFrame *dest, int count);

	template <int C>
	uint32_t _resample(AudioFrame *p_dest, int p_todo);

public:
	_FORCE_INLINE_ void flush() {
		rb_read_pos = 0;
		rb_write_pos = 0;
		offset = 0;
	}

	_FORCE_INLINE_ bool is_ready() const {
		return rb != NULL;
	}

	_FORCE_INLINE_ int get_total() const {

		return rb_len - 1;
	}

	_FORCE_INLINE_ int get_todo() const { //return amount of frames to mix

		int todo;
		int read_pos_cache = rb_read_pos;

		if (read_pos_cache == rb_write_pos) {
			todo = rb_len - 1;
		} else if (read_pos_cache > rb_write_pos) {

			todo = read_pos_cache - rb_write_pos - 1;
		} else {

			todo = (rb_len - rb_write_pos) + read_pos_cache - 1;
		}

		return todo;
	}

	_FORCE_INLINE_ bool has_data() const {
		return rb && rb_read_pos != rb_write_pos;
	}

	_FORCE_INLINE_ float *get_write_buffer() { return read_buf; }
	_FORCE_INLINE_ void write(uint32_t p_frames) {

		ERR_FAIL_COND(p_frames >= rb_len);

		switch (channels) {
			case 1: {

				for (uint32_t i = 0; i < p_frames; i++) {

					rb[rb_write_pos] = read_buf[i];
					rb_write_pos = (rb_write_pos + 1) & rb_mask;
				}
			} break;
			case 2: {

				for (uint32_t i = 0; i < p_frames; i++) {

					rb[(rb_write_pos << 1) + 0] = read_buf[(i << 1) + 0];
					rb[(rb_write_pos << 1) + 1] = read_buf[(i << 1) + 1];
					rb_write_pos = (rb_write_pos + 1) & rb_mask;
				}
			} break;
			case 4: {

				for (uint32_t i = 0; i < p_frames; i++) {

					rb[(rb_write_pos << 2) + 0] = read_buf[(i << 2) + 0];
					rb[(rb_write_pos << 2) + 1] = read_buf[(i << 2) + 1];
					rb[(rb_write_pos << 2) + 2] = read_buf[(i << 2) + 2];
					rb[(rb_write_pos << 2) + 3] = read_buf[(i << 2) + 3];
					rb_write_pos = (rb_write_pos + 1) & rb_mask;
				}
			} break;
			case 6: {

				for (uint32_t i = 0; i < p_frames; i++) {

					rb[(rb_write_pos * 6) + 0] = read_buf[(i * 6) + 0];
					rb[(rb_write_pos * 6) + 1] = read_buf[(i * 6) + 1];
					rb[(rb_write_pos * 6) + 2] = read_buf[(i * 6) + 2];
					rb[(rb_write_pos * 6) + 3] = read_buf[(i * 6) + 3];
					rb[(rb_write_pos * 6) + 4] = read_buf[(i * 6) + 4];
					rb[(rb_write_pos * 6) + 5] = read_buf[(i * 6) + 5];
					rb_write_pos = (rb_write_pos + 1) & rb_mask;
				}
			} break;
		}
	}

	int get_channel_count() const;

	Error setup(int p_channels, int p_src_mix_rate, int p_target_mix_rate, int p_buffer_msec, int p_minbuff_needed = -1);
	void clear();
	bool mix(AudioFrame *p_dest, int p_frames);

	AudioRBResampler();
	~AudioRBResampler();
};

#endif // AUDIO_RB_RESAMPLER_H
