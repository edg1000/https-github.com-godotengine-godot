/**************************************************************************/
/*  render_scene_buffers_gles3.h                                          */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef RENDER_SCENE_BUFFERS_GLES3_H
#define RENDER_SCENE_BUFFERS_GLES3_H

#ifdef GLES3_ENABLED

#include "servers/rendering/storage/render_scene_buffers.h"

#include "platform_gl.h"

class RenderSceneBuffersGLES3 : public RenderSceneBuffers {
	GDCLASS(RenderSceneBuffersGLES3, RenderSceneBuffers);

public:
	Size2i internal_size; // Size of the buffer we render 3D content to.
	Size2i target_size; // Size of our output buffer (render target).
	RS::ViewportScaling3DMode scaling_3d_mode = RS::VIEWPORT_SCALING_3D_MODE_OFF;
	//float fsr_sharpness = 0.2f;
	//RS::ViewportScreenSpaceAA screen_space_aa = RS::VIEWPORT_SCREEN_SPACE_AA_DISABLED;
	//bool use_taa = false;
	//bool use_debanding = false;
	uint32_t view_count = 1;

	RID render_target;

	struct FBDEF {
		GLuint color = 0;
		GLuint depth = 0;
		GLuint fbo = 0;
	};

	struct RTMSAA3D {
		RS::ViewportMSAA mode = RS::VIEWPORT_MSAA_DISABLED;
		bool needs_resolve = false;
		GLsizei samples = 1;
		GLuint color = 0;
		GLuint depth = 0;
		GLuint fbo = 0;

		bool check_fbo_cache = false;
		Vector<FBDEF> cached_fbos;
	} msaa3d; // MSAA buffers used to render 3D

	FBDEF internal3d; // buffers used to either render 3D (scaled/post) or to resolve MSAA into

	FBDEF backbuffer3d; // our back buffer

	// Built-in textures used for ping pong image processing and blurring.
	struct Blur {
		RID texture;

		struct Mipmap {
			RID texture;
			int width;
			int height;
			GLuint fbo;
		};

		Vector<Mipmap> mipmaps;
	};

	Blur blur[2]; //the second one starts from the first mipmap

private:
	void _clear_msaa3d_buffers();
	void _clear_intermediate_buffers();
	void _clear_back_buffers();

	GLuint _rt_get_cached_fbo(GLuint p_color, GLuint p_depth, GLsizei p_samples, uint32_t p_view_count);

public:
	virtual ~RenderSceneBuffersGLES3();
	virtual void configure(const RenderSceneBuffersConfiguration *p_config) override;

	virtual void set_fsr_sharpness(float p_fsr_sharpness) override{};
	virtual void set_texture_mipmap_bias(float p_texture_mipmap_bias) override{};
	virtual void set_use_debanding(bool p_use_debanding) override{};

	void free_render_buffer_data();

	void check_backbuffer(bool p_need_color, bool p_need_depth); // check if we need to initialise our backbuffer

	GLuint get_render_fbo();
	GLuint get_msaa3d_fbo() const { return msaa3d.fbo; }
	GLuint get_msaa3d_color() const { return msaa3d.color; }
	GLuint get_msaa3d_depth() const { return msaa3d.depth; }
	bool get_msaa_needs_resolve() const { return msaa3d.needs_resolve; }
	GLuint get_internal_fbo() const { return internal3d.fbo; }
	GLuint get_internal_color() const { return internal3d.color; }
	GLuint get_internal_depth() const { return internal3d.depth; }
	GLuint get_backbuffer_fbo() const { return backbuffer3d.fbo; }
	GLuint get_backbuffer() const { return backbuffer3d.color; }
	GLuint get_backbuffer_depth() const { return backbuffer3d.depth; }

	// Getters

	_FORCE_INLINE_ RID get_render_target() const { return render_target; }
	_FORCE_INLINE_ uint32_t get_view_count() const { return view_count; }
	_FORCE_INLINE_ Size2i get_internal_size() const { return internal_size; }
	_FORCE_INLINE_ Size2i get_target_size() const { return target_size; }
	_FORCE_INLINE_ RS::ViewportScaling3DMode get_scaling_3d_mode() const { return scaling_3d_mode; }
	//_FORCE_INLINE_ float get_fsr_sharpness() const { return fsr_sharpness; }
	_FORCE_INLINE_ RS::ViewportMSAA get_msaa_3d() const { return msaa3d.mode; }
	//_FORCE_INLINE_ RS::ViewportScreenSpaceAA get_screen_space_aa() const { return screen_space_aa; }
	//_FORCE_INLINE_ bool get_use_taa() const { return use_taa; }
	//_FORCE_INLINE_ bool get_use_debanding() const { return use_debanding; }
};

#endif // GLES3_ENABLED

#endif // RENDER_SCENE_BUFFERS_GLES3_H
