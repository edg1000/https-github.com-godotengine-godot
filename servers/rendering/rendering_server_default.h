/*************************************************************************/
/*  rendering_server_default.h                                           */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
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

#ifndef RENDERING_SERVER_DEFAULT_H
#define RENDERING_SERVER_DEFAULT_H

#include "core/math/octree.h"
#include "core/templates/ordered_hash_map.h"
#include "renderer_canvas_cull.h"
#include "renderer_scene_cull.h"
#include "renderer_viewport.h"
#include "rendering_server_globals.h"
#include "servers/rendering/renderer_compositor.h"
#include "servers/rendering_server.h"

class RenderingServerDefault : public RenderingServer {
	enum {
		MAX_INSTANCE_CULL = 8192,
		MAX_INSTANCE_LIGHTS = 4,
		LIGHT_CACHE_DIRTY = -1,
		MAX_LIGHTS_CULLED = 256,
		MAX_ROOM_CULL = 32,
		MAX_EXTERIOR_PORTALS = 128,
		MAX_LIGHT_SAMPLERS = 256,
		INSTANCE_ROOMLESS_MASK = (1 << 20)

	};

	static int changes;
	RID test_cube;

	int black_margin[4];
	RID black_image[4];

	struct FrameDrawnCallbacks {
		ObjectID object;
		StringName method;
		Variant param;
	};

	List<FrameDrawnCallbacks> frame_drawn_callbacks;

	void _draw_margins();
	static void _changes_changed() {}

	uint64_t frame_profile_frame;
	Vector<FrameProfileArea> frame_profile;

	float frame_setup_time = 0;

	//for printing
	bool print_gpu_profile = false;
	OrderedHashMap<String, float> print_gpu_profile_task_time;
	uint64_t print_frame_profile_ticks_from = 0;
	uint32_t print_frame_profile_frame_count = 0;

public:
	//if editor is redrawing when it shouldn't, enable this and put a breakpoint in _changes_changed()
	//#define DEBUG_CHANGES

#ifdef DEBUG_CHANGES
	_FORCE_INLINE_ static void redraw_request() {
		changes++;
		_changes_changed();
	}

#define DISPLAY_CHANGED \
	changes++;          \
	_changes_changed();

#else
	_FORCE_INLINE_ static void redraw_request() { changes++; }

#define DISPLAY_CHANGED \
	changes++;
#endif

#define BIND0R(m_r, m_name) \
	m_r m_name() { return BINDBASE->m_name(); }
#define BIND0RC(m_r, m_name) \
	m_r m_name() const { return BINDBASE->m_name(); }
#define BIND1R(m_r, m_name, m_type1) \
	m_r m_name(m_type1 arg1) { return BINDBASE->m_name(arg1); }
#define BIND1RC(m_r, m_name, m_type1) \
	m_r m_name(m_type1 arg1) const { return BINDBASE->m_name(arg1); }
#define BIND2R(m_r, m_name, m_type1, m_type2) \
	m_r m_name(m_type1 arg1, m_type2 arg2) { return BINDBASE->m_name(arg1, arg2); }
#define BIND2RC(m_r, m_name, m_type1, m_type2) \
	m_r m_name(m_type1 arg1, m_type2 arg2) const { return BINDBASE->m_name(arg1, arg2); }
#define BIND3R(m_r, m_name, m_type1, m_type2, m_type3) \
	m_r m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3) { return BINDBASE->m_name(arg1, arg2, arg3); }
#define BIND3RC(m_r, m_name, m_type1, m_type2, m_type3) \
	m_r m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3) const { return BINDBASE->m_name(arg1, arg2, arg3); }
#define BIND4R(m_r, m_name, m_type1, m_type2, m_type3, m_type4) \
	m_r m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4) { return BINDBASE->m_name(arg1, arg2, arg3, arg4); }
#define BIND4RC(m_r, m_name, m_type1, m_type2, m_type3, m_type4) \
	m_r m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4) const { return BINDBASE->m_name(arg1, arg2, arg3, arg4); }
#define BIND5R(m_r, m_name, m_type1, m_type2, m_type3, m_type4, m_type5) \
	m_r m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5) { return BINDBASE->m_name(arg1, arg2, arg3, arg4, arg5); }
#define BIND5RC(m_r, m_name, m_type1, m_type2, m_type3, m_type4, m_type5) \
	m_r m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5) const { return BINDBASE->m_name(arg1, arg2, arg3, arg4, arg5); }
#define BIND6R(m_r, m_name, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6) \
	m_r m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5, m_type6 arg6) { return BINDBASE->m_name(arg1, arg2, arg3, arg4, arg5, arg6); }
#define BIND6RC(m_r, m_name, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6) \
	m_r m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5, m_type6 arg6) const { return BINDBASE->m_name(arg1, arg2, arg3, arg4, arg5, arg6); }

#define BIND0(m_name) \
	void m_name() { DISPLAY_CHANGED BINDBASE->m_name(); }
#define BIND1(m_name, m_type1) \
	void m_name(m_type1 arg1) { DISPLAY_CHANGED BINDBASE->m_name(arg1); }
#define BIND1C(m_name, m_type1) \
	void m_name(m_type1 arg1) const { DISPLAY_CHANGED BINDBASE->m_name(arg1); }
#define BIND2(m_name, m_type1, m_type2) \
	void m_name(m_type1 arg1, m_type2 arg2) { DISPLAY_CHANGED BINDBASE->m_name(arg1, arg2); }
#define BIND2C(m_name, m_type1, m_type2) \
	void m_name(m_type1 arg1, m_type2 arg2) const { BINDBASE->m_name(arg1, arg2); }
#define BIND3(m_name, m_type1, m_type2, m_type3) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3) { DISPLAY_CHANGED BINDBASE->m_name(arg1, arg2, arg3); }
#define BIND4(m_name, m_type1, m_type2, m_type3, m_type4) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4) { DISPLAY_CHANGED BINDBASE->m_name(arg1, arg2, arg3, arg4); }
#define BIND5(m_name, m_type1, m_type2, m_type3, m_type4, m_type5) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5) { DISPLAY_CHANGED BINDBASE->m_name(arg1, arg2, arg3, arg4, arg5); }
#define BIND6(m_name, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5, m_type6 arg6) { DISPLAY_CHANGED BINDBASE->m_name(arg1, arg2, arg3, arg4, arg5, arg6); }
#define BIND7(m_name, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6, m_type7) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5, m_type6 arg6, m_type7 arg7) { DISPLAY_CHANGED BINDBASE->m_name(arg1, arg2, arg3, arg4, arg5, arg6, arg7); }
#define BIND8(m_name, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6, m_type7, m_type8) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5, m_type6 arg6, m_type7 arg7, m_type8 arg8) { DISPLAY_CHANGED BINDBASE->m_name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8); }
#define BIND9(m_name, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6, m_type7, m_type8, m_type9) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5, m_type6 arg6, m_type7 arg7, m_type8 arg8, m_type9 arg9) { DISPLAY_CHANGED BINDBASE->m_name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9); }
#define BIND10(m_name, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6, m_type7, m_type8, m_type9, m_type10) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5, m_type6 arg6, m_type7 arg7, m_type8 arg8, m_type9 arg9, m_type10 arg10) { DISPLAY_CHANGED BINDBASE->m_name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10); }
#define BIND11(m_name, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6, m_type7, m_type8, m_type9, m_type10, m_type11) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5, m_type6 arg6, m_type7 arg7, m_type8 arg8, m_type9 arg9, m_type10 arg10, m_type11 arg11) { DISPLAY_CHANGED BINDBASE->m_name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11); }
#define BIND12(m_name, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6, m_type7, m_type8, m_type9, m_type10, m_type11, m_type12) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5, m_type6 arg6, m_type7 arg7, m_type8 arg8, m_type9 arg9, m_type10 arg10, m_type11 arg11, m_type12 arg12) { DISPLAY_CHANGED BINDBASE->m_name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12); }
#define BIND13(m_name, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6, m_type7, m_type8, m_type9, m_type10, m_type11, m_type12, m_type13) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5, m_type6 arg6, m_type7 arg7, m_type8 arg8, m_type9 arg9, m_type10 arg10, m_type11 arg11, m_type12 arg12, m_type13 arg13) { DISPLAY_CHANGED BINDBASE->m_name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13); }
#define BIND14(m_name, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6, m_type7, m_type8, m_type9, m_type10, m_type11, m_type12, m_type13, m_type14) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5, m_type6 arg6, m_type7 arg7, m_type8 arg8, m_type9 arg9, m_type10 arg10, m_type11 arg11, m_type12 arg12, m_type13 arg13, m_type14 arg14) { DISPLAY_CHANGED BINDBASE->m_name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14); }
#define BIND15(m_name, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6, m_type7, m_type8, m_type9, m_type10, m_type11, m_type12, m_type13, m_type14, m_type15) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5, m_type6 arg6, m_type7 arg7, m_type8 arg8, m_type9 arg9, m_type10 arg10, m_type11 arg11, m_type12 arg12, m_type13 arg13, m_type14 arg14, m_type15 arg15) { DISPLAY_CHANGED BINDBASE->m_name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15); }

//from now on, calls forwarded to this singleton
#define BINDBASE RSG::storage

	/* TEXTURE API */

	//these go pass-through, as they can be called from any thread
	BIND1R(RID, texture_2d_create, const Ref<Image> &)
	BIND2R(RID, texture_2d_layered_create, const Vector<Ref<Image>> &, TextureLayeredType)
	BIND6R(RID, texture_3d_create, Image::Format, int, int, int, bool, const Vector<Ref<Image>> &)
	BIND1R(RID, texture_proxy_create, RID)

	//goes pass-through
	BIND3(texture_2d_update_immediate, RID, const Ref<Image> &, int)
	//these go through command queue if they are in another thread
	BIND3(texture_2d_update, RID, const Ref<Image> &, int)
	BIND2(texture_3d_update, RID, const Vector<Ref<Image>> &)
	BIND2(texture_proxy_update, RID, RID)

	//these also go pass-through
	BIND0R(RID, texture_2d_placeholder_create)
	BIND1R(RID, texture_2d_layered_placeholder_create, TextureLayeredType)
	BIND0R(RID, texture_3d_placeholder_create)

	BIND1RC(Ref<Image>, texture_2d_get, RID)
	BIND2RC(Ref<Image>, texture_2d_layer_get, RID, int)
	BIND1RC(Vector<Ref<Image>>, texture_3d_get, RID)

	BIND2(texture_replace, RID, RID)

	BIND3(texture_set_size_override, RID, int, int)
// FIXME: Disabled during Vulkan refactoring, should be ported.
#if 0
	BIND2(texture_bind, RID, uint32_t)
#endif

	BIND3(texture_set_detect_3d_callback, RID, TextureDetectCallback, void *)
	BIND3(texture_set_detect_normal_callback, RID, TextureDetectCallback, void *)
	BIND3(texture_set_detect_roughness_callback, RID, TextureDetectRoughnessCallback, void *)

	BIND2(texture_set_path, RID, const String &)
	BIND1RC(String, texture_get_path, RID)
	BIND1(texture_debug_usage, List<TextureInfo> *)

	BIND2(texture_set_force_redraw_if_visible, RID, bool)

	/* SHADER API */

	BIND0R(RID, shader_create)

	BIND2(shader_set_code, RID, const String &)
	BIND1RC(String, shader_get_code, RID)

	BIND2C(shader_get_param_list, RID, List<PropertyInfo> *)

	BIND3(shader_set_default_texture_param, RID, const StringName &, RID)
	BIND2RC(RID, shader_get_default_texture_param, RID, const StringName &)
	BIND2RC(Variant, shader_get_param_default, RID, const StringName &)

	BIND1RC(ShaderNativeSourceCode, shader_get_native_source_code, RID)

	/* COMMON MATERIAL API */

	BIND0R(RID, material_create)

	BIND2(material_set_shader, RID, RID)

	BIND3(material_set_param, RID, const StringName &, const Variant &)
	BIND2RC(Variant, material_get_param, RID, const StringName &)

	BIND2(material_set_render_priority, RID, int)
	BIND2(material_set_next_pass, RID, RID)

	/* MESH API */

	virtual RID mesh_create_from_surfaces(const Vector<SurfaceData> &p_surfaces, int p_blend_shape_count = 0) {
		RID mesh = mesh_create();
		mesh_set_blend_shape_count(mesh, p_blend_shape_count);
		for (int i = 0; i < p_surfaces.size(); i++) {
			mesh_add_surface(mesh, p_surfaces[i]);
		}
		return mesh;
	}

	BIND2(mesh_set_blend_shape_count, RID, int)

	BIND0R(RID, mesh_create)

	BIND2(mesh_add_surface, RID, const SurfaceData &)

	BIND1RC(int, mesh_get_blend_shape_count, RID)

	BIND2(mesh_set_blend_shape_mode, RID, BlendShapeMode)
	BIND1RC(BlendShapeMode, mesh_get_blend_shape_mode, RID)

	BIND4(mesh_surface_update_region, RID, int, int, const Vector<uint8_t> &)

	BIND3(mesh_surface_set_material, RID, int, RID)
	BIND2RC(RID, mesh_surface_get_material, RID, int)

	BIND2RC(SurfaceData, mesh_get_surface, RID, int)

	BIND1RC(int, mesh_get_surface_count, RID)

	BIND2(mesh_set_custom_aabb, RID, const AABB &)
	BIND1RC(AABB, mesh_get_custom_aabb, RID)

	BIND2(mesh_set_shadow_mesh, RID, RID)

	BIND1(mesh_clear, RID)

	/* MULTIMESH API */

	BIND0R(RID, multimesh_create)

	BIND5(multimesh_allocate, RID, int, MultimeshTransformFormat, bool, bool)
	BIND1RC(int, multimesh_get_instance_count, RID)

	BIND2(multimesh_set_mesh, RID, RID)
	BIND3(multimesh_instance_set_transform, RID, int, const Transform &)
	BIND3(multimesh_instance_set_transform_2d, RID, int, const Transform2D &)
	BIND3(multimesh_instance_set_color, RID, int, const Color &)
	BIND3(multimesh_instance_set_custom_data, RID, int, const Color &)

	BIND1RC(RID, multimesh_get_mesh, RID)
	BIND1RC(AABB, multimesh_get_aabb, RID)

	BIND2RC(Transform, multimesh_instance_get_transform, RID, int)
	BIND2RC(Transform2D, multimesh_instance_get_transform_2d, RID, int)
	BIND2RC(Color, multimesh_instance_get_color, RID, int)
	BIND2RC(Color, multimesh_instance_get_custom_data, RID, int)

	BIND2(multimesh_set_buffer, RID, const Vector<float> &)
	BIND1RC(Vector<float>, multimesh_get_buffer, RID)

	BIND2(multimesh_set_visible_instances, RID, int)
	BIND1RC(int, multimesh_get_visible_instances, RID)

	/* IMMEDIATE API */

	BIND0R(RID, immediate_create)
	BIND3(immediate_begin, RID, PrimitiveType, RID)
	BIND2(immediate_vertex, RID, const Vector3 &)
	BIND2(immediate_normal, RID, const Vector3 &)
	BIND2(immediate_tangent, RID, const Plane &)
	BIND2(immediate_color, RID, const Color &)
	BIND2(immediate_uv, RID, const Vector2 &)
	BIND2(immediate_uv2, RID, const Vector2 &)
	BIND1(immediate_end, RID)
	BIND1(immediate_clear, RID)
	BIND2(immediate_set_material, RID, RID)
	BIND1RC(RID, immediate_get_material, RID)

	/* SKELETON API */

	BIND0R(RID, skeleton_create)
	BIND3(skeleton_allocate, RID, int, bool)
	BIND1RC(int, skeleton_get_bone_count, RID)
	BIND3(skeleton_bone_set_transform, RID, int, const Transform &)
	BIND2RC(Transform, skeleton_bone_get_transform, RID, int)
	BIND3(skeleton_bone_set_transform_2d, RID, int, const Transform2D &)
	BIND2RC(Transform2D, skeleton_bone_get_transform_2d, RID, int)
	BIND2(skeleton_set_base_transform_2d, RID, const Transform2D &)

	/* Light API */

	BIND0R(RID, directional_light_create)
	BIND0R(RID, omni_light_create)
	BIND0R(RID, spot_light_create)

	BIND2(light_set_color, RID, const Color &)
	BIND3(light_set_param, RID, LightParam, float)
	BIND2(light_set_shadow, RID, bool)
	BIND2(light_set_shadow_color, RID, const Color &)
	BIND2(light_set_projector, RID, RID)
	BIND2(light_set_negative, RID, bool)
	BIND2(light_set_cull_mask, RID, uint32_t)
	BIND2(light_set_reverse_cull_face_mode, RID, bool)
	BIND2(light_set_bake_mode, RID, LightBakeMode)
	BIND2(light_set_max_sdfgi_cascade, RID, uint32_t)

	BIND2(light_omni_set_shadow_mode, RID, LightOmniShadowMode)

	BIND2(light_directional_set_shadow_mode, RID, LightDirectionalShadowMode)
	BIND2(light_directional_set_blend_splits, RID, bool)
	BIND2(light_directional_set_sky_only, RID, bool)
	BIND2(light_directional_set_shadow_depth_range_mode, RID, LightDirectionalShadowDepthRangeMode)

	/* PROBE API */

	BIND0R(RID, reflection_probe_create)

	BIND2(reflection_probe_set_update_mode, RID, ReflectionProbeUpdateMode)
	BIND2(reflection_probe_set_intensity, RID, float)
	BIND2(reflection_probe_set_ambient_color, RID, const Color &)
	BIND2(reflection_probe_set_ambient_energy, RID, float)
	BIND2(reflection_probe_set_ambient_mode, RID, ReflectionProbeAmbientMode)
	BIND2(reflection_probe_set_max_distance, RID, float)
	BIND2(reflection_probe_set_extents, RID, const Vector3 &)
	BIND2(reflection_probe_set_origin_offset, RID, const Vector3 &)
	BIND2(reflection_probe_set_as_interior, RID, bool)
	BIND2(reflection_probe_set_enable_box_projection, RID, bool)
	BIND2(reflection_probe_set_enable_shadows, RID, bool)
	BIND2(reflection_probe_set_cull_mask, RID, uint32_t)
	BIND2(reflection_probe_set_resolution, RID, int)
	BIND2(reflection_probe_set_lod_threshold, RID, float)

	/* DECAL API */

	BIND0R(RID, decal_create)

	BIND2(decal_set_extents, RID, const Vector3 &)
	BIND3(decal_set_texture, RID, DecalTexture, RID)
	BIND2(decal_set_emission_energy, RID, float)
	BIND2(decal_set_albedo_mix, RID, float)
	BIND2(decal_set_modulate, RID, const Color &)
	BIND2(decal_set_cull_mask, RID, uint32_t)
	BIND4(decal_set_distance_fade, RID, bool, float, float)
	BIND3(decal_set_fade, RID, float, float)
	BIND2(decal_set_normal_fade, RID, float)

	/* BAKED LIGHT API */

	BIND0R(RID, gi_probe_create)

	BIND8(gi_probe_allocate, RID, const Transform &, const AABB &, const Vector3i &, const Vector<uint8_t> &, const Vector<uint8_t> &, const Vector<uint8_t> &, const Vector<int> &)

	BIND1RC(AABB, gi_probe_get_bounds, RID)
	BIND1RC(Vector3i, gi_probe_get_octree_size, RID)
	BIND1RC(Vector<uint8_t>, gi_probe_get_octree_cells, RID)
	BIND1RC(Vector<uint8_t>, gi_probe_get_data_cells, RID)
	BIND1RC(Vector<uint8_t>, gi_probe_get_distance_field, RID)
	BIND1RC(Vector<int>, gi_probe_get_level_counts, RID)
	BIND1RC(Transform, gi_probe_get_to_cell_xform, RID)

	BIND2(gi_probe_set_dynamic_range, RID, float)
	BIND1RC(float, gi_probe_get_dynamic_range, RID)

	BIND2(gi_probe_set_propagation, RID, float)
	BIND1RC(float, gi_probe_get_propagation, RID)

	BIND2(gi_probe_set_energy, RID, float)
	BIND1RC(float, gi_probe_get_energy, RID)

	BIND2(gi_probe_set_ao, RID, float)
	BIND1RC(float, gi_probe_get_ao, RID)

	BIND2(gi_probe_set_ao_size, RID, float)
	BIND1RC(float, gi_probe_get_ao_size, RID)

	BIND2(gi_probe_set_bias, RID, float)
	BIND1RC(float, gi_probe_get_bias, RID)

	BIND2(gi_probe_set_normal_bias, RID, float)
	BIND1RC(float, gi_probe_get_normal_bias, RID)

	BIND2(gi_probe_set_interior, RID, bool)
	BIND1RC(bool, gi_probe_is_interior, RID)

	BIND2(gi_probe_set_use_two_bounces, RID, bool)
	BIND1RC(bool, gi_probe_is_using_two_bounces, RID)

	BIND2(gi_probe_set_anisotropy_strength, RID, float)
	BIND1RC(float, gi_probe_get_anisotropy_strength, RID)

	/* LIGHTMAP */

	BIND0R(RID, lightmap_create)

	BIND3(lightmap_set_textures, RID, RID, bool)
	BIND2(lightmap_set_probe_bounds, RID, const AABB &)
	BIND2(lightmap_set_probe_interior, RID, bool)
	BIND5(lightmap_set_probe_capture_data, RID, const PackedVector3Array &, const PackedColorArray &, const PackedInt32Array &, const PackedInt32Array &)
	BIND1RC(PackedVector3Array, lightmap_get_probe_capture_points, RID)
	BIND1RC(PackedColorArray, lightmap_get_probe_capture_sh, RID)
	BIND1RC(PackedInt32Array, lightmap_get_probe_capture_tetrahedra, RID)
	BIND1RC(PackedInt32Array, lightmap_get_probe_capture_bsp_tree, RID)
	BIND1(lightmap_set_probe_capture_update_speed, float)

	/* PARTICLES */

	BIND0R(RID, particles_create)

	BIND2(particles_set_emitting, RID, bool)
	BIND1R(bool, particles_get_emitting, RID)
	BIND2(particles_set_amount, RID, int)
	BIND2(particles_set_lifetime, RID, float)
	BIND2(particles_set_one_shot, RID, bool)
	BIND2(particles_set_pre_process_time, RID, float)
	BIND2(particles_set_explosiveness_ratio, RID, float)
	BIND2(particles_set_randomness_ratio, RID, float)
	BIND2(particles_set_custom_aabb, RID, const AABB &)
	BIND2(particles_set_speed_scale, RID, float)
	BIND2(particles_set_use_local_coordinates, RID, bool)
	BIND2(particles_set_process_material, RID, RID)
	BIND2(particles_set_fixed_fps, RID, int)
	BIND2(particles_set_fractional_delta, RID, bool)
	BIND1R(bool, particles_is_inactive, RID)
	BIND1(particles_request_process, RID)
	BIND1(particles_restart, RID)
	BIND6(particles_emit, RID, const Transform &, const Vector3 &, const Color &, const Color &, uint32_t)
	BIND2(particles_set_subemitter, RID, RID)
	BIND2(particles_set_collision_base_size, RID, float)

	BIND2(particles_set_draw_order, RID, RS::ParticlesDrawOrder)

	BIND2(particles_set_draw_passes, RID, int)
	BIND3(particles_set_draw_pass_mesh, RID, int, RID)

	BIND1R(AABB, particles_get_current_aabb, RID)
	BIND2(particles_set_emission_transform, RID, const Transform &)

	/* PARTICLES COLLISION */

	BIND0R(RID, particles_collision_create)

	BIND2(particles_collision_set_collision_type, RID, ParticlesCollisionType)
	BIND2(particles_collision_set_cull_mask, RID, uint32_t)
	BIND2(particles_collision_set_sphere_radius, RID, float)
	BIND2(particles_collision_set_box_extents, RID, const Vector3 &)
	BIND2(particles_collision_set_attractor_strength, RID, float)
	BIND2(particles_collision_set_attractor_directionality, RID, float)
	BIND2(particles_collision_set_attractor_attenuation, RID, float)
	BIND2(particles_collision_set_field_texture, RID, RID)
	BIND1(particles_collision_height_field_update, RID)
	BIND2(particles_collision_set_height_field_resolution, RID, ParticlesCollisionHeightfieldResolution)

#undef BINDBASE
//from now on, calls forwarded to this singleton
#define BINDBASE RSG::scene

	/* CAMERA API */

	BIND0R(RID, camera_create)
	BIND4(camera_set_perspective, RID, float, float, float)
	BIND4(camera_set_orthogonal, RID, float, float, float)
	BIND5(camera_set_frustum, RID, float, Vector2, float, float)
	BIND2(camera_set_transform, RID, const Transform &)
	BIND2(camera_set_cull_mask, RID, uint32_t)
	BIND2(camera_set_environment, RID, RID)
	BIND2(camera_set_camera_effects, RID, RID)
	BIND2(camera_set_use_vertical_aspect, RID, bool)

#undef BINDBASE
//from now on, calls forwarded to this singleton
#define BINDBASE RSG::viewport

	/* VIEWPORT TARGET API */

	BIND0R(RID, viewport_create)

	BIND2(viewport_set_use_xr, RID, bool)
	BIND3(viewport_set_size, RID, int, int)

	BIND2(viewport_set_active, RID, bool)
	BIND2(viewport_set_parent_viewport, RID, RID)

	BIND2(viewport_set_clear_mode, RID, ViewportClearMode)

	BIND3(viewport_attach_to_screen, RID, const Rect2 &, int)
	BIND2(viewport_set_render_direct_to_screen, RID, bool)

	BIND2(viewport_set_update_mode, RID, ViewportUpdateMode)
	BIND2(viewport_set_vflip, RID, bool)

	BIND1RC(RID, viewport_get_texture, RID)

	BIND2(viewport_set_hide_scenario, RID, bool)
	BIND2(viewport_set_hide_canvas, RID, bool)
	BIND2(viewport_set_disable_environment, RID, bool)

	BIND2(viewport_attach_camera, RID, RID)
	BIND2(viewport_set_scenario, RID, RID)
	BIND2(viewport_attach_canvas, RID, RID)

	BIND2(viewport_remove_canvas, RID, RID)
	BIND3(viewport_set_canvas_transform, RID, RID, const Transform2D &)
	BIND2(viewport_set_transparent_background, RID, bool)
	BIND2(viewport_set_snap_2d_transforms_to_pixel, RID, bool)
	BIND2(viewport_set_snap_2d_vertices_to_pixel, RID, bool)

	BIND2(viewport_set_default_canvas_item_texture_filter, RID, CanvasItemTextureFilter)
	BIND2(viewport_set_default_canvas_item_texture_repeat, RID, CanvasItemTextureRepeat)

	BIND2(viewport_set_global_canvas_transform, RID, const Transform2D &)
	BIND4(viewport_set_canvas_stacking, RID, RID, int, int)
	BIND3(viewport_set_shadow_atlas_size, RID, int, bool)
	BIND3(viewport_set_sdf_oversize_and_scale, RID, ViewportSDFOversize, ViewportSDFScale)
	BIND3(viewport_set_shadow_atlas_quadrant_subdivision, RID, int, int)
	BIND2(viewport_set_msaa, RID, ViewportMSAA)
	BIND2(viewport_set_screen_space_aa, RID, ViewportScreenSpaceAA)
	BIND2(viewport_set_use_debanding, RID, bool)
	BIND2(viewport_set_lod_threshold, RID, float)

	BIND2R(int, viewport_get_render_info, RID, ViewportRenderInfo)
	BIND2(viewport_set_debug_draw, RID, ViewportDebugDraw)

	BIND2(viewport_set_measure_render_time, RID, bool)
	BIND1RC(float, viewport_get_measured_render_time_cpu, RID)
	BIND1RC(float, viewport_get_measured_render_time_gpu, RID)

	/* ENVIRONMENT API */

#undef BINDBASE
//from now on, calls forwarded to this singleton
#define BINDBASE RSG::scene

	BIND2(directional_shadow_atlas_set_size, int, bool)
	BIND1(gi_probe_set_quality, GIProbeQuality)

	/* SKY API */

	BIND0R(RID, sky_create)
	BIND2(sky_set_radiance_size, RID, int)
	BIND2(sky_set_mode, RID, SkyMode)
	BIND2(sky_set_material, RID, RID)
	BIND4R(Ref<Image>, sky_bake_panorama, RID, float, bool, const Size2i &)

	BIND0R(RID, environment_create)

	BIND2(environment_set_background, RID, EnvironmentBG)
	BIND2(environment_set_sky, RID, RID)
	BIND2(environment_set_sky_custom_fov, RID, float)
	BIND2(environment_set_sky_orientation, RID, const Basis &)
	BIND2(environment_set_bg_color, RID, const Color &)
	BIND2(environment_set_bg_energy, RID, float)
	BIND2(environment_set_canvas_max_layer, RID, int)
	BIND7(environment_set_ambient_light, RID, const Color &, EnvironmentAmbientSource, float, float, EnvironmentReflectionSource, const Color &)

// FIXME: Disabled during Vulkan refactoring, should be ported.
#if 0
	BIND2(environment_set_camera_feed_id, RID, int)
#endif
	BIND6(environment_set_ssr, RID, bool, int, float, float, float)
	BIND1(environment_set_ssr_roughness_quality, EnvironmentSSRRoughnessQuality)

	BIND10(environment_set_ssao, RID, bool, float, float, float, float, float, float, float, float)
	BIND6(environment_set_ssao_quality, EnvironmentSSAOQuality, bool, float, int, float, float)

	BIND11(environment_set_glow, RID, bool, Vector<float>, float, float, float, float, EnvironmentGlowBlendMode, float, float, float)
	BIND1(environment_glow_set_use_bicubic_upscale, bool)
	BIND1(environment_glow_set_use_high_quality, bool)

	BIND9(environment_set_tonemap, RID, EnvironmentToneMapper, float, float, bool, float, float, float, float)

	BIND7(environment_set_adjustment, RID, bool, float, float, float, bool, RID)

	BIND9(environment_set_fog, RID, bool, const Color &, float, float, float, float, float, float)
	BIND11(environment_set_volumetric_fog, RID, bool, float, const Color &, float, float, float, float, EnvVolumetricFogShadowFilter, bool, float)

	BIND2(environment_set_volumetric_fog_volume_size, int, int)
	BIND1(environment_set_volumetric_fog_filter_active, bool)
	BIND1(environment_set_volumetric_fog_directional_shadow_shrink_size, int)
	BIND1(environment_set_volumetric_fog_positional_shadow_shrink_size, int)

	BIND11(environment_set_sdfgi, RID, bool, EnvironmentSDFGICascades, float, EnvironmentSDFGIYScale, bool, bool, bool, float, float, float)
	BIND1(environment_set_sdfgi_ray_count, EnvironmentSDFGIRayCount)
	BIND1(environment_set_sdfgi_frames_to_converge, EnvironmentSDFGIFramesToConverge)
	BIND1(environment_set_sdfgi_frames_to_update_light, EnvironmentSDFGIFramesToUpdateLight)

	BIND3R(Ref<Image>, environment_bake_panorama, RID, bool, const Size2i &)

	BIND3(screen_space_roughness_limiter_set_active, bool, float, float)
	BIND1(sub_surface_scattering_set_quality, SubSurfaceScatteringQuality)
	BIND2(sub_surface_scattering_set_scale, float, float)

	/* CAMERA EFFECTS */

	BIND0R(RID, camera_effects_create)

	BIND2(camera_effects_set_dof_blur_quality, DOFBlurQuality, bool)
	BIND1(camera_effects_set_dof_blur_bokeh_shape, DOFBokehShape)

	BIND8(camera_effects_set_dof_blur, RID, bool, float, float, bool, float, float, float)
	BIND3(camera_effects_set_custom_exposure, RID, bool, float)

	BIND1(shadows_quality_set, ShadowQuality);
	BIND1(directional_shadow_quality_set, ShadowQuality);

	/* SCENARIO API */

#undef BINDBASE
#define BINDBASE RSG::scene

	BIND0R(RID, scenario_create)

	BIND2(scenario_set_debug, RID, ScenarioDebugMode)
	BIND2(scenario_set_environment, RID, RID)
	BIND2(scenario_set_camera_effects, RID, RID)
	BIND2(scenario_set_fallback_environment, RID, RID)

	/* INSTANCING API */
	BIND0R(RID, instance_create)

	BIND2(instance_set_base, RID, RID)
	BIND2(instance_set_scenario, RID, RID)
	BIND2(instance_set_layer_mask, RID, uint32_t)
	BIND2(instance_set_transform, RID, const Transform &)
	BIND2(instance_attach_object_instance_id, RID, ObjectID)
	BIND3(instance_set_blend_shape_weight, RID, int, float)
	BIND3(instance_set_surface_material, RID, int, RID)
	BIND2(instance_set_visible, RID, bool)

	BIND2(instance_set_custom_aabb, RID, AABB)

	BIND2(instance_attach_skeleton, RID, RID)
	BIND2(instance_set_exterior, RID, bool)

	BIND2(instance_set_extra_visibility_margin, RID, real_t)

	// don't use these in a game!
	BIND2RC(Vector<ObjectID>, instances_cull_aabb, const AABB &, RID)
	BIND3RC(Vector<ObjectID>, instances_cull_ray, const Vector3 &, const Vector3 &, RID)
	BIND2RC(Vector<ObjectID>, instances_cull_convex, const Vector<Plane> &, RID)

	BIND3(instance_geometry_set_flag, RID, InstanceFlags, bool)
	BIND2(instance_geometry_set_cast_shadows_setting, RID, ShadowCastingSetting)
	BIND2(instance_geometry_set_material_override, RID, RID)

	BIND5(instance_geometry_set_draw_range, RID, float, float, float, float)
	BIND2(instance_geometry_set_as_instance_lod, RID, RID)
	BIND4(instance_geometry_set_lightmap, RID, RID, const Rect2 &, int)
	BIND2(instance_geometry_set_lod_bias, RID, float)

	BIND3(instance_geometry_set_shader_parameter, RID, const StringName &, const Variant &)
	BIND2RC(Variant, instance_geometry_get_shader_parameter, RID, const StringName &)
	BIND2RC(Variant, instance_geometry_get_shader_parameter_default_value, RID, const StringName &)
	BIND2C(instance_geometry_get_shader_parameter_list, RID, List<PropertyInfo> *)

	BIND3R(TypedArray<Image>, bake_render_uv2, RID, const Vector<RID> &, const Size2i &)

	BIND1(gi_set_use_half_resolution, bool)

#undef BINDBASE
//from now on, calls forwarded to this singleton
#define BINDBASE RSG::canvas

	/* CANVAS (2D) */

	BIND0R(RID, canvas_create)
	BIND3(canvas_set_item_mirroring, RID, RID, const Point2 &)
	BIND2(canvas_set_modulate, RID, const Color &)
	BIND3(canvas_set_parent, RID, RID, float)
	BIND1(canvas_set_disable_scale, bool)

	BIND0R(RID, canvas_texture_create)
	BIND3(canvas_texture_set_channel, RID, CanvasTextureChannel, RID)
	BIND3(canvas_texture_set_shading_parameters, RID, const Color &, float)

	BIND2(canvas_texture_set_texture_filter, RID, CanvasItemTextureFilter)
	BIND2(canvas_texture_set_texture_repeat, RID, CanvasItemTextureRepeat)

	BIND0R(RID, canvas_item_create)
	BIND2(canvas_item_set_parent, RID, RID)

	BIND2(canvas_item_set_default_texture_filter, RID, CanvasItemTextureFilter)
	BIND2(canvas_item_set_default_texture_repeat, RID, CanvasItemTextureRepeat)

	BIND2(canvas_item_set_visible, RID, bool)
	BIND2(canvas_item_set_light_mask, RID, int)

	BIND2(canvas_item_set_update_when_visible, RID, bool)

	BIND2(canvas_item_set_transform, RID, const Transform2D &)
	BIND2(canvas_item_set_clip, RID, bool)
	BIND2(canvas_item_set_distance_field_mode, RID, bool)
	BIND3(canvas_item_set_custom_rect, RID, bool, const Rect2 &)
	BIND2(canvas_item_set_modulate, RID, const Color &)
	BIND2(canvas_item_set_self_modulate, RID, const Color &)

	BIND2(canvas_item_set_draw_behind_parent, RID, bool)

	BIND5(canvas_item_add_line, RID, const Point2 &, const Point2 &, const Color &, float)
	BIND5(canvas_item_add_polyline, RID, const Vector<Point2> &, const Vector<Color> &, float, bool)
	BIND4(canvas_item_add_multiline, RID, const Vector<Point2> &, const Vector<Color> &, float)
	BIND3(canvas_item_add_rect, RID, const Rect2 &, const Color &)
	BIND4(canvas_item_add_circle, RID, const Point2 &, float, const Color &)
	BIND6(canvas_item_add_texture_rect, RID, const Rect2 &, RID, bool, const Color &, bool)
	BIND7(canvas_item_add_texture_rect_region, RID, const Rect2 &, RID, const Rect2 &, const Color &, bool, bool)
	BIND10(canvas_item_add_nine_patch, RID, const Rect2 &, const Rect2 &, RID, const Vector2 &, const Vector2 &, NinePatchAxisMode, NinePatchAxisMode, bool, const Color &)
	BIND6(canvas_item_add_primitive, RID, const Vector<Point2> &, const Vector<Color> &, const Vector<Point2> &, RID, float)
	BIND5(canvas_item_add_polygon, RID, const Vector<Point2> &, const Vector<Color> &, const Vector<Point2> &, RID)
	BIND9(canvas_item_add_triangle_array, RID, const Vector<int> &, const Vector<Point2> &, const Vector<Color> &, const Vector<Point2> &, const Vector<int> &, const Vector<float> &, RID, int)
	BIND5(canvas_item_add_mesh, RID, const RID &, const Transform2D &, const Color &, RID)
	BIND3(canvas_item_add_multimesh, RID, RID, RID)
	BIND3(canvas_item_add_particles, RID, RID, RID)
	BIND2(canvas_item_add_set_transform, RID, const Transform2D &)
	BIND2(canvas_item_add_clip_ignore, RID, bool)
	BIND2(canvas_item_set_sort_children_by_y, RID, bool)
	BIND2(canvas_item_set_z_index, RID, int)
	BIND2(canvas_item_set_z_as_relative_to_parent, RID, bool)
	BIND3(canvas_item_set_copy_to_backbuffer, RID, bool, const Rect2 &)
	BIND2(canvas_item_attach_skeleton, RID, RID)

	BIND1(canvas_item_clear, RID)
	BIND2(canvas_item_set_draw_index, RID, int)

	BIND2(canvas_item_set_material, RID, RID)

	BIND2(canvas_item_set_use_parent_material, RID, bool)

	BIND6(canvas_item_set_canvas_group_mode, RID, CanvasGroupMode, float, bool, float, bool)

	BIND0R(RID, canvas_light_create)

	BIND2(canvas_light_set_mode, RID, CanvasLightMode)

	BIND2(canvas_light_attach_to_canvas, RID, RID)
	BIND2(canvas_light_set_enabled, RID, bool)
	BIND2(canvas_light_set_texture_scale, RID, float)
	BIND2(canvas_light_set_transform, RID, const Transform2D &)
	BIND2(canvas_light_set_texture, RID, RID)
	BIND2(canvas_light_set_texture_offset, RID, const Vector2 &)
	BIND2(canvas_light_set_color, RID, const Color &)
	BIND2(canvas_light_set_height, RID, float)
	BIND2(canvas_light_set_energy, RID, float)
	BIND3(canvas_light_set_z_range, RID, int, int)
	BIND3(canvas_light_set_layer_range, RID, int, int)
	BIND2(canvas_light_set_item_cull_mask, RID, int)
	BIND2(canvas_light_set_item_shadow_cull_mask, RID, int)
	BIND2(canvas_light_set_directional_distance, RID, float)

	BIND2(canvas_light_set_blend_mode, RID, CanvasLightBlendMode)

	BIND2(canvas_light_set_shadow_enabled, RID, bool)
	BIND2(canvas_light_set_shadow_filter, RID, CanvasLightShadowFilter)
	BIND2(canvas_light_set_shadow_color, RID, const Color &)
	BIND2(canvas_light_set_shadow_smooth, RID, float)

	BIND0R(RID, canvas_light_occluder_create)
	BIND2(canvas_light_occluder_attach_to_canvas, RID, RID)
	BIND2(canvas_light_occluder_set_enabled, RID, bool)
	BIND2(canvas_light_occluder_set_polygon, RID, RID)
	BIND2(canvas_light_occluder_set_as_sdf_collision, RID, bool)
	BIND2(canvas_light_occluder_set_transform, RID, const Transform2D &)
	BIND2(canvas_light_occluder_set_light_mask, RID, int)

	BIND0R(RID, canvas_occluder_polygon_create)
	BIND3(canvas_occluder_polygon_set_shape, RID, const Vector<Vector2> &, bool)

	BIND2(canvas_occluder_polygon_set_cull_mode, RID, CanvasOccluderPolygonCullMode)

	BIND1(canvas_set_shadow_texture_size, int)

	/* GLOBAL VARIABLES */

#undef BINDBASE
//from now on, calls forwarded to this singleton
#define BINDBASE RSG::storage

	BIND3(global_variable_add, const StringName &, GlobalVariableType, const Variant &)
	BIND1(global_variable_remove, const StringName &)
	BIND0RC(Vector<StringName>, global_variable_get_list)
	BIND2(global_variable_set, const StringName &, const Variant &)
	BIND2(global_variable_set_override, const StringName &, const Variant &)
	BIND1RC(GlobalVariableType, global_variable_get_type, const StringName &)
	BIND1RC(Variant, global_variable_get, const StringName &)

	BIND1(global_variables_load_settings, bool)
	BIND0(global_variables_clear)

	/* BLACK BARS */

	virtual void black_bars_set_margins(int p_left, int p_top, int p_right, int p_bottom);
	virtual void black_bars_set_images(RID p_left, RID p_top, RID p_right, RID p_bottom);

	/* FREE */

	virtual void free(RID p_rid); ///< free RIDs associated with the visual server

	/* EVENT QUEUING */

	virtual void request_frame_drawn_callback(Object *p_where, const StringName &p_method, const Variant &p_userdata);

	virtual void draw(bool p_swap_buffers, double frame_step);
	virtual void sync();
	virtual bool has_changed() const;
	virtual void init();
	virtual void finish();

	/* STATUS INFORMATION */

	virtual int get_render_info(RenderInfo p_info);
	virtual String get_video_adapter_name() const;
	virtual String get_video_adapter_vendor() const;

	virtual void set_frame_profiling_enabled(bool p_enable);
	virtual Vector<FrameProfileArea> get_frame_profile();
	virtual uint64_t get_frame_profile_frame();

	virtual RID get_test_cube();

	/* TESTING */

	virtual float get_frame_setup_time_cpu() const;

	virtual void set_boot_image(const Ref<Image> &p_image, const Color &p_color, bool p_scale, bool p_use_filter = true);
	virtual void set_default_clear_color(const Color &p_color);

	virtual bool has_feature(Features p_feature) const;

	virtual bool has_os_feature(const String &p_feature) const;
	virtual void set_debug_generate_wireframes(bool p_generate);

	virtual void call_set_use_vsync(bool p_enable);

	virtual bool is_low_end() const;

	virtual void sdfgi_set_debug_probe_select(const Vector3 &p_position, const Vector3 &p_dir);

	virtual void set_print_gpu_profile(bool p_enable);

	RenderingServerDefault();
	~RenderingServerDefault();

#undef DISPLAY_CHANGED

#undef BIND0R
#undef BIND1RC
#undef BIND2RC
#undef BIND3RC
#undef BIND4RC

#undef BIND1
#undef BIND2
#undef BIND3
#undef BIND4
#undef BIND5
#undef BIND6
#undef BIND7
#undef BIND8
#undef BIND9
#undef BIND10
};

#endif
