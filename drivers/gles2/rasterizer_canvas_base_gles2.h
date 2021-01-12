/*************************************************************************/
/*  rasterizer_canvas_base_gles2.h                                       */
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

#pragma once

#include "drivers/gles_common/rasterizer_platforms.h"
#ifdef GLES2_BACKEND_ENABLED

#include "drivers/gles_common/rasterizer_array.h"
#include "drivers/gles_common/rasterizer_common_stubs.h"
#include "drivers/gles_common/rasterizer_storage_common.h"
#include "drivers/gles_common/rasterizer_version.h"
#include "rasterizer_scene_gles2.h"
#include "rasterizer_storage_gles2.h"
#include "servers/rendering/renderer_compositor.h"

#include "shaders/canvas.glsl.gen.h"
#include "shaders/canvas_shadow.glsl.gen.h"
#include "shaders/lens_distorted.glsl.gen.h"

class RasterizerCanvasBaseGLES2 : public StubsCanvas {
public:
	enum {
		INSTANCE_ATTRIB_BASE = 8,
	};

	struct Uniforms {
		Transform3D projection_matrix;

		Transform2D modelview_matrix;
		Transform2D extra_matrix;

		Color final_modulate;

		float time;
	};

	struct Data {
		GLuint canvas_quad_vertices;
		GLuint polygon_buffer;
		GLuint polygon_index_buffer;

		uint32_t polygon_buffer_size;
		uint32_t polygon_index_buffer_size;

		GLuint ninepatch_vertices;
		GLuint ninepatch_elements;
	} data;

	struct State {
		Uniforms uniforms;
		bool canvas_texscreen_used;
		CanvasShaderGLES2 canvas_shader;
		CanvasShadowShaderGLES2 canvas_shadow_shader;
		LensDistortedShaderGLES2 lens_shader;

		bool using_texture_rect;

		bool using_light_angle;
		bool using_modulate;
		bool using_large_vertex;

		bool using_ninepatch;
		bool using_skeleton;

		Transform2D skeleton_transform;
		Transform2D skeleton_transform_inverse;
		Size2i skeleton_texture_size;

		RID current_tex;
		RID current_normal;
		RasterizerStorageGLES2::Texture *current_tex_ptr;

		Transform3D vp;
		Light *using_light;
		bool using_shadow;
		bool using_transparent_rt;

		// new for Godot 4.0
		// min mag filter is per item, and repeat
		RS::CanvasItemTextureFilter current_filter = RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR;
		RS::CanvasItemTextureRepeat current_repeat = RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED;
	} state;

	typedef void Texture;

	RasterizerSceneGLES2 *scene_render;

	RasterizerStorageGLES2 *storage;

	// allow user to choose api usage
	GLenum _buffer_upload_usage_flag;

	void _set_uniforms();

	virtual RID light_internal_create();
	virtual void light_internal_update(RID p_rid, Light *p_light);
	virtual void light_internal_free(RID p_rid);

	virtual void canvas_begin();
	virtual void canvas_end();

protected:
	void _legacy_draw_primitive(Item::CommandPrimitive *p_pr, RasterizerStorageGLES2::Material *p_material);
	void _legacy_draw_line(Item::CommandPrimitive *p_pr, RasterizerStorageGLES2::Material *p_material);
	void _legacy_draw_poly_triangles(Item::CommandPolygon *p_poly, RasterizerStorageGLES2::Material *p_material);

public:
	void _draw_gui_primitive(int p_points, const Vector2 *p_vertices, const Color *p_colors, const Vector2 *p_uvs, const float *p_light_angles = nullptr);
	void _draw_polygon(const int *p_indices, int p_index_count, int p_vertex_count, const Vector2 *p_vertices, const Vector2 *p_uvs, const Color *p_colors, bool p_singlecolor, const float *p_weights = NULL, const int *p_bones = NULL);
	void _draw_generic(GLuint p_primitive, int p_vertex_count, const Vector2 *p_vertices, const Vector2 *p_uvs, const Color *p_colors, bool p_singlecolor);
	void _draw_generic_indices(GLuint p_primitive, const int *p_indices, int p_index_count, int p_vertex_count, const Vector2 *p_vertices, const Vector2 *p_uvs, const Color *p_colors, bool p_singlecolor);

	void _bind_quad_buffer();
	void _copy_texscreen(const Rect2 &p_rect);
	void _copy_screen(const Rect2 &p_rect);

	//virtual void draw_window_margins(int *black_margin, RID *black_image) override;
	void draw_generic_textured_rect(const Rect2 &p_rect, const Rect2 &p_src);
	void draw_lens_distortion_rect(const Rect2 &p_rect, float p_k1, float p_k2, const Vector2 &p_eye_center, float p_oversample);

	virtual void reset_canvas();
	virtual void canvas_light_shadow_buffer_update(RID p_buffer, const Transform2D &p_light_xform, int p_light_mask, float p_near, float p_far, LightOccluderInstance *p_occluders, CameraMatrix *p_xform_cache);
	virtual void canvas_debug_viewport_shadows(Light *p_lights_with_shadow) override;

	RasterizerStorageGLES2::Texture *_bind_canvas_texture(const RID &p_texture, const RID &p_normal_map);
	void _set_texture_rect_mode(bool p_texture_rect, bool p_light_angle = false, bool p_modulate = false, bool p_large_vertex = false);

	// NEW API
	struct PolyData {
		LocalVector<int> indices;
		LocalVector<Point2> points;
		LocalVector<Color> colors;
		LocalVector<Point2> uvs;
	};

	RendererCanvasRender::PolygonID request_polygon(const Vector<int> &p_indices, const Vector<Point2> &p_points, const Vector<Color> &p_colors, const Vector<Point2> &p_uvs = Vector<Point2>(), const Vector<int> &p_bones = Vector<int>(), const Vector<float> &p_weights = Vector<float>()) override;
	void free_polygon(PolygonID p_polygon) override;

	RasterizerPooledIndirectList<PolyData> _polydata;

	//////////////////////
	void initialize();
	void finalize();

	RasterizerCanvasBaseGLES2();
};

#endif // GLES2_BACKEND_ENABLED
