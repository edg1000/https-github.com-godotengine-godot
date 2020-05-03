/*************************************************************************/
/*  video_stream_webm.h                                                  */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
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

#ifndef VIDEO_STREAM_WEBM_H
#define VIDEO_STREAM_WEBM_H

#include "core/io/resource_loader.h"
#include "scene/resources/video_stream.h"

class WebMFrame;
class WebMDemuxer;
class VPXDecoder;
class OpusVorbisDecoder;

class VideoStreamPlaybackWebm : public VideoStreamPlayback {

	GDCLASS(VideoStreamPlaybackWebm, VideoStreamPlayback);

	String file_name;
	int audio_track;

	WebMDemuxer *webm;
	VPXDecoder *video;
	OpusVorbisDecoder *audio;

	WebMFrame **video_frames, *audio_frame;
	int video_frames_pos, video_frames_capacity;

	int num_decoded_samples, samples_offset;
	AudioMixCallback mix_callback;
	void *mix_udata;

	bool playing, paused;
	double delay_compensation;
	double time, video_frame_delay, video_pos;

	Vector<uint8_t> frame_data;
	Ref<ImageTexture> texture;

	float *pcm;

public:
	VideoStreamPlaybackWebm();
	~VideoStreamPlaybackWebm();

	bool open_file(const String &p_file);

	virtual void stop() override;
	virtual void play() override;

	virtual bool is_playing() const override;

	virtual void set_paused(bool p_paused) override;
	virtual bool is_paused() const override;

	virtual void set_loop(bool p_enable) override;
	virtual bool has_loop() const override;

	virtual float get_length() const override;

	virtual float get_playback_position() const override;
	virtual void seek(float p_time) override;

	virtual void set_audio_track(int p_idx) override;

	virtual Ref<Texture2D> get_texture() const override;
	virtual void update(float p_delta) override;

	virtual void set_mix_callback(AudioMixCallback p_callback, void *p_userdata) override;
	virtual int get_channels() const override;
	virtual int get_mix_rate() const override;

private:
	inline bool has_enough_video_frames() const;
	bool should_process(WebMFrame &video_frame);

	void delete_pointers();
};

/**/

class VideoStreamWebm : public VideoStream {

	GDCLASS(VideoStreamWebm, VideoStream);

	String file;
	int audio_track;

protected:
	static void _bind_methods();

public:
	VideoStreamWebm();

	virtual Ref<VideoStreamPlayback> instance_playback() override;

	virtual void set_file(const String &p_file);
	String get_file();
	virtual void set_audio_track(int p_track) override;
};

class ResourceFormatLoaderWebm : public ResourceFormatLoader {
public:
	virtual RES load(const String &p_path, const String &p_original_path = "", Error *r_error = nullptr, bool p_use_sub_threads = false, float *r_progress = nullptr, bool p_no_cache = false) override;
	virtual void get_recognized_extensions(List<String> *p_extensions) const override;
	virtual bool handles_type(const String &p_type) const override;
	virtual String get_resource_type(const String &p_path) const override;
};

#endif // VIDEO_STREAM_WEBM_H
