/**************************************************************************/
/*  gi.cpp                                                                */
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

#include "gi.h"

#include "core/config/project_settings.h"
#include "servers/rendering/renderer_rd/renderer_compositor_rd.h"
#include "servers/rendering/renderer_rd/renderer_scene_render_rd.h"
#include "servers/rendering/renderer_rd/storage_rd/material_storage.h"
#include "servers/rendering/renderer_rd/storage_rd/render_scene_buffers_rd.h"
#include "servers/rendering/renderer_rd/storage_rd/texture_storage.h"
#include "servers/rendering/renderer_rd/uniform_set_cache_rd.h"
#include "servers/rendering/rendering_server_default.h"

//#define DIRTY_ALL_FRAMES

using namespace RendererRD;

const Vector3i GI::SDFGI::Cascade::DIRTY_ALL = Vector3i(0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF);
const Vector3i GI::SDFGI::CASCADE_SIZE = Vector3i(GI::SDFGI::CASCADE_H_SIZE, GI::SDFGI::CASCADE_V_SIZE, GI::SDFGI::CASCADE_H_SIZE);

GI *GI::singleton = nullptr;

////////////////////////////////////////////////////////////////////////////////
// VOXEL GI STORAGE

RID GI::voxel_gi_allocate() {
	return voxel_gi_owner.allocate_rid();
}

void GI::voxel_gi_free(RID p_voxel_gi) {
	voxel_gi_allocate_data(p_voxel_gi, Transform3D(), AABB(), Vector3i(), Vector<uint8_t>(), Vector<uint8_t>(), Vector<uint8_t>(), Vector<int>()); //deallocate
	VoxelGI *voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
	voxel_gi->dependency.deleted_notify(p_voxel_gi);
	voxel_gi_owner.free(p_voxel_gi);
}

void GI::voxel_gi_initialize(RID p_voxel_gi) {
	voxel_gi_owner.initialize_rid(p_voxel_gi, VoxelGI());
}

void GI::voxel_gi_allocate_data(RID p_voxel_gi, const Transform3D &p_to_cell_xform, const AABB &p_aabb, const Vector3i &p_octree_size, const Vector<uint8_t> &p_octree_cells, const Vector<uint8_t> &p_data_cells, const Vector<uint8_t> &p_distance_field, const Vector<int> &p_level_counts) {
	VoxelGI *voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
	ERR_FAIL_NULL(voxel_gi);

	if (voxel_gi->octree_buffer.is_valid()) {
		RD::get_singleton()->free(voxel_gi->octree_buffer);
		RD::get_singleton()->free(voxel_gi->data_buffer);
		if (voxel_gi->sdf_texture.is_valid()) {
			RD::get_singleton()->free(voxel_gi->sdf_texture);
		}

		voxel_gi->sdf_texture = RID();
		voxel_gi->octree_buffer = RID();
		voxel_gi->data_buffer = RID();
		voxel_gi->octree_buffer_size = 0;
		voxel_gi->data_buffer_size = 0;
		voxel_gi->cell_count = 0;
	}

	voxel_gi->to_cell_xform = p_to_cell_xform;
	voxel_gi->bounds = p_aabb;
	voxel_gi->octree_size = p_octree_size;
	voxel_gi->level_counts = p_level_counts;

	if (p_octree_cells.size()) {
		ERR_FAIL_COND(p_octree_cells.size() % 32 != 0); //cells size must be a multiple of 32

		uint32_t cell_count = p_octree_cells.size() / 32;

		ERR_FAIL_COND(p_data_cells.size() != (int)cell_count * 16); //see that data size matches

		voxel_gi->cell_count = cell_count;
		voxel_gi->octree_buffer = RD::get_singleton()->storage_buffer_create(p_octree_cells.size(), p_octree_cells);
		voxel_gi->octree_buffer_size = p_octree_cells.size();
		voxel_gi->data_buffer = RD::get_singleton()->storage_buffer_create(p_data_cells.size(), p_data_cells);
		voxel_gi->data_buffer_size = p_data_cells.size();

		if (p_distance_field.size()) {
			RD::TextureFormat tf;
			tf.format = RD::DATA_FORMAT_R8_UNORM;
			tf.width = voxel_gi->octree_size.x;
			tf.height = voxel_gi->octree_size.y;
			tf.depth = voxel_gi->octree_size.z;
			tf.texture_type = RD::TEXTURE_TYPE_3D;
			tf.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT | RD::TEXTURE_USAGE_CAN_COPY_FROM_BIT;
			Vector<Vector<uint8_t>> s;
			s.push_back(p_distance_field);
			voxel_gi->sdf_texture = RD::get_singleton()->texture_create(tf, RD::TextureView(), s);
			RD::get_singleton()->set_resource_name(voxel_gi->sdf_texture, "VoxelGI SDF Texture");
		}
#if 0
			{
				RD::TextureFormat tf;
				tf.format = RD::DATA_FORMAT_R8_UNORM;
				tf.width = voxel_gi->octree_size.x;
				tf.height = voxel_gi->octree_size.y;
				tf.depth = voxel_gi->octree_size.z;
				tf.type = RD::TEXTURE_TYPE_3D;
				tf.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_STORAGE_BIT | RD::TEXTURE_USAGE_CAN_COPY_TO_BIT;
				tf.shareable_formats.push_back(RD::DATA_FORMAT_R8_UNORM);
				tf.shareable_formats.push_back(RD::DATA_FORMAT_R8_UINT);
				voxel_gi->sdf_texture = RD::get_singleton()->texture_create(tf, RD::TextureView());
				RD::get_singleton()->set_resource_name(voxel_gi->sdf_texture, "VoxelGI SDF Texture");
			}
			RID shared_tex;
			{
				RD::TextureView tv;
				tv.format_override = RD::DATA_FORMAT_R8_UINT;
				shared_tex = RD::get_singleton()->texture_create_shared(tv, voxel_gi->sdf_texture);
			}
			//update SDF texture
			Vector<RD::Uniform> uniforms;
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_STORAGE_BUFFER;
				u.binding = 1;
				u.append_id(voxel_gi->octree_buffer);
				uniforms.push_back(u);
			}
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_STORAGE_BUFFER;
				u.binding = 2;
				u.append_id(voxel_gi->data_buffer);
				uniforms.push_back(u);
			}
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
				u.binding = 3;
				u.append_id(shared_tex);
				uniforms.push_back(u);
			}

			RID uniform_set = RD::get_singleton()->uniform_set_create(uniforms, voxel_gi_sdf_shader_version_shader, 0);

			{
				uint32_t push_constant[4] = { 0, 0, 0, 0 };

				for (int i = 0; i < voxel_gi->level_counts.size() - 1; i++) {
					push_constant[0] += voxel_gi->level_counts[i];
				}
				push_constant[1] = push_constant[0] + voxel_gi->level_counts[voxel_gi->level_counts.size() - 1];

				print_line("offset: " + itos(push_constant[0]));
				print_line("size: " + itos(push_constant[1]));
				//create SDF
				RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();
				RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, voxel_gi_sdf_shader_pipeline);
				RD::get_singleton()->compute_list_bind_uniform_set(compute_list, uniform_set, 0);
				RD::get_singleton()->compute_list_set_push_constant(compute_list, push_constant, sizeof(uint32_t) * 4);
				RD::get_singleton()->compute_list_dispatch(compute_list, voxel_gi->octree_size.x / 4, voxel_gi->octree_size.y / 4, voxel_gi->octree_size.z / 4);
				RD::get_singleton()->compute_list_end();
			}

			RD::get_singleton()->free(uniform_set);
			RD::get_singleton()->free(shared_tex);
		}
#endif
	}

	voxel_gi->version++;
	voxel_gi->data_version++;

	voxel_gi->dependency.changed_notify(Dependency::DEPENDENCY_CHANGED_AABB);
}

AABB GI::voxel_gi_get_bounds(RID p_voxel_gi) const {
	VoxelGI *voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
	ERR_FAIL_NULL_V(voxel_gi, AABB());

	return voxel_gi->bounds;
}

Vector3i GI::voxel_gi_get_octree_size(RID p_voxel_gi) const {
	VoxelGI *voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
	ERR_FAIL_NULL_V(voxel_gi, Vector3i());
	return voxel_gi->octree_size;
}

Vector<uint8_t> GI::voxel_gi_get_octree_cells(RID p_voxel_gi) const {
	VoxelGI *voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
	ERR_FAIL_NULL_V(voxel_gi, Vector<uint8_t>());

	if (voxel_gi->octree_buffer.is_valid()) {
		return RD::get_singleton()->buffer_get_data(voxel_gi->octree_buffer);
	}
	return Vector<uint8_t>();
}

Vector<uint8_t> GI::voxel_gi_get_data_cells(RID p_voxel_gi) const {
	VoxelGI *voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
	ERR_FAIL_NULL_V(voxel_gi, Vector<uint8_t>());

	if (voxel_gi->data_buffer.is_valid()) {
		return RD::get_singleton()->buffer_get_data(voxel_gi->data_buffer);
	}
	return Vector<uint8_t>();
}

Vector<uint8_t> GI::voxel_gi_get_distance_field(RID p_voxel_gi) const {
	VoxelGI *voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
	ERR_FAIL_NULL_V(voxel_gi, Vector<uint8_t>());

	if (voxel_gi->data_buffer.is_valid()) {
		return RD::get_singleton()->texture_get_data(voxel_gi->sdf_texture, 0);
	}
	return Vector<uint8_t>();
}

Vector<int> GI::voxel_gi_get_level_counts(RID p_voxel_gi) const {
	VoxelGI *voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
	ERR_FAIL_NULL_V(voxel_gi, Vector<int>());

	return voxel_gi->level_counts;
}

Transform3D GI::voxel_gi_get_to_cell_xform(RID p_voxel_gi) const {
	VoxelGI *voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
	ERR_FAIL_NULL_V(voxel_gi, Transform3D());

	return voxel_gi->to_cell_xform;
}

void GI::voxel_gi_set_dynamic_range(RID p_voxel_gi, float p_range) {
	VoxelGI *voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
	ERR_FAIL_NULL(voxel_gi);

	voxel_gi->dynamic_range = p_range;
	voxel_gi->version++;
}

float GI::voxel_gi_get_dynamic_range(RID p_voxel_gi) const {
	VoxelGI *voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
	ERR_FAIL_NULL_V(voxel_gi, 0);

	return voxel_gi->dynamic_range;
}

void GI::voxel_gi_set_propagation(RID p_voxel_gi, float p_range) {
	VoxelGI *voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
	ERR_FAIL_NULL(voxel_gi);

	voxel_gi->propagation = p_range;
	voxel_gi->version++;
}

float GI::voxel_gi_get_propagation(RID p_voxel_gi) const {
	VoxelGI *voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
	ERR_FAIL_NULL_V(voxel_gi, 0);
	return voxel_gi->propagation;
}

void GI::voxel_gi_set_energy(RID p_voxel_gi, float p_energy) {
	VoxelGI *voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
	ERR_FAIL_NULL(voxel_gi);

	voxel_gi->energy = p_energy;
}

float GI::voxel_gi_get_energy(RID p_voxel_gi) const {
	VoxelGI *voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
	ERR_FAIL_NULL_V(voxel_gi, 0);
	return voxel_gi->energy;
}

void GI::voxel_gi_set_baked_exposure_normalization(RID p_voxel_gi, float p_baked_exposure) {
	VoxelGI *voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
	ERR_FAIL_NULL(voxel_gi);

	voxel_gi->baked_exposure = p_baked_exposure;
}

float GI::voxel_gi_get_baked_exposure_normalization(RID p_voxel_gi) const {
	VoxelGI *voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
	ERR_FAIL_NULL_V(voxel_gi, 0);
	return voxel_gi->baked_exposure;
}

void GI::voxel_gi_set_bias(RID p_voxel_gi, float p_bias) {
	VoxelGI *voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
	ERR_FAIL_NULL(voxel_gi);

	voxel_gi->bias = p_bias;
}

float GI::voxel_gi_get_bias(RID p_voxel_gi) const {
	VoxelGI *voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
	ERR_FAIL_NULL_V(voxel_gi, 0);
	return voxel_gi->bias;
}

void GI::voxel_gi_set_normal_bias(RID p_voxel_gi, float p_normal_bias) {
	VoxelGI *voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
	ERR_FAIL_NULL(voxel_gi);

	voxel_gi->normal_bias = p_normal_bias;
}

float GI::voxel_gi_get_normal_bias(RID p_voxel_gi) const {
	VoxelGI *voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
	ERR_FAIL_NULL_V(voxel_gi, 0);
	return voxel_gi->normal_bias;
}

void GI::voxel_gi_set_interior(RID p_voxel_gi, bool p_enable) {
	VoxelGI *voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
	ERR_FAIL_NULL(voxel_gi);

	voxel_gi->interior = p_enable;
}

void GI::voxel_gi_set_use_two_bounces(RID p_voxel_gi, bool p_enable) {
	VoxelGI *voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
	ERR_FAIL_NULL(voxel_gi);

	voxel_gi->use_two_bounces = p_enable;
	voxel_gi->version++;
}

bool GI::voxel_gi_is_using_two_bounces(RID p_voxel_gi) const {
	VoxelGI *voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
	ERR_FAIL_NULL_V(voxel_gi, false);
	return voxel_gi->use_two_bounces;
}

bool GI::voxel_gi_is_interior(RID p_voxel_gi) const {
	VoxelGI *voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
	ERR_FAIL_NULL_V(voxel_gi, 0);
	return voxel_gi->interior;
}

uint32_t GI::voxel_gi_get_version(RID p_voxel_gi) const {
	VoxelGI *voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
	ERR_FAIL_NULL_V(voxel_gi, 0);
	return voxel_gi->version;
}

uint32_t GI::voxel_gi_get_data_version(RID p_voxel_gi) {
	VoxelGI *voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
	ERR_FAIL_NULL_V(voxel_gi, 0);
	return voxel_gi->data_version;
}

RID GI::voxel_gi_get_octree_buffer(RID p_voxel_gi) const {
	VoxelGI *voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
	ERR_FAIL_NULL_V(voxel_gi, RID());
	return voxel_gi->octree_buffer;
}

RID GI::voxel_gi_get_data_buffer(RID p_voxel_gi) const {
	VoxelGI *voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
	ERR_FAIL_NULL_V(voxel_gi, RID());
	return voxel_gi->data_buffer;
}

RID GI::voxel_gi_get_sdf_texture(RID p_voxel_gi) {
	VoxelGI *voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
	ERR_FAIL_NULL_V(voxel_gi, RID());

	return voxel_gi->sdf_texture;
}

Dependency *GI::voxel_gi_get_dependency(RID p_voxel_gi) const {
	VoxelGI *voxel_gi = voxel_gi_owner.get_or_null(p_voxel_gi);
	ERR_FAIL_NULL_V(voxel_gi, nullptr);

	return &voxel_gi->dependency;
}

void GI::sdfgi_reset() {
	sdfgi_current_version++;
}

////////////////////////////////////////////////////////////////////////////////
// SDFGI

static RID create_clear_texture(const RD::TextureFormat &p_format, const String &p_name) {
	RID texture = RD::get_singleton()->texture_create(p_format, RD::TextureView());
	ERR_FAIL_COND_V_MSG(texture.is_null(), RID(), String("Cannot create texture: ") + p_name);

	RD::get_singleton()->set_resource_name(texture, p_name);
	RD::get_singleton()->texture_clear(texture, Color(0, 0, 0, 0), 0, p_format.mipmaps, 0, p_format.array_layers);

	return texture;
}

void GI::SDFGI::create(RID p_env, const Vector3 &p_world_position, uint32_t p_requested_history_size, GI *p_gi) {
	//RendererRD::TextureStorage *texture_storage = RendererRD::TextureStorage::get_singleton();
	//RendererRD::MaterialStorage *material_storage = RendererRD::MaterialStorage::get_singleton();

	gi = p_gi;
	num_cascades = RendererSceneRenderRD::get_singleton()->environment_get_sdfgi_cascades(p_env);
	min_cell_size = RendererSceneRenderRD::get_singleton()->environment_get_sdfgi_min_cell_size(p_env);
	uses_occlusion = RendererSceneRenderRD::get_singleton()->environment_get_sdfgi_use_occlusion(p_env);
	using_filter = uses_occlusion;
	y_scale_mode = RendererSceneRenderRD::get_singleton()->environment_get_sdfgi_y_scale(p_env);
	static const float y_scale[3] = { 2.0, 1.5, 1.0 };
	y_mult = y_scale[y_scale_mode];
	version = gi->sdfgi_current_version;
	cascades.resize(num_cascades);

	solid_cell_ratio = gi->sdfgi_solid_cell_ratio;
	solid_cell_count = uint32_t(float(SDFGI::CASCADE_H_SIZE * SDFGI::CASCADE_V_SIZE * SDFGI::CASCADE_H_SIZE) * solid_cell_ratio);

	float base_cell_size = min_cell_size;

	RD::TextureFormat tf_base;
	tf_base.format = RD::DATA_FORMAT_R8_UNORM;
	tf_base.width = CASCADE_H_SIZE;
	tf_base.height = CASCADE_V_SIZE;
	tf_base.depth = CASCADE_H_SIZE;
	tf_base.texture_type = RD::TEXTURE_TYPE_3D;
	tf_base.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_STORAGE_BIT | RD::TEXTURE_USAGE_CAN_COPY_TO_BIT | RD::TEXTURE_USAGE_CAN_COPY_FROM_BIT;

	{
		RD::TextureFormat tf_voxel = tf_base;
		tf_voxel.format = RD::DATA_FORMAT_R32G32_UINT; // 4x4 region in a cache friendly format
		tf_voxel.width /= 4;
		tf_voxel.height /= 4;
		tf_voxel.depth /= 4;
		tf_voxel.texture_type = RD::TEXTURE_TYPE_3D;
		tf_voxel.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_STORAGE_BIT | RD::TEXTURE_USAGE_CAN_COPY_TO_BIT | RD::TEXTURE_USAGE_CAN_COPY_FROM_BIT;
		RD::TextureFormat tf_sdf = tf_base;

		tf_voxel.height *= cascades.size();
		voxel_bits_tex = create_clear_texture(tf_voxel, "SDFGI Voxel Field");
	}

	{
		RD::TextureFormat tf_voxel = tf_base;
		tf_voxel.format = RD::DATA_FORMAT_R16_UINT; // 4x4 region in a cache friendly format
		tf_voxel.width /= SDF_REGION_SIZE;
		tf_voxel.height /= SDF_REGION_SIZE;
		tf_voxel.depth /= SDF_REGION_SIZE;
		tf_voxel.height *= cascades.size();
		tf_voxel.texture_type = RD::TEXTURE_TYPE_3D;
		tf_voxel.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_STORAGE_BIT | RD::TEXTURE_USAGE_CAN_COPY_TO_BIT | RD::TEXTURE_USAGE_CAN_COPY_FROM_BIT;
		region_version_data = create_clear_texture(tf_voxel, "SDFGI Region Version");
	}

	{
		RD::TextureFormat tf_voxel_region = tf_base;
		tf_voxel_region.format = RD::DATA_FORMAT_R8_UINT;
		tf_voxel_region.width /= REGION_CELLS;
		tf_voxel_region.height /= REGION_CELLS;
		tf_voxel_region.depth /= REGION_CELLS;
		tf_voxel_region.texture_type = RD::TEXTURE_TYPE_3D;
		tf_voxel_region.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_STORAGE_BIT | RD::TEXTURE_USAGE_CAN_COPY_TO_BIT | RD::TEXTURE_USAGE_CAN_COPY_FROM_BIT;
		RD::TextureFormat tf_sdf = tf_base;

		tf_voxel_region.height *= cascades.size();
		voxel_region_tex = create_clear_texture(tf_voxel_region, "SDFGI Voxel Regions");
	}

	{
		{
			RD::TextureFormat tf_light = tf_base;
			tf_light.format = RD::DATA_FORMAT_R32_UINT;
			tf_light.height *= cascades.size();
			tf_light.shareable_formats.push_back(RD::DATA_FORMAT_E5B9G9R9_UFLOAT_PACK32);
			tf_light.shareable_formats.push_back(RD::DATA_FORMAT_R32_UINT);

			light_tex_data = create_clear_texture(tf_light, "SDFGI Cascade Light Data");

			RD::TextureView tv;
			tv.format_override = RD::DATA_FORMAT_E5B9G9R9_UFLOAT_PACK32;
			light_tex = RD::get_singleton()->texture_create_shared(tv, light_tex_data);
		}

		{ // Albedo texture, this is anisotropic (x6).
			RD::TextureFormat tf_render_albedo = tf_base;
			tf_render_albedo.width /= 2; // Albedo is half size..
			tf_render_albedo.height /= 2;
			tf_render_albedo.depth /= 2;
			tf_render_albedo.depth *= 6; // ..but anisotropic.
			tf_render_albedo.format = RD::DATA_FORMAT_R16_UINT;
			render_albedo = create_clear_texture(tf_render_albedo, "SDFGI Render Albedo");
		}

		{ // Emission texture, this is anisotropic but in a different way (main light, then x6 aniso weight) to save space.
			RD::TextureFormat tf_render_emission = tf_base;
			tf_render_emission.width /= 2; // Emission is half size..
			tf_render_emission.height /= 2;
			tf_render_emission.depth /= 2;
			tf_render_emission.format = RD::DATA_FORMAT_R32_UINT;
			render_emission = create_clear_texture(tf_render_emission, "SDFGI Render Emission");
			render_emission_aniso = create_clear_texture(tf_render_emission, "SDFGI Render Emission Aniso");
		}

		{ // Solid bits
			RD::TextureFormat tf_render_solid_bits = tf_base;
			tf_render_solid_bits.format = RD::DATA_FORMAT_R32_UINT;

			for (int i = 0; i < 2; i++) {
				render_solid_bits[i] = create_clear_texture(tf_render_solid_bits, String("SDFGI Render Solid Bits ") + itos(i));
			}
			render_aniso_normals = create_clear_texture(tf_render_solid_bits, String("SDFGI Render Solid Bits "));
		}
	}

	light_process_buffer_render = RD::get_singleton()->storage_buffer_create(sizeof(SDFGI::Cascade::LightProcessCell) * solid_cell_count);
	light_process_dispatch_buffer_render = RD::get_singleton()->storage_buffer_create(sizeof(uint32_t) * 4, Vector<uint8_t>(), RD::STORAGE_BUFFER_USAGE_DISPATCH_INDIRECT);

	cascades_ubo = RD::get_singleton()->uniform_buffer_create(sizeof(SDFGI::Cascade::UBO) * SDFGI::MAX_CASCADES);

	// lightprobes
	Vector3i PROBE_DIVISOR = CASCADE_SIZE / PROBE_CELLS;

	{
		RD::TextureFormat tf_lightprobes;
		tf_lightprobes.texture_type = RD::TEXTURE_TYPE_2D_ARRAY;
		tf_lightprobes.format = RD::DATA_FORMAT_E5B9G9R9_UFLOAT_PACK32;
		tf_lightprobes.width = (PROBE_DIVISOR.x + 1) * (LIGHTPROBE_OCT_SIZE + 2); // Divisor +1 because need an extra one on the outside of the box for proper interpolation. This contains also the Z probes towards x+
		tf_lightprobes.height = (PROBE_DIVISOR.y + 1) * (PROBE_DIVISOR.z + 1) * (LIGHTPROBE_OCT_SIZE + 2); // OctSize +2 because it needs a border to interpolate properly
		tf_lightprobes.depth = 1;
		tf_lightprobes.array_layers = cascades.size();
		tf_lightprobes.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_STORAGE_BIT | RD::TEXTURE_USAGE_CAN_COPY_TO_BIT | RD::TEXTURE_USAGE_CAN_COPY_FROM_BIT;
		tf_lightprobes.shareable_formats.push_back(RD::DATA_FORMAT_R32_UINT);
		tf_lightprobes.shareable_formats.push_back(RD::DATA_FORMAT_E5B9G9R9_UFLOAT_PACK32);

		lightprobe_specular_tex = create_clear_texture(tf_lightprobes, String("SDFGI Lighprobe Specular"));
		lightprobe_diffuse_tex = create_clear_texture(tf_lightprobes, String("SDFGI Lighprobe Diffuse"));
		lightprobe_diffuse_filter_tex = create_clear_texture(tf_lightprobes, String("SDFGI Lighprobe Diffuse Filtered"));

		RD::TextureView tv;
		tv.format_override = RD::DATA_FORMAT_R32_UINT;
		lightprobe_diffuse_data = RD::get_singleton()->texture_create_shared(tv, lightprobe_diffuse_tex);
		lightprobe_diffuse_filter_data = RD::get_singleton()->texture_create_shared(tv, lightprobe_diffuse_filter_tex);
		lightprobe_specular_data = RD::get_singleton()->texture_create_shared(tv, lightprobe_specular_tex);

		RD::TextureFormat tf_cache_data = tf_lightprobes;
		tf_cache_data.format = RD::DATA_FORMAT_R32_UINT;
		tf_cache_data.width = (PROBE_DIVISOR.x + 1) * LIGHTPROBE_OCT_SIZE;
		tf_cache_data.height = (PROBE_DIVISOR.y + 1) * (PROBE_DIVISOR.z + 1) * LIGHTPROBE_OCT_SIZE;
		tf_cache_data.array_layers *= LIGHTPROBE_HISTORY_FRAMES;
		tf_cache_data.shareable_formats.clear();
		lightprobe_hit_cache_data = create_clear_texture(tf_cache_data, String("SDFGI Lighprobe Hit Cache"));

		tf_cache_data.format = RD::DATA_FORMAT_R16_UINT;
		lightprobe_hit_cache_version_data = create_clear_texture(tf_cache_data, String("SDFGI Lighprobe Hit Cache Version"));

		tf_cache_data.format = RD::DATA_FORMAT_R16G16B16A16_UINT;
		lightprobe_moving_average_history = create_clear_texture(tf_cache_data, String("SDFGI Lighprobe Moving Average History"));

		tf_cache_data.array_layers = cascades.size();
		lightprobe_moving_average = create_clear_texture(tf_cache_data, String("SDFGI Lighprobe Moving Average"));

		RD::TextureFormat tf_ambient = tf_lightprobes;
		tf_ambient.width = (PROBE_DIVISOR.x + 1);
		tf_ambient.height = (PROBE_DIVISOR.y + 1) * (PROBE_DIVISOR.z + 1);

		lightprobe_ambient_tex = create_clear_texture(tf_ambient, String("SDFGI Lighprobe Ambient"));
		lightprobe_ambient_data = RD::get_singleton()->texture_create_shared(tv, lightprobe_ambient_tex);

		RD::TextureFormat tf_neighbours;
		tf_neighbours.texture_type = RD::TEXTURE_TYPE_2D_ARRAY;
		tf_neighbours.format = RD::DATA_FORMAT_R8_UINT;
		tf_neighbours.width = (PROBE_DIVISOR.x + 1);
		tf_neighbours.height = (PROBE_DIVISOR.y + 1) * (PROBE_DIVISOR.z + 1);
		tf_neighbours.depth = 1;
		tf_neighbours.array_layers = cascades.size();
		tf_neighbours.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_STORAGE_BIT | RD::TEXTURE_USAGE_CAN_COPY_TO_BIT | RD::TEXTURE_USAGE_CAN_COPY_FROM_BIT;
		lightprobe_neighbour_map = create_clear_texture(tf_neighbours, String("SDFGI Neighbour Map"));
	}

	// Occlusion

	{
		RD::TextureFormat tf_occlusion;
		tf_occlusion.texture_type = RD::TEXTURE_TYPE_2D_ARRAY;
		tf_occlusion.format = RD::DATA_FORMAT_R8_UNORM;
		tf_occlusion.width = (PROBE_DIVISOR.x + 1) * OCCLUSION_OCT_SIZE; // Divisor +1 because need an extra one on the outside of the box for proper interpolation. This contains also the Z probes towards x+
		tf_occlusion.height = (PROBE_DIVISOR.y + 1) * (PROBE_DIVISOR.z + 1) * OCCLUSION_OCT_SIZE; // OctSize +2 because it needs a border to interpolate properly
		tf_occlusion.depth = 1;
		tf_occlusion.array_layers = cascades.size();
		tf_occlusion.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_STORAGE_BIT | RD::TEXTURE_USAGE_CAN_COPY_TO_BIT | RD::TEXTURE_USAGE_CAN_COPY_FROM_BIT;

		occlusion_process = create_clear_texture(tf_occlusion, String("SDFGI Occlusion Compute"));

		tf_occlusion.format = RD::DATA_FORMAT_R16G16_UNORM;
		tf_occlusion.width = (PROBE_DIVISOR.x + 1) * (OCCLUSION_OCT_SIZE + 2); // Divisor +1 because need an extra one on the outside of the box for proper interpolation. This contains also the Z probes towards x+
		tf_occlusion.height = (PROBE_DIVISOR.y + 1) * (PROBE_DIVISOR.z + 1) * (OCCLUSION_OCT_SIZE + 2); // OctSize +2 because it needs a border to interpolate properly

		occlusion_tex = create_clear_texture(tf_occlusion, String("SDFGI Occlusion Texture"));
	}

#if 0
	RD::TextureFormat tf_occlusion = tf_sdf;
	tf_occlusion.format = RD::DATA_FORMAT_R16_UINT;
	tf_occlusion.shareable_formats.push_back(RD::DATA_FORMAT_R16_UINT);
	tf_occlusion.shareable_formats.push_back(RD::DATA_FORMAT_R4G4B4A4_UNORM_PACK16);
	tf_occlusion.depth *= cascades.size(); //use depth for occlusion slices
	tf_occlusion.width *= 2; //use width for the other half

	RD::TextureFormat tf_light = tf_sdf;
	tf_light.format = RD::DATA_FORMAT_R32_UINT;
	tf_light.shareable_formats.push_back(RD::DATA_FORMAT_R32_UINT);
	tf_light.shareable_formats.push_back(RD::DATA_FORMAT_E5B9G9R9_UFLOAT_PACK32);

	RD::TextureFormat tf_aniso0 = tf_sdf;
	tf_aniso0.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
	RD::TextureFormat tf_aniso1 = tf_sdf;
	tf_aniso1.format = RD::DATA_FORMAT_R8G8_UNORM;

	int passes = nearest_shift(CASCADE_SIZE) - 1;

	//store lightprobe SH
	RD::TextureFormat tf_probes;
	tf_probes.format = RD::DATA_FORMAT_R16G16B16A16_SFLOAT;
	tf_probes.width = probe_axis_count * probe_axis_count;
	tf_probes.height = probe_axis_count * SDFGI::SH_SIZE;
	tf_probes.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_STORAGE_BIT | RD::TEXTURE_USAGE_CAN_COPY_TO_BIT | RD::TEXTURE_USAGE_CAN_COPY_FROM_BIT;
	tf_probes.texture_type = RD::TEXTURE_TYPE_2D_ARRAY;

	history_size = p_requested_history_size;

	RD::TextureFormat tf_probe_history = tf_probes;
	tf_probe_history.format = RD::DATA_FORMAT_R16G16B16A16_SINT; //signed integer because SH are signed
	tf_probe_history.array_layers = history_size;

	RD::TextureFormat tf_probe_average = tf_probes;
	tf_probe_average.format = RD::DATA_FORMAT_R32G32B32A32_SINT; //signed integer because SH are signed
	tf_probe_average.texture_type = RD::TEXTURE_TYPE_2D;

	lightprobe_history_scroll = create_clear_texture(tf_probe_history, "SDFGI LightProbe History Scroll");
	lightprobe_average_scroll = create_clear_texture(tf_probe_average, "SDFGI LightProbe Average Scroll");

	{
		//octahedral lightprobes
		RD::TextureFormat tf_octprobes = tf_probes;
		tf_octprobes.array_layers = cascades.size() * 2;
		tf_octprobes.format = RD::DATA_FORMAT_R32_UINT; //pack well with RGBE
		tf_octprobes.width = probe_axis_count * probe_axis_count * (SDFGI::LIGHTPROBE_OCT_SIZE + 2);
		tf_octprobes.height = probe_axis_count * (SDFGI::LIGHTPROBE_OCT_SIZE + 2);
		tf_octprobes.shareable_formats.push_back(RD::DATA_FORMAT_R32_UINT);
		tf_octprobes.shareable_formats.push_back(RD::DATA_FORMAT_E5B9G9R9_UFLOAT_PACK32);
		//lightprobe texture is an octahedral texture

		lightprobe_data = create_clear_texture(tf_octprobes, "SDFGI LightProbe Data");
		RD::TextureView tv;
		tv.format_override = RD::DATA_FORMAT_E5B9G9R9_UFLOAT_PACK32;
		lightprobe_texture = RD::get_singleton()->texture_create_shared(tv, lightprobe_data);

		//texture handling ambient data, to integrate with volumetric foc
		RD::TextureFormat tf_ambient = tf_probes;
		tf_ambient.array_layers = cascades.size();
		tf_ambient.format = RD::DATA_FORMAT_R16G16B16A16_SFLOAT; //pack well with RGBE
		tf_ambient.width = probe_axis_count * probe_axis_count;
		tf_ambient.height = probe_axis_count;
		tf_ambient.texture_type = RD::TEXTURE_TYPE_2D_ARRAY;
		//lightprobe texture is an octahedral texture
		ambient_texture = create_clear_texture(tf_ambient, "SDFGI Ambient Texture");
	}


	occlusion_data = create_clear_texture(tf_occlusion, "SDFGI Occlusion Data");
	{
		RD::TextureView tv;
		tv.format_override = RD::DATA_FORMAT_R4G4B4A4_UNORM_PACK16;
		occlusion_texture = RD::get_singleton()->texture_create_shared(tv, occlusion_data);
	}
#endif
	for (SDFGI::Cascade &cascade : cascades) {
		/* 3D Textures */

		cascade.light_process_buffer = RD::get_singleton()->storage_buffer_create(sizeof(SDFGI::Cascade::LightProcessCell) * solid_cell_count);
		cascade.light_process_dispatch_buffer = RD::get_singleton()->storage_buffer_create(sizeof(uint32_t) * 4, Vector<uint8_t>(), RD::STORAGE_BUFFER_USAGE_DISPATCH_INDIRECT);
		cascade.light_process_dispatch_buffer_copy = RD::get_singleton()->storage_buffer_create(sizeof(uint32_t) * 4, Vector<uint8_t>(), RD::STORAGE_BUFFER_USAGE_DISPATCH_INDIRECT);

		cascade.light_position_bufer = RD::get_singleton()->storage_buffer_create(sizeof(SDFGIShader::Light) * MAX(SDFGI::MAX_STATIC_LIGHTS, SDFGI::MAX_DYNAMIC_LIGHTS));

		cascade.cell_size = base_cell_size;
		Vector3 world_position = p_world_position;
		world_position.y *= y_mult;
		Vector3i probe_cells = CASCADE_SIZE / PROBE_CELLS;
		Vector3 probe_size = Vector3(1, 1, 1) * cascade.cell_size * Vector3(probe_cells);
		Vector3i probe_pos = Vector3i((world_position / probe_size + Vector3(0.5, 0.5, 0.5)).floor());
		cascade.position = probe_pos * probe_cells;

		cascade.dirty_regions = SDFGI::Cascade::DIRTY_ALL;

		// lightprobes

		base_cell_size *= 2.0;
#if 0
		/* Probe History */

		cascade.lightprobe_history_tex = RD::get_singleton()->texture_create(tf_probe_history, RD::TextureView());
		RD::get_singleton()->set_resource_name(cascade.lightprobe_history_tex, "SDFGI Cascade LightProbe History Texture");
		RD::get_singleton()->texture_clear(cascade.lightprobe_history_tex, Color(0, 0, 0, 0), 0, 1, 0, tf_probe_history.array_layers); //needs to be cleared for average to work

		cascade.lightprobe_average_tex = RD::get_singleton()->texture_create(tf_probe_average, RD::TextureView());
		RD::get_singleton()->set_resource_name(cascade.lightprobe_average_tex, "SDFGI Cascade LightProbe Average Texture");
		RD::get_singleton()->texture_clear(cascade.lightprobe_average_tex, Color(0, 0, 0, 0), 0, 1, 0, 1); //needs to be cleared for average to work

		/* Buffers */

		cascade.solid_cell_buffer = RD::get_singleton()->storage_buffer_create(sizeof(SDFGI::Cascade::SolidCell) * solid_cell_count);
		cascade.solid_cell_dispatch_buffer = RD::get_singleton()->storage_buffer_create(sizeof(uint32_t) * 4, Vector<uint8_t>(), RD::STORAGE_BUFFER_USAGE_DISPATCH_INDIRECT);
		cascade.lights_buffer = RD::get_singleton()->storage_buffer_create(sizeof(SDFGIShader::Light) * MAX(SDFGI::MAX_STATIC_LIGHTS, SDFGI::MAX_DYNAMIC_LIGHTS));
		{
			Vector<RD::Uniform> uniforms;
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
				u.binding = 1;
				u.append_id(render_sdf[(passes & 1) ? 1 : 0]); //if passes are even, we read from buffer 0, else we read from buffer 1
				uniforms.push_back(u);
			}
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
				u.binding = 2;
				u.append_id(render_albedo);
				uniforms.push_back(u);
			}
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
				u.binding = 3;
				for (int j = 0; j < 8; j++) {
					u.append_id(render_occlusion[j]);
				}
				uniforms.push_back(u);
			}
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
				u.binding = 4;
				u.append_id(render_emission);
				uniforms.push_back(u);
			}
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
				u.binding = 5;
				u.append_id(render_emission_aniso);
				uniforms.push_back(u);
			}
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
				u.binding = 6;
				u.append_id(render_geom_facing);
				uniforms.push_back(u);
			}

			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
				u.binding = 7;
				u.append_id(cascade.sdf_tex);
				uniforms.push_back(u);
			}
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
				u.binding = 8;
				u.append_id(occlusion_data);
				uniforms.push_back(u);
			}
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_STORAGE_BUFFER;
				u.binding = 10;
				u.append_id(cascade.solid_cell_dispatch_buffer);
				uniforms.push_back(u);
			}
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_STORAGE_BUFFER;
				u.binding = 11;
				u.append_id(cascade.solid_cell_buffer);
				uniforms.push_back(u);
			}

			cascade.sdf_store_uniform_set = RD::get_singleton()->uniform_set_create(uniforms, gi->sdfgi_shader.preprocess.version_get_shader(gi->sdfgi_shader.preprocess_shader, SDFGIShader::PRE_PROCESS_STORE), 0);
		}

		{
			Vector<RD::Uniform> uniforms;
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
				u.binding = 1;
				u.append_id(render_albedo);
				uniforms.push_back(u);
			}
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
				u.binding = 2;
				u.append_id(render_geom_facing);
				uniforms.push_back(u);
			}
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
				u.binding = 3;
				u.append_id(render_emission);
				uniforms.push_back(u);
			}
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
				u.binding = 4;
				u.append_id(render_emission_aniso);
				uniforms.push_back(u);
			}
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_STORAGE_BUFFER;
				u.binding = 5;
				u.append_id(cascade.solid_cell_dispatch_buffer);
				uniforms.push_back(u);
			}
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_STORAGE_BUFFER;
				u.binding = 6;
				u.append_id(cascade.solid_cell_buffer);
				uniforms.push_back(u);
			}

			cascade.scroll_uniform_set = RD::get_singleton()->uniform_set_create(uniforms, gi->sdfgi_shader.preprocess.version_get_shader(gi->sdfgi_shader.preprocess_shader, SDFGIShader::PRE_PROCESS_SCROLL), 0);
		}
		{
			Vector<RD::Uniform> uniforms;
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
				u.binding = 1;
				for (int j = 0; j < 8; j++) {
					u.append_id(render_occlusion[j]);
				}
				uniforms.push_back(u);
			}
			{
				RD::Uniform u;
				u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
				u.binding = 2;
				u.append_id(occlusion_data);
				uniforms.push_back(u);
			}

			cascade.scroll_occlusion_uniform_set = RD::get_singleton()->uniform_set_create(uniforms, gi->sdfgi_shader.preprocess.version_get_shader(gi->sdfgi_shader.preprocess_shader, SDFGIShader::PRE_PROCESS_SCROLL_OCCLUSION), 0);
		}
#endif
	}
#if 0
	//direct light
	for (SDFGI::Cascade &cascade : cascades) {

		Vector<RD::Uniform> uniforms;
		{
			RD::Uniform u;
			u.binding = 1;
			u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
			for (uint32_t j = 0; j < SDFGI::MAX_CASCADES; j++) {
				if (j < cascades.size()) {
					u.append_id(cascades[j].sdf_tex);
				} else {
					u.append_id(texture_storage->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_3D_WHITE));
				}
			}
			uniforms.push_back(u);
		}
		{
			RD::Uniform u;
			u.binding = 2;
			u.uniform_type = RD::UNIFORM_TYPE_SAMPLER;
			u.append_id(material_storage->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED));
			uniforms.push_back(u);
		}
		{
			RD::Uniform u;
			u.binding = 3;
			u.uniform_type = RD::UNIFORM_TYPE_STORAGE_BUFFER;
			u.append_id(cascade.solid_cell_dispatch_buffer);
			uniforms.push_back(u);
		}
		{
			RD::Uniform u;
			u.binding = 4;
			u.uniform_type = RD::UNIFORM_TYPE_STORAGE_BUFFER;
			u.append_id(cascade.solid_cell_buffer);
			uniforms.push_back(u);
		}
		{
			RD::Uniform u;
			u.binding = 5;
			u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
			u.append_id(cascade.light_data);
			uniforms.push_back(u);
		}
		{
			RD::Uniform u;
			u.binding = 6;
			u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
			u.append_id(cascade.light_aniso_0_tex);
			uniforms.push_back(u);
		}
		{
			RD::Uniform u;
			u.binding = 7;
			u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
			u.append_id(cascade.light_aniso_1_tex);
			uniforms.push_back(u);
		}
		{
			RD::Uniform u;
			u.binding = 8;
			u.uniform_type = RD::UNIFORM_TYPE_UNIFORM_BUFFER;
			u.append_id(cascades_ubo);
			uniforms.push_back(u);
		}
		{
			RD::Uniform u;
			u.binding = 9;
			u.uniform_type = RD::UNIFORM_TYPE_STORAGE_BUFFER;
			u.append_id(cascade.lights_buffer);
			uniforms.push_back(u);
		}
		{
			RD::Uniform u;
			u.binding = 10;
			u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
			u.append_id(lightprobe_texture);
			uniforms.push_back(u);
		}
		{
			RD::Uniform u;
			u.binding = 11;
			u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
			u.append_id(occlusion_texture);
			uniforms.push_back(u);
		}

		cascade.sdf_direct_light_static_uniform_set = RD::get_singleton()->uniform_set_create(uniforms, gi->sdfgi_shader.direct_light.version_get_shader(gi->sdfgi_shader.direct_light_shader, SDFGIShader::DIRECT_LIGHT_MODE_STATIC), 0);
		cascade.sdf_direct_light_dynamic_uniform_set = RD::get_singleton()->uniform_set_create(uniforms, gi->sdfgi_shader.direct_light.version_get_shader(gi->sdfgi_shader.direct_light_shader, SDFGIShader::DIRECT_LIGHT_MODE_DYNAMIC), 0);

	}
#endif

#if 0
	//preprocess initialize uniform set
	{
		Vector<RD::Uniform> uniforms;
		{
			RD::Uniform u;
			u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
			u.binding = 1;
			u.append_id(render_albedo);
			uniforms.push_back(u);
		}
		{
			RD::Uniform u;
			u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
			u.binding = 2;
			u.append_id(render_sdf[0]);
			uniforms.push_back(u);
		}

		sdf_initialize_uniform_set = RD::get_singleton()->uniform_set_create(uniforms, gi->sdfgi_shader.preprocess.version_get_shader(gi->sdfgi_shader.preprocess_shader, SDFGIShader::PRE_PROCESS_JUMP_FLOOD_INITIALIZE), 0);
	}

	{
		Vector<RD::Uniform> uniforms;
		{
			RD::Uniform u;
			u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
			u.binding = 1;
			u.append_id(render_albedo);
			uniforms.push_back(u);
		}
		{
			RD::Uniform u;
			u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
			u.binding = 2;
			u.append_id(render_sdf_half[0]);
			uniforms.push_back(u);
		}

		sdf_initialize_half_uniform_set = RD::get_singleton()->uniform_set_create(uniforms, gi->sdfgi_shader.preprocess.version_get_shader(gi->sdfgi_shader.preprocess_shader, SDFGIShader::PRE_PROCESS_JUMP_FLOOD_INITIALIZE_HALF), 0);
	}

	//jump flood uniform set
	{
		Vector<RD::Uniform> uniforms;
		{
			RD::Uniform u;
			u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
			u.binding = 1;
			u.append_id(render_sdf[0]);
			uniforms.push_back(u);
		}
		{
			RD::Uniform u;
			u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
			u.binding = 2;
			u.append_id(render_sdf[1]);
			uniforms.push_back(u);
		}

		jump_flood_uniform_set[0] = RD::get_singleton()->uniform_set_create(uniforms, gi->sdfgi_shader.preprocess.version_get_shader(gi->sdfgi_shader.preprocess_shader, SDFGIShader::PRE_PROCESS_JUMP_FLOOD), 0);
		RID aux0 = uniforms.write[0].get_id(0);
		RID aux1 = uniforms.write[1].get_id(0);
		uniforms.write[0].set_id(0, aux1);
		uniforms.write[1].set_id(0, aux0);
		jump_flood_uniform_set[1] = RD::get_singleton()->uniform_set_create(uniforms, gi->sdfgi_shader.preprocess.version_get_shader(gi->sdfgi_shader.preprocess_shader, SDFGIShader::PRE_PROCESS_JUMP_FLOOD), 0);
	}
	//jump flood half uniform set
	{
		Vector<RD::Uniform> uniforms;
		{
			RD::Uniform u;
			u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
			u.binding = 1;
			u.append_id(render_sdf_half[0]);
			uniforms.push_back(u);
		}
		{
			RD::Uniform u;
			u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
			u.binding = 2;
			u.append_id(render_sdf_half[1]);
			uniforms.push_back(u);
		}

		jump_flood_half_uniform_set[0] = RD::get_singleton()->uniform_set_create(uniforms, gi->sdfgi_shader.preprocess.version_get_shader(gi->sdfgi_shader.preprocess_shader, SDFGIShader::PRE_PROCESS_JUMP_FLOOD), 0);
		RID aux0 = uniforms.write[0].get_id(0);
		RID aux1 = uniforms.write[1].get_id(0);
		uniforms.write[0].set_id(0, aux1);
		uniforms.write[1].set_id(0, aux0);
		jump_flood_half_uniform_set[1] = RD::get_singleton()->uniform_set_create(uniforms, gi->sdfgi_shader.preprocess.version_get_shader(gi->sdfgi_shader.preprocess_shader, SDFGIShader::PRE_PROCESS_JUMP_FLOOD), 0);
	}

	//upscale half size sdf
	{
		Vector<RD::Uniform> uniforms;
		{
			RD::Uniform u;
			u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
			u.binding = 1;
			u.append_id(render_albedo);
			uniforms.push_back(u);
		}
		{
			RD::Uniform u;
			u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
			u.binding = 2;
			u.append_id(render_sdf_half[(passes & 1) ? 0 : 1]); //reverse pass order because half size
			uniforms.push_back(u);
		}
		{
			RD::Uniform u;
			u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
			u.binding = 3;
			u.append_id(render_sdf[(passes & 1) ? 0 : 1]); //reverse pass order because it needs an extra JFA pass
			uniforms.push_back(u);
		}

		upscale_jfa_uniform_set_index = (passes & 1) ? 0 : 1;
		sdf_upscale_uniform_set = RD::get_singleton()->uniform_set_create(uniforms, gi->sdfgi_shader.preprocess.version_get_shader(gi->sdfgi_shader.preprocess_shader, SDFGIShader::PRE_PROCESS_JUMP_FLOOD_UPSCALE), 0);
	}

	//occlusion uniform set
	{
		Vector<RD::Uniform> uniforms;
		{
			RD::Uniform u;
			u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
			u.binding = 1;
			u.append_id(render_albedo);
			uniforms.push_back(u);
		}
		{
			RD::Uniform u;
			u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
			u.binding = 2;
			for (int i = 0; i < 8; i++) {
				u.append_id(render_occlusion[i]);
			}
			uniforms.push_back(u);
		}
		{
			RD::Uniform u;
			u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
			u.binding = 3;
			u.append_id(render_geom_facing);
			uniforms.push_back(u);
		}

		occlusion_uniform_set = RD::get_singleton()->uniform_set_create(uniforms, gi->sdfgi_shader.preprocess.version_get_shader(gi->sdfgi_shader.preprocess_shader, SDFGIShader::PRE_PROCESS_OCCLUSION), 0);
	}

	for (uint32_t i = 0; i < cascades.size(); i++) {
		//integrate uniform

		Vector<RD::Uniform> uniforms;

		{
			RD::Uniform u;
			u.binding = 1;
			u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
			for (uint32_t j = 0; j < SDFGI::MAX_CASCADES; j++) {
				if (j < cascades.size()) {
					u.append_id(cascades[j].sdf_tex);
				} else {
					u.append_id(texture_storage->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_3D_WHITE));
				}
			}
			uniforms.push_back(u);
		}
		{
			RD::Uniform u;
			u.binding = 2;
			u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
			for (uint32_t j = 0; j < SDFGI::MAX_CASCADES; j++) {
				if (j < cascades.size()) {
					u.append_id(cascades[j].light_tex);
				} else {
					u.append_id(texture_storage->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_3D_WHITE));
				}
			}
			uniforms.push_back(u);
		}
		{
			RD::Uniform u;
			u.binding = 3;
			u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
			for (uint32_t j = 0; j < SDFGI::MAX_CASCADES; j++) {
				if (j < cascades.size()) {
					u.append_id(cascades[j].light_aniso_0_tex);
				} else {
					u.append_id(texture_storage->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_3D_WHITE));
				}
			}
			uniforms.push_back(u);
		}
		{
			RD::Uniform u;
			u.binding = 4;
			u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
			for (uint32_t j = 0; j < SDFGI::MAX_CASCADES; j++) {
				if (j < cascades.size()) {
					u.append_id(cascades[j].light_aniso_1_tex);
				} else {
					u.append_id(texture_storage->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_3D_WHITE));
				}
			}
			uniforms.push_back(u);
		}
		{
			RD::Uniform u;
			u.uniform_type = RD::UNIFORM_TYPE_SAMPLER;
			u.binding = 6;
			u.append_id(material_storage->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED));
			uniforms.push_back(u);
		}

		{
			RD::Uniform u;
			u.uniform_type = RD::UNIFORM_TYPE_UNIFORM_BUFFER;
			u.binding = 7;
			u.append_id(cascades_ubo);
			uniforms.push_back(u);
		}
		{
			RD::Uniform u;
			u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
			u.binding = 8;
			u.append_id(lightprobe_data);
			uniforms.push_back(u);
		}

		{
			RD::Uniform u;
			u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
			u.binding = 9;
			u.append_id(cascades[i].lightprobe_history_tex);
			uniforms.push_back(u);
		}
		{
			RD::Uniform u;
			u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
			u.binding = 10;
			u.append_id(cascades[i].lightprobe_average_tex);
			uniforms.push_back(u);
		}

		{
			RD::Uniform u;
			u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
			u.binding = 11;
			u.append_id(lightprobe_history_scroll);
			uniforms.push_back(u);
		}
		{
			RD::Uniform u;
			u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
			u.binding = 12;
			u.append_id(lightprobe_average_scroll);
			uniforms.push_back(u);
		}
		{
			RD::Uniform u;
			u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
			u.binding = 13;
			RID parent_average;
			if (cascades.size() == 1) {
				// If there is only one SDFGI cascade, we can't use the previous cascade for blending.
				parent_average = cascades[i].lightprobe_average_tex;
			} else if (i < cascades.size() - 1) {
				parent_average = cascades[i + 1].lightprobe_average_tex;
			} else {
				parent_average = cascades[i - 1].lightprobe_average_tex; //to use something, but it won't be used
			}
			u.append_id(parent_average);
			uniforms.push_back(u);
		}
		{
			RD::Uniform u;
			u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
			u.binding = 14;
			u.append_id(ambient_texture);
			uniforms.push_back(u);
		}

		cascades[i].integrate_uniform_set = RD::get_singleton()->uniform_set_create(uniforms, gi->sdfgi_shader.integrate.version_get_shader(gi->sdfgi_shader.integrate_shader, 0), 0);
	}
#endif
	bounce_feedback = RendererSceneRenderRD::get_singleton()->environment_get_sdfgi_bounce_feedback(p_env);
	energy = RendererSceneRenderRD::get_singleton()->environment_get_sdfgi_energy(p_env);
	normal_bias = RendererSceneRenderRD::get_singleton()->environment_get_sdfgi_normal_bias(p_env);
	probe_bias = RendererSceneRenderRD::get_singleton()->environment_get_sdfgi_probe_bias(p_env);
	reads_sky = RendererSceneRenderRD::get_singleton()->environment_get_sdfgi_read_sky_light(p_env);
}

void GI::SDFGI::render_region(Ref<RenderSceneBuffersRD> p_render_buffers, int p_region, const PagedArray<RenderGeometryInstance *> &p_instances, float p_exposure_normalization) {
	//print_line("rendering region " + itos(p_region));
	ERR_FAIL_COND(p_render_buffers.is_null()); // we wouldn't be here if this failed but...
	AABB bounds;
	Vector3i from;
	Vector3i size;
	Vector3i scroll;
	Vector3i region_ofs;

	int cascade = get_pending_region_data(p_region, from, size, bounds, scroll, region_ofs);
	ERR_FAIL_COND(cascade < 0);

	//initialize render
	//@TODO: Add a clear region to RenderingDevice to optimize this part
	RD::get_singleton()->texture_clear(render_albedo, Color(0, 0, 0, 0), 0, 1, 0, 1);
	RD::get_singleton()->texture_clear(render_emission, Color(0, 0, 0, 0), 0, 1, 0, 1);
	RD::get_singleton()->texture_clear(render_emission_aniso, Color(0, 0, 0, 0), 0, 1, 0, 1);
	RD::get_singleton()->texture_clear(render_solid_bits[0], Color(0, 0, 0, 0), 0, 1, 0, 1);
	RD::get_singleton()->texture_clear(render_solid_bits[1], Color(0, 0, 0, 0), 0, 1, 0, 1);
	RD::get_singleton()->texture_clear(render_aniso_normals, Color(0, 0, 0, 0), 0, 1, 0, 1);
	RD::get_singleton()->buffer_clear(light_process_dispatch_buffer_render, 0, sizeof(uint32_t) * 4);

	if (scroll == SDFGI::Cascade::DIRTY_ALL) {
		RD::get_singleton()->buffer_clear(light_process_buffer_render, 0, sizeof(SDFGI::Cascade::LightProcessCell) * solid_cell_count);

		RD::get_singleton()->texture_clear(lightprobe_hit_cache_data, Color(0, 0, 0, 0), 0, 1, cascade * LIGHTPROBE_HISTORY_FRAMES, LIGHTPROBE_HISTORY_FRAMES);
		RD::get_singleton()->texture_clear(lightprobe_hit_cache_version_data, Color(0, 0, 0, 0), 0, 1, cascade * LIGHTPROBE_HISTORY_FRAMES, LIGHTPROBE_HISTORY_FRAMES);
		RD::get_singleton()->texture_clear(lightprobe_moving_average_history, Color(0, 0, 0, 0), 0, 1, cascade * LIGHTPROBE_HISTORY_FRAMES, LIGHTPROBE_HISTORY_FRAMES);
		RD::get_singleton()->texture_clear(lightprobe_moving_average, Color(0, 0, 0, 0), 0, 1, cascade, 1);
		RD::get_singleton()->texture_clear(lightprobe_specular_data, Color(0, 0, 0, 0), 0, 1, cascade, 1);
		RD::get_singleton()->texture_clear(lightprobe_diffuse_data, Color(0, 0, 0, 0), 0, 1, cascade, 1);

		// needs to be cleared to 1 (max distance)
		RD::get_singleton()->texture_clear(occlusion_process, Color(1, 1, 1, 1), 0, 1, cascade, 1);
	}

	//print_line("rendering cascade " + itos(p_region) + " objects: " + itos(p_cull_count) + " bounds: " + bounds + " from: " + from + " size: " + size + " cell size: " + rtos(cascades[cascade].cell_size));

	RendererSceneRenderRD::get_singleton()->_render_sdfgi(p_render_buffers, from, size, bounds, p_instances, render_albedo, render_emission, render_emission_aniso, render_solid_bits[0], render_solid_bits[1], render_aniso_normals, p_exposure_normalization);

	RD::get_singleton()->draw_command_begin_label("SDFGI Create Cascade SDF");

	RENDER_TIMESTAMP("> SDFGI Update SDF");
	//done rendering! must update SDF
	//clear dispatch indirect data

	SDFGIShader::PreprocessPushConstant push_constant;
	memset(&push_constant, 0, sizeof(SDFGIShader::PreprocessPushConstant));

	Vector3i dispatch_size;
	//scroll
	if (scroll != SDFGI::Cascade::DIRTY_ALL) {
		//for scroll
		push_constant.scroll[0] = scroll.x;
		push_constant.scroll[1] = scroll.y;
		push_constant.scroll[2] = scroll.z;

		for (int i = 0; i < 3; i++) {
			if (scroll[i] > 0) {
				push_constant.offset[i] = 0;
				push_constant.limit[i] = scroll[i];
				dispatch_size[i] = scroll[i];
			} else if (scroll[i] < 0) {
				push_constant.offset[i] = CASCADE_SIZE[i] + scroll[i];
				push_constant.limit[i] = CASCADE_SIZE[i];
				dispatch_size[i] = -scroll[i];
			} else {
				push_constant.offset[i] = 0;
				push_constant.limit[i] = CASCADE_SIZE[i];
				dispatch_size[i] = CASCADE_SIZE[i];
			}
		}

	} else {
		//for no scroll
		push_constant.scroll[0] = 0;
		push_constant.scroll[1] = 0;
		push_constant.scroll[2] = 0;

		push_constant.offset[0] = 0;
		push_constant.offset[1] = 0;
		push_constant.offset[2] = 0;

		push_constant.limit[0] = CASCADE_SIZE[0];
		push_constant.limit[1] = CASCADE_SIZE[1];
		push_constant.limit[2] = CASCADE_SIZE[2];

		dispatch_size.x = CASCADE_SIZE[0];
		dispatch_size.y = CASCADE_SIZE[1];
		dispatch_size.z = CASCADE_SIZE[2];
	}
	push_constant.grid_size[0] = CASCADE_SIZE[0];
	push_constant.grid_size[1] = CASCADE_SIZE[1];
	push_constant.grid_size[2] = CASCADE_SIZE[2];

	Vector3i probe_axis_count = CASCADE_SIZE / PROBE_CELLS + Vector3i(1, 1, 1);

	push_constant.probe_axis_size[0] = probe_axis_count[0];
	push_constant.probe_axis_size[1] = probe_axis_count[1];
	push_constant.probe_axis_size[2] = probe_axis_count[2];

	push_constant.region_world_pos[0] = region_ofs[0];
	push_constant.region_world_pos[1] = region_ofs[1];
	push_constant.region_world_pos[2] = region_ofs[2];

	if (cascade < int(cascades.size() - 1) && scroll != SDFGI::Cascade::DIRTY_ALL) {
		Vector3 upper_position = cascades[cascade + 1].position;

		// Get proper upper cascade position (before scroll)
		for (int k = 0; k < 3; k++) {
			if (cascades[cascade + 1].dirty_regions[k] != 0) {
				upper_position[k] -= cascades[cascade + 1].dirty_regions[k];
			}
		}

		push_constant.upper_region_world_pos[0] = upper_position[0] / REGION_CELLS;
		push_constant.upper_region_world_pos[1] = upper_position[1] / REGION_CELLS;
		push_constant.upper_region_world_pos[2] = upper_position[2] / REGION_CELLS;

	} else {
		push_constant.upper_region_world_pos[0] = 0;
		push_constant.upper_region_world_pos[1] = 0;
		push_constant.upper_region_world_pos[2] = 0;
	}

	push_constant.probe_cells = PROBE_CELLS;
	push_constant.cascade_count = cascades.size();

	push_constant.ray_hit_cache_frames = LIGHTPROBE_HISTORY_FRAMES;

	push_constant.cascade = cascade;

	cascades[cascade].latest_version++;

	push_constant.region_version = cascades[cascade].latest_version;

	//full size jumpflood
	RENDER_TIMESTAMP("SDFGI Scroll");

	RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();

	if (scroll != SDFGI::Cascade::DIRTY_ALL) {
		// Scroll happened

		{ // Light Scroll
			RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->sdfgi_shader.preprocess_pipeline[SDFGIShader::PRE_PROCESS_LIGHT_SCROLL]);
			RID uniform_set = UniformSetCacheRD::get_singleton()->get_cache(
					gi->sdfgi_shader.preprocess_shader_version[SDFGIShader::PRE_PROCESS_LIGHT_SCROLL],
					0,
					RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 1, light_tex_data),
					RD::Uniform(RD::UNIFORM_TYPE_STORAGE_BUFFER, 5, light_process_dispatch_buffer_render),
					RD::Uniform(RD::UNIFORM_TYPE_STORAGE_BUFFER, 6, light_process_buffer_render),
					RD::Uniform(RD::UNIFORM_TYPE_STORAGE_BUFFER, 7, cascades[cascade].light_process_dispatch_buffer_copy),
					RD::Uniform(RD::UNIFORM_TYPE_STORAGE_BUFFER, 8, cascades[cascade].light_process_buffer)

			);

			SDFGIShader::PreprocessPushConstant push_constant_scroll = push_constant;

			for (int i = 0; i < 3; i++) {
				if (scroll[i] > 0) {
					push_constant_scroll.limit[i] = CASCADE_SIZE[i] - scroll[i];
					push_constant_scroll.offset[i] = 1; //+1 because one extra is rendered below for consistency with neighbouring voxels.
				} else if (scroll[i] < 0) {
					push_constant_scroll.limit[i] = CASCADE_SIZE[i] - 1; // -1 because one extra is rendered below for consistency with neighbouring voxels.
					push_constant_scroll.offset[i] = -scroll[i];
				} else {
					push_constant_scroll.limit[i] = CASCADE_SIZE[i];
					push_constant_scroll.offset[i] = 0;
				}
			}

			RD::get_singleton()->compute_list_bind_uniform_set(compute_list, uniform_set, 0);
			RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant_scroll, sizeof(SDFGIShader::PreprocessPushConstant));
			RD::get_singleton()->compute_list_dispatch_indirect(compute_list, cascades[cascade].light_process_dispatch_buffer, 0);
		}

		{ // Probe Scroll
			RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->sdfgi_shader.preprocess_pipeline[SDFGIShader::PRE_PROCESS_LIGHTPROBE_SCROLL]);

			RID uniform_set = UniformSetCacheRD::get_singleton()->get_cache(
					gi->sdfgi_shader.preprocess_shader_version[SDFGIShader::PRE_PROCESS_LIGHTPROBE_SCROLL],
					0,
					RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 1, lightprobe_specular_data),
					RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 2, lightprobe_diffuse_data),
					RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 3, lightprobe_ambient_data),
					RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 4, lightprobe_hit_cache_data),
					RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 5, lightprobe_moving_average_history),
					RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 6, lightprobe_moving_average),
					RD::Uniform(RD::UNIFORM_TYPE_TEXTURE, 7, occlusion_tex),
					RD::Uniform(RD::UNIFORM_TYPE_SAMPLER, 8, RendererRD::MaterialStorage::get_singleton()->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED)));

			RD::get_singleton()->compute_list_bind_uniform_set(compute_list, uniform_set, 0);

			Vector3i dispatch_cells = dispatch_size / PROBE_CELLS + Vector3i(1, 1, 1);

			SDFGIShader::PreprocessPushConstant push_constant_scroll = push_constant;

			for (int i = 0; i < 3; i++) {
				if (scroll[i] < 0) {
					push_constant_scroll.offset[i] += REGION_CELLS;
					dispatch_cells[i]--;
				} else if (scroll[i] > 0) {
					dispatch_cells[i]--;
				}
			}

			RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant_scroll, sizeof(SDFGIShader::PreprocessPushConstant));
			RD::get_singleton()->compute_list_dispatch(compute_list, dispatch_cells.x, dispatch_cells.y, dispatch_cells.z);
		}
	}

	{
		RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->sdfgi_shader.preprocess_pipeline[SDFGIShader::PRE_PROCESS_OCCLUSION]);

		RID uniform_set = UniformSetCacheRD::get_singleton()->get_cache(
				gi->sdfgi_shader.preprocess_shader_version[SDFGIShader::PRE_PROCESS_OCCLUSION],
				0,
				RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 1, render_solid_bits[0]),
				RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 2, render_solid_bits[1]),
				RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 3, render_aniso_normals),
				RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 4, occlusion_process));

		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, uniform_set, 0);
		RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(SDFGIShader::PreprocessPushConstant));
		RD::get_singleton()->compute_list_dispatch(compute_list, dispatch_size.x / PROBE_CELLS, dispatch_size.y / PROBE_CELLS, dispatch_size.z / PROBE_CELLS);
	}

	{
		RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->sdfgi_shader.preprocess_pipeline[SDFGIShader::PRE_PROCESS_REGION_STORE]);

		RID uniform_set = UniformSetCacheRD::get_singleton()->get_cache(
				gi->sdfgi_shader.preprocess_shader_version[SDFGIShader::PRE_PROCESS_REGION_STORE],
				0,
				RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 1, render_aniso_normals),
				RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 2, voxel_bits_tex),
				RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 3, voxel_region_tex),
				RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 4, region_version_data));

		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, uniform_set, 0);
		RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(SDFGIShader::PreprocessPushConstant));
		RD::get_singleton()->compute_list_dispatch_threads(compute_list, dispatch_size.x, dispatch_size.y, dispatch_size.z);
	}

	RD::get_singleton()->compute_list_add_barrier(compute_list); // store needs another barrier

	RENDER_TIMESTAMP("SDFGI Store SDF");

	{
		RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->sdfgi_shader.preprocess_pipeline[SDFGIShader::PRE_PROCESS_OCCLUSION_STORE]);

		RID uniform_set = UniformSetCacheRD::get_singleton()->get_cache(
				gi->sdfgi_shader.preprocess_shader_version[SDFGIShader::PRE_PROCESS_OCCLUSION_STORE],
				0,
				RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 1, occlusion_process),
				RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 2, occlusion_tex),
				RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 3, lightprobe_neighbour_map));

		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, uniform_set, 0);
		RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(SDFGIShader::PreprocessPushConstant));
		RD::get_singleton()->compute_list_dispatch(compute_list, dispatch_size.x / PROBE_CELLS + 1, dispatch_size.y / PROBE_CELLS + 1, dispatch_size.z / PROBE_CELLS + 1);
	}

	RD::get_singleton()->compute_list_add_barrier(compute_list);

	{
		// Storing light happens last (after barrier) because it needs occlusion information.
		RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->sdfgi_shader.preprocess_pipeline[SDFGIShader::PRE_PROCESS_LIGHT_STORE]);
		RID uniform_set = UniformSetCacheRD::get_singleton()->get_cache(
				gi->sdfgi_shader.preprocess_shader_version[SDFGIShader::PRE_PROCESS_LIGHT_STORE],
				0,
				RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 1, render_albedo),
				RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 2, render_emission),
				RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 3, render_emission_aniso),
				RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 4, render_aniso_normals),
				RD::Uniform(RD::UNIFORM_TYPE_STORAGE_BUFFER, 5, light_process_dispatch_buffer_render),
				RD::Uniform(RD::UNIFORM_TYPE_STORAGE_BUFFER, 6, light_process_buffer_render),
				RD::Uniform(RD::UNIFORM_TYPE_TEXTURE, 7, occlusion_tex), // read as texture since we need to sample
				RD::Uniform(RD::UNIFORM_TYPE_SAMPLER, 8, RendererRD::MaterialStorage::get_singleton()->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED)));

		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, uniform_set, 0);

		Vector3i store_size = dispatch_size;

		if (scroll != SDFGI::Cascade::DIRTY_ALL) {
			for (int i = 0; i < 3; i++) {
				if (scroll[i] > 0) {
					push_constant.offset[i] = 0;
					push_constant.limit[i] = scroll[i] + 1; //extra voxel to properly store light
					store_size[i] += 1;
				} else if (scroll[i] < 0) {
					push_constant.offset[i] = CASCADE_SIZE[i] + scroll[i] - 1;
					push_constant.limit[i] = CASCADE_SIZE[i];
					store_size[i] += 1;
				} else {
					push_constant.offset[i] = 0;
					push_constant.limit[i] = CASCADE_SIZE[i];
				}
			}
		}

		RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(SDFGIShader::PreprocessPushConstant));
		RD::get_singleton()->compute_list_dispatch_threads(compute_list, store_size.x, store_size.y, store_size.z);
		// Store processed ones into cascade
		SWAP(light_process_dispatch_buffer_render, cascades[cascade].light_process_dispatch_buffer);
		SWAP(light_process_buffer_render, cascades[cascade].light_process_buffer);

		cascades[cascade].static_lights_dirty = true;
		cascades[cascade].dynamic_lights_dirty = true;

		RD::get_singleton()->compute_list_add_barrier(compute_list);
	}

	RD::get_singleton()->compute_list_end(compute_list);

	RD::get_singleton()->buffer_copy(cascades[cascade].light_process_dispatch_buffer, cascades[cascade].light_process_dispatch_buffer_copy, 0, 0, sizeof(uint32_t) * 4);

	cascades[cascade].baked_exposure_normalization = p_exposure_normalization;

#if 0
		if (cascades[cascade].dirty_regions != SDFGI::Cascade::DIRTY_ALL) {
			RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();

			Vector3i dirty = cascades[cascade].dirty_regions;
			Vector3i groups;
			groups.x = CASCADE_SIZE - ABS(dirty.x);
			groups.y = CASCADE_SIZE - ABS(dirty.y);
			groups.z = CASCADE_SIZE - ABS(dirty.z);

			RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(SDFGIShader::PreprocessPushConstant));
			RD::get_singleton()->compute_list_dispatch_threads(compute_list, groups.x, groups.y, groups.z);

			//no barrier, continue together

			{
				//scroll probes and their history also

				SDFGIShader::IntegratePushConstant ipush_constant;
				ipush_constant.grid_size[1] = CASCADE_SIZE;
				ipush_constant.grid_size[2] = CASCADE_SIZE;
				ipush_constant.grid_size[0] = CASCADE_SIZE;
				ipush_constant.max_cascades = cascades.size();
				ipush_constant.probe_axis_size = probe_axis_count;
				ipush_constant.history_index = 0;
				ipush_constant.history_size = history_size;
				ipush_constant.ray_count = 0;
				ipush_constant.ray_bias = 0;
				ipush_constant.sky_mode = 0;
				ipush_constant.sky_energy = 0;
				ipush_constant.sky_color[0] = 0;
				ipush_constant.sky_color[1] = 0;
				ipush_constant.sky_color[2] = 0;
				ipush_constant.y_mult = y_mult;
				ipush_constant.store_ambient_texture = false;

				ipush_constant.image_size[0] = probe_axis_count * probe_axis_count;
				ipush_constant.image_size[1] = probe_axis_count;

				int32_t probe_divisor = CASCADE_SIZE / SDFGI::PROBE_DIVISOR;
				ipush_constant.cascade = cascade;
				ipush_constant.world_offset[0] = cascades[cascade].position.x / probe_divisor;
				ipush_constant.world_offset[1] = cascades[cascade].position.y / probe_divisor;
				ipush_constant.world_offset[2] = cascades[cascade].position.z / probe_divisor;

				ipush_constant.scroll[0] = dirty.x / probe_divisor;
				ipush_constant.scroll[1] = dirty.y / probe_divisor;
				ipush_constant.scroll[2] = dirty.z / probe_divisor;

				RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->sdfgi_shader.integrate_pipeline[SDFGIShader::INTEGRATE_MODE_SCROLL]);
				RD::get_singleton()->compute_list_bind_uniform_set(compute_list, cascades[cascade].integrate_uniform_set, 0);
				RD::get_singleton()->compute_list_bind_uniform_set(compute_list, gi->sdfgi_shader.integrate_default_sky_uniform_set, 1);
				RD::get_singleton()->compute_list_set_push_constant(compute_list, &ipush_constant, sizeof(SDFGIShader::IntegratePushConstant));
				RD::get_singleton()->compute_list_dispatch_threads(compute_list, probe_axis_count * probe_axis_count, probe_axis_count, 1);

				RD::get_singleton()->compute_list_add_barrier(compute_list);

				RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->sdfgi_shader.integrate_pipeline[SDFGIShader::INTEGRATE_MODE_SCROLL_STORE]);
				RD::get_singleton()->compute_list_bind_uniform_set(compute_list, cascades[cascade].integrate_uniform_set, 0);
				RD::get_singleton()->compute_list_bind_uniform_set(compute_list, gi->sdfgi_shader.integrate_default_sky_uniform_set, 1);
				RD::get_singleton()->compute_list_set_push_constant(compute_list, &ipush_constant, sizeof(SDFGIShader::IntegratePushConstant));
				RD::get_singleton()->compute_list_dispatch_threads(compute_list, probe_axis_count * probe_axis_count, probe_axis_count, 1);

				RD::get_singleton()->compute_list_add_barrier(compute_list);

				if (bounce_feedback > 0.0) {
					//multibounce requires this to be stored so direct light can read from it

					RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->sdfgi_shader.integrate_pipeline[SDFGIShader::INTEGRATE_MODE_STORE]);

					//convert to octahedral to store
					ipush_constant.image_size[0] *= SDFGI::LIGHTPROBE_OCT_SIZE;
					ipush_constant.image_size[1] *= SDFGI::LIGHTPROBE_OCT_SIZE;

					RD::get_singleton()->compute_list_bind_uniform_set(compute_list, cascades[cascade].integrate_uniform_set, 0);
					RD::get_singleton()->compute_list_bind_uniform_set(compute_list, gi->sdfgi_shader.integrate_default_sky_uniform_set, 1);
					RD::get_singleton()->compute_list_set_push_constant(compute_list, &ipush_constant, sizeof(SDFGIShader::IntegratePushConstant));
					RD::get_singleton()->compute_list_dispatch_threads(compute_list, probe_axis_count * probe_axis_count * SDFGI::LIGHTPROBE_OCT_SIZE, probe_axis_count * SDFGI::LIGHTPROBE_OCT_SIZE, 1);
				}
			}

			//ok finally barrier
			RD::get_singleton()->compute_list_end();
		}

		//clear dispatch indirect data
		uint32_t dispatch_indirct_data[4] = { 0, 0, 0, 0 };
		RD::get_singleton()->buffer_update(cascades[cascade].solid_cell_dispatch_buffer, 0, sizeof(uint32_t) * 4, dispatch_indirct_data);

		RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();

		bool half_size = true; //much faster, very little difference
		static const int optimized_jf_group_size = 8;

		if (half_size) {
			push_constant.grid_size >>= 1;

			uint32_t cascade_half_size = CASCADE_SIZE >> 1;
			RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->sdfgi_shader.preprocess_pipeline[SDFGIShader::PRE_PROCESS_JUMP_FLOOD_INITIALIZE_HALF]);
			RD::get_singleton()->compute_list_bind_uniform_set(compute_list, sdf_initialize_half_uniform_set, 0);
			RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(SDFGIShader::PreprocessPushConstant));
			RD::get_singleton()->compute_list_dispatch_threads(compute_list, cascade_half_size, cascade_half_size, cascade_half_size);
			RD::get_singleton()->compute_list_add_barrier(compute_list);

			//must start with regular jumpflood

			push_constant.half_size = true;
			{
				RENDER_TIMESTAMP("SDFGI Jump Flood (Half-Size)");

				uint32_t s = cascade_half_size;

				RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->sdfgi_shader.preprocess_pipeline[SDFGIShader::PRE_PROCESS_JUMP_FLOOD]);

				int jf_us = 0;
				//start with regular jump flood for very coarse reads, as this is impossible to optimize
				while (s > 1) {
					s /= 2;
					push_constant.step_size = s;
					RD::get_singleton()->compute_list_bind_uniform_set(compute_list, jump_flood_half_uniform_set[jf_us], 0);
					RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(SDFGIShader::PreprocessPushConstant));
					RD::get_singleton()->compute_list_dispatch_threads(compute_list, cascade_half_size, cascade_half_size, cascade_half_size);
					RD::get_singleton()->compute_list_add_barrier(compute_list);
					jf_us = jf_us == 0 ? 1 : 0;

					if (cascade_half_size / (s / 2) >= optimized_jf_group_size) {
						break;
					}
				}

				RENDER_TIMESTAMP("SDFGI Jump Flood Optimized (Half-Size)");

				//continue with optimized jump flood for smaller reads
				RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->sdfgi_shader.preprocess_pipeline[SDFGIShader::PRE_PROCESS_JUMP_FLOOD_OPTIMIZED]);
				while (s > 1) {
					s /= 2;
					push_constant.step_size = s;
					RD::get_singleton()->compute_list_bind_uniform_set(compute_list, jump_flood_half_uniform_set[jf_us], 0);
					RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(SDFGIShader::PreprocessPushConstant));
					RD::get_singleton()->compute_list_dispatch_threads(compute_list, cascade_half_size, cascade_half_size, cascade_half_size);
					RD::get_singleton()->compute_list_add_barrier(compute_list);
					jf_us = jf_us == 0 ? 1 : 0;
				}
			}

			// restore grid size for last passes
			push_constant.grid_size = CASCADE_SIZE;

			RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->sdfgi_shader.preprocess_pipeline[SDFGIShader::PRE_PROCESS_JUMP_FLOOD_UPSCALE]);
			RD::get_singleton()->compute_list_bind_uniform_set(compute_list, sdf_upscale_uniform_set, 0);
			RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(SDFGIShader::PreprocessPushConstant));
			RD::get_singleton()->compute_list_dispatch_threads(compute_list, CASCADE_SIZE, CASCADE_SIZE, CASCADE_SIZE);
			RD::get_singleton()->compute_list_add_barrier(compute_list);

			//run one pass of fullsize jumpflood to fix up half size artifacts

			push_constant.half_size = false;
			push_constant.step_size = 1;
			RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->sdfgi_shader.preprocess_pipeline[SDFGIShader::PRE_PROCESS_JUMP_FLOOD_OPTIMIZED]);
			RD::get_singleton()->compute_list_bind_uniform_set(compute_list, jump_flood_uniform_set[upscale_jfa_uniform_set_index], 0);
			RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(SDFGIShader::PreprocessPushConstant));
			RD::get_singleton()->compute_list_dispatch_threads(compute_list, CASCADE_SIZE, CASCADE_SIZE, CASCADE_SIZE);
			RD::get_singleton()->compute_list_add_barrier(compute_list);

		} else {
			//full size jumpflood
			RENDER_TIMESTAMP("SDFGI Jump Flood (Full-Size)");

			RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->sdfgi_shader.preprocess_pipeline[SDFGIShader::PRE_PROCESS_JUMP_FLOOD_INITIALIZE]);
			RD::get_singleton()->compute_list_bind_uniform_set(compute_list, sdf_initialize_uniform_set, 0);
			RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(SDFGIShader::PreprocessPushConstant));
			RD::get_singleton()->compute_list_dispatch_threads(compute_list, CASCADE_SIZE, CASCADE_SIZE, CASCADE_SIZE);

			RD::get_singleton()->compute_list_add_barrier(compute_list);

			push_constant.half_size = false;
			{
				uint32_t s = CASCADE_SIZE;

				RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->sdfgi_shader.preprocess_pipeline[SDFGIShader::PRE_PROCESS_JUMP_FLOOD]);

				int jf_us = 0;
				//start with regular jump flood for very coarse reads, as this is impossible to optimize
				while (s > 1) {
					s /= 2;
					push_constant.step_size = s;
					RD::get_singleton()->compute_list_bind_uniform_set(compute_list, jump_flood_uniform_set[jf_us], 0);
					RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(SDFGIShader::PreprocessPushConstant));
					RD::get_singleton()->compute_list_dispatch_threads(compute_list, CASCADE_SIZE, CASCADE_SIZE, CASCADE_SIZE);
					RD::get_singleton()->compute_list_add_barrier(compute_list);
					jf_us = jf_us == 0 ? 1 : 0;

					if (CASCADE_SIZE / (s / 2) >= optimized_jf_group_size) {
						break;
					}
				}

				RENDER_TIMESTAMP("SDFGI Jump Flood Optimized (Full-Size)");

				//continue with optimized jump flood for smaller reads
				RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->sdfgi_shader.preprocess_pipeline[SDFGIShader::PRE_PROCESS_JUMP_FLOOD_OPTIMIZED]);
				while (s > 1) {
					s /= 2;
					push_constant.step_size = s;
					RD::get_singleton()->compute_list_bind_uniform_set(compute_list, jump_flood_uniform_set[jf_us], 0);
					RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(SDFGIShader::PreprocessPushConstant));
					RD::get_singleton()->compute_list_dispatch_threads(compute_list, CASCADE_SIZE, CASCADE_SIZE, CASCADE_SIZE);
					RD::get_singleton()->compute_list_add_barrier(compute_list);
					jf_us = jf_us == 0 ? 1 : 0;
				}
			}
		}

		RENDER_TIMESTAMP("SDFGI Occlusion");

		// occlusion
		{
			uint32_t probe_size = CASCADE_SIZE / SDFGI::PROBE_DIVISOR;
			Vector3i probe_global_pos = cascades[cascade].position / probe_size;

			RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->sdfgi_shader.preprocess_pipeline[SDFGIShader::PRE_PROCESS_OCCLUSION]);
			RD::get_singleton()->compute_list_bind_uniform_set(compute_list, occlusion_uniform_set, 0);
			for (int i = 0; i < 8; i++) {
				//dispatch all at once for performance
				Vector3i offset(i & 1, (i >> 1) & 1, (i >> 2) & 1);

				if ((probe_global_pos.x & 1) != 0) {
					offset.x = (offset.x + 1) & 1;
				}
				if ((probe_global_pos.y & 1) != 0) {
					offset.y = (offset.y + 1) & 1;
				}
				if ((probe_global_pos.z & 1) != 0) {
					offset.z = (offset.z + 1) & 1;
				}
				push_constant.probe_offset[0] = offset.x;
				push_constant.probe_offset[1] = offset.y;
				push_constant.probe_offset[2] = offset.z;
				push_constant.occlusion_index = i;
				RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(SDFGIShader::PreprocessPushConstant));

				Vector3i groups = Vector3i(probe_size + 1, probe_size + 1, probe_size + 1) - offset; //if offset, it's one less probe per axis to compute
				RD::get_singleton()->compute_list_dispatch(compute_list, groups.x, groups.y, groups.z);
			}
			RD::get_singleton()->compute_list_add_barrier(compute_list);
		}

		RENDER_TIMESTAMP("SDFGI Store");

		// store
		RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->sdfgi_shader.preprocess_pipeline[SDFGIShader::PRE_PROCESS_STORE]);
		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, cascades[cascade].sdf_store_uniform_set, 0);
		RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(SDFGIShader::PreprocessPushConstant));
		RD::get_singleton()->compute_list_dispatch_threads(compute_list, CASCADE_SIZE, CASCADE_SIZE, CASCADE_SIZE);

		RD::get_singleton()->compute_list_end();

		//clear these textures, as they will have previous garbage on next draw
		RD::get_singleton()->texture_clear(cascades[cascade].light_tex, Color(0, 0, 0, 0), 0, 1, 0, 1);
		RD::get_singleton()->texture_clear(cascades[cascade].light_aniso_0_tex, Color(0, 0, 0, 0), 0, 1, 0, 1);
		RD::get_singleton()->texture_clear(cascades[cascade].light_aniso_1_tex, Color(0, 0, 0, 0), 0, 1, 0, 1);

#endif
	RD::get_singleton()->draw_command_end_label();
	RENDER_TIMESTAMP("< SDFGI Update SDF");
}

void GI::SDFGI::free_data() {
	// we don't free things here, we handle SDFGI differently at the moment destructing the object when it needs to change.
}

GI::SDFGI::~SDFGI() {
	for (const SDFGI::Cascade &c : cascades) {
		RD::get_singleton()->free(c.light_process_buffer);
		RD::get_singleton()->free(c.light_process_dispatch_buffer);
		RD::get_singleton()->free(c.light_process_dispatch_buffer_copy);
	}

	RD::get_singleton()->free(render_albedo);
	RD::get_singleton()->free(render_emission);
	RD::get_singleton()->free(render_emission_aniso);

	RD::get_singleton()->free(render_solid_bits[0]);
	RD::get_singleton()->free(render_solid_bits[1]);

	RD::get_singleton()->free(voxel_bits_tex);
	RD::get_singleton()->free(voxel_region_tex);
	RD::get_singleton()->free(light_tex_data);
	RD::get_singleton()->free(region_version_data);

	RD::get_singleton()->free(light_process_buffer_render);
	RD::get_singleton()->free(light_process_dispatch_buffer_render);

	RD::get_singleton()->free(cascades_ubo);

	RD::get_singleton()->free(lightprobe_specular_tex);
	RD::get_singleton()->free(lightprobe_diffuse_tex);
	RD::get_singleton()->free(lightprobe_ambient_tex);
	RD::get_singleton()->free(lightprobe_hit_cache_data);
	RD::get_singleton()->free(lightprobe_hit_cache_version_data);

	if (debug_probes_scene_data_ubo.is_valid()) {
		RD::get_singleton()->free(debug_probes_scene_data_ubo);
		debug_probes_scene_data_ubo = RID();
	}

#if 0
	for (int i = 0; i < 8; i++) {
		RD::get_singleton()->free(render_occlusion[i]);
	}

	RD::get_singleton()->free(render_geom_facing);

	RD::get_singleton()->free(lightprobe_data);
	RD::get_singleton()->free(lightprobe_history_scroll);
	RD::get_singleton()->free(lightprobe_average_scroll);
	RD::get_singleton()->free(occlusion_data);
	RD::get_singleton()->free(ambient_texture);

	RD::get_singleton()->free(cascades_ubo);


	if (RD::get_singleton()->uniform_set_is_valid(debug_probes_uniform_set)) {
		RD::get_singleton()->free(debug_probes_uniform_set);
	}
	debug_probes_uniform_set = RID();

#endif
}

void GI::SDFGI::update(RID p_env, const Vector3 &p_world_position) {
	bounce_feedback = RendererSceneRenderRD::get_singleton()->environment_get_sdfgi_bounce_feedback(p_env);
	energy = RendererSceneRenderRD::get_singleton()->environment_get_sdfgi_energy(p_env);
	normal_bias = RendererSceneRenderRD::get_singleton()->environment_get_sdfgi_normal_bias(p_env);
	probe_bias = RendererSceneRenderRD::get_singleton()->environment_get_sdfgi_probe_bias(p_env);
	reads_sky = RendererSceneRenderRD::get_singleton()->environment_get_sdfgi_read_sky_light(p_env);

	int32_t drag_margin = PROBE_CELLS / 2;

	int idx = 0;
	for (SDFGI::Cascade &cascade : cascades) {
		cascade.dirty_regions = Vector3i();

		Vector3 probe_half_size = Vector3(1, 1, 1) * cascade.cell_size * float(PROBE_CELLS) * 0.5;
		probe_half_size = Vector3(0, 0, 0);

		Vector3 world_position = p_world_position;
		world_position.y *= y_mult;
		Vector3i pos_in_cascade = Vector3i((world_position + probe_half_size) / cascade.cell_size);

		for (int j = 0; j < 3; j++) {
			if (pos_in_cascade[j] < cascade.position[j]) {
				while (pos_in_cascade[j] < (cascade.position[j] - drag_margin)) {
					cascade.position[j] -= drag_margin * 2;
					cascade.dirty_regions[j] += drag_margin * 2;
				}
			} else if (pos_in_cascade[j] > cascade.position[j]) {
				while (pos_in_cascade[j] > (cascade.position[j] + drag_margin)) {
					cascade.position[j] += drag_margin * 2;
					cascade.dirty_regions[j] -= drag_margin * 2;
				}
			}

			if (cascade.dirty_regions[j] == 0) {
				continue; // not dirty
			} else if (uint32_t(ABS(cascade.dirty_regions[j])) >= uint32_t(CASCADE_SIZE[j])) {
				//moved too much, just redraw everything (make all dirty)
				cascade.dirty_regions = SDFGI::Cascade::DIRTY_ALL;
				break;
			}
		}
#ifdef DIRTY_ALL_FRAMES
		// DEBUG
		cascade.dirty_regions = SDFGI::Cascade::DIRTY_ALL;
		break;
#endif
		if (cascade.dirty_regions != Vector3i() && cascade.dirty_regions != SDFGI::Cascade::DIRTY_ALL) {
			//see how much the total dirty volume represents from the total volume
			uint32_t total_volume = CASCADE_H_SIZE * CASCADE_V_SIZE * CASCADE_H_SIZE;
			uint32_t safe_volume = 1;
			for (int j = 0; j < 3; j++) {
				safe_volume *= CASCADE_SIZE[j] - ABS(cascade.dirty_regions[j]);
			}
			uint32_t dirty_volume = total_volume - safe_volume;
			if (dirty_volume > (safe_volume / 2)) {
				//more than half the volume is dirty, make all dirty so its only rendered once
				cascade.dirty_regions = SDFGI::Cascade::DIRTY_ALL;
			}
		}

		if (cascade.dirty_regions != Vector3i()) {
			uint32_t dirty_mask = 0;
			for (int j = 0; j < 3; j++) {
				if (cascade.dirty_regions[j] != 0) {
					dirty_mask |= (1 << j);
				}
			}
			// Notify all cascades equal or smaller than this that some motion happened.
			// In an axis, which is used for light ray cache invalidation.

			for (int j = idx; j >= 0; j--) {
				cascades[j].motion_accum |= dirty_mask;
			}
		}

		idx++;
	}

	update_frame++;
}

void GI::SDFGI::update_light() {
	RD::get_singleton()->draw_command_begin_label("SDFGI Update dynamic Light");

	/* Update dynamic light */

	RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();
	RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->sdfgi_shader.direct_light_pipeline[SDFGIShader::DIRECT_LIGHT_MODE_DYNAMIC]);

	SDFGIShader::DirectLightPushConstant push_constant;

	push_constant.grid_size[0] = CASCADE_SIZE[0];
	push_constant.grid_size[1] = CASCADE_SIZE[1];
	push_constant.grid_size[2] = CASCADE_SIZE[2];
	push_constant.max_cascades = cascades.size();
	push_constant.probe_axis_size[0] = CASCADE_SIZE[0] / PROBE_CELLS + 1;
	push_constant.probe_axis_size[1] = CASCADE_SIZE[1] / PROBE_CELLS + 1;
	push_constant.probe_axis_size[2] = CASCADE_SIZE[2] / PROBE_CELLS + 1;

	push_constant.bounce_feedback = bounce_feedback;
	push_constant.y_mult = y_mult;
	push_constant.use_occlusion = uses_occlusion;
	push_constant.probe_cell_size = PROBE_CELLS;

	for (uint32_t i = 0; i < cascades.size(); i++) {
		SDFGI::Cascade &cascade = cascades[i];
		push_constant.light_count = cascade_dynamic_light_count[i];
		push_constant.cascade = i;
		push_constant.dirty_dynamic_update = cascades[i].dynamic_lights_dirty;

		cascades[i].dynamic_lights_dirty = false;

		if (gi->sdfgi_frames_to_update_light == RS::ENV_SDFGI_UPDATE_LIGHT_IN_1_FRAME) {
			push_constant.process_offset = 0;
			push_constant.process_increment = 1;
		} else {
			static const uint32_t frames_to_update_table[RS::ENV_SDFGI_UPDATE_LIGHT_MAX] = {
				1, 2, 4, 8, 16
			};

			uint32_t frames_to_update = frames_to_update_table[gi->sdfgi_frames_to_update_light];

			push_constant.process_offset = RSG::rasterizer->get_frame_number() % frames_to_update;
			push_constant.process_increment = frames_to_update;
		}

		RID uniform_set = UniformSetCacheRD::get_singleton()->get_cache(
				gi->sdfgi_shader.direct_light_shader_version[SDFGIShader::DIRECT_LIGHT_MODE_DYNAMIC],
				0,
				RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 1, voxel_bits_tex),
				RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 2, voxel_region_tex),
				RD::Uniform(RD::UNIFORM_TYPE_SAMPLER, 3, RendererRD::MaterialStorage::get_singleton()->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED)),
				RD::Uniform(RD::UNIFORM_TYPE_STORAGE_BUFFER, 4, cascade.light_process_dispatch_buffer_copy),
				RD::Uniform(RD::UNIFORM_TYPE_STORAGE_BUFFER, 5, cascade.light_process_buffer),
				RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 6, light_tex_data),
				RD::Uniform(RD::UNIFORM_TYPE_UNIFORM_BUFFER, 7, cascades_ubo),
				RD::Uniform(RD::UNIFORM_TYPE_STORAGE_BUFFER, 8, cascade.light_position_bufer),
				RD::Uniform(RD::UNIFORM_TYPE_TEXTURE, 9, lightprobe_diffuse_tex));

		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, uniform_set, 0);

		RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(SDFGIShader::DirectLightPushConstant));
		RD::get_singleton()->compute_list_dispatch_indirect(compute_list, cascade.light_process_dispatch_buffer, 0);
	}
	RD::get_singleton()->compute_list_end(RD::BARRIER_MASK_COMPUTE);
	RD::get_singleton()->draw_command_end_label();
}

void GI::SDFGI::update_probes(RID p_env, SkyRD::Sky *p_sky) {
	RD::get_singleton()->draw_command_begin_label("SDFGI Update Probes");

	SDFGIShader::IntegratePushConstant push_constant;
	push_constant.grid_size[0] = CASCADE_SIZE[0];
	push_constant.grid_size[1] = CASCADE_SIZE[1];
	push_constant.grid_size[2] = CASCADE_SIZE[2];
	push_constant.max_cascades = cascades.size();
	push_constant.probe_axis_size[0] = CASCADE_SIZE[0] / PROBE_CELLS + 1;
	push_constant.probe_axis_size[1] = CASCADE_SIZE[1] / PROBE_CELLS + 1;
	push_constant.probe_axis_size[2] = CASCADE_SIZE[2] / PROBE_CELLS + 1;
	push_constant.history_index = render_pass % LIGHTPROBE_HISTORY_FRAMES;
	push_constant.history_size = LIGHTPROBE_HISTORY_FRAMES;
	push_constant.ray_bias = probe_bias;
	push_constant.store_ambient_texture = RendererSceneRenderRD::get_singleton()->environment_get_volumetric_fog_enabled(p_env);
	push_constant.sky_mode = SDFGIShader::IntegratePushConstant::SKY_MODE_DISABLED;
	push_constant.y_mult = y_mult;

	RID integrate_sky_uniform_set;

	if (reads_sky && p_env.is_valid()) {
		push_constant.sky_energy = RendererSceneRenderRD::get_singleton()->environment_get_bg_energy_multiplier(p_env);

		if (RendererSceneRenderRD::get_singleton()->environment_get_background(p_env) == RS::ENV_BG_CLEAR_COLOR) {
			push_constant.sky_mode = SDFGIShader::IntegratePushConstant::SKY_MODE_COLOR;
			Color c = RSG::texture_storage->get_default_clear_color().srgb_to_linear();
			push_constant.sky_color[0] = c.r;
			push_constant.sky_color[1] = c.g;
			push_constant.sky_color[2] = c.b;
		} else if (RendererSceneRenderRD::get_singleton()->environment_get_background(p_env) == RS::ENV_BG_COLOR) {
			push_constant.sky_mode = SDFGIShader::IntegratePushConstant::SKY_MODE_COLOR;
			Color c = RendererSceneRenderRD::get_singleton()->environment_get_bg_color(p_env);
			push_constant.sky_color[0] = c.r;
			push_constant.sky_color[1] = c.g;
			push_constant.sky_color[2] = c.b;

		} else if (RendererSceneRenderRD::get_singleton()->environment_get_background(p_env) == RS::ENV_BG_SKY) {
			if (p_sky && p_sky->radiance.is_valid()) {
				integrate_sky_uniform_set = UniformSetCacheRD::get_singleton()->get_cache(
						gi->sdfgi_shader.integrate.version_get_shader(gi->sdfgi_shader.integrate_shader, 0),
						1,
						RD::Uniform(RD::UNIFORM_TYPE_TEXTURE, 0, p_sky->radiance),
						RD::Uniform(RD::UNIFORM_TYPE_SAMPLER, 1, RendererRD::MaterialStorage::get_singleton()->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR_WITH_MIPMAPS, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED)));

				push_constant.sky_mode = SDFGIShader::IntegratePushConstant::SKY_MODE_SKY;
			}
		}
	}

	if (integrate_sky_uniform_set.is_null()) {
		integrate_sky_uniform_set = UniformSetCacheRD::get_singleton()->get_cache(
				gi->sdfgi_shader.integrate.version_get_shader(gi->sdfgi_shader.integrate_shader, 0),
				1,
				RD::Uniform(RD::UNIFORM_TYPE_TEXTURE, 0, RendererRD::TextureStorage::get_singleton()->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_CUBEMAP_WHITE)),
				RD::Uniform(RD::UNIFORM_TYPE_SAMPLER, 1, RendererRD::MaterialStorage::get_singleton()->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR_WITH_MIPMAPS, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED)));
	}

	render_pass++;

	RID integrate_unifom_set = UniformSetCacheRD::get_singleton()->get_cache(
			gi->sdfgi_shader.integrate.version_get_shader(gi->sdfgi_shader.integrate_shader, 0),
			0,
			RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 1, voxel_bits_tex),
			RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 2, voxel_region_tex),
			RD::Uniform(RD::UNIFORM_TYPE_TEXTURE, 3, light_tex),
			RD::Uniform(RD::UNIFORM_TYPE_SAMPLER, 4, RendererRD::MaterialStorage::get_singleton()->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED)),
			RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 5, lightprobe_specular_data),
			RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 6, lightprobe_diffuse_data),
			RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 7, lightprobe_ambient_data),
			RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 8, lightprobe_hit_cache_data),
			RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 9, lightprobe_hit_cache_version_data),
			RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 10, region_version_data),
			RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 11, lightprobe_moving_average_history),
			RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 12, lightprobe_moving_average),
			RD::Uniform(RD::UNIFORM_TYPE_UNIFORM_BUFFER, 13, cascades_ubo));

	RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();
	RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->sdfgi_shader.integrate_pipeline[SDFGIShader::INTEGRATE_MODE_PROCESS]);

	Vector3i probe_axis_count = CASCADE_SIZE / PROBE_CELLS + Vector3i(1, 1, 1);

	int32_t probe_divisor = PROBE_CELLS;
	for (uint32_t i = 0; i < cascades.size(); i++) {
		push_constant.cascade = i;
		push_constant.motion_accum = cascades[i].motion_accum;
		cascades[i].motion_accum = 0; //clear after use.

		push_constant.world_offset[0] = cascades[i].position.x / probe_divisor;
		push_constant.world_offset[1] = cascades[i].position.y / probe_divisor;
		push_constant.world_offset[2] = cascades[i].position.z / probe_divisor;

		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, integrate_unifom_set, 0);
		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, integrate_sky_uniform_set, 1);

		RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(SDFGIShader::IntegratePushConstant));
		RD::get_singleton()->compute_list_dispatch_threads(compute_list, probe_axis_count.x * LIGHTPROBE_OCT_SIZE, probe_axis_count.y * probe_axis_count.z * LIGHTPROBE_OCT_SIZE, 1);
	}

	if (using_filter) {
		RD::get_singleton()->compute_list_add_barrier(compute_list);

		RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->sdfgi_shader.integrate_pipeline[SDFGIShader::INTEGRATE_MODE_FILTER]);

		integrate_unifom_set = UniformSetCacheRD::get_singleton()->get_cache(
				gi->sdfgi_shader.integrate_shader_version[SDFGIShader::INTEGRATE_MODE_FILTER],
				0,
				RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 1, lightprobe_diffuse_data),
				RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 2, lightprobe_diffuse_filter_data),
				RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 3, lightprobe_neighbour_map));

		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, integrate_unifom_set, 0);

		for (uint32_t i = 0; i < cascades.size(); i++) {
			push_constant.cascade = i;
			RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(SDFGIShader::IntegratePushConstant));

			push_constant.world_offset[0] = cascades[i].position.x / probe_divisor;
			push_constant.world_offset[1] = cascades[i].position.y / probe_divisor;
			push_constant.world_offset[2] = cascades[i].position.z / probe_divisor;

			RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(SDFGIShader::IntegratePushConstant));
			RD::get_singleton()->compute_list_dispatch_threads(compute_list, probe_axis_count.x * LIGHTPROBE_OCT_SIZE, probe_axis_count.y * probe_axis_count.z * LIGHTPROBE_OCT_SIZE, 1);
		}
	}

	RD::get_singleton()->compute_list_end();

	RD::get_singleton()->draw_command_end_label();
}

void GI::SDFGI::store_probes() {
#if 0
	RD::get_singleton()->barrier(RD::BARRIER_MASK_COMPUTE, RD::BARRIER_MASK_COMPUTE);
	RD::get_singleton()->draw_command_begin_label("SDFGI Store Probes");

	SDFGIShader::IntegratePushConstant push_constant;
	push_constant.grid_size[1] = CASCADE_SIZE;
	push_constant.grid_size[2] = CASCADE_SIZE;
	push_constant.grid_size[0] = CASCADE_SIZE;
	push_constant.max_cascades = cascades.size();
	push_constant.probe_axis_size = probe_axis_count;
	push_constant.history_index = render_pass % history_size;
	push_constant.history_size = history_size;
	static const uint32_t ray_count[RS::ENV_SDFGI_RAY_COUNT_MAX] = { 4, 8, 16, 32, 64, 96, 128 };
	push_constant.ray_count = ray_count[gi->sdfgi_ray_count];
	push_constant.ray_bias = probe_bias;
	push_constant.image_size[0] = probe_axis_count * probe_axis_count;
	push_constant.image_size[1] = probe_axis_count;
	push_constant.store_ambient_texture = false;

	push_constant.sky_mode = 0;
	push_constant.y_mult = y_mult;

	// Then store values into the lightprobe texture. Separating these steps has a small performance hit, but it allows for multiple bounces
	RENDER_TIMESTAMP("Average SDFGI Probes");

	RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();
	RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->sdfgi_shader.integrate_pipeline[SDFGIShader::INTEGRATE_MODE_STORE]);

	//convert to octahedral to store
	push_constant.image_size[0] *= SDFGI::LIGHTPROBE_OCT_SIZE;
	push_constant.image_size[1] *= SDFGI::LIGHTPROBE_OCT_SIZE;

	for (uint32_t i = 0; i < cascades.size(); i++) {
		push_constant.cascade = i;
		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, cascades[i].integrate_uniform_set, 0);
		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, gi->sdfgi_shader.integrate_default_sky_uniform_set, 1);
		RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(SDFGIShader::IntegratePushConstant));
		RD::get_singleton()->compute_list_dispatch_threads(compute_list, probe_axis_count * probe_axis_count * SDFGI::LIGHTPROBE_OCT_SIZE, probe_axis_count * SDFGI::LIGHTPROBE_OCT_SIZE, 1);
	}

	RD::get_singleton()->compute_list_end(RD::BARRIER_MASK_COMPUTE);

	RD::get_singleton()->draw_command_end_label();
#endif
}

int GI::SDFGI::get_pending_region_count() const {
	int dirty_count = 0;
	for (const RendererRD::GI::SDFGI::Cascade &c : cascades) {
		if (c.dirty_regions == RendererRD::GI::SDFGI::Cascade::DIRTY_ALL) {
			dirty_count++;
		} else {
			for (int j = 0; j < 3; j++) {
				if (c.dirty_regions[j] != 0) {
					dirty_count++;
				}
			}
		}
	}

	return dirty_count;
}

int GI::SDFGI::get_pending_region_data(int p_region, Vector3i &r_local_offset, Vector3i &r_local_size, AABB &r_bounds, Vector3i &r_scroll, Vector3i &r_region_world) const {
	// higher cascades need to be procesed first
	int dirty_count = 0;
	for (uint32_t i = 0; i < cascades.size(); i++) {
		const SDFGI::Cascade &c = cascades[i];

		if (c.dirty_regions == SDFGI::Cascade::DIRTY_ALL) {
			if (dirty_count == p_region) {
				r_local_offset = Vector3i();
				r_local_size = CASCADE_SIZE;
				r_scroll = SDFGI::Cascade::DIRTY_ALL;
				r_region_world = c.position / REGION_CELLS;

				r_bounds.position = Vector3((Vector3i(1, 1, 1) * -(CASCADE_SIZE >> 1) + c.position)) * c.cell_size * Vector3(1, 1.0 / y_mult, 1);
				r_bounds.size = Vector3(r_local_size) * c.cell_size * Vector3(1, 1.0 / y_mult, 1);
				return i;
			}
			dirty_count++;
		} else {
			for (int j = 0; j < 3; j++) {
				if (c.dirty_regions[j] != 0) {
					if (dirty_count == p_region) {
						Vector3i from = Vector3i(0, 0, 0);
						Vector3i to = CASCADE_SIZE;

						r_scroll = Vector3i();
						r_scroll[j] = c.dirty_regions[j];

						if (c.dirty_regions[j] > 0) {
							//fill from the beginning
							to[j] = c.dirty_regions[j] + 2; // 2 extra voxels needed to rebuild light properly
						} else {
							//fill from the end
							from[j] = to[j] + c.dirty_regions[j] - 2; // 2 extra voxels needed to rebuild light properly
						}

						r_local_offset = from;
						r_local_size = to - from;

						Vector3i cascade_position = c.position;

						// Remove next axes positions so we don't voxelize the wrong region
						for (int k = j + 1; k < 3; k++) {
							if (c.dirty_regions[k] != 0) {
								cascade_position[k] += c.dirty_regions[k];
							}
						}

						r_region_world = cascade_position / REGION_CELLS;

						r_bounds.position = Vector3(from + Vector3i(1, 1, 1) * -Vector3(CASCADE_SIZE >> 1) + cascade_position) * c.cell_size * Vector3(1, 1.0 / y_mult, 1);
						r_bounds.size = Vector3(r_local_size) * c.cell_size * Vector3(1, 1.0 / y_mult, 1);

						return i;
					}

					dirty_count++;
				}
			}
		}
	}
	return -1;
}

void GI::SDFGI::update_cascades() {
	//update cascades
	SDFGI::Cascade::UBO cascade_data[SDFGI::MAX_CASCADES];

	for (uint32_t i = 0; i < cascades.size(); i++) {
		Vector3 pos = Vector3((Vector3i(1, 1, 1) * -(CASCADE_SIZE >> 1) + cascades[i].position)) * cascades[i].cell_size;

		cascade_data[i].offset[0] = pos.x;
		cascade_data[i].offset[1] = pos.y;
		cascade_data[i].offset[2] = pos.z;
		cascade_data[i].to_cell = 1.0 / cascades[i].cell_size;
		cascade_data[i].region_world_offset[0] = cascades[i].position.x / REGION_CELLS;
		cascade_data[i].region_world_offset[1] = cascades[i].position.y / REGION_CELLS;
		cascade_data[i].region_world_offset[2] = cascades[i].position.z / REGION_CELLS;
		cascade_data[i].pad = 0;
	}

	RD::get_singleton()->buffer_update(cascades_ubo, 0, sizeof(SDFGI::Cascade::UBO) * SDFGI::MAX_CASCADES, cascade_data, RD::BARRIER_MASK_COMPUTE);
}

void GI::SDFGI::debug_draw(uint32_t p_view_count, const Projection *p_projections, const Transform3D &p_transform, int p_width, int p_height, RID p_render_target, RID p_texture, const Vector<RID> &p_texture_views) {
	RendererRD::TextureStorage *texture_storage = RendererRD::TextureStorage::get_singleton();
	RendererRD::MaterialStorage *material_storage = RendererRD::MaterialStorage::get_singleton();
	RendererRD::CopyEffects *copy_effects = RendererRD::CopyEffects::get_singleton();

	for (uint32_t v = 0; v < p_view_count; v++) {
		RID uniform_set = UniformSetCacheRD::get_singleton()->get_cache(
				gi->sdfgi_shader.debug_shader_version,
				0,
				RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 1, voxel_bits_tex),
				RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 2, voxel_region_tex),
				RD::Uniform(RD::UNIFORM_TYPE_TEXTURE, 3, light_tex),
				RD::Uniform(RD::UNIFORM_TYPE_SAMPLER, 4, material_storage->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED)),
				RD::Uniform(RD::UNIFORM_TYPE_SAMPLER, 5, material_storage->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_NEAREST, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED)),
				RD::Uniform(RD::UNIFORM_TYPE_TEXTURE, 6, lightprobe_diffuse_tex),
				RD::Uniform(RD::UNIFORM_TYPE_TEXTURE, 7, occlusion_tex),
				RD::Uniform(RD::UNIFORM_TYPE_UNIFORM_BUFFER, 8, cascades_ubo),
				RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 9, p_texture_views[v]));

		RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();
		RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->sdfgi_shader.debug_pipeline);
		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, uniform_set, 0);

		SDFGIShader::DebugPushConstant push_constant;
		push_constant.grid_size[0] = CASCADE_SIZE[0];
		push_constant.grid_size[1] = CASCADE_SIZE[1];
		push_constant.grid_size[2] = CASCADE_SIZE[2];
		push_constant.max_cascades = cascades.size();
		push_constant.screen_size[0] = p_width;
		push_constant.screen_size[1] = p_height;
		push_constant.y_mult = y_mult;

		push_constant.z_near = -p_projections[v].get_z_near();

		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				push_constant.cam_basis[i][j] = p_transform.basis.rows[j][i];
			}
		}

		push_constant.cam_origin[0] = p_transform.origin[0];
		push_constant.cam_origin[1] = p_transform.origin[1];
		push_constant.cam_origin[2] = p_transform.origin[2];

		// need to properly unproject for asymmetric projection matrices in stereo..
		Projection inv_projection = p_projections[v].inverse();
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 3; j++) {
				push_constant.inv_projection[j][i] = inv_projection.columns[i][j];
			}
		}

		RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(SDFGIShader::DebugPushConstant));

		RD::get_singleton()->compute_list_dispatch_threads(compute_list, p_width, p_height, 1);
		RD::get_singleton()->compute_list_end();
	}

	Size2i rtsize = texture_storage->render_target_get_size(p_render_target);
	copy_effects->copy_to_fb_rect(p_texture, texture_storage->render_target_get_rd_framebuffer(p_render_target), Rect2i(Point2i(), rtsize), true, false, false, false, RID(), p_view_count > 1);
}

void GI::SDFGI::debug_probes(RID p_framebuffer, const uint32_t p_view_count, const Projection *p_camera_with_transforms, bool p_will_continue_color, bool p_will_continue_depth) {
	// setup scene data
	{
		SDFGIShader::DebugProbesSceneData scene_data;

		if (debug_probes_scene_data_ubo.is_null()) {
			debug_probes_scene_data_ubo = RD::get_singleton()->uniform_buffer_create(sizeof(SDFGIShader::DebugProbesSceneData));
		}

		for (uint32_t v = 0; v < p_view_count; v++) {
			RendererRD::MaterialStorage::store_camera(p_camera_with_transforms[v], scene_data.projection[v]);
		}

		RD::get_singleton()->buffer_update(debug_probes_scene_data_ubo, 0, sizeof(SDFGIShader::DebugProbesSceneData), &scene_data, RD::BARRIER_MASK_RASTER);
	}

	// setup push constant
	SDFGIShader::DebugProbesPushConstant push_constant;

	//gen spheres from strips
	uint32_t band_points = 128;
	push_constant.band_power = 32;
	push_constant.sections_in_band = ((band_points / 2) - 1);
	push_constant.band_mask = band_points - 2;
	push_constant.section_arc = Math_TAU / float(push_constant.sections_in_band);
	push_constant.y_mult = y_mult;
	push_constant.oct_size = LIGHTPROBE_OCT_SIZE;

	Vector3i probe_axis_count = CASCADE_SIZE / PROBE_CELLS + Vector3i(1, 1, 1);
	uint32_t total_points = push_constant.sections_in_band * band_points;
	uint32_t total_probes = probe_axis_count.x * probe_axis_count.y * probe_axis_count.z;

	push_constant.grid_size[0] = CASCADE_SIZE[0];
	push_constant.grid_size[1] = CASCADE_SIZE[1];
	push_constant.grid_size[2] = CASCADE_SIZE[2];
	push_constant.cascade = 0;

	push_constant.probe_axis_size[0] = probe_axis_count[0];
	push_constant.probe_axis_size[1] = probe_axis_count[1];
	push_constant.probe_axis_size[2] = probe_axis_count[2];

	SDFGIShader::ProbeDebugMode mode = p_view_count > 1 ? SDFGIShader::PROBE_DEBUG_PROBES_MULTIVIEW : SDFGIShader::PROBE_DEBUG_PROBES;

	RID debug_probes_uniform_set = UniformSetCacheRD::get_singleton()->get_cache(
			gi->sdfgi_shader.debug_probes_shader_version[mode],
			0,
			RD::Uniform(RD::UNIFORM_TYPE_UNIFORM_BUFFER, 1, cascades_ubo),
			RD::Uniform(RD::UNIFORM_TYPE_TEXTURE, 2, lightprobe_diffuse_tex),
			RD::Uniform(RD::UNIFORM_TYPE_SAMPLER, 3, RendererRD::MaterialStorage::get_singleton()->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED)),
			RD::Uniform(RD::UNIFORM_TYPE_UNIFORM_BUFFER, 4, debug_probes_scene_data_ubo));

	RD::DrawListID draw_list = RD::get_singleton()->draw_list_begin(p_framebuffer, RD::INITIAL_ACTION_CONTINUE, p_will_continue_color ? RD::FINAL_ACTION_CONTINUE : RD::FINAL_ACTION_READ, RD::INITIAL_ACTION_CONTINUE, p_will_continue_depth ? RD::FINAL_ACTION_CONTINUE : RD::FINAL_ACTION_READ);
	RD::get_singleton()->draw_command_begin_label("Debug SDFGI");

	RD::get_singleton()->draw_list_bind_render_pipeline(draw_list, gi->sdfgi_shader.debug_probes_pipeline[mode].get_render_pipeline(RD::INVALID_FORMAT_ID, RD::get_singleton()->framebuffer_get_format(p_framebuffer)));
	RD::get_singleton()->draw_list_bind_uniform_set(draw_list, debug_probes_uniform_set, 0);
	RD::get_singleton()->draw_list_set_push_constant(draw_list, &push_constant, sizeof(SDFGIShader::DebugProbesPushConstant));
	RD::get_singleton()->draw_list_draw(draw_list, false, total_probes, total_points);

	if (gi->sdfgi_debug_probe_dir != Vector3()) {
		uint32_t cascade = 0;
		Vector3 offset = Vector3((Vector3i(1, 1, 1) * -(CASCADE_SIZE >> 1) + cascades[cascade].position)) * cascades[cascade].cell_size * Vector3(1.0, 1.0 / y_mult, 1.0);
		Vector3 probe_size = cascades[cascade].cell_size * PROBE_CELLS * Vector3(1.0, 1.0 / y_mult, 1.0);
		Vector3 ray_from = gi->sdfgi_debug_probe_pos;
		Vector3 ray_to = gi->sdfgi_debug_probe_pos + gi->sdfgi_debug_probe_dir * cascades[cascade].cell_size * Math::sqrt(3.0) * CASCADE_SIZE[0];
		float sphere_radius = 0.2;
		float closest_dist = 1e20;
		gi->sdfgi_debug_probe_enabled = false;

		Vector3i probe_from = cascades[cascade].position / PROBE_CELLS;
		for (int i = 0; i < (CASCADE_SIZE[0] / PROBE_CELLS + 1); i++) {
			for (int j = 0; j < (CASCADE_SIZE[1] / PROBE_CELLS + 1); j++) {
				for (int k = 0; k < (CASCADE_SIZE[2] / PROBE_CELLS + 1); k++) {
					Vector3 pos = offset + probe_size * Vector3(i, j, k);
					Vector3 res;
					if (Geometry3D::segment_intersects_sphere(ray_from, ray_to, pos, sphere_radius, &res)) {
						float d = ray_from.distance_to(res);
						if (d < closest_dist) {
							closest_dist = d;
							gi->sdfgi_debug_probe_enabled = true;
							gi->sdfgi_debug_probe_index = probe_from + Vector3i(i, j, k);
						}
					}
				}
			}
		}

		print_line("Pos: ", gi->sdfgi_debug_probe_pos);
		print_line("Dir: ", gi->sdfgi_debug_probe_dir);
		print_line("Select: ", gi->sdfgi_debug_probe_index);

		gi->sdfgi_debug_probe_dir = Vector3();
	}

	if (gi->sdfgi_debug_probe_enabled) {
		mode = p_view_count > 1 ? SDFGIShader::PROBE_DEBUG_OCCLUSION_MULTIVIEW : SDFGIShader::PROBE_DEBUG_OCCLUSION;

		debug_probes_uniform_set = UniformSetCacheRD::get_singleton()->get_cache(
				gi->sdfgi_shader.debug_probes_shader_version[mode],
				0,
				RD::Uniform(RD::UNIFORM_TYPE_UNIFORM_BUFFER, 1, cascades_ubo),
				RD::Uniform(RD::UNIFORM_TYPE_TEXTURE, 2, occlusion_tex),
				RD::Uniform(RD::UNIFORM_TYPE_SAMPLER, 3, RendererRD::MaterialStorage::get_singleton()->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED)),
				RD::Uniform(RD::UNIFORM_TYPE_UNIFORM_BUFFER, 4, debug_probes_scene_data_ubo));

		uint32_t cascade = 0;
		uint32_t probe_cells = PROBE_CELLS;
		Vector3i probe_from = cascades[cascade].position / probe_cells;
		Vector3i ofs = gi->sdfgi_debug_probe_index - probe_from;
		if (ofs.x < 0 || ofs.y < 0 || ofs.z < 0 || ofs.x > (CASCADE_SIZE[0] / PROBE_CELLS + 1) || ofs.y > (CASCADE_SIZE[1] / PROBE_CELLS + 1) || ofs.z > (CASCADE_SIZE[2] / PROBE_CELLS + 1)) {
			RD::get_singleton()->draw_command_end_label();
			RD::get_singleton()->draw_list_end();
			return;
		}

		uint32_t index = ofs.z * probe_axis_count.x * probe_axis_count.y + ofs.y * probe_axis_count.x + ofs.x;

		push_constant.probe_debug_index = index;
		push_constant.oct_size = OCCLUSION_OCT_SIZE;

		RD::get_singleton()->draw_list_bind_render_pipeline(draw_list, gi->sdfgi_shader.debug_probes_pipeline[mode].get_render_pipeline(RD::INVALID_FORMAT_ID, RD::get_singleton()->framebuffer_get_format(p_framebuffer)));
		RD::get_singleton()->draw_list_bind_uniform_set(draw_list, debug_probes_uniform_set, 0);
		RD::get_singleton()->draw_list_set_push_constant(draw_list, &push_constant, sizeof(SDFGIShader::DebugProbesPushConstant));
		RD::get_singleton()->draw_list_draw(draw_list, false, 1, total_points);

		// Lines

		mode = p_view_count > 1 ? SDFGIShader::PROBE_DEBUG_LINES_MULTIVIEW : SDFGIShader::PROBE_DEBUG_LINES;

		debug_probes_uniform_set = UniformSetCacheRD::get_singleton()->get_cache(
				gi->sdfgi_shader.debug_probes_shader_version[mode],
				0,
				RD::Uniform(RD::UNIFORM_TYPE_UNIFORM_BUFFER, 1, cascades_ubo),
				RD::Uniform(RD::UNIFORM_TYPE_TEXTURE, 2, occlusion_process),
				RD::Uniform(RD::UNIFORM_TYPE_SAMPLER, 3, RendererRD::MaterialStorage::get_singleton()->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED)),
				RD::Uniform(RD::UNIFORM_TYPE_UNIFORM_BUFFER, 4, debug_probes_scene_data_ubo));

		int total_lines = OCCLUSION_OCT_SIZE * OCCLUSION_OCT_SIZE * 2;
		RD::get_singleton()->draw_list_bind_render_pipeline(draw_list, gi->sdfgi_shader.debug_probes_pipeline[mode].get_render_pipeline(RD::INVALID_FORMAT_ID, RD::get_singleton()->framebuffer_get_format(p_framebuffer)));
		RD::get_singleton()->draw_list_bind_uniform_set(draw_list, debug_probes_uniform_set, 0);
		RD::get_singleton()->draw_list_set_push_constant(draw_list, &push_constant, sizeof(SDFGIShader::DebugProbesPushConstant));
		RD::get_singleton()->draw_list_draw(draw_list, false, 1, total_lines);
	}

	RD::get_singleton()->draw_command_end_label();
	RD::get_singleton()->draw_list_end();
}

void GI::SDFGI::pre_process_gi(const Transform3D &p_transform, RenderDataRD *p_render_data) {
	RendererRD::LightStorage *light_storage = RendererRD::LightStorage::get_singleton();
	/* Update general SDFGI Buffer */

	SDFGIData sdfgi_data;

	sdfgi_data.grid_size[0] = CASCADE_SIZE[0];
	sdfgi_data.grid_size[1] = CASCADE_SIZE[1];
	sdfgi_data.grid_size[2] = CASCADE_SIZE[2];

	sdfgi_data.max_cascades = cascades.size();

	sdfgi_data.probe_axis_size[0] = CASCADE_SIZE[0] / PROBE_CELLS + 1;
	sdfgi_data.probe_axis_size[1] = CASCADE_SIZE[1] / PROBE_CELLS + 1;
	sdfgi_data.probe_axis_size[2] = CASCADE_SIZE[2] / PROBE_CELLS + 1;

	sdfgi_data.y_mult = y_mult;
	sdfgi_data.normal_bias = normal_bias;
	;

	sdfgi_data.energy = energy;

	for (int32_t i = 0; i < sdfgi_data.max_cascades; i++) {
		SDFGIData::ProbeCascadeData &c = sdfgi_data.cascades[i];
		Vector3 pos = Vector3((Vector3i(1, 1, 1) * -(CASCADE_SIZE >> 1) + cascades[i].position)) * cascades[i].cell_size;
		Vector3 cam_origin = p_transform.origin;
		cam_origin.y *= y_mult;
		pos -= cam_origin; //make pos local to camera, to reduce numerical error
		c.position[0] = pos.x;
		c.position[1] = pos.y;
		c.position[2] = pos.z;
		c.to_probe = 1.0 / (float(CASCADE_SIZE[0]) * cascades[i].cell_size / float(sdfgi_data.probe_axis_size[0] - 1));

		c.region_world_offset[0] = cascades[i].position.x / REGION_CELLS;
		c.region_world_offset[1] = cascades[i].position.y / REGION_CELLS;
		c.region_world_offset[2] = cascades[i].position.z / REGION_CELLS;

		c.to_cell = 1.0 / cascades[i].cell_size;
		c.exposure_normalization = 1.0;
		if (p_render_data->camera_attributes.is_valid()) {
			float exposure_normalization = RSG::camera_attributes->camera_attributes_get_exposure_normalization_factor(p_render_data->camera_attributes);
			c.exposure_normalization = exposure_normalization / cascades[i].baked_exposure_normalization;
		}
	}

	RD::get_singleton()->buffer_update(gi->sdfgi_ubo, 0, sizeof(SDFGIData), &sdfgi_data, RD::BARRIER_MASK_COMPUTE);

	/* Update dynamic lights in SDFGI cascades */

	for (uint32_t i = 0; i < cascades.size(); i++) {
		SDFGI::Cascade &cascade = cascades[i];

		SDFGIShader::Light lights[SDFGI::MAX_DYNAMIC_LIGHTS];
		uint32_t idx = 0;
		for (uint32_t j = 0; j < (uint32_t)p_render_data->sdfgi_update_data->directional_lights->size(); j++) {
			if (idx == SDFGI::MAX_DYNAMIC_LIGHTS) {
				break;
			}

			RID light_instance = p_render_data->sdfgi_update_data->directional_lights->get(j);
			ERR_CONTINUE(!light_storage->owns_light_instance(light_instance));

			RID light = light_storage->light_instance_get_base_light(light_instance);
			Transform3D light_transform = light_storage->light_instance_get_base_transform(light_instance);

			if (RSG::light_storage->light_directional_get_sky_mode(light) == RS::LIGHT_DIRECTIONAL_SKY_MODE_SKY_ONLY) {
				continue;
			}

			Vector3 dir = -light_transform.basis.get_column(Vector3::AXIS_Z);
			dir.y *= y_mult;
			dir.normalize();
			lights[idx].direction[0] = dir.x;
			lights[idx].direction[1] = dir.y;
			lights[idx].direction[2] = dir.z;
			Color color = RSG::light_storage->light_get_color(light);
			color = color.srgb_to_linear();
			lights[idx].color[0] = color.r;
			lights[idx].color[1] = color.g;
			lights[idx].color[2] = color.b;
			lights[idx].type = RS::LIGHT_DIRECTIONAL;
			lights[idx].energy = RSG::light_storage->light_get_param(light, RS::LIGHT_PARAM_ENERGY) * RSG::light_storage->light_get_param(light, RS::LIGHT_PARAM_INDIRECT_ENERGY);
			if (RendererSceneRenderRD::get_singleton()->is_using_physical_light_units()) {
				lights[idx].energy *= RSG::light_storage->light_get_param(light, RS::LIGHT_PARAM_INTENSITY);
			}

			if (p_render_data->camera_attributes.is_valid()) {
				lights[idx].energy *= RSG::camera_attributes->camera_attributes_get_exposure_normalization_factor(p_render_data->camera_attributes);
			}

			lights[idx].has_shadow = RSG::light_storage->light_has_shadow(light);

			idx++;
		}

		AABB cascade_aabb;
		cascade_aabb.position = Vector3((Vector3i(1, 1, 1) * -(CASCADE_SIZE >> 1) + cascade.position)) * cascade.cell_size;
		cascade_aabb.size = Vector3(1, 1, 1) * CASCADE_SIZE * cascade.cell_size;

		for (uint32_t j = 0; j < p_render_data->sdfgi_update_data->positional_light_count; j++) {
			if (idx == SDFGI::MAX_DYNAMIC_LIGHTS) {
				break;
			}

			RID light_instance = p_render_data->sdfgi_update_data->positional_light_instances[j];
			ERR_CONTINUE(!light_storage->owns_light_instance(light_instance));

			RID light = light_storage->light_instance_get_base_light(light_instance);
			AABB light_aabb = light_storage->light_instance_get_base_aabb(light_instance);
			Transform3D light_transform = light_storage->light_instance_get_base_transform(light_instance);

			uint32_t max_sdfgi_cascade = RSG::light_storage->light_get_max_sdfgi_cascade(light);
			if (i > max_sdfgi_cascade) {
				continue;
			}

			if (!cascade_aabb.intersects(light_aabb)) {
				continue;
			}

			Vector3 dir = -light_transform.basis.get_column(Vector3::AXIS_Z);
			//faster to not do this here
			//dir.y *= y_mult;
			//dir.normalize();
			lights[idx].direction[0] = dir.x;
			lights[idx].direction[1] = dir.y;
			lights[idx].direction[2] = dir.z;
			Vector3 pos = light_transform.origin;
			pos.y *= y_mult;
			lights[idx].position[0] = pos.x;
			lights[idx].position[1] = pos.y;
			lights[idx].position[2] = pos.z;
			Color color = RSG::light_storage->light_get_color(light);
			color = color.srgb_to_linear();
			lights[idx].color[0] = color.r;
			lights[idx].color[1] = color.g;
			lights[idx].color[2] = color.b;
			lights[idx].type = RSG::light_storage->light_get_type(light);

			lights[idx].energy = RSG::light_storage->light_get_param(light, RS::LIGHT_PARAM_ENERGY) * RSG::light_storage->light_get_param(light, RS::LIGHT_PARAM_INDIRECT_ENERGY);
			if (RendererSceneRenderRD::get_singleton()->is_using_physical_light_units()) {
				lights[idx].energy *= RSG::light_storage->light_get_param(light, RS::LIGHT_PARAM_INTENSITY);

				// Convert from Luminous Power to Luminous Intensity
				if (lights[idx].type == RS::LIGHT_OMNI) {
					lights[idx].energy *= 1.0 / (Math_PI * 4.0);
				} else if (lights[idx].type == RS::LIGHT_SPOT) {
					// Spot Lights are not physically accurate, Luminous Intensity should change in relation to the cone angle.
					// We make this assumption to keep them easy to control.
					lights[idx].energy *= 1.0 / Math_PI;
				}
			}

			if (p_render_data->camera_attributes.is_valid()) {
				lights[idx].energy *= RSG::camera_attributes->camera_attributes_get_exposure_normalization_factor(p_render_data->camera_attributes);
			}

			lights[idx].has_shadow = RSG::light_storage->light_has_shadow(light);
			lights[idx].attenuation = RSG::light_storage->light_get_param(light, RS::LIGHT_PARAM_ATTENUATION);
			lights[idx].radius = RSG::light_storage->light_get_param(light, RS::LIGHT_PARAM_RANGE);
			lights[idx].cos_spot_angle = Math::cos(Math::deg_to_rad(RSG::light_storage->light_get_param(light, RS::LIGHT_PARAM_SPOT_ANGLE)));
			lights[idx].inv_spot_attenuation = 1.0f / RSG::light_storage->light_get_param(light, RS::LIGHT_PARAM_SPOT_ATTENUATION);

			idx++;
		}

		if (idx > 0) {
			RD::get_singleton()->buffer_update(cascade.light_position_bufer, 0, idx * sizeof(SDFGIShader::Light), lights, RD::BARRIER_MASK_COMPUTE);
		}

		cascade_dynamic_light_count[i] = idx;
	}
}

void GI::SDFGI::render_static_lights(RenderDataRD *p_render_data, Ref<RenderSceneBuffersRD> p_render_buffers, uint32_t p_cascade_count, const uint32_t *p_cascade_indices, const PagedArray<RID> *p_positional_light_cull_result) {
	ERR_FAIL_COND(p_render_buffers.is_null()); // we wouldn't be here if this failed but...

	RendererRD::LightStorage *light_storage = RendererRD::LightStorage::get_singleton();

	RD::get_singleton()->draw_command_begin_label("SDFGI Render Static Lights");

	update_cascades();

	SDFGIShader::Light lights[SDFGI::MAX_STATIC_LIGHTS];
	uint32_t light_count[SDFGI::MAX_STATIC_LIGHTS];

	for (uint32_t i = 0; i < p_cascade_count; i++) {
		ERR_CONTINUE(p_cascade_indices[i] >= cascades.size());

		SDFGI::Cascade &cc = cascades[p_cascade_indices[i]];

		{ //fill light buffer

			AABB cascade_aabb;
			cascade_aabb.position = Vector3((Vector3i(1, 1, 1) * -(CASCADE_SIZE >> 1) + cc.position)) * cc.cell_size;
			cascade_aabb.size = Vector3(1, 1, 1) * CASCADE_SIZE * cc.cell_size;

			int idx = 0;

			for (uint32_t j = 0; j < (uint32_t)p_positional_light_cull_result[i].size(); j++) {
				if (idx == SDFGI::MAX_STATIC_LIGHTS) {
					break;
				}

				RID light_instance = p_positional_light_cull_result[i][j];
				ERR_CONTINUE(!light_storage->owns_light_instance(light_instance));

				RID light = light_storage->light_instance_get_base_light(light_instance);
				AABB light_aabb = light_storage->light_instance_get_base_aabb(light_instance);
				Transform3D light_transform = light_storage->light_instance_get_base_transform(light_instance);

				uint32_t max_sdfgi_cascade = RSG::light_storage->light_get_max_sdfgi_cascade(light);
				if (p_cascade_indices[i] > max_sdfgi_cascade) {
					continue;
				}

				if (!cascade_aabb.intersects(light_aabb)) {
					continue;
				}

				lights[idx].type = RSG::light_storage->light_get_type(light);

				Vector3 dir = -light_transform.basis.get_column(Vector3::AXIS_Z);
				if (lights[idx].type == RS::LIGHT_DIRECTIONAL) {
					dir.y *= y_mult; //only makes sense for directional
					dir.normalize();
				}
				lights[idx].direction[0] = dir.x;
				lights[idx].direction[1] = dir.y;
				lights[idx].direction[2] = dir.z;
				Vector3 pos = light_transform.origin;
				pos.y *= y_mult;
				lights[idx].position[0] = pos.x;
				lights[idx].position[1] = pos.y;
				lights[idx].position[2] = pos.z;
				Color color = RSG::light_storage->light_get_color(light);
				color = color.srgb_to_linear();
				lights[idx].color[0] = color.r;
				lights[idx].color[1] = color.g;
				lights[idx].color[2] = color.b;

				lights[idx].energy = RSG::light_storage->light_get_param(light, RS::LIGHT_PARAM_ENERGY) * RSG::light_storage->light_get_param(light, RS::LIGHT_PARAM_INDIRECT_ENERGY);
				if (RendererSceneRenderRD::get_singleton()->is_using_physical_light_units()) {
					lights[idx].energy *= RSG::light_storage->light_get_param(light, RS::LIGHT_PARAM_INTENSITY);

					// Convert from Luminous Power to Luminous Intensity
					if (lights[idx].type == RS::LIGHT_OMNI) {
						lights[idx].energy *= 1.0 / (Math_PI * 4.0);
					} else if (lights[idx].type == RS::LIGHT_SPOT) {
						// Spot Lights are not physically accurate, Luminous Intensity should change in relation to the cone angle.
						// We make this assumption to keep them easy to control.
						lights[idx].energy *= 1.0 / Math_PI;
					}
				}

				if (p_render_data->camera_attributes.is_valid()) {
					lights[idx].energy *= RSG::camera_attributes->camera_attributes_get_exposure_normalization_factor(p_render_data->camera_attributes);
				}

				lights[idx].has_shadow = RSG::light_storage->light_has_shadow(light);
				lights[idx].attenuation = RSG::light_storage->light_get_param(light, RS::LIGHT_PARAM_ATTENUATION);
				lights[idx].radius = RSG::light_storage->light_get_param(light, RS::LIGHT_PARAM_RANGE);
				lights[idx].cos_spot_angle = Math::cos(Math::deg_to_rad(RSG::light_storage->light_get_param(light, RS::LIGHT_PARAM_SPOT_ANGLE)));
				lights[idx].inv_spot_attenuation = 1.0f / RSG::light_storage->light_get_param(light, RS::LIGHT_PARAM_SPOT_ATTENUATION);

				idx++;
			}

			if (idx > 0) {
				RD::get_singleton()->buffer_update(cc.light_position_bufer, 0, idx * sizeof(SDFGIShader::Light), lights);
			}

			light_count[i] = idx;
		}
	}

	/* Static Lights */
	RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();

	RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->sdfgi_shader.direct_light_pipeline[SDFGIShader::DIRECT_LIGHT_MODE_STATIC]);

	SDFGIShader::DirectLightPushConstant dl_push_constant;

	Vector3i probe_axis_count = CASCADE_SIZE / PROBE_CELLS + Vector3i(1, 1, 1);

	dl_push_constant.grid_size[0] = CASCADE_SIZE[0];
	dl_push_constant.grid_size[1] = CASCADE_SIZE[1];
	dl_push_constant.grid_size[2] = CASCADE_SIZE[2];
	dl_push_constant.max_cascades = cascades.size();
	dl_push_constant.probe_axis_size[0] = probe_axis_count[0];
	dl_push_constant.probe_axis_size[1] = probe_axis_count[1];
	dl_push_constant.probe_axis_size[2] = probe_axis_count[2];
	dl_push_constant.bounce_feedback = 0.0; // this is static light, do not multibounce yet
	dl_push_constant.y_mult = y_mult;
	dl_push_constant.use_occlusion = uses_occlusion;
	dl_push_constant.probe_cell_size = PROBE_CELLS;

	//all must be processed
	dl_push_constant.process_offset = 0;
	dl_push_constant.process_increment = 1;

	for (uint32_t i = 0; i < p_cascade_count; i++) {
		ERR_CONTINUE(p_cascade_indices[i] >= cascades.size());

		SDFGI::Cascade &cc = cascades[p_cascade_indices[i]];

		dl_push_constant.light_count = light_count[i];
		dl_push_constant.cascade = p_cascade_indices[i];

		if (dl_push_constant.light_count > 0) {
			RID uniform_set = UniformSetCacheRD::get_singleton()->get_cache(
					gi->sdfgi_shader.direct_light_shader_version[SDFGIShader::DIRECT_LIGHT_MODE_DYNAMIC],
					0,
					RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 1, voxel_bits_tex),
					RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 2, voxel_region_tex),
					RD::Uniform(RD::UNIFORM_TYPE_SAMPLER, 3, RendererRD::MaterialStorage::get_singleton()->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED)),
					RD::Uniform(RD::UNIFORM_TYPE_STORAGE_BUFFER, 4, cc.light_process_dispatch_buffer_copy),
					RD::Uniform(RD::UNIFORM_TYPE_STORAGE_BUFFER, 5, cc.light_process_buffer),
					RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 6, light_tex_data),
					RD::Uniform(RD::UNIFORM_TYPE_UNIFORM_BUFFER, 9, cascades_ubo),
					RD::Uniform(RD::UNIFORM_TYPE_STORAGE_BUFFER, 10, cc.light_position_bufer));

			RD::get_singleton()->compute_list_bind_uniform_set(compute_list, uniform_set, 0);
			RD::get_singleton()->compute_list_set_push_constant(compute_list, &dl_push_constant, sizeof(SDFGIShader::DirectLightPushConstant));
			RD::get_singleton()->compute_list_dispatch_indirect(compute_list, cc.light_process_dispatch_buffer, 0);
		}
	}

	RD::get_singleton()->compute_list_end();

	RD::get_singleton()->draw_command_end_label();
}

////////////////////////////////////////////////////////////////////////////////
// VoxelGIInstance

void GI::VoxelGIInstance::update(bool p_update_light_instances, const Vector<RID> &p_light_instances, const PagedArray<RenderGeometryInstance *> &p_dynamic_objects) {
	RendererRD::LightStorage *light_storage = RendererRD::LightStorage::get_singleton();
	RendererRD::MaterialStorage *material_storage = RendererRD::MaterialStorage::get_singleton();

	uint32_t data_version = gi->voxel_gi_get_data_version(probe);

	// (RE)CREATE IF NEEDED

	if (last_probe_data_version != data_version) {
		//need to re-create everything
		free_resources();

		Vector3i octree_size = gi->voxel_gi_get_octree_size(probe);

		if (octree_size != Vector3i()) {
			//can create a 3D texture
			Vector<int> levels = gi->voxel_gi_get_level_counts(probe);

			RD::TextureFormat tf;
			tf.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
			tf.width = octree_size.x;
			tf.height = octree_size.y;
			tf.depth = octree_size.z;
			tf.texture_type = RD::TEXTURE_TYPE_3D;
			tf.mipmaps = levels.size();

			tf.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_STORAGE_BIT | RD::TEXTURE_USAGE_CAN_COPY_TO_BIT;

			texture = RD::get_singleton()->texture_create(tf, RD::TextureView());
			RD::get_singleton()->set_resource_name(texture, "VoxelGI Instance Texture");

			RD::get_singleton()->texture_clear(texture, Color(0, 0, 0, 0), 0, levels.size(), 0, 1);

			{
				int total_elements = 0;
				for (int i = 0; i < levels.size(); i++) {
					total_elements += levels[i];
				}

				write_buffer = RD::get_singleton()->storage_buffer_create(total_elements * 16);
			}

			for (int i = 0; i < levels.size(); i++) {
				VoxelGIInstance::Mipmap mipmap;
				mipmap.texture = RD::get_singleton()->texture_create_shared_from_slice(RD::TextureView(), texture, 0, i, 1, RD::TEXTURE_SLICE_3D);
				mipmap.level = levels.size() - i - 1;
				mipmap.cell_offset = 0;
				for (uint32_t j = 0; j < mipmap.level; j++) {
					mipmap.cell_offset += levels[j];
				}
				mipmap.cell_count = levels[mipmap.level];

				Vector<RD::Uniform> uniforms;
				{
					RD::Uniform u;
					u.uniform_type = RD::UNIFORM_TYPE_STORAGE_BUFFER;
					u.binding = 1;
					u.append_id(gi->voxel_gi_get_octree_buffer(probe));
					uniforms.push_back(u);
				}
				{
					RD::Uniform u;
					u.uniform_type = RD::UNIFORM_TYPE_STORAGE_BUFFER;
					u.binding = 2;
					u.append_id(gi->voxel_gi_get_data_buffer(probe));
					uniforms.push_back(u);
				}

				{
					RD::Uniform u;
					u.uniform_type = RD::UNIFORM_TYPE_STORAGE_BUFFER;
					u.binding = 4;
					u.append_id(write_buffer);
					uniforms.push_back(u);
				}
				{
					RD::Uniform u;
					u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
					u.binding = 9;
					u.append_id(gi->voxel_gi_get_sdf_texture(probe));
					uniforms.push_back(u);
				}
				{
					RD::Uniform u;
					u.uniform_type = RD::UNIFORM_TYPE_SAMPLER;
					u.binding = 10;
					u.append_id(material_storage->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR_WITH_MIPMAPS, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED));
					uniforms.push_back(u);
				}

				{
					Vector<RD::Uniform> copy_uniforms = uniforms;
					if (i == 0) {
						{
							RD::Uniform u;
							u.uniform_type = RD::UNIFORM_TYPE_UNIFORM_BUFFER;
							u.binding = 3;
							u.append_id(gi->voxel_gi_lights_uniform);
							copy_uniforms.push_back(u);
						}

						mipmap.uniform_set = RD::get_singleton()->uniform_set_create(copy_uniforms, gi->voxel_gi_lighting_shader_version_shaders[VOXEL_GI_SHADER_VERSION_COMPUTE_LIGHT], 0);

						copy_uniforms = uniforms; //restore

						{
							RD::Uniform u;
							u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
							u.binding = 5;
							u.append_id(texture);
							copy_uniforms.push_back(u);
						}
						mipmap.second_bounce_uniform_set = RD::get_singleton()->uniform_set_create(copy_uniforms, gi->voxel_gi_lighting_shader_version_shaders[VOXEL_GI_SHADER_VERSION_COMPUTE_SECOND_BOUNCE], 0);
					} else {
						mipmap.uniform_set = RD::get_singleton()->uniform_set_create(copy_uniforms, gi->voxel_gi_lighting_shader_version_shaders[VOXEL_GI_SHADER_VERSION_COMPUTE_MIPMAP], 0);
					}
				}

				{
					RD::Uniform u;
					u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
					u.binding = 5;
					u.append_id(mipmap.texture);
					uniforms.push_back(u);
				}

				mipmap.write_uniform_set = RD::get_singleton()->uniform_set_create(uniforms, gi->voxel_gi_lighting_shader_version_shaders[VOXEL_GI_SHADER_VERSION_WRITE_TEXTURE], 0);

				mipmaps.push_back(mipmap);
			}

			{
				uint32_t dynamic_map_size = MAX(MAX(octree_size.x, octree_size.y), octree_size.z);
				uint32_t oversample = nearest_power_of_2_templated(4);
				int mipmap_index = 0;

				while (mipmap_index < mipmaps.size()) {
					VoxelGIInstance::DynamicMap dmap;

					if (oversample > 0) {
						dmap.size = dynamic_map_size * (1 << oversample);
						dmap.mipmap = -1;
						oversample--;
					} else {
						dmap.size = dynamic_map_size >> mipmap_index;
						dmap.mipmap = mipmap_index;
						mipmap_index++;
					}

					RD::TextureFormat dtf;
					dtf.width = dmap.size;
					dtf.height = dmap.size;
					dtf.format = RD::DATA_FORMAT_R16G16B16A16_SFLOAT;
					dtf.usage_bits = RD::TEXTURE_USAGE_STORAGE_BIT;

					if (dynamic_maps.size() == 0) {
						dtf.usage_bits |= RD::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT;
					}
					dmap.texture = RD::get_singleton()->texture_create(dtf, RD::TextureView());
					RD::get_singleton()->set_resource_name(dmap.texture, "VoxelGI Instance DMap Texture");

					if (dynamic_maps.size() == 0) {
						// Render depth for first one.
						// Use 16-bit depth when supported to improve performance.
						dtf.format = RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_D16_UNORM, RD::TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) ? RD::DATA_FORMAT_D16_UNORM : RD::DATA_FORMAT_X8_D24_UNORM_PACK32;
						dtf.usage_bits = RD::TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
						dmap.fb_depth = RD::get_singleton()->texture_create(dtf, RD::TextureView());
						RD::get_singleton()->set_resource_name(dmap.fb_depth, "VoxelGI Instance DMap FB Depth");
					}

					//just use depth as-is
					dtf.format = RD::DATA_FORMAT_R32_SFLOAT;
					dtf.usage_bits = RD::TEXTURE_USAGE_STORAGE_BIT | RD::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT;

					dmap.depth = RD::get_singleton()->texture_create(dtf, RD::TextureView());
					RD::get_singleton()->set_resource_name(dmap.depth, "VoxelGI Instance DMap Depth");

					if (dynamic_maps.size() == 0) {
						dtf.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
						dtf.usage_bits = RD::TEXTURE_USAGE_STORAGE_BIT | RD::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT;
						dmap.albedo = RD::get_singleton()->texture_create(dtf, RD::TextureView());
						RD::get_singleton()->set_resource_name(dmap.albedo, "VoxelGI Instance DMap Albedo");
						dmap.normal = RD::get_singleton()->texture_create(dtf, RD::TextureView());
						RD::get_singleton()->set_resource_name(dmap.normal, "VoxelGI Instance DMap Normal");
						dmap.orm = RD::get_singleton()->texture_create(dtf, RD::TextureView());
						RD::get_singleton()->set_resource_name(dmap.orm, "VoxelGI Instance DMap ORM");

						Vector<RID> fb;
						fb.push_back(dmap.albedo);
						fb.push_back(dmap.normal);
						fb.push_back(dmap.orm);
						fb.push_back(dmap.texture); //emission
						fb.push_back(dmap.depth);
						fb.push_back(dmap.fb_depth);

						dmap.fb = RD::get_singleton()->framebuffer_create(fb);

						{
							Vector<RD::Uniform> uniforms;
							{
								RD::Uniform u;
								u.uniform_type = RD::UNIFORM_TYPE_UNIFORM_BUFFER;
								u.binding = 3;
								u.append_id(gi->voxel_gi_lights_uniform);
								uniforms.push_back(u);
							}

							{
								RD::Uniform u;
								u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
								u.binding = 5;
								u.append_id(dmap.albedo);
								uniforms.push_back(u);
							}
							{
								RD::Uniform u;
								u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
								u.binding = 6;
								u.append_id(dmap.normal);
								uniforms.push_back(u);
							}
							{
								RD::Uniform u;
								u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
								u.binding = 7;
								u.append_id(dmap.orm);
								uniforms.push_back(u);
							}
							{
								RD::Uniform u;
								u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
								u.binding = 8;
								u.append_id(dmap.fb_depth);
								uniforms.push_back(u);
							}
							{
								RD::Uniform u;
								u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
								u.binding = 9;
								u.append_id(gi->voxel_gi_get_sdf_texture(probe));
								uniforms.push_back(u);
							}
							{
								RD::Uniform u;
								u.uniform_type = RD::UNIFORM_TYPE_SAMPLER;
								u.binding = 10;
								u.append_id(material_storage->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR_WITH_MIPMAPS, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED));
								uniforms.push_back(u);
							}
							{
								RD::Uniform u;
								u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
								u.binding = 11;
								u.append_id(dmap.texture);
								uniforms.push_back(u);
							}
							{
								RD::Uniform u;
								u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
								u.binding = 12;
								u.append_id(dmap.depth);
								uniforms.push_back(u);
							}

							dmap.uniform_set = RD::get_singleton()->uniform_set_create(uniforms, gi->voxel_gi_lighting_shader_version_shaders[VOXEL_GI_SHADER_VERSION_DYNAMIC_OBJECT_LIGHTING], 0);
						}
					} else {
						bool plot = dmap.mipmap >= 0;
						bool write = dmap.mipmap < (mipmaps.size() - 1);

						Vector<RD::Uniform> uniforms;

						{
							RD::Uniform u;
							u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
							u.binding = 5;
							u.append_id(dynamic_maps[dynamic_maps.size() - 1].texture);
							uniforms.push_back(u);
						}
						{
							RD::Uniform u;
							u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
							u.binding = 6;
							u.append_id(dynamic_maps[dynamic_maps.size() - 1].depth);
							uniforms.push_back(u);
						}

						if (write) {
							{
								RD::Uniform u;
								u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
								u.binding = 7;
								u.append_id(dmap.texture);
								uniforms.push_back(u);
							}
							{
								RD::Uniform u;
								u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
								u.binding = 8;
								u.append_id(dmap.depth);
								uniforms.push_back(u);
							}
						}

						{
							RD::Uniform u;
							u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
							u.binding = 9;
							u.append_id(gi->voxel_gi_get_sdf_texture(probe));
							uniforms.push_back(u);
						}
						{
							RD::Uniform u;
							u.uniform_type = RD::UNIFORM_TYPE_SAMPLER;
							u.binding = 10;
							u.append_id(material_storage->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR_WITH_MIPMAPS, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED));
							uniforms.push_back(u);
						}

						if (plot) {
							{
								RD::Uniform u;
								u.uniform_type = RD::UNIFORM_TYPE_IMAGE;
								u.binding = 11;
								u.append_id(mipmaps[dmap.mipmap].texture);
								uniforms.push_back(u);
							}
						}

						dmap.uniform_set = RD::get_singleton()->uniform_set_create(
								uniforms,
								gi->voxel_gi_lighting_shader_version_shaders[(write && plot) ? VOXEL_GI_SHADER_VERSION_DYNAMIC_SHRINK_WRITE_PLOT : (write ? VOXEL_GI_SHADER_VERSION_DYNAMIC_SHRINK_WRITE : VOXEL_GI_SHADER_VERSION_DYNAMIC_SHRINK_PLOT)],
								0);
					}

					dynamic_maps.push_back(dmap);
				}
			}
		}

		last_probe_data_version = data_version;
		p_update_light_instances = true; //just in case

		RendererSceneRenderRD::get_singleton()->base_uniforms_changed();
	}

	// UDPDATE TIME

	if (has_dynamic_object_data) {
		//if it has dynamic object data, it needs to be cleared
		RD::get_singleton()->texture_clear(texture, Color(0, 0, 0, 0), 0, mipmaps.size(), 0, 1);
	}

	uint32_t light_count = 0;

	if (p_update_light_instances || p_dynamic_objects.size() > 0) {
		light_count = MIN(gi->voxel_gi_max_lights, (uint32_t)p_light_instances.size());

		{
			Transform3D to_cell = gi->voxel_gi_get_to_cell_xform(probe);
			Transform3D to_probe_xform = to_cell * transform.affine_inverse();

			//update lights

			for (uint32_t i = 0; i < light_count; i++) {
				VoxelGILight &l = gi->voxel_gi_lights[i];
				RID light_instance = p_light_instances[i];
				RID light = light_storage->light_instance_get_base_light(light_instance);

				l.type = RSG::light_storage->light_get_type(light);
				if (l.type == RS::LIGHT_DIRECTIONAL && RSG::light_storage->light_directional_get_sky_mode(light) == RS::LIGHT_DIRECTIONAL_SKY_MODE_SKY_ONLY) {
					light_count--;
					continue;
				}

				l.attenuation = RSG::light_storage->light_get_param(light, RS::LIGHT_PARAM_ATTENUATION);
				l.energy = RSG::light_storage->light_get_param(light, RS::LIGHT_PARAM_ENERGY) * RSG::light_storage->light_get_param(light, RS::LIGHT_PARAM_INDIRECT_ENERGY);

				if (RendererSceneRenderRD::get_singleton()->is_using_physical_light_units()) {
					l.energy *= RSG::light_storage->light_get_param(light, RS::LIGHT_PARAM_INTENSITY);

					l.energy *= gi->voxel_gi_get_baked_exposure_normalization(probe);

					// Convert from Luminous Power to Luminous Intensity
					if (l.type == RS::LIGHT_OMNI) {
						l.energy *= 1.0 / (Math_PI * 4.0);
					} else if (l.type == RS::LIGHT_SPOT) {
						// Spot Lights are not physically accurate, Luminous Intensity should change in relation to the cone angle.
						// We make this assumption to keep them easy to control.
						l.energy *= 1.0 / Math_PI;
					}
				}

				l.radius = to_cell.basis.xform(Vector3(RSG::light_storage->light_get_param(light, RS::LIGHT_PARAM_RANGE), 0, 0)).length();
				Color color = RSG::light_storage->light_get_color(light).srgb_to_linear();
				l.color[0] = color.r;
				l.color[1] = color.g;
				l.color[2] = color.b;

				l.cos_spot_angle = Math::cos(Math::deg_to_rad(RSG::light_storage->light_get_param(light, RS::LIGHT_PARAM_SPOT_ANGLE)));
				l.inv_spot_attenuation = 1.0f / RSG::light_storage->light_get_param(light, RS::LIGHT_PARAM_SPOT_ATTENUATION);

				Transform3D xform = light_storage->light_instance_get_base_transform(light_instance);

				Vector3 pos = to_probe_xform.xform(xform.origin);
				Vector3 dir = to_probe_xform.basis.xform(-xform.basis.get_column(2)).normalized();

				l.position[0] = pos.x;
				l.position[1] = pos.y;
				l.position[2] = pos.z;

				l.direction[0] = dir.x;
				l.direction[1] = dir.y;
				l.direction[2] = dir.z;

				l.has_shadow = RSG::light_storage->light_has_shadow(light);
			}

			RD::get_singleton()->buffer_update(gi->voxel_gi_lights_uniform, 0, sizeof(VoxelGILight) * light_count, gi->voxel_gi_lights);
		}
	}

	if (has_dynamic_object_data || p_update_light_instances || p_dynamic_objects.size()) {
		// PROCESS MIPMAPS
		if (mipmaps.size()) {
			//can update mipmaps

			Vector3i probe_size = gi->voxel_gi_get_octree_size(probe);

			VoxelGIPushConstant push_constant;

			push_constant.limits[0] = probe_size.x;
			push_constant.limits[1] = probe_size.y;
			push_constant.limits[2] = probe_size.z;
			push_constant.stack_size = mipmaps.size();
			push_constant.emission_scale = 1.0;
			push_constant.propagation = gi->voxel_gi_get_propagation(probe);
			push_constant.dynamic_range = gi->voxel_gi_get_dynamic_range(probe);
			push_constant.light_count = light_count;
			push_constant.aniso_strength = 0;

			/*		print_line("probe update to version " + itos(last_probe_version));
			print_line("propagation " + rtos(push_constant.propagation));
			print_line("dynrange " + rtos(push_constant.dynamic_range));
	*/
			RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();

			int passes;
			if (p_update_light_instances) {
				passes = gi->voxel_gi_is_using_two_bounces(probe) ? 2 : 1;
			} else {
				passes = 1; //only re-blitting is necessary
			}
			int wg_size = 64;
			int64_t wg_limit_x = (int64_t)RD::get_singleton()->limit_get(RD::LIMIT_MAX_COMPUTE_WORKGROUP_COUNT_X);

			for (int pass = 0; pass < passes; pass++) {
				if (p_update_light_instances) {
					for (int i = 0; i < mipmaps.size(); i++) {
						if (i == 0) {
							RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->voxel_gi_lighting_shader_version_pipelines[pass == 0 ? VOXEL_GI_SHADER_VERSION_COMPUTE_LIGHT : VOXEL_GI_SHADER_VERSION_COMPUTE_SECOND_BOUNCE]);
						} else if (i == 1) {
							RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->voxel_gi_lighting_shader_version_pipelines[VOXEL_GI_SHADER_VERSION_COMPUTE_MIPMAP]);
						}

						if (pass == 1 || i > 0) {
							RD::get_singleton()->compute_list_add_barrier(compute_list); //wait til previous step is done
						}
						if (pass == 0 || i > 0) {
							RD::get_singleton()->compute_list_bind_uniform_set(compute_list, mipmaps[i].uniform_set, 0);
						} else {
							RD::get_singleton()->compute_list_bind_uniform_set(compute_list, mipmaps[i].second_bounce_uniform_set, 0);
						}

						push_constant.cell_offset = mipmaps[i].cell_offset;
						push_constant.cell_count = mipmaps[i].cell_count;

						int64_t wg_todo = (mipmaps[i].cell_count + wg_size - 1) / wg_size;
						while (wg_todo) {
							int64_t wg_count = MIN(wg_todo, wg_limit_x);
							RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(VoxelGIPushConstant));
							RD::get_singleton()->compute_list_dispatch(compute_list, wg_count, 1, 1);
							wg_todo -= wg_count;
							push_constant.cell_offset += wg_count * wg_size;
						}
					}

					RD::get_singleton()->compute_list_add_barrier(compute_list); //wait til previous step is done
				}

				RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->voxel_gi_lighting_shader_version_pipelines[VOXEL_GI_SHADER_VERSION_WRITE_TEXTURE]);

				for (int i = 0; i < mipmaps.size(); i++) {
					RD::get_singleton()->compute_list_bind_uniform_set(compute_list, mipmaps[i].write_uniform_set, 0);

					push_constant.cell_offset = mipmaps[i].cell_offset;
					push_constant.cell_count = mipmaps[i].cell_count;

					int64_t wg_todo = (mipmaps[i].cell_count + wg_size - 1) / wg_size;
					while (wg_todo) {
						int64_t wg_count = MIN(wg_todo, wg_limit_x);
						RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(VoxelGIPushConstant));
						RD::get_singleton()->compute_list_dispatch(compute_list, wg_count, 1, 1);
						wg_todo -= wg_count;
						push_constant.cell_offset += wg_count * wg_size;
					}
				}
			}

			RD::get_singleton()->compute_list_end();
		}
	}

	has_dynamic_object_data = false; //clear until dynamic object data is used again

	if (p_dynamic_objects.size() && dynamic_maps.size()) {
		Vector3i octree_size = gi->voxel_gi_get_octree_size(probe);
		int multiplier = dynamic_maps[0].size / MAX(MAX(octree_size.x, octree_size.y), octree_size.z);

		Transform3D oversample_scale;
		oversample_scale.basis.scale(Vector3(multiplier, multiplier, multiplier));

		Transform3D to_cell = oversample_scale * gi->voxel_gi_get_to_cell_xform(probe);
		Transform3D to_world_xform = transform * to_cell.affine_inverse();
		Transform3D to_probe_xform = to_world_xform.affine_inverse();

		AABB probe_aabb(Vector3(), octree_size);

		//this could probably be better parallelized in compute..
		for (int i = 0; i < (int)p_dynamic_objects.size(); i++) {
			RenderGeometryInstance *instance = p_dynamic_objects[i];

			//transform aabb to voxel_gi
			AABB aabb = (to_probe_xform * instance->get_transform()).xform(instance->get_aabb());

			//this needs to wrap to grid resolution to avoid jitter
			//also extend margin a bit just in case
			Vector3i begin = aabb.position - Vector3i(1, 1, 1);
			Vector3i end = aabb.position + aabb.size + Vector3i(1, 1, 1);

			for (int j = 0; j < 3; j++) {
				if ((end[j] - begin[j]) & 1) {
					end[j]++; //for half extents split, it needs to be even
				}
				begin[j] = MAX(begin[j], 0);
				end[j] = MIN(end[j], octree_size[j] * multiplier);
			}

			//aabb = aabb.intersection(probe_aabb); //intersect
			aabb.position = begin;
			aabb.size = end - begin;

			//print_line("aabb: " + aabb);

			for (int j = 0; j < 6; j++) {
				//if (j != 0 && j != 3) {
				//	continue;
				//}
				static const Vector3 render_z[6] = {
					Vector3(1, 0, 0),
					Vector3(0, 1, 0),
					Vector3(0, 0, 1),
					Vector3(-1, 0, 0),
					Vector3(0, -1, 0),
					Vector3(0, 0, -1),
				};
				static const Vector3 render_up[6] = {
					Vector3(0, 1, 0),
					Vector3(0, 0, 1),
					Vector3(0, 1, 0),
					Vector3(0, 1, 0),
					Vector3(0, 0, 1),
					Vector3(0, 1, 0),
				};

				Vector3 render_dir = render_z[j];
				Vector3 up_dir = render_up[j];

				Vector3 center = aabb.get_center();
				Transform3D xform;
				xform.set_look_at(center - aabb.size * 0.5 * render_dir, center, up_dir);

				Vector3 x_dir = xform.basis.get_column(0).abs();
				int x_axis = int(Vector3(0, 1, 2).dot(x_dir));
				Vector3 y_dir = xform.basis.get_column(1).abs();
				int y_axis = int(Vector3(0, 1, 2).dot(y_dir));
				Vector3 z_dir = -xform.basis.get_column(2);
				int z_axis = int(Vector3(0, 1, 2).dot(z_dir.abs()));

				Rect2i rect(aabb.position[x_axis], aabb.position[y_axis], aabb.size[x_axis], aabb.size[y_axis]);
				bool x_flip = bool(Vector3(1, 1, 1).dot(xform.basis.get_column(0)) < 0);
				bool y_flip = bool(Vector3(1, 1, 1).dot(xform.basis.get_column(1)) < 0);
				bool z_flip = bool(Vector3(1, 1, 1).dot(xform.basis.get_column(2)) > 0);

				Projection cm;
				cm.set_orthogonal(-rect.size.width / 2, rect.size.width / 2, -rect.size.height / 2, rect.size.height / 2, 0.0001, aabb.size[z_axis]);

				if (RendererSceneRenderRD::get_singleton()->cull_argument.size() == 0) {
					RendererSceneRenderRD::get_singleton()->cull_argument.push_back(nullptr);
				}
				RendererSceneRenderRD::get_singleton()->cull_argument[0] = instance;

				float exposure_normalization = 1.0;
				if (RendererSceneRenderRD::get_singleton()->is_using_physical_light_units()) {
					exposure_normalization = gi->voxel_gi_get_baked_exposure_normalization(probe);
				}

				RendererSceneRenderRD::get_singleton()->_render_material(to_world_xform * xform, cm, true, RendererSceneRenderRD::get_singleton()->cull_argument, dynamic_maps[0].fb, Rect2i(Vector2i(), rect.size), exposure_normalization);

				VoxelGIDynamicPushConstant push_constant;
				memset(&push_constant, 0, sizeof(VoxelGIDynamicPushConstant));
				push_constant.limits[0] = octree_size.x;
				push_constant.limits[1] = octree_size.y;
				push_constant.limits[2] = octree_size.z;
				push_constant.light_count = p_light_instances.size();
				push_constant.x_dir[0] = x_dir[0];
				push_constant.x_dir[1] = x_dir[1];
				push_constant.x_dir[2] = x_dir[2];
				push_constant.y_dir[0] = y_dir[0];
				push_constant.y_dir[1] = y_dir[1];
				push_constant.y_dir[2] = y_dir[2];
				push_constant.z_dir[0] = z_dir[0];
				push_constant.z_dir[1] = z_dir[1];
				push_constant.z_dir[2] = z_dir[2];
				push_constant.z_base = xform.origin[z_axis];
				push_constant.z_sign = (z_flip ? -1.0 : 1.0);
				push_constant.pos_multiplier = float(1.0) / multiplier;
				push_constant.dynamic_range = gi->voxel_gi_get_dynamic_range(probe);
				push_constant.flip_x = x_flip;
				push_constant.flip_y = y_flip;
				push_constant.rect_pos[0] = rect.position[0];
				push_constant.rect_pos[1] = rect.position[1];
				push_constant.rect_size[0] = rect.size[0];
				push_constant.rect_size[1] = rect.size[1];
				push_constant.prev_rect_ofs[0] = 0;
				push_constant.prev_rect_ofs[1] = 0;
				push_constant.prev_rect_size[0] = 0;
				push_constant.prev_rect_size[1] = 0;
				push_constant.on_mipmap = false;
				push_constant.propagation = gi->voxel_gi_get_propagation(probe);
				push_constant.pad[0] = 0;
				push_constant.pad[1] = 0;
				push_constant.pad[2] = 0;

				//process lighting
				RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();
				RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->voxel_gi_lighting_shader_version_pipelines[VOXEL_GI_SHADER_VERSION_DYNAMIC_OBJECT_LIGHTING]);
				RD::get_singleton()->compute_list_bind_uniform_set(compute_list, dynamic_maps[0].uniform_set, 0);
				RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(VoxelGIDynamicPushConstant));
				RD::get_singleton()->compute_list_dispatch(compute_list, (rect.size.x - 1) / 8 + 1, (rect.size.y - 1) / 8 + 1, 1);
				//print_line("rect: " + itos(i) + ": " + rect);

				for (int k = 1; k < dynamic_maps.size(); k++) {
					// enlarge the rect if needed so all pixels fit when downscaled,
					// this ensures downsampling is smooth and optimal because no pixels are left behind

					//x
					if (rect.position.x & 1) {
						rect.size.x++;
						push_constant.prev_rect_ofs[0] = 1; //this is used to ensure reading is also optimal
					} else {
						push_constant.prev_rect_ofs[0] = 0;
					}
					if (rect.size.x & 1) {
						rect.size.x++;
					}

					rect.position.x >>= 1;
					rect.size.x = MAX(1, rect.size.x >> 1);

					//y
					if (rect.position.y & 1) {
						rect.size.y++;
						push_constant.prev_rect_ofs[1] = 1;
					} else {
						push_constant.prev_rect_ofs[1] = 0;
					}
					if (rect.size.y & 1) {
						rect.size.y++;
					}

					rect.position.y >>= 1;
					rect.size.y = MAX(1, rect.size.y >> 1);

					//shrink limits to ensure plot does not go outside map
					if (dynamic_maps[k].mipmap > 0) {
						for (int l = 0; l < 3; l++) {
							push_constant.limits[l] = MAX(1, push_constant.limits[l] >> 1);
						}
					}

					//print_line("rect: " + itos(i) + ": " + rect);
					push_constant.rect_pos[0] = rect.position[0];
					push_constant.rect_pos[1] = rect.position[1];
					push_constant.prev_rect_size[0] = push_constant.rect_size[0];
					push_constant.prev_rect_size[1] = push_constant.rect_size[1];
					push_constant.rect_size[0] = rect.size[0];
					push_constant.rect_size[1] = rect.size[1];
					push_constant.on_mipmap = dynamic_maps[k].mipmap > 0;

					RD::get_singleton()->compute_list_add_barrier(compute_list);

					if (dynamic_maps[k].mipmap < 0) {
						RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->voxel_gi_lighting_shader_version_pipelines[VOXEL_GI_SHADER_VERSION_DYNAMIC_SHRINK_WRITE]);
					} else if (k < dynamic_maps.size() - 1) {
						RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->voxel_gi_lighting_shader_version_pipelines[VOXEL_GI_SHADER_VERSION_DYNAMIC_SHRINK_WRITE_PLOT]);
					} else {
						RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, gi->voxel_gi_lighting_shader_version_pipelines[VOXEL_GI_SHADER_VERSION_DYNAMIC_SHRINK_PLOT]);
					}
					RD::get_singleton()->compute_list_bind_uniform_set(compute_list, dynamic_maps[k].uniform_set, 0);
					RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(VoxelGIDynamicPushConstant));
					RD::get_singleton()->compute_list_dispatch(compute_list, (rect.size.x - 1) / 8 + 1, (rect.size.y - 1) / 8 + 1, 1);
				}

				RD::get_singleton()->compute_list_end();
			}
		}

		has_dynamic_object_data = true; //clear until dynamic object data is used again
	}

	last_probe_version = gi->voxel_gi_get_version(probe);
}

void GI::VoxelGIInstance::free_resources() {
	if (texture.is_valid()) {
		RD::get_singleton()->free(texture);
		RD::get_singleton()->free(write_buffer);

		texture = RID();
		write_buffer = RID();
		mipmaps.clear();
	}

	for (int i = 0; i < dynamic_maps.size(); i++) {
		RD::get_singleton()->free(dynamic_maps[i].texture);
		RD::get_singleton()->free(dynamic_maps[i].depth);

		// these only exist on the first level...
		if (dynamic_maps[i].fb_depth.is_valid()) {
			RD::get_singleton()->free(dynamic_maps[i].fb_depth);
		}
		if (dynamic_maps[i].albedo.is_valid()) {
			RD::get_singleton()->free(dynamic_maps[i].albedo);
		}
		if (dynamic_maps[i].normal.is_valid()) {
			RD::get_singleton()->free(dynamic_maps[i].normal);
		}
		if (dynamic_maps[i].orm.is_valid()) {
			RD::get_singleton()->free(dynamic_maps[i].orm);
		}
	}
	dynamic_maps.clear();
}

void GI::VoxelGIInstance::debug(RD::DrawListID p_draw_list, RID p_framebuffer, const Projection &p_camera_with_transform, bool p_lighting, bool p_emission, float p_alpha) {
	RendererRD::MaterialStorage *material_storage = RendererRD::MaterialStorage::get_singleton();

	if (mipmaps.size() == 0) {
		return;
	}

	Projection cam_transform = (p_camera_with_transform * Projection(transform)) * Projection(gi->voxel_gi_get_to_cell_xform(probe).affine_inverse());

	int level = 0;
	Vector3i octree_size = gi->voxel_gi_get_octree_size(probe);

	VoxelGIDebugPushConstant push_constant;
	push_constant.alpha = p_alpha;
	push_constant.dynamic_range = gi->voxel_gi_get_dynamic_range(probe);
	push_constant.cell_offset = mipmaps[level].cell_offset;
	push_constant.level = level;

	push_constant.bounds[0] = octree_size.x >> level;
	push_constant.bounds[1] = octree_size.y >> level;
	push_constant.bounds[2] = octree_size.z >> level;
	push_constant.pad = 0;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			push_constant.projection[i * 4 + j] = cam_transform.columns[i][j];
		}
	}

	if (gi->voxel_gi_debug_uniform_set.is_valid()) {
		RD::get_singleton()->free(gi->voxel_gi_debug_uniform_set);
	}
	Vector<RD::Uniform> uniforms;
	{
		RD::Uniform u;
		u.uniform_type = RD::UNIFORM_TYPE_STORAGE_BUFFER;
		u.binding = 1;
		u.append_id(gi->voxel_gi_get_data_buffer(probe));
		uniforms.push_back(u);
	}
	{
		RD::Uniform u;
		u.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
		u.binding = 2;
		u.append_id(texture);
		uniforms.push_back(u);
	}
	{
		RD::Uniform u;
		u.uniform_type = RD::UNIFORM_TYPE_SAMPLER;
		u.binding = 3;
		u.append_id(material_storage->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_NEAREST, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED));
		uniforms.push_back(u);
	}

	int cell_count;
	if (!p_emission && p_lighting && has_dynamic_object_data) {
		cell_count = push_constant.bounds[0] * push_constant.bounds[1] * push_constant.bounds[2];
	} else {
		cell_count = mipmaps[level].cell_count;
	}

	gi->voxel_gi_debug_uniform_set = RD::get_singleton()->uniform_set_create(uniforms, gi->voxel_gi_debug_shader_version_shaders[0], 0);

	int voxel_gi_debug_pipeline = VOXEL_GI_DEBUG_COLOR;
	if (p_emission) {
		voxel_gi_debug_pipeline = VOXEL_GI_DEBUG_EMISSION;
	} else if (p_lighting) {
		voxel_gi_debug_pipeline = has_dynamic_object_data ? VOXEL_GI_DEBUG_LIGHT_FULL : VOXEL_GI_DEBUG_LIGHT;
	}
	RD::get_singleton()->draw_list_bind_render_pipeline(
			p_draw_list,
			gi->voxel_gi_debug_shader_version_pipelines[voxel_gi_debug_pipeline].get_render_pipeline(RD::INVALID_ID, RD::get_singleton()->framebuffer_get_format(p_framebuffer)));
	RD::get_singleton()->draw_list_bind_uniform_set(p_draw_list, gi->voxel_gi_debug_uniform_set, 0);
	RD::get_singleton()->draw_list_set_push_constant(p_draw_list, &push_constant, sizeof(VoxelGIDebugPushConstant));
	RD::get_singleton()->draw_list_draw(p_draw_list, false, cell_count, 36);
}

////////////////////////////////////////////////////////////////////////////////
// GI

GI::GI() {
	singleton = this;

	sdfgi_ray_count = RS::EnvironmentSDFGIRayCount(CLAMP(int32_t(GLOBAL_GET("rendering/global_illumination/sdfgi/probe_ray_count")), 0, int32_t(RS::ENV_SDFGI_RAY_COUNT_MAX - 1)));
	sdfgi_frames_to_converge = RS::EnvironmentSDFGIFramesToConverge(CLAMP(int32_t(GLOBAL_GET("rendering/global_illumination/sdfgi/frames_to_converge")), 0, int32_t(RS::ENV_SDFGI_CONVERGE_MAX - 1)));
	sdfgi_frames_to_update_light = RS::EnvironmentSDFGIFramesToUpdateLight(CLAMP(int32_t(GLOBAL_GET("rendering/global_illumination/sdfgi/frames_to_update_lights")), 0, int32_t(RS::ENV_SDFGI_UPDATE_LIGHT_MAX - 1)));
}

GI::~GI() {
	singleton = nullptr;
}

void GI::init(SkyRD *p_sky) {
	/* GI */

	{
		//kinda complicated to compute the amount of slots, we try to use as many as we can

		voxel_gi_lights = memnew_arr(VoxelGILight, voxel_gi_max_lights);
		voxel_gi_lights_uniform = RD::get_singleton()->uniform_buffer_create(voxel_gi_max_lights * sizeof(VoxelGILight));
		voxel_gi_quality = RS::VoxelGIQuality(CLAMP(int(GLOBAL_GET("rendering/global_illumination/voxel_gi/quality")), 0, 1));

		String defines = "\n#define MAX_LIGHTS " + itos(voxel_gi_max_lights) + "\n";

		Vector<String> versions;
		versions.push_back("\n#define MODE_COMPUTE_LIGHT\n");
		versions.push_back("\n#define MODE_SECOND_BOUNCE\n");
		versions.push_back("\n#define MODE_UPDATE_MIPMAPS\n");
		versions.push_back("\n#define MODE_WRITE_TEXTURE\n");
		versions.push_back("\n#define MODE_DYNAMIC\n#define MODE_DYNAMIC_LIGHTING\n");
		versions.push_back("\n#define MODE_DYNAMIC\n#define MODE_DYNAMIC_SHRINK\n#define MODE_DYNAMIC_SHRINK_WRITE\n");
		versions.push_back("\n#define MODE_DYNAMIC\n#define MODE_DYNAMIC_SHRINK\n#define MODE_DYNAMIC_SHRINK_PLOT\n");
		versions.push_back("\n#define MODE_DYNAMIC\n#define MODE_DYNAMIC_SHRINK\n#define MODE_DYNAMIC_SHRINK_PLOT\n#define MODE_DYNAMIC_SHRINK_WRITE\n");

		voxel_gi_shader.initialize(versions, defines);
		voxel_gi_lighting_shader_version = voxel_gi_shader.version_create();
		for (int i = 0; i < VOXEL_GI_SHADER_VERSION_MAX; i++) {
			voxel_gi_lighting_shader_version_shaders[i] = voxel_gi_shader.version_get_shader(voxel_gi_lighting_shader_version, i);
			voxel_gi_lighting_shader_version_pipelines[i] = RD::get_singleton()->compute_pipeline_create(voxel_gi_lighting_shader_version_shaders[i]);
		}
	}

	{
		String defines;
		Vector<String> versions;
		versions.push_back("\n#define MODE_DEBUG_COLOR\n");
		versions.push_back("\n#define MODE_DEBUG_LIGHT\n");
		versions.push_back("\n#define MODE_DEBUG_EMISSION\n");
		versions.push_back("\n#define MODE_DEBUG_LIGHT\n#define MODE_DEBUG_LIGHT_FULL\n");

		voxel_gi_debug_shader.initialize(versions, defines);
		voxel_gi_debug_shader_version = voxel_gi_debug_shader.version_create();
		for (int i = 0; i < VOXEL_GI_DEBUG_MAX; i++) {
			voxel_gi_debug_shader_version_shaders[i] = voxel_gi_debug_shader.version_get_shader(voxel_gi_debug_shader_version, i);

			RD::PipelineRasterizationState rs;
			rs.cull_mode = RD::POLYGON_CULL_FRONT;
			RD::PipelineDepthStencilState ds;
			ds.enable_depth_test = true;
			ds.enable_depth_write = true;
			ds.depth_compare_operator = RD::COMPARE_OP_LESS_OR_EQUAL;

			voxel_gi_debug_shader_version_pipelines[i].setup(voxel_gi_debug_shader_version_shaders[i], RD::RENDER_PRIMITIVE_TRIANGLES, rs, RD::PipelineMultisampleState(), ds, RD::PipelineColorBlendState::create_disabled(), 0);
		}
	}

	/* SDGFI */

	{
		Vector<String> preprocess_modes;
		preprocess_modes.push_back("\n#define MODE_REGION_STORE\n");
		preprocess_modes.push_back("\n#define MODE_LIGHT_STORE\n");
		preprocess_modes.push_back("\n#define MODE_LIGHT_SCROLL\n");
		preprocess_modes.push_back("\n#define MODE_OCCLUSION\n");
		preprocess_modes.push_back("\n#define MODE_OCCLUSION_STORE\n");
		preprocess_modes.push_back("\n#define MODE_LIGHTPROBE_SCROLL\n");

		String defines = "\n#define LIGHTPROBE_OCT_SIZE " + itos(SDFGI::LIGHTPROBE_OCT_SIZE) + "\n#define OCCLUSION_OCT_SIZE " + itos(SDFGI::OCCLUSION_OCT_SIZE) + "\n";

		sdfgi_shader.preprocess.initialize(preprocess_modes, defines);
		sdfgi_shader.preprocess_shader = sdfgi_shader.preprocess.version_create();
		for (int i = 0; i < SDFGIShader::PRE_PROCESS_MAX; i++) {
			sdfgi_shader.preprocess_shader_version[i] = sdfgi_shader.preprocess.version_get_shader(sdfgi_shader.preprocess_shader, i);
			sdfgi_shader.preprocess_pipeline[i] = RD::get_singleton()->compute_pipeline_create(sdfgi_shader.preprocess_shader_version[i]);
		}
	}

	{
		//calculate tables
		String defines = "\n#define LIGHTPROBE_OCT_SIZE " + itos(SDFGI::LIGHTPROBE_OCT_SIZE) + "\n#define OCCLUSION_OCT_SIZE " + itos(SDFGI::OCCLUSION_OCT_SIZE) + "\n";

		Vector<String> direct_light_modes;
		direct_light_modes.push_back("\n#define MODE_PROCESS_STATIC\n");
		direct_light_modes.push_back("\n#define MODE_PROCESS_DYNAMIC\n");
		sdfgi_shader.direct_light.initialize(direct_light_modes, defines);
		sdfgi_shader.direct_light_shader = sdfgi_shader.direct_light.version_create();
		for (int i = 0; i < SDFGIShader::DIRECT_LIGHT_MODE_MAX; i++) {
			sdfgi_shader.direct_light_shader_version[i] = sdfgi_shader.direct_light.version_get_shader(sdfgi_shader.direct_light_shader, i);
			sdfgi_shader.direct_light_pipeline[i] = RD::get_singleton()->compute_pipeline_create(sdfgi_shader.direct_light_shader_version[i]);
		}
	}

	{
		//calculate tables
		String defines = "\n#define LIGHTPROBE_OCT_SIZE " + itos(SDFGI::LIGHTPROBE_OCT_SIZE) + "\n#define OCCLUSION_OCT_SIZE " + itos(SDFGI::OCCLUSION_OCT_SIZE) + "\n";

		defines += "\n#define SH_SIZE " + itos(SDFGI::SH_SIZE) + "\n";
		if (p_sky->sky_use_cubemap_array) {
			defines += "\n#define USE_CUBEMAP_ARRAY\n";
		}

		Vector<String> integrate_modes;
		integrate_modes.push_back("\n#define MODE_PROCESS\n");
		integrate_modes.push_back("\n#define MODE_FILTER\n");
		sdfgi_shader.integrate.initialize(integrate_modes, defines);
		sdfgi_shader.integrate_shader = sdfgi_shader.integrate.version_create();

		for (int i = 0; i < SDFGIShader::INTEGRATE_MODE_MAX; i++) {
			sdfgi_shader.integrate_shader_version[i] = sdfgi_shader.integrate.version_get_shader(sdfgi_shader.integrate_shader, i);
			sdfgi_shader.integrate_pipeline[i] = RD::get_singleton()->compute_pipeline_create(sdfgi_shader.integrate_shader_version[i]);
		}
	}

	//GK
	{
		//calculate tables
		String defines = "\n#define LIGHTPROBE_OCT_SIZE " + itos(SDFGI::LIGHTPROBE_OCT_SIZE) + "\n#define OCCLUSION_OCT_SIZE " + itos(SDFGI::OCCLUSION_OCT_SIZE) + "\n";
		if (RendererSceneRenderRD::get_singleton()->is_vrs_supported()) {
			defines += "\n#define USE_VRS\n";
		}
		if (!RD::get_singleton()->sampler_is_format_supported_for_filter(RD::DATA_FORMAT_R8G8_UINT, RD::SAMPLER_FILTER_LINEAR)) {
			defines += "\n#define SAMPLE_VOXEL_GI_NEAREST\n";
		}

		Vector<String> gi_modes;

		gi_modes.push_back("\n#define USE_VOXEL_GI_INSTANCES\n"); // MODE_VOXEL_GI
		gi_modes.push_back("\n#define USE_SDFGI\n"); // MODE_SDFGI
		gi_modes.push_back("\n#define USE_SDFGI\n\n#define USE_VOXEL_GI_INSTANCES\n"); // MODE_COMBINED

		shader.initialize(gi_modes, defines);
		shader_version = shader.version_create();

		Vector<RD::PipelineSpecializationConstant> specialization_constants;

		{
			RD::PipelineSpecializationConstant sc;
			sc.type = RD::PIPELINE_SPECIALIZATION_CONSTANT_TYPE_BOOL;
			sc.constant_id = 0; // SHADER_SPECIALIZATION_HALF_RES
			sc.bool_value = false;
			specialization_constants.push_back(sc);

			sc.type = RD::PIPELINE_SPECIALIZATION_CONSTANT_TYPE_BOOL;
			sc.constant_id = 1; // SHADER_SPECIALIZATION_USE_FULL_PROJECTION_MATRIX
			sc.bool_value = false;
			specialization_constants.push_back(sc);

			sc.type = RD::PIPELINE_SPECIALIZATION_CONSTANT_TYPE_BOOL;
			sc.constant_id = 2; // SHADER_SPECIALIZATION_USE_VRS
			sc.bool_value = false;
			specialization_constants.push_back(sc);
		}

		for (int v = 0; v < SHADER_SPECIALIZATION_VARIATIONS; v++) {
			specialization_constants.ptrw()[0].bool_value = (v & SHADER_SPECIALIZATION_HALF_RES) ? true : false;
			specialization_constants.ptrw()[1].bool_value = (v & SHADER_SPECIALIZATION_USE_FULL_PROJECTION_MATRIX) ? true : false;
			specialization_constants.ptrw()[2].bool_value = (v & SHADER_SPECIALIZATION_USE_VRS) ? true : false;
			for (int i = 0; i < MODE_MAX; i++) {
				pipelines[v][i] = RD::get_singleton()->compute_pipeline_create(shader.version_get_shader(shader_version, i), specialization_constants);
			}
		}

		sdfgi_ubo = RD::get_singleton()->uniform_buffer_create(sizeof(SDFGIData));
	}
	{
		String defines = "\n#define LIGHTPROBE_OCT_SIZE " + itos(SDFGI::LIGHTPROBE_OCT_SIZE) + "\n#define OCCLUSION_OCT_SIZE " + itos(SDFGI::OCCLUSION_OCT_SIZE) + "\n";
		Vector<String> debug_modes;
		debug_modes.push_back("");
		sdfgi_shader.debug.initialize(debug_modes, defines);
		sdfgi_shader.debug_shader = sdfgi_shader.debug.version_create();
		sdfgi_shader.debug_shader_version = sdfgi_shader.debug.version_get_shader(sdfgi_shader.debug_shader, 0);
		sdfgi_shader.debug_pipeline = RD::get_singleton()->compute_pipeline_create(sdfgi_shader.debug_shader_version);
	}
	{
		String defines = "\n#define LIGHTPROBE_OCT_SIZE " + itos(SDFGI::LIGHTPROBE_OCT_SIZE) + "\n#define OCCLUSION_OCT_SIZE " + itos(SDFGI::OCCLUSION_OCT_SIZE) + "\n";

		Vector<String> versions;
		versions.push_back("\n#define MODE_PROBES\n");
		versions.push_back("\n#define MODE_PROBES\n#define USE_MULTIVIEW\n");
		versions.push_back("\n#define MODE_OCCLUSION_FILTER\n");
		versions.push_back("\n#define MODE_OCCLUSION_FILTER\n#define USE_MULTIVIEW\n");
		versions.push_back("\n#define MODE_OCCLUSION_LINES\n");
		versions.push_back("\n#define MODE_OCCLUSION_LINES\n#define USE_MULTIVIEW\n");

		sdfgi_shader.debug_probes.initialize(versions, defines);

		// TODO disable multiview versions if turned off

		sdfgi_shader.debug_probes_shader = sdfgi_shader.debug_probes.version_create();

		{
			RD::PipelineRasterizationState rs;
			rs.cull_mode = RD::POLYGON_CULL_DISABLED;
			RD::PipelineDepthStencilState ds;
			ds.enable_depth_test = true;
			ds.enable_depth_write = true;
			ds.depth_compare_operator = RD::COMPARE_OP_LESS_OR_EQUAL;
			RD::PipelineColorBlendState cb = RD::PipelineColorBlendState::create_disabled();
			RD::RenderPrimitive rp = RD::RENDER_PRIMITIVE_TRIANGLE_STRIPS;

			for (int i = 0; i < SDFGIShader::PROBE_DEBUG_MAX; i++) {
				// TODO check if version is enabled

				if (i == SDFGIShader::PROBE_DEBUG_OCCLUSION || i == SDFGIShader::PROBE_DEBUG_OCCLUSION_MULTIVIEW) {
					rs.wireframe = true;
					cb = RD::PipelineColorBlendState::create_blend();
				} else if (i == SDFGIShader::PROBE_DEBUG_LINES || i == SDFGIShader::PROBE_DEBUG_LINES_MULTIVIEW) {
					rs.wireframe = false;
					rp = RD::RENDER_PRIMITIVE_LINES;
				}

				sdfgi_shader.debug_probes_shader_version[i] = sdfgi_shader.debug_probes.version_get_shader(sdfgi_shader.debug_probes_shader, i);
				sdfgi_shader.debug_probes_pipeline[i].setup(sdfgi_shader.debug_probes_shader_version[i], rp, rs, RD::PipelineMultisampleState(), ds, cb, 0);
			}
		}
	}
	default_voxel_gi_buffer = RD::get_singleton()->uniform_buffer_create(sizeof(VoxelGIData) * MAX_VOXEL_GI_INSTANCES);
	half_resolution = GLOBAL_GET("rendering/global_illumination/gi/use_half_resolution");
}

void GI::free() {
	if (default_voxel_gi_buffer.is_valid()) {
		RD::get_singleton()->free(default_voxel_gi_buffer);
	}
	if (voxel_gi_lights_uniform.is_valid()) {
		RD::get_singleton()->free(voxel_gi_lights_uniform);
	}
	if (sdfgi_ubo.is_valid()) {
		RD::get_singleton()->free(sdfgi_ubo);
	}

	if (voxel_gi_debug_shader_version.is_valid()) {
		voxel_gi_debug_shader.version_free(voxel_gi_debug_shader_version);
	}
	if (voxel_gi_lighting_shader_version.is_valid()) {
		voxel_gi_shader.version_free(voxel_gi_lighting_shader_version);
	}
	if (shader_version.is_valid()) {
		shader.version_free(shader_version);
	}
	if (sdfgi_shader.debug_probes_shader.is_valid()) {
		sdfgi_shader.debug_probes.version_free(sdfgi_shader.debug_probes_shader);
	}
	if (sdfgi_shader.debug_shader.is_valid()) {
		sdfgi_shader.debug.version_free(sdfgi_shader.debug_shader);
	}
	if (sdfgi_shader.direct_light_shader.is_valid()) {
		sdfgi_shader.direct_light.version_free(sdfgi_shader.direct_light_shader);
	}
	if (sdfgi_shader.integrate_shader.is_valid()) {
		sdfgi_shader.integrate.version_free(sdfgi_shader.integrate_shader);
	}
	if (sdfgi_shader.preprocess_shader.is_valid()) {
		sdfgi_shader.preprocess.version_free(sdfgi_shader.preprocess_shader);
	}

	if (voxel_gi_lights) {
		memdelete_arr(voxel_gi_lights);
	}
}

Ref<GI::SDFGI> GI::create_sdfgi(RID p_env, const Vector3 &p_world_position, uint32_t p_requested_history_size) {
	Ref<SDFGI> sdfgi;
	sdfgi.instantiate();

	sdfgi->create(p_env, p_world_position, p_requested_history_size, this);

	return sdfgi;
}

void GI::setup_voxel_gi_instances(RenderDataRD *p_render_data, Ref<RenderSceneBuffersRD> p_render_buffers, const Transform3D &p_transform, const PagedArray<RID> &p_voxel_gi_instances, uint32_t &r_voxel_gi_instances_used) {
	ERR_FAIL_COND(p_render_buffers.is_null());

	RendererRD::TextureStorage *texture_storage = RendererRD::TextureStorage::get_singleton();
	ERR_FAIL_NULL(texture_storage);

	r_voxel_gi_instances_used = 0;

	Ref<RenderBuffersGI> rbgi = p_render_buffers->get_custom_data(RB_SCOPE_GI);
	ERR_FAIL_COND(rbgi.is_null());

	RID voxel_gi_buffer = rbgi->get_voxel_gi_buffer();
	VoxelGIData voxel_gi_data[MAX_VOXEL_GI_INSTANCES];

	bool voxel_gi_instances_changed = false;

	Transform3D to_camera;
	to_camera.origin = p_transform.origin; //only translation, make local

	for (int i = 0; i < MAX_VOXEL_GI_INSTANCES; i++) {
		RID texture;
		if (i < (int)p_voxel_gi_instances.size()) {
			VoxelGIInstance *gipi = voxel_gi_instance_owner.get_or_null(p_voxel_gi_instances[i]);

			if (gipi) {
				texture = gipi->texture;
				VoxelGIData &gipd = voxel_gi_data[i];

				RID base_probe = gipi->probe;

				Transform3D to_cell = voxel_gi_get_to_cell_xform(gipi->probe) * gipi->transform.affine_inverse() * to_camera;

				gipd.xform[0] = to_cell.basis.rows[0][0];
				gipd.xform[1] = to_cell.basis.rows[1][0];
				gipd.xform[2] = to_cell.basis.rows[2][0];
				gipd.xform[3] = 0;
				gipd.xform[4] = to_cell.basis.rows[0][1];
				gipd.xform[5] = to_cell.basis.rows[1][1];
				gipd.xform[6] = to_cell.basis.rows[2][1];
				gipd.xform[7] = 0;
				gipd.xform[8] = to_cell.basis.rows[0][2];
				gipd.xform[9] = to_cell.basis.rows[1][2];
				gipd.xform[10] = to_cell.basis.rows[2][2];
				gipd.xform[11] = 0;
				gipd.xform[12] = to_cell.origin.x;
				gipd.xform[13] = to_cell.origin.y;
				gipd.xform[14] = to_cell.origin.z;
				gipd.xform[15] = 1;

				Vector3 bounds = voxel_gi_get_octree_size(base_probe);

				gipd.bounds[0] = bounds.x;
				gipd.bounds[1] = bounds.y;
				gipd.bounds[2] = bounds.z;

				gipd.dynamic_range = voxel_gi_get_dynamic_range(base_probe) * voxel_gi_get_energy(base_probe);
				gipd.bias = voxel_gi_get_bias(base_probe);
				gipd.normal_bias = voxel_gi_get_normal_bias(base_probe);
				gipd.blend_ambient = !voxel_gi_is_interior(base_probe);
				gipd.mipmaps = gipi->mipmaps.size();
				gipd.exposure_normalization = 1.0;
				if (p_render_data->camera_attributes.is_valid()) {
					float exposure_normalization = RSG::camera_attributes->camera_attributes_get_exposure_normalization_factor(p_render_data->camera_attributes);
					gipd.exposure_normalization = exposure_normalization / voxel_gi_get_baked_exposure_normalization(base_probe);
				}
			}

			r_voxel_gi_instances_used++;
		}

		if (texture == RID()) {
			texture = texture_storage->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_3D_WHITE);
		}

		if (texture != rbgi->voxel_gi_textures[i]) {
			voxel_gi_instances_changed = true;
			rbgi->voxel_gi_textures[i] = texture;
		}
	}

	if (voxel_gi_instances_changed) {
		for (uint32_t v = 0; v < RendererSceneRender::MAX_RENDER_VIEWS; v++) {
			if (RD::get_singleton()->uniform_set_is_valid(rbgi->uniform_set[v])) {
				RD::get_singleton()->free(rbgi->uniform_set[v]);
			}
			rbgi->uniform_set[v] = RID();
		}
	}

	if (p_voxel_gi_instances.size() > 0) {
		RD::get_singleton()->draw_command_begin_label("VoxelGIs Setup");

		RD::get_singleton()->buffer_update(voxel_gi_buffer, 0, sizeof(VoxelGIData) * MIN((uint64_t)MAX_VOXEL_GI_INSTANCES, p_voxel_gi_instances.size()), voxel_gi_data, RD::BARRIER_MASK_COMPUTE);

		RD::get_singleton()->draw_command_end_label();
	}
}

RID GI::RenderBuffersGI::get_voxel_gi_buffer() {
	if (voxel_gi_buffer.is_null()) {
		voxel_gi_buffer = RD::get_singleton()->uniform_buffer_create(sizeof(GI::VoxelGIData) * GI::MAX_VOXEL_GI_INSTANCES);
	}
	return voxel_gi_buffer;
}

void GI::RenderBuffersGI::free_data() {
	for (uint32_t v = 0; v < RendererSceneRender::MAX_RENDER_VIEWS; v++) {
		if (RD::get_singleton()->uniform_set_is_valid(uniform_set[v])) {
			RD::get_singleton()->free(uniform_set[v]);
		}
		uniform_set[v] = RID();
	}

	if (scene_data_ubo.is_valid()) {
		RD::get_singleton()->free(scene_data_ubo);
		scene_data_ubo = RID();
	}

	if (voxel_gi_buffer.is_valid()) {
		RD::get_singleton()->free(voxel_gi_buffer);
		voxel_gi_buffer = RID();
	}
}

void GI::process_gi(Ref<RenderSceneBuffersRD> p_render_buffers, const RID *p_normal_roughness_slices, RID p_voxel_gi_buffer, RID p_environment, uint32_t p_view_count, const Projection *p_projections, const Vector3 *p_eye_offsets, const Transform3D &p_cam_transform, const PagedArray<RID> &p_voxel_gi_instances) {
	RendererRD::TextureStorage *texture_storage = RendererRD::TextureStorage::get_singleton();

	ERR_FAIL_COND_MSG(p_view_count > 2, "Maximum of 2 views supported for Processing GI.");

	RD::get_singleton()->draw_command_begin_label("GI Render");

	ERR_FAIL_COND(p_render_buffers.is_null());

	Ref<RenderBuffersGI> rbgi = p_render_buffers->get_custom_data(RB_SCOPE_GI);
	ERR_FAIL_COND(rbgi.is_null());

	Size2i internal_size = p_render_buffers->get_internal_size();

	if (rbgi->using_half_size_gi != half_resolution) {
		p_render_buffers->clear_context(RB_SCOPE_GI);
	}

	if (!p_render_buffers->has_texture(RB_SCOPE_GI, RB_TEX_AMBIENT)) {
		Size2i size = internal_size;
		uint32_t usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_STORAGE_BIT;

		if (half_resolution) {
			size.x >>= 1;
			size.y >>= 1;
		}

		p_render_buffers->create_texture(RB_SCOPE_GI, RB_TEX_AMBIENT, RD::DATA_FORMAT_R16G16B16A16_SFLOAT, usage_bits, RD::TEXTURE_SAMPLES_1, size);
		p_render_buffers->create_texture(RB_SCOPE_GI, RB_TEX_REFLECTION, RD::DATA_FORMAT_R16G16B16A16_SFLOAT, usage_bits, RD::TEXTURE_SAMPLES_1, size);

		rbgi->using_half_size_gi = half_resolution;
	}

	// Setup our scene data
	{
		SceneData scene_data;

		if (rbgi->scene_data_ubo.is_null()) {
			rbgi->scene_data_ubo = RD::get_singleton()->uniform_buffer_create(sizeof(SceneData));
		}

		for (uint32_t v = 0; v < p_view_count; v++) {
			RendererRD::MaterialStorage::store_camera(p_projections[v].inverse(), scene_data.inv_projection[v]);
			scene_data.eye_offset[v][0] = p_eye_offsets[v].x;
			scene_data.eye_offset[v][1] = p_eye_offsets[v].y;
			scene_data.eye_offset[v][2] = p_eye_offsets[v].z;
			scene_data.eye_offset[v][3] = 0.0;
		}

		// Note that we will be ignoring the origin of this transform.
		RendererRD::MaterialStorage::store_transform(p_cam_transform, scene_data.cam_transform);

		scene_data.screen_size[0] = internal_size.x;
		scene_data.screen_size[1] = internal_size.y;

		RD::get_singleton()->buffer_update(rbgi->scene_data_ubo, 0, sizeof(SceneData), &scene_data, RD::BARRIER_MASK_COMPUTE);
	}

	// Now compute the contents of our buffers.
	RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin(true);

	// Render each eye separately.
	// We need to look into whether we can make our compute shader use Multiview but not sure that works or makes a difference..

	// setup our push constant

	PushConstant push_constant;

	push_constant.max_voxel_gi_instances = MIN((uint64_t)MAX_VOXEL_GI_INSTANCES, p_voxel_gi_instances.size());
	push_constant.high_quality_vct = voxel_gi_quality == RS::VOXEL_GI_QUALITY_HIGH;

	// these should be the same for all views
	push_constant.orthogonal = p_projections[0].is_orthogonal();
	push_constant.z_near = p_projections[0].get_z_near();
	push_constant.z_far = p_projections[0].get_z_far();

	// these are only used if we have 1 view, else we use the projections in our scene data
	push_constant.proj_info[0] = -2.0f / (internal_size.x * p_projections[0].columns[0][0]);
	push_constant.proj_info[1] = -2.0f / (internal_size.y * p_projections[0].columns[1][1]);
	push_constant.proj_info[2] = (1.0f - p_projections[0].columns[0][2]) / p_projections[0].columns[0][0];
	push_constant.proj_info[3] = (1.0f + p_projections[0].columns[1][2]) / p_projections[0].columns[1][1];

	bool use_sdfgi = p_render_buffers->has_custom_data(RB_SCOPE_SDFGI);
	bool use_voxel_gi_instances = push_constant.max_voxel_gi_instances > 0;

	Ref<SDFGI> sdfgi;
	if (use_sdfgi) {
		sdfgi = p_render_buffers->get_custom_data(RB_SCOPE_SDFGI);
	}

	uint32_t pipeline_specialization = 0;
	if (rbgi->using_half_size_gi) {
		pipeline_specialization |= SHADER_SPECIALIZATION_HALF_RES;
	}
	if (p_view_count > 1) {
		pipeline_specialization |= SHADER_SPECIALIZATION_USE_FULL_PROJECTION_MATRIX;
	}
	bool has_vrs_texture = p_render_buffers->has_texture(RB_SCOPE_VRS, RB_TEXTURE);
	if (has_vrs_texture) {
		pipeline_specialization |= SHADER_SPECIALIZATION_USE_VRS;
	}

	Mode mode = (use_sdfgi && use_voxel_gi_instances) ? MODE_COMBINED : (use_sdfgi ? MODE_SDFGI : MODE_VOXEL_GI);

	for (uint32_t v = 0; v < p_view_count; v++) {
		push_constant.view_index = v;

		// setup our uniform set
		if (rbgi->uniform_set[v].is_null() || !RD::get_singleton()->uniform_set_is_valid(rbgi->uniform_set[v])) {
			RD::Uniform vgiu;
			vgiu.uniform_type = RD::UNIFORM_TYPE_TEXTURE;
			vgiu.binding = 17;
			for (int i = 0; i < MAX_VOXEL_GI_INSTANCES; i++) {
				vgiu.append_id(rbgi->voxel_gi_textures[i]);
			}

			rbgi->uniform_set[v] = UniformSetCacheRD::get_singleton()->get_cache(
					shader.version_get_shader(shader_version, 0),
					0,
					RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 1, sdfgi->voxel_bits_tex),
					RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 2, sdfgi->voxel_region_tex),
					RD::Uniform(RD::UNIFORM_TYPE_TEXTURE, 3, sdfgi->light_tex),
					RD::Uniform(RD::UNIFORM_TYPE_TEXTURE, 4, sdfgi->lightprobe_specular_tex),
					RD::Uniform(RD::UNIFORM_TYPE_TEXTURE, 5, sdfgi->using_filter ? sdfgi->lightprobe_diffuse_filter_tex : sdfgi->lightprobe_diffuse_tex),
					RD::Uniform(RD::UNIFORM_TYPE_TEXTURE, 6, sdfgi->occlusion_tex),
					RD::Uniform(RD::UNIFORM_TYPE_SAMPLER, 7, RendererRD::MaterialStorage::get_singleton()->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED)),
					RD::Uniform(RD::UNIFORM_TYPE_SAMPLER, 8, RendererRD::MaterialStorage::get_singleton()->sampler_rd_get_default(RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR_WITH_MIPMAPS, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED)),

					RD::Uniform(RD::UNIFORM_TYPE_TEXTURE, 10, p_render_buffers->get_depth_texture(v)),
					RD::Uniform(RD::UNIFORM_TYPE_TEXTURE, 11, p_normal_roughness_slices[v]),
					RD::Uniform(RD::UNIFORM_TYPE_TEXTURE, 12, p_voxel_gi_buffer.is_valid() ? p_voxel_gi_buffer : texture_storage->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_BLACK)),

					RD::Uniform(RD::UNIFORM_TYPE_UNIFORM_BUFFER, 15, sdfgi_ubo),
					RD::Uniform(RD::UNIFORM_TYPE_UNIFORM_BUFFER, 16, rbgi->get_voxel_gi_buffer()),
					vgiu, // 17
					RD::Uniform(RD::UNIFORM_TYPE_UNIFORM_BUFFER, 18, rbgi->scene_data_ubo),
					RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 19, has_vrs_texture ? p_render_buffers->get_texture_slice(RB_SCOPE_VRS, RB_TEXTURE, v, 0) : texture_storage->texture_rd_get_default(RendererRD::TextureStorage::DEFAULT_RD_TEXTURE_VRS)),

					RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 20, p_render_buffers->get_texture_slice(RB_SCOPE_GI, RB_TEX_AMBIENT, v, 0)),
					RD::Uniform(RD::UNIFORM_TYPE_IMAGE, 21, p_render_buffers->get_texture_slice(RB_SCOPE_GI, RB_TEX_REFLECTION, v, 0))

			);
		}

		RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, pipelines[pipeline_specialization][mode]);
		RD::get_singleton()->compute_list_bind_uniform_set(compute_list, rbgi->uniform_set[v], 0);
		RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(PushConstant));

		if (rbgi->using_half_size_gi) {
			RD::get_singleton()->compute_list_dispatch_threads(compute_list, internal_size.x >> 1, internal_size.y >> 1, 1);
		} else {
			RD::get_singleton()->compute_list_dispatch_threads(compute_list, internal_size.x, internal_size.y, 1);
		}
	}

	//do barrier later to allow oeverlap
	//RD::get_singleton()->compute_list_end(RD::BARRIER_MASK_NO_BARRIER); //no barriers, let other compute, raster and transfer happen at the same time
	RD::get_singleton()->draw_command_end_label();
}

RID GI::voxel_gi_instance_create(RID p_base) {
	VoxelGIInstance voxel_gi;
	voxel_gi.gi = this;
	voxel_gi.probe = p_base;
	RID rid = voxel_gi_instance_owner.make_rid(voxel_gi);
	return rid;
}

void GI::voxel_gi_instance_free(RID p_rid) {
	GI::VoxelGIInstance *voxel_gi = voxel_gi_instance_owner.get_or_null(p_rid);
	voxel_gi->free_resources();
	voxel_gi_instance_owner.free(p_rid);
}

void GI::voxel_gi_instance_set_transform_to_data(RID p_probe, const Transform3D &p_xform) {
	VoxelGIInstance *voxel_gi = voxel_gi_instance_owner.get_or_null(p_probe);
	ERR_FAIL_NULL(voxel_gi);

	voxel_gi->transform = p_xform;
}

bool GI::voxel_gi_needs_update(RID p_probe) const {
	VoxelGIInstance *voxel_gi = voxel_gi_instance_owner.get_or_null(p_probe);
	ERR_FAIL_NULL_V(voxel_gi, false);

	return voxel_gi->last_probe_version != voxel_gi_get_version(voxel_gi->probe);
}

void GI::voxel_gi_update(RID p_probe, bool p_update_light_instances, const Vector<RID> &p_light_instances, const PagedArray<RenderGeometryInstance *> &p_dynamic_objects) {
	VoxelGIInstance *voxel_gi = voxel_gi_instance_owner.get_or_null(p_probe);
	ERR_FAIL_NULL(voxel_gi);

	voxel_gi->update(p_update_light_instances, p_light_instances, p_dynamic_objects);
}

void GI::debug_voxel_gi(RID p_voxel_gi, RD::DrawListID p_draw_list, RID p_framebuffer, const Projection &p_camera_with_transform, bool p_lighting, bool p_emission, float p_alpha) {
	VoxelGIInstance *voxel_gi = voxel_gi_instance_owner.get_or_null(p_voxel_gi);
	ERR_FAIL_NULL(voxel_gi);

	voxel_gi->debug(p_draw_list, p_framebuffer, p_camera_with_transform, p_lighting, p_emission, p_alpha);
}
