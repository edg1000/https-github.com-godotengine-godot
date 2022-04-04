/*************************************************************************/
/*  mesh_storage.cpp                                                     */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
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

#include "mesh_storage.h"

using namespace RendererRD;

MeshStorage *MeshStorage::singleton = nullptr;

MeshStorage *MeshStorage::get_singleton() {
	return singleton;
}

MeshStorage::MeshStorage() {
	singleton = this;

	default_rd_storage_buffer = RD::get_singleton()->storage_buffer_create(sizeof(uint32_t) * 4);

	//default rd buffers
	{
		Vector<uint8_t> buffer;
		{
			buffer.resize(sizeof(float) * 3);
			{
				uint8_t *w = buffer.ptrw();
				float *fptr = (float *)w;
				fptr[0] = 0.0;
				fptr[1] = 0.0;
				fptr[2] = 0.0;
			}
			mesh_default_rd_buffers[DEFAULT_RD_BUFFER_VERTEX] = RD::get_singleton()->vertex_buffer_create(buffer.size(), buffer);
		}

		{ //normal
			buffer.resize(sizeof(float) * 3);
			{
				uint8_t *w = buffer.ptrw();
				float *fptr = (float *)w;
				fptr[0] = 1.0;
				fptr[1] = 0.0;
				fptr[2] = 0.0;
			}
			mesh_default_rd_buffers[DEFAULT_RD_BUFFER_NORMAL] = RD::get_singleton()->vertex_buffer_create(buffer.size(), buffer);
		}

		{ //tangent
			buffer.resize(sizeof(float) * 4);
			{
				uint8_t *w = buffer.ptrw();
				float *fptr = (float *)w;
				fptr[0] = 1.0;
				fptr[1] = 0.0;
				fptr[2] = 0.0;
				fptr[3] = 0.0;
			}
			mesh_default_rd_buffers[DEFAULT_RD_BUFFER_TANGENT] = RD::get_singleton()->vertex_buffer_create(buffer.size(), buffer);
		}

		{ //color
			buffer.resize(sizeof(float) * 4);
			{
				uint8_t *w = buffer.ptrw();
				float *fptr = (float *)w;
				fptr[0] = 1.0;
				fptr[1] = 1.0;
				fptr[2] = 1.0;
				fptr[3] = 1.0;
			}
			mesh_default_rd_buffers[DEFAULT_RD_BUFFER_COLOR] = RD::get_singleton()->vertex_buffer_create(buffer.size(), buffer);
		}

		{ //tex uv 1
			buffer.resize(sizeof(float) * 2);
			{
				uint8_t *w = buffer.ptrw();
				float *fptr = (float *)w;
				fptr[0] = 0.0;
				fptr[1] = 0.0;
			}
			mesh_default_rd_buffers[DEFAULT_RD_BUFFER_TEX_UV] = RD::get_singleton()->vertex_buffer_create(buffer.size(), buffer);
		}
		{ //tex uv 2
			buffer.resize(sizeof(float) * 2);
			{
				uint8_t *w = buffer.ptrw();
				float *fptr = (float *)w;
				fptr[0] = 0.0;
				fptr[1] = 0.0;
			}
			mesh_default_rd_buffers[DEFAULT_RD_BUFFER_TEX_UV2] = RD::get_singleton()->vertex_buffer_create(buffer.size(), buffer);
		}

		for (int i = 0; i < RS::ARRAY_CUSTOM_COUNT; i++) {
			buffer.resize(sizeof(float) * 4);
			{
				uint8_t *w = buffer.ptrw();
				float *fptr = (float *)w;
				fptr[0] = 0.0;
				fptr[1] = 0.0;
				fptr[2] = 0.0;
				fptr[3] = 0.0;
			}
			mesh_default_rd_buffers[DEFAULT_RD_BUFFER_CUSTOM0 + i] = RD::get_singleton()->vertex_buffer_create(buffer.size(), buffer);
		}

		{ //bones
			buffer.resize(sizeof(uint32_t) * 4);
			{
				uint8_t *w = buffer.ptrw();
				uint32_t *fptr = (uint32_t *)w;
				fptr[0] = 0;
				fptr[1] = 0;
				fptr[2] = 0;
				fptr[3] = 0;
			}
			mesh_default_rd_buffers[DEFAULT_RD_BUFFER_BONES] = RD::get_singleton()->vertex_buffer_create(buffer.size(), buffer);
		}

		{ //weights
			buffer.resize(sizeof(float) * 4);
			{
				uint8_t *w = buffer.ptrw();
				float *fptr = (float *)w;
				fptr[0] = 0.0;
				fptr[1] = 0.0;
				fptr[2] = 0.0;
				fptr[3] = 0.0;
			}
			mesh_default_rd_buffers[DEFAULT_RD_BUFFER_WEIGHTS] = RD::get_singleton()->vertex_buffer_create(buffer.size(), buffer);
		}
	}

	{
		Vector<String> skeleton_modes;
		skeleton_modes.push_back("\n#define MODE_2D\n");
		skeleton_modes.push_back("");

		skeleton_shader.shader.initialize(skeleton_modes);
		skeleton_shader.version = skeleton_shader.shader.version_create();
		for (int i = 0; i < SkeletonShader::SHADER_MODE_MAX; i++) {
			skeleton_shader.version_shader[i] = skeleton_shader.shader.version_get_shader(skeleton_shader.version, i);
			skeleton_shader.pipeline[i] = RD::get_singleton()->compute_pipeline_create(skeleton_shader.version_shader[i]);
		}

		{
			Vector<RD::Uniform> uniforms;
			{
				RD::Uniform u;
				u.binding = 0;
				u.uniform_type = RD::UNIFORM_TYPE_STORAGE_BUFFER;
				u.append_id(default_rd_storage_buffer);
				uniforms.push_back(u);
			}
			skeleton_shader.default_skeleton_uniform_set = RD::get_singleton()->uniform_set_create(uniforms, skeleton_shader.version_shader[0], SkeletonShader::UNIFORM_SET_SKELETON);
		}
	}
}

MeshStorage::~MeshStorage() {
	//def buffers
	for (int i = 0; i < DEFAULT_RD_BUFFER_MAX; i++) {
		RD::get_singleton()->free(mesh_default_rd_buffers[i]);
	}

	skeleton_shader.shader.version_free(skeleton_shader.version);

	RD::get_singleton()->free(default_rd_storage_buffer);

	singleton = nullptr;
}

/* MESH API */

RID MeshStorage::mesh_allocate() {
	return mesh_owner.allocate_rid();
}

void MeshStorage::mesh_initialize(RID p_rid) {
	mesh_owner.initialize_rid(p_rid, Mesh());
}

void MeshStorage::mesh_free(RID p_rid) {
	mesh_clear(p_rid);
	mesh_set_shadow_mesh(p_rid, RID());
	Mesh *mesh = mesh_owner.get_or_null(p_rid);
	mesh->dependency.deleted_notify(p_rid);
	if (mesh->instances.size()) {
		ERR_PRINT("deleting mesh with active instances");
	}
	if (mesh->shadow_owners.size()) {
		for (Set<Mesh *>::Element *E = mesh->shadow_owners.front(); E; E = E->next()) {
			Mesh *shadow_owner = E->get();
			shadow_owner->shadow_mesh = RID();
			shadow_owner->dependency.changed_notify(RendererStorage::DEPENDENCY_CHANGED_MESH);
		}
	}
	mesh_owner.free(p_rid);
}

void MeshStorage::mesh_set_blend_shape_count(RID p_mesh, int p_blend_shape_count) {
	ERR_FAIL_COND(p_blend_shape_count < 0);

	Mesh *mesh = mesh_owner.get_or_null(p_mesh);
	ERR_FAIL_COND(!mesh);

	ERR_FAIL_COND(mesh->surface_count > 0); //surfaces already exist

	mesh->blend_shape_count = p_blend_shape_count;
}

/// Returns stride
void MeshStorage::mesh_add_surface(RID p_mesh, const RS::SurfaceData &p_surface) {
	Mesh *mesh = mesh_owner.get_or_null(p_mesh);
	ERR_FAIL_COND(!mesh);

	ERR_FAIL_COND(mesh->surface_count == RS::MAX_MESH_SURFACES);

#ifdef DEBUG_ENABLED
	//do a validation, to catch errors first
	{
		uint32_t stride = 0;
		uint32_t attrib_stride = 0;
		uint32_t skin_stride = 0;

		for (int i = 0; i < RS::ARRAY_WEIGHTS; i++) {
			if ((p_surface.format & (1 << i))) {
				switch (i) {
					case RS::ARRAY_VERTEX: {
						if (p_surface.format & RS::ARRAY_FLAG_USE_2D_VERTICES) {
							stride += sizeof(float) * 2;
						} else {
							stride += sizeof(float) * 3;
						}

					} break;
					case RS::ARRAY_NORMAL: {
						stride += sizeof(int32_t);

					} break;
					case RS::ARRAY_TANGENT: {
						stride += sizeof(int32_t);

					} break;
					case RS::ARRAY_COLOR: {
						attrib_stride += sizeof(uint32_t);
					} break;
					case RS::ARRAY_TEX_UV: {
						attrib_stride += sizeof(float) * 2;

					} break;
					case RS::ARRAY_TEX_UV2: {
						attrib_stride += sizeof(float) * 2;

					} break;
					case RS::ARRAY_CUSTOM0:
					case RS::ARRAY_CUSTOM1:
					case RS::ARRAY_CUSTOM2:
					case RS::ARRAY_CUSTOM3: {
						int idx = i - RS::ARRAY_CUSTOM0;
						uint32_t fmt_shift[RS::ARRAY_CUSTOM_COUNT] = { RS::ARRAY_FORMAT_CUSTOM0_SHIFT, RS::ARRAY_FORMAT_CUSTOM1_SHIFT, RS::ARRAY_FORMAT_CUSTOM2_SHIFT, RS::ARRAY_FORMAT_CUSTOM3_SHIFT };
						uint32_t fmt = (p_surface.format >> fmt_shift[idx]) & RS::ARRAY_FORMAT_CUSTOM_MASK;
						uint32_t fmtsize[RS::ARRAY_CUSTOM_MAX] = { 4, 4, 4, 8, 4, 8, 12, 16 };
						attrib_stride += fmtsize[fmt];

					} break;
					case RS::ARRAY_WEIGHTS:
					case RS::ARRAY_BONES: {
						//uses a separate array
						bool use_8 = p_surface.format & RS::ARRAY_FLAG_USE_8_BONE_WEIGHTS;
						skin_stride += sizeof(int16_t) * (use_8 ? 16 : 8);
					} break;
				}
			}
		}

		int expected_size = stride * p_surface.vertex_count;
		ERR_FAIL_COND_MSG(expected_size != p_surface.vertex_data.size(), "Size of vertex data provided (" + itos(p_surface.vertex_data.size()) + ") does not match expected (" + itos(expected_size) + ")");

		int bs_expected_size = expected_size * mesh->blend_shape_count;

		ERR_FAIL_COND_MSG(bs_expected_size != p_surface.blend_shape_data.size(), "Size of blend shape data provided (" + itos(p_surface.blend_shape_data.size()) + ") does not match expected (" + itos(bs_expected_size) + ")");

		int expected_attrib_size = attrib_stride * p_surface.vertex_count;
		ERR_FAIL_COND_MSG(expected_attrib_size != p_surface.attribute_data.size(), "Size of attribute data provided (" + itos(p_surface.attribute_data.size()) + ") does not match expected (" + itos(expected_attrib_size) + ")");

		if ((p_surface.format & RS::ARRAY_FORMAT_WEIGHTS) && (p_surface.format & RS::ARRAY_FORMAT_BONES)) {
			expected_size = skin_stride * p_surface.vertex_count;
			ERR_FAIL_COND_MSG(expected_size != p_surface.skin_data.size(), "Size of skin data provided (" + itos(p_surface.skin_data.size()) + ") does not match expected (" + itos(expected_size) + ")");
		}
	}

#endif

	Mesh::Surface *s = memnew(Mesh::Surface);

	s->format = p_surface.format;
	s->primitive = p_surface.primitive;

	bool use_as_storage = (p_surface.skin_data.size() || mesh->blend_shape_count > 0);

	s->vertex_buffer = RD::get_singleton()->vertex_buffer_create(p_surface.vertex_data.size(), p_surface.vertex_data, use_as_storage);
	s->vertex_buffer_size = p_surface.vertex_data.size();

	if (p_surface.attribute_data.size()) {
		s->attribute_buffer = RD::get_singleton()->vertex_buffer_create(p_surface.attribute_data.size(), p_surface.attribute_data);
	}
	if (p_surface.skin_data.size()) {
		s->skin_buffer = RD::get_singleton()->vertex_buffer_create(p_surface.skin_data.size(), p_surface.skin_data, use_as_storage);
		s->skin_buffer_size = p_surface.skin_data.size();
	}

	s->vertex_count = p_surface.vertex_count;

	if (p_surface.format & RS::ARRAY_FORMAT_BONES) {
		mesh->has_bone_weights = true;
	}

	if (p_surface.index_count) {
		bool is_index_16 = p_surface.vertex_count <= 65536;

		s->index_buffer = RD::get_singleton()->index_buffer_create(p_surface.index_count, is_index_16 ? RD::INDEX_BUFFER_FORMAT_UINT16 : RD::INDEX_BUFFER_FORMAT_UINT32, p_surface.index_data, false);
		s->index_count = p_surface.index_count;
		s->index_array = RD::get_singleton()->index_array_create(s->index_buffer, 0, s->index_count);
		if (p_surface.lods.size()) {
			s->lods = memnew_arr(Mesh::Surface::LOD, p_surface.lods.size());
			s->lod_count = p_surface.lods.size();

			for (int i = 0; i < p_surface.lods.size(); i++) {
				uint32_t indices = p_surface.lods[i].index_data.size() / (is_index_16 ? 2 : 4);
				s->lods[i].index_buffer = RD::get_singleton()->index_buffer_create(indices, is_index_16 ? RD::INDEX_BUFFER_FORMAT_UINT16 : RD::INDEX_BUFFER_FORMAT_UINT32, p_surface.lods[i].index_data);
				s->lods[i].index_array = RD::get_singleton()->index_array_create(s->lods[i].index_buffer, 0, indices);
				s->lods[i].edge_length = p_surface.lods[i].edge_length;
				s->lods[i].index_count = indices;
			}
		}
	}

	s->aabb = p_surface.aabb;
	s->bone_aabbs = p_surface.bone_aabbs; //only really useful for returning them.

	if (mesh->blend_shape_count > 0) {
		s->blend_shape_buffer = RD::get_singleton()->storage_buffer_create(p_surface.blend_shape_data.size(), p_surface.blend_shape_data);
	}

	if (use_as_storage) {
		Vector<RD::Uniform> uniforms;
		{
			RD::Uniform u;
			u.binding = 0;
			u.uniform_type = RD::UNIFORM_TYPE_STORAGE_BUFFER;
			u.append_id(s->vertex_buffer);
			uniforms.push_back(u);
		}
		{
			RD::Uniform u;
			u.binding = 1;
			u.uniform_type = RD::UNIFORM_TYPE_STORAGE_BUFFER;
			if (s->skin_buffer.is_valid()) {
				u.append_id(s->skin_buffer);
			} else {
				u.append_id(default_rd_storage_buffer);
			}
			uniforms.push_back(u);
		}
		{
			RD::Uniform u;
			u.binding = 2;
			u.uniform_type = RD::UNIFORM_TYPE_STORAGE_BUFFER;
			if (s->blend_shape_buffer.is_valid()) {
				u.append_id(s->blend_shape_buffer);
			} else {
				u.append_id(default_rd_storage_buffer);
			}
			uniforms.push_back(u);
		}

		s->uniform_set = RD::get_singleton()->uniform_set_create(uniforms, skeleton_shader.version_shader[0], SkeletonShader::UNIFORM_SET_SURFACE);
	}

	if (mesh->surface_count == 0) {
		mesh->bone_aabbs = p_surface.bone_aabbs;
		mesh->aabb = p_surface.aabb;
	} else {
		if (mesh->bone_aabbs.size() < p_surface.bone_aabbs.size()) {
			// ArrayMesh::_surface_set_data only allocates bone_aabbs up to max_bone
			// Each surface may affect different numbers of bones.
			mesh->bone_aabbs.resize(p_surface.bone_aabbs.size());
		}
		for (int i = 0; i < p_surface.bone_aabbs.size(); i++) {
			mesh->bone_aabbs.write[i].merge_with(p_surface.bone_aabbs[i]);
		}
		mesh->aabb.merge_with(p_surface.aabb);
	}

	s->material = p_surface.material;

	mesh->surfaces = (Mesh::Surface **)memrealloc(mesh->surfaces, sizeof(Mesh::Surface *) * (mesh->surface_count + 1));
	mesh->surfaces[mesh->surface_count] = s;
	mesh->surface_count++;

	for (MeshInstance *mi : mesh->instances) {
		_mesh_instance_add_surface(mi, mesh, mesh->surface_count - 1);
	}

	mesh->dependency.changed_notify(RendererStorage::DEPENDENCY_CHANGED_MESH);

	for (Set<Mesh *>::Element *E = mesh->shadow_owners.front(); E; E = E->next()) {
		Mesh *shadow_owner = E->get();
		shadow_owner->shadow_mesh = RID();
		shadow_owner->dependency.changed_notify(RendererStorage::DEPENDENCY_CHANGED_MESH);
	}

	mesh->material_cache.clear();
}

int MeshStorage::mesh_get_blend_shape_count(RID p_mesh) const {
	const Mesh *mesh = mesh_owner.get_or_null(p_mesh);
	ERR_FAIL_COND_V(!mesh, -1);
	return mesh->blend_shape_count;
}

void MeshStorage::mesh_set_blend_shape_mode(RID p_mesh, RS::BlendShapeMode p_mode) {
	Mesh *mesh = mesh_owner.get_or_null(p_mesh);
	ERR_FAIL_COND(!mesh);
	ERR_FAIL_INDEX((int)p_mode, 2);

	mesh->blend_shape_mode = p_mode;
}

RS::BlendShapeMode MeshStorage::mesh_get_blend_shape_mode(RID p_mesh) const {
	Mesh *mesh = mesh_owner.get_or_null(p_mesh);
	ERR_FAIL_COND_V(!mesh, RS::BLEND_SHAPE_MODE_NORMALIZED);
	return mesh->blend_shape_mode;
}

void MeshStorage::mesh_surface_update_vertex_region(RID p_mesh, int p_surface, int p_offset, const Vector<uint8_t> &p_data) {
	Mesh *mesh = mesh_owner.get_or_null(p_mesh);
	ERR_FAIL_COND(!mesh);
	ERR_FAIL_UNSIGNED_INDEX((uint32_t)p_surface, mesh->surface_count);
	ERR_FAIL_COND(p_data.size() == 0);
	uint64_t data_size = p_data.size();
	const uint8_t *r = p_data.ptr();

	RD::get_singleton()->buffer_update(mesh->surfaces[p_surface]->vertex_buffer, p_offset, data_size, r);
}

void MeshStorage::mesh_surface_update_attribute_region(RID p_mesh, int p_surface, int p_offset, const Vector<uint8_t> &p_data) {
	Mesh *mesh = mesh_owner.get_or_null(p_mesh);
	ERR_FAIL_COND(!mesh);
	ERR_FAIL_UNSIGNED_INDEX((uint32_t)p_surface, mesh->surface_count);
	ERR_FAIL_COND(p_data.size() == 0);
	ERR_FAIL_COND(mesh->surfaces[p_surface]->attribute_buffer.is_null());
	uint64_t data_size = p_data.size();
	const uint8_t *r = p_data.ptr();

	RD::get_singleton()->buffer_update(mesh->surfaces[p_surface]->attribute_buffer, p_offset, data_size, r);
}

void MeshStorage::mesh_surface_update_skin_region(RID p_mesh, int p_surface, int p_offset, const Vector<uint8_t> &p_data) {
	Mesh *mesh = mesh_owner.get_or_null(p_mesh);
	ERR_FAIL_COND(!mesh);
	ERR_FAIL_UNSIGNED_INDEX((uint32_t)p_surface, mesh->surface_count);
	ERR_FAIL_COND(p_data.size() == 0);
	ERR_FAIL_COND(mesh->surfaces[p_surface]->skin_buffer.is_null());
	uint64_t data_size = p_data.size();
	const uint8_t *r = p_data.ptr();

	RD::get_singleton()->buffer_update(mesh->surfaces[p_surface]->skin_buffer, p_offset, data_size, r);
}

void MeshStorage::mesh_surface_set_material(RID p_mesh, int p_surface, RID p_material) {
	Mesh *mesh = mesh_owner.get_or_null(p_mesh);
	ERR_FAIL_COND(!mesh);
	ERR_FAIL_UNSIGNED_INDEX((uint32_t)p_surface, mesh->surface_count);
	mesh->surfaces[p_surface]->material = p_material;

	mesh->dependency.changed_notify(RendererStorage::DEPENDENCY_CHANGED_MATERIAL);
	mesh->material_cache.clear();
}

RID MeshStorage::mesh_surface_get_material(RID p_mesh, int p_surface) const {
	Mesh *mesh = mesh_owner.get_or_null(p_mesh);
	ERR_FAIL_COND_V(!mesh, RID());
	ERR_FAIL_UNSIGNED_INDEX_V((uint32_t)p_surface, mesh->surface_count, RID());

	return mesh->surfaces[p_surface]->material;
}

RS::SurfaceData MeshStorage::mesh_get_surface(RID p_mesh, int p_surface) const {
	Mesh *mesh = mesh_owner.get_or_null(p_mesh);
	ERR_FAIL_COND_V(!mesh, RS::SurfaceData());
	ERR_FAIL_UNSIGNED_INDEX_V((uint32_t)p_surface, mesh->surface_count, RS::SurfaceData());

	Mesh::Surface &s = *mesh->surfaces[p_surface];

	RS::SurfaceData sd;
	sd.format = s.format;
	sd.vertex_data = RD::get_singleton()->buffer_get_data(s.vertex_buffer);
	if (s.attribute_buffer.is_valid()) {
		sd.attribute_data = RD::get_singleton()->buffer_get_data(s.attribute_buffer);
	}
	if (s.skin_buffer.is_valid()) {
		sd.skin_data = RD::get_singleton()->buffer_get_data(s.skin_buffer);
	}
	sd.vertex_count = s.vertex_count;
	sd.index_count = s.index_count;
	sd.primitive = s.primitive;

	if (sd.index_count) {
		sd.index_data = RD::get_singleton()->buffer_get_data(s.index_buffer);
	}
	sd.aabb = s.aabb;
	for (uint32_t i = 0; i < s.lod_count; i++) {
		RS::SurfaceData::LOD lod;
		lod.edge_length = s.lods[i].edge_length;
		lod.index_data = RD::get_singleton()->buffer_get_data(s.lods[i].index_buffer);
		sd.lods.push_back(lod);
	}

	sd.bone_aabbs = s.bone_aabbs;

	if (s.blend_shape_buffer.is_valid()) {
		sd.blend_shape_data = RD::get_singleton()->buffer_get_data(s.blend_shape_buffer);
	}

	return sd;
}

int MeshStorage::mesh_get_surface_count(RID p_mesh) const {
	Mesh *mesh = mesh_owner.get_or_null(p_mesh);
	ERR_FAIL_COND_V(!mesh, 0);
	return mesh->surface_count;
}

void MeshStorage::mesh_set_custom_aabb(RID p_mesh, const AABB &p_aabb) {
	Mesh *mesh = mesh_owner.get_or_null(p_mesh);
	ERR_FAIL_COND(!mesh);
	mesh->custom_aabb = p_aabb;
}

AABB MeshStorage::mesh_get_custom_aabb(RID p_mesh) const {
	Mesh *mesh = mesh_owner.get_or_null(p_mesh);
	ERR_FAIL_COND_V(!mesh, AABB());
	return mesh->custom_aabb;
}

AABB MeshStorage::mesh_get_aabb(RID p_mesh, RID p_skeleton) {
	Mesh *mesh = mesh_owner.get_or_null(p_mesh);
	ERR_FAIL_COND_V(!mesh, AABB());

	if (mesh->custom_aabb != AABB()) {
		return mesh->custom_aabb;
	}

	Skeleton *skeleton = skeleton_owner.get_or_null(p_skeleton);

	if (!skeleton || skeleton->size == 0) {
		return mesh->aabb;
	}

	AABB aabb;

	for (uint32_t i = 0; i < mesh->surface_count; i++) {
		AABB laabb;
		if ((mesh->surfaces[i]->format & RS::ARRAY_FORMAT_BONES) && mesh->surfaces[i]->bone_aabbs.size()) {
			int bs = mesh->surfaces[i]->bone_aabbs.size();
			const AABB *skbones = mesh->surfaces[i]->bone_aabbs.ptr();

			int sbs = skeleton->size;
			ERR_CONTINUE(bs > sbs);
			const float *baseptr = skeleton->data.ptr();

			bool first = true;

			if (skeleton->use_2d) {
				for (int j = 0; j < bs; j++) {
					if (skbones[0].size == Vector3()) {
						continue; //bone is unused
					}

					const float *dataptr = baseptr + j * 8;

					Transform3D mtx;

					mtx.basis.elements[0].x = dataptr[0];
					mtx.basis.elements[1].x = dataptr[1];
					mtx.origin.x = dataptr[3];

					mtx.basis.elements[0].y = dataptr[4];
					mtx.basis.elements[1].y = dataptr[5];
					mtx.origin.y = dataptr[7];

					AABB baabb = mtx.xform(skbones[j]);

					if (first) {
						laabb = baabb;
						first = false;
					} else {
						laabb.merge_with(baabb);
					}
				}
			} else {
				for (int j = 0; j < bs; j++) {
					if (skbones[0].size == Vector3()) {
						continue; //bone is unused
					}

					const float *dataptr = baseptr + j * 12;

					Transform3D mtx;

					mtx.basis.elements[0][0] = dataptr[0];
					mtx.basis.elements[0][1] = dataptr[1];
					mtx.basis.elements[0][2] = dataptr[2];
					mtx.origin.x = dataptr[3];
					mtx.basis.elements[1][0] = dataptr[4];
					mtx.basis.elements[1][1] = dataptr[5];
					mtx.basis.elements[1][2] = dataptr[6];
					mtx.origin.y = dataptr[7];
					mtx.basis.elements[2][0] = dataptr[8];
					mtx.basis.elements[2][1] = dataptr[9];
					mtx.basis.elements[2][2] = dataptr[10];
					mtx.origin.z = dataptr[11];

					AABB baabb = mtx.xform(skbones[j]);
					if (first) {
						laabb = baabb;
						first = false;
					} else {
						laabb.merge_with(baabb);
					}
				}
			}

			if (laabb.size == Vector3()) {
				laabb = mesh->surfaces[i]->aabb;
			}
		} else {
			laabb = mesh->surfaces[i]->aabb;
		}

		if (i == 0) {
			aabb = laabb;
		} else {
			aabb.merge_with(laabb);
		}
	}

	return aabb;
}

void MeshStorage::mesh_set_shadow_mesh(RID p_mesh, RID p_shadow_mesh) {
	Mesh *mesh = mesh_owner.get_or_null(p_mesh);
	ERR_FAIL_COND(!mesh);

	Mesh *shadow_mesh = mesh_owner.get_or_null(mesh->shadow_mesh);
	if (shadow_mesh) {
		shadow_mesh->shadow_owners.erase(mesh);
	}
	mesh->shadow_mesh = p_shadow_mesh;

	shadow_mesh = mesh_owner.get_or_null(mesh->shadow_mesh);

	if (shadow_mesh) {
		shadow_mesh->shadow_owners.insert(mesh);
	}

	mesh->dependency.changed_notify(RendererStorage::DEPENDENCY_CHANGED_MESH);
}

void MeshStorage::mesh_clear(RID p_mesh) {
	Mesh *mesh = mesh_owner.get_or_null(p_mesh);
	ERR_FAIL_COND(!mesh);
	for (uint32_t i = 0; i < mesh->surface_count; i++) {
		Mesh::Surface &s = *mesh->surfaces[i];
		RD::get_singleton()->free(s.vertex_buffer); //clears arrays as dependency automatically, including all versions
		if (s.attribute_buffer.is_valid()) {
			RD::get_singleton()->free(s.attribute_buffer);
		}
		if (s.skin_buffer.is_valid()) {
			RD::get_singleton()->free(s.skin_buffer);
		}
		if (s.versions) {
			memfree(s.versions); //reallocs, so free with memfree.
		}

		if (s.index_buffer.is_valid()) {
			RD::get_singleton()->free(s.index_buffer);
		}

		if (s.lod_count) {
			for (uint32_t j = 0; j < s.lod_count; j++) {
				RD::get_singleton()->free(s.lods[j].index_buffer);
			}
			memdelete_arr(s.lods);
		}

		if (s.blend_shape_buffer.is_valid()) {
			RD::get_singleton()->free(s.blend_shape_buffer);
		}

		memdelete(mesh->surfaces[i]);
	}
	if (mesh->surfaces) {
		memfree(mesh->surfaces);
	}

	mesh->surfaces = nullptr;
	mesh->surface_count = 0;
	mesh->material_cache.clear();
	//clear instance data
	for (MeshInstance *mi : mesh->instances) {
		_mesh_instance_clear(mi);
	}
	mesh->has_bone_weights = false;
	mesh->dependency.changed_notify(RendererStorage::DEPENDENCY_CHANGED_MESH);

	for (Set<Mesh *>::Element *E = mesh->shadow_owners.front(); E; E = E->next()) {
		Mesh *shadow_owner = E->get();
		shadow_owner->shadow_mesh = RID();
		shadow_owner->dependency.changed_notify(RendererStorage::DEPENDENCY_CHANGED_MESH);
	}
}

bool MeshStorage::mesh_needs_instance(RID p_mesh, bool p_has_skeleton) {
	Mesh *mesh = mesh_owner.get_or_null(p_mesh);
	ERR_FAIL_COND_V(!mesh, false);

	return mesh->blend_shape_count > 0 || (mesh->has_bone_weights && p_has_skeleton);
}

/* MESH INSTANCE */

RID MeshStorage::mesh_instance_create(RID p_base) {
	Mesh *mesh = mesh_owner.get_or_null(p_base);
	ERR_FAIL_COND_V(!mesh, RID());

	RID rid = mesh_instance_owner.make_rid();
	MeshInstance *mi = mesh_instance_owner.get_or_null(rid);

	mi->mesh = mesh;

	for (uint32_t i = 0; i < mesh->surface_count; i++) {
		_mesh_instance_add_surface(mi, mesh, i);
	}

	mi->I = mesh->instances.push_back(mi);

	mi->dirty = true;

	return rid;
}

void MeshStorage::mesh_instance_free(RID p_rid) {
	MeshInstance *mi = mesh_instance_owner.get_or_null(p_rid);
	_mesh_instance_clear(mi);
	mi->mesh->instances.erase(mi->I);
	mi->I = nullptr;

	mesh_instance_owner.free(p_rid);
}

void MeshStorage::mesh_instance_set_skeleton(RID p_mesh_instance, RID p_skeleton) {
	MeshInstance *mi = mesh_instance_owner.get_or_null(p_mesh_instance);
	if (mi->skeleton == p_skeleton) {
		return;
	}
	mi->skeleton = p_skeleton;
	mi->skeleton_version = 0;
	mi->dirty = true;
}

void MeshStorage::mesh_instance_set_blend_shape_weight(RID p_mesh_instance, int p_shape, float p_weight) {
	MeshInstance *mi = mesh_instance_owner.get_or_null(p_mesh_instance);
	ERR_FAIL_COND(!mi);
	ERR_FAIL_INDEX(p_shape, (int)mi->blend_weights.size());
	mi->blend_weights[p_shape] = p_weight;
	mi->weights_dirty = true;
	//will be eventually updated
}

void MeshStorage::_mesh_instance_clear(MeshInstance *mi) {
	for (uint32_t i = 0; i < mi->surfaces.size(); i++) {
		if (mi->surfaces[i].versions) {
			for (uint32_t j = 0; j < mi->surfaces[i].version_count; j++) {
				RD::get_singleton()->free(mi->surfaces[i].versions[j].vertex_array);
			}
			memfree(mi->surfaces[i].versions);
		}
		if (mi->surfaces[i].vertex_buffer.is_valid()) {
			RD::get_singleton()->free(mi->surfaces[i].vertex_buffer);
		}
	}
	mi->surfaces.clear();

	if (mi->blend_weights_buffer.is_valid()) {
		RD::get_singleton()->free(mi->blend_weights_buffer);
	}
	mi->blend_weights.clear();
	mi->weights_dirty = false;
	mi->skeleton_version = 0;
}

void MeshStorage::_mesh_instance_add_surface(MeshInstance *mi, Mesh *mesh, uint32_t p_surface) {
	if (mesh->blend_shape_count > 0 && mi->blend_weights_buffer.is_null()) {
		mi->blend_weights.resize(mesh->blend_shape_count);
		for (uint32_t i = 0; i < mi->blend_weights.size(); i++) {
			mi->blend_weights[i] = 0;
		}
		mi->blend_weights_buffer = RD::get_singleton()->storage_buffer_create(sizeof(float) * mi->blend_weights.size(), mi->blend_weights.to_byte_array());
		mi->weights_dirty = true;
	}

	MeshInstance::Surface s;
	if (mesh->blend_shape_count > 0 || (mesh->surfaces[p_surface]->format & RS::ARRAY_FORMAT_BONES)) {
		//surface warrants transform
		s.vertex_buffer = RD::get_singleton()->vertex_buffer_create(mesh->surfaces[p_surface]->vertex_buffer_size, Vector<uint8_t>(), true);

		Vector<RD::Uniform> uniforms;
		{
			RD::Uniform u;
			u.binding = 1;
			u.uniform_type = RD::UNIFORM_TYPE_STORAGE_BUFFER;
			u.append_id(s.vertex_buffer);
			uniforms.push_back(u);
		}
		{
			RD::Uniform u;
			u.binding = 2;
			u.uniform_type = RD::UNIFORM_TYPE_STORAGE_BUFFER;
			if (mi->blend_weights_buffer.is_valid()) {
				u.append_id(mi->blend_weights_buffer);
			} else {
				u.append_id(default_rd_storage_buffer);
			}
			uniforms.push_back(u);
		}
		s.uniform_set = RD::get_singleton()->uniform_set_create(uniforms, skeleton_shader.version_shader[0], SkeletonShader::UNIFORM_SET_INSTANCE);
	}

	mi->surfaces.push_back(s);
	mi->dirty = true;
}

void MeshStorage::mesh_instance_check_for_update(RID p_mesh_instance) {
	MeshInstance *mi = mesh_instance_owner.get_or_null(p_mesh_instance);

	bool needs_update = mi->dirty;

	if (mi->weights_dirty && !mi->weight_update_list.in_list()) {
		dirty_mesh_instance_weights.add(&mi->weight_update_list);
		needs_update = true;
	}

	if (mi->array_update_list.in_list()) {
		return;
	}

	if (!needs_update && mi->skeleton.is_valid()) {
		Skeleton *sk = skeleton_owner.get_or_null(mi->skeleton);
		if (sk && sk->version != mi->skeleton_version) {
			needs_update = true;
		}
	}

	if (needs_update) {
		dirty_mesh_instance_arrays.add(&mi->array_update_list);
	}
}

void MeshStorage::update_mesh_instances() {
	while (dirty_mesh_instance_weights.first()) {
		MeshInstance *mi = dirty_mesh_instance_weights.first()->self();

		if (mi->blend_weights_buffer.is_valid()) {
			RD::get_singleton()->buffer_update(mi->blend_weights_buffer, 0, mi->blend_weights.size() * sizeof(float), mi->blend_weights.ptr());
		}
		dirty_mesh_instance_weights.remove(&mi->weight_update_list);
		mi->weights_dirty = false;
	}
	if (dirty_mesh_instance_arrays.first() == nullptr) {
		return; //nothing to do
	}

	//process skeletons and blend shapes
	RD::ComputeListID compute_list = RD::get_singleton()->compute_list_begin();

	while (dirty_mesh_instance_arrays.first()) {
		MeshInstance *mi = dirty_mesh_instance_arrays.first()->self();

		Skeleton *sk = skeleton_owner.get_or_null(mi->skeleton);

		for (uint32_t i = 0; i < mi->surfaces.size(); i++) {
			if (mi->surfaces[i].uniform_set == RID() || mi->mesh->surfaces[i]->uniform_set == RID()) {
				continue;
			}

			bool array_is_2d = mi->mesh->surfaces[i]->format & RS::ARRAY_FLAG_USE_2D_VERTICES;

			RD::get_singleton()->compute_list_bind_compute_pipeline(compute_list, skeleton_shader.pipeline[array_is_2d ? SkeletonShader::SHADER_MODE_2D : SkeletonShader::SHADER_MODE_3D]);

			RD::get_singleton()->compute_list_bind_uniform_set(compute_list, mi->surfaces[i].uniform_set, SkeletonShader::UNIFORM_SET_INSTANCE);
			RD::get_singleton()->compute_list_bind_uniform_set(compute_list, mi->mesh->surfaces[i]->uniform_set, SkeletonShader::UNIFORM_SET_SURFACE);
			if (sk && sk->uniform_set_mi.is_valid()) {
				RD::get_singleton()->compute_list_bind_uniform_set(compute_list, sk->uniform_set_mi, SkeletonShader::UNIFORM_SET_SKELETON);
			} else {
				RD::get_singleton()->compute_list_bind_uniform_set(compute_list, skeleton_shader.default_skeleton_uniform_set, SkeletonShader::UNIFORM_SET_SKELETON);
			}

			SkeletonShader::PushConstant push_constant;

			push_constant.has_normal = mi->mesh->surfaces[i]->format & RS::ARRAY_FORMAT_NORMAL;
			push_constant.has_tangent = mi->mesh->surfaces[i]->format & RS::ARRAY_FORMAT_TANGENT;
			push_constant.has_skeleton = sk != nullptr && sk->use_2d == array_is_2d && (mi->mesh->surfaces[i]->format & RS::ARRAY_FORMAT_BONES);
			push_constant.has_blend_shape = mi->mesh->blend_shape_count > 0;

			push_constant.vertex_count = mi->mesh->surfaces[i]->vertex_count;
			push_constant.vertex_stride = (mi->mesh->surfaces[i]->vertex_buffer_size / mi->mesh->surfaces[i]->vertex_count) / 4;
			push_constant.skin_stride = (mi->mesh->surfaces[i]->skin_buffer_size / mi->mesh->surfaces[i]->vertex_count) / 4;
			push_constant.skin_weight_offset = (mi->mesh->surfaces[i]->format & RS::ARRAY_FLAG_USE_8_BONE_WEIGHTS) ? 4 : 2;

			push_constant.blend_shape_count = mi->mesh->blend_shape_count;
			push_constant.normalized_blend_shapes = mi->mesh->blend_shape_mode == RS::BLEND_SHAPE_MODE_NORMALIZED;
			push_constant.pad0 = 0;
			push_constant.pad1 = 0;

			RD::get_singleton()->compute_list_set_push_constant(compute_list, &push_constant, sizeof(SkeletonShader::PushConstant));

			//dispatch without barrier, so all is done at the same time
			RD::get_singleton()->compute_list_dispatch_threads(compute_list, push_constant.vertex_count, 1, 1);
		}

		mi->dirty = false;
		if (sk) {
			mi->skeleton_version = sk->version;
		}
		dirty_mesh_instance_arrays.remove(&mi->array_update_list);
	}

	RD::get_singleton()->compute_list_end();
}

void MeshStorage::_mesh_surface_generate_version_for_input_mask(Mesh::Surface::Version &v, Mesh::Surface *s, uint32_t p_input_mask, MeshInstance::Surface *mis) {
	Vector<RD::VertexAttribute> attributes;
	Vector<RID> buffers;

	uint32_t stride = 0;
	uint32_t attribute_stride = 0;
	uint32_t skin_stride = 0;

	for (int i = 0; i < RS::ARRAY_INDEX; i++) {
		RD::VertexAttribute vd;
		RID buffer;
		vd.location = i;

		if (!(s->format & (1 << i))) {
			// Not supplied by surface, use default value
			buffer = mesh_default_rd_buffers[i];
			vd.stride = 0;
			switch (i) {
				case RS::ARRAY_VERTEX: {
					vd.format = RD::DATA_FORMAT_R32G32B32_SFLOAT;

				} break;
				case RS::ARRAY_NORMAL: {
					vd.format = RD::DATA_FORMAT_R32G32B32_SFLOAT;
				} break;
				case RS::ARRAY_TANGENT: {
					vd.format = RD::DATA_FORMAT_R32G32B32A32_SFLOAT;
				} break;
				case RS::ARRAY_COLOR: {
					vd.format = RD::DATA_FORMAT_R32G32B32A32_SFLOAT;

				} break;
				case RS::ARRAY_TEX_UV: {
					vd.format = RD::DATA_FORMAT_R32G32_SFLOAT;

				} break;
				case RS::ARRAY_TEX_UV2: {
					vd.format = RD::DATA_FORMAT_R32G32_SFLOAT;
				} break;
				case RS::ARRAY_CUSTOM0:
				case RS::ARRAY_CUSTOM1:
				case RS::ARRAY_CUSTOM2:
				case RS::ARRAY_CUSTOM3: {
					//assumed weights too
					vd.format = RD::DATA_FORMAT_R32G32B32A32_SFLOAT;
				} break;
				case RS::ARRAY_BONES: {
					//assumed weights too
					vd.format = RD::DATA_FORMAT_R32G32B32A32_UINT;
				} break;
				case RS::ARRAY_WEIGHTS: {
					//assumed weights too
					vd.format = RD::DATA_FORMAT_R32G32B32A32_SFLOAT;
				} break;
			}
		} else {
			//Supplied, use it

			vd.stride = 1; //mark that it needs a stride set (default uses 0)

			switch (i) {
				case RS::ARRAY_VERTEX: {
					vd.offset = stride;

					if (s->format & RS::ARRAY_FLAG_USE_2D_VERTICES) {
						vd.format = RD::DATA_FORMAT_R32G32_SFLOAT;
						stride += sizeof(float) * 2;
					} else {
						vd.format = RD::DATA_FORMAT_R32G32B32_SFLOAT;
						stride += sizeof(float) * 3;
					}

					if (mis) {
						buffer = mis->vertex_buffer;
					} else {
						buffer = s->vertex_buffer;
					}

				} break;
				case RS::ARRAY_NORMAL: {
					vd.offset = stride;

					vd.format = RD::DATA_FORMAT_A2B10G10R10_UNORM_PACK32;

					stride += sizeof(uint32_t);
					if (mis) {
						buffer = mis->vertex_buffer;
					} else {
						buffer = s->vertex_buffer;
					}
				} break;
				case RS::ARRAY_TANGENT: {
					vd.offset = stride;

					vd.format = RD::DATA_FORMAT_A2B10G10R10_UNORM_PACK32;
					stride += sizeof(uint32_t);
					if (mis) {
						buffer = mis->vertex_buffer;
					} else {
						buffer = s->vertex_buffer;
					}
				} break;
				case RS::ARRAY_COLOR: {
					vd.offset = attribute_stride;

					vd.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
					attribute_stride += sizeof(int8_t) * 4;
					buffer = s->attribute_buffer;
				} break;
				case RS::ARRAY_TEX_UV: {
					vd.offset = attribute_stride;

					vd.format = RD::DATA_FORMAT_R32G32_SFLOAT;
					attribute_stride += sizeof(float) * 2;
					buffer = s->attribute_buffer;

				} break;
				case RS::ARRAY_TEX_UV2: {
					vd.offset = attribute_stride;

					vd.format = RD::DATA_FORMAT_R32G32_SFLOAT;
					attribute_stride += sizeof(float) * 2;
					buffer = s->attribute_buffer;
				} break;
				case RS::ARRAY_CUSTOM0:
				case RS::ARRAY_CUSTOM1:
				case RS::ARRAY_CUSTOM2:
				case RS::ARRAY_CUSTOM3: {
					vd.offset = attribute_stride;

					int idx = i - RS::ARRAY_CUSTOM0;
					uint32_t fmt_shift[RS::ARRAY_CUSTOM_COUNT] = { RS::ARRAY_FORMAT_CUSTOM0_SHIFT, RS::ARRAY_FORMAT_CUSTOM1_SHIFT, RS::ARRAY_FORMAT_CUSTOM2_SHIFT, RS::ARRAY_FORMAT_CUSTOM3_SHIFT };
					uint32_t fmt = (s->format >> fmt_shift[idx]) & RS::ARRAY_FORMAT_CUSTOM_MASK;
					uint32_t fmtsize[RS::ARRAY_CUSTOM_MAX] = { 4, 4, 4, 8, 4, 8, 12, 16 };
					RD::DataFormat fmtrd[RS::ARRAY_CUSTOM_MAX] = { RD::DATA_FORMAT_R8G8B8A8_UNORM, RD::DATA_FORMAT_R8G8B8A8_SNORM, RD::DATA_FORMAT_R16G16_SFLOAT, RD::DATA_FORMAT_R16G16B16A16_SFLOAT, RD::DATA_FORMAT_R32_SFLOAT, RD::DATA_FORMAT_R32G32_SFLOAT, RD::DATA_FORMAT_R32G32B32_SFLOAT, RD::DATA_FORMAT_R32G32B32A32_SFLOAT };
					vd.format = fmtrd[fmt];
					attribute_stride += fmtsize[fmt];
					buffer = s->attribute_buffer;
				} break;
				case RS::ARRAY_BONES: {
					vd.offset = skin_stride;

					vd.format = RD::DATA_FORMAT_R16G16B16A16_UINT;
					skin_stride += sizeof(int16_t) * 4;
					buffer = s->skin_buffer;
				} break;
				case RS::ARRAY_WEIGHTS: {
					vd.offset = skin_stride;

					vd.format = RD::DATA_FORMAT_R16G16B16A16_UNORM;
					skin_stride += sizeof(int16_t) * 4;
					buffer = s->skin_buffer;
				} break;
			}
		}

		if (!(p_input_mask & (1 << i))) {
			continue; // Shader does not need this, skip it (but computing stride was important anyway)
		}

		attributes.push_back(vd);
		buffers.push_back(buffer);
	}

	//update final stride
	for (int i = 0; i < attributes.size(); i++) {
		if (attributes[i].stride == 0) {
			continue; //default location
		}
		int loc = attributes[i].location;

		if (loc < RS::ARRAY_COLOR) {
			attributes.write[i].stride = stride;
		} else if (loc < RS::ARRAY_BONES) {
			attributes.write[i].stride = attribute_stride;
		} else {
			attributes.write[i].stride = skin_stride;
		}
	}

	v.input_mask = p_input_mask;
	v.vertex_format = RD::get_singleton()->vertex_format_create(attributes);
	v.vertex_array = RD::get_singleton()->vertex_array_create(s->vertex_count, v.vertex_format, buffers);
}

////////////////// MULTIMESH

RID MeshStorage::multimesh_allocate() {
	return multimesh_owner.allocate_rid();
}
void MeshStorage::multimesh_initialize(RID p_rid) {
	multimesh_owner.initialize_rid(p_rid, MultiMesh());
}

void MeshStorage::multimesh_free(RID p_rid) {
	_update_dirty_multimeshes();
	multimesh_allocate_data(p_rid, 0, RS::MULTIMESH_TRANSFORM_2D);
	MultiMesh *multimesh = multimesh_owner.get_or_null(p_rid);
	multimesh->dependency.deleted_notify(p_rid);
	multimesh_owner.free(p_rid);
}

void MeshStorage::multimesh_allocate_data(RID p_multimesh, int p_instances, RS::MultimeshTransformFormat p_transform_format, bool p_use_colors, bool p_use_custom_data) {
	MultiMesh *multimesh = multimesh_owner.get_or_null(p_multimesh);
	ERR_FAIL_COND(!multimesh);

	if (multimesh->instances == p_instances && multimesh->xform_format == p_transform_format && multimesh->uses_colors == p_use_colors && multimesh->uses_custom_data == p_use_custom_data) {
		return;
	}

	if (multimesh->buffer.is_valid()) {
		RD::get_singleton()->free(multimesh->buffer);
		multimesh->buffer = RID();
		multimesh->uniform_set_2d = RID(); //cleared by dependency
		multimesh->uniform_set_3d = RID(); //cleared by dependency
	}

	if (multimesh->data_cache_dirty_regions) {
		memdelete_arr(multimesh->data_cache_dirty_regions);
		multimesh->data_cache_dirty_regions = nullptr;
		multimesh->data_cache_used_dirty_regions = 0;
	}

	multimesh->instances = p_instances;
	multimesh->xform_format = p_transform_format;
	multimesh->uses_colors = p_use_colors;
	multimesh->color_offset_cache = p_transform_format == RS::MULTIMESH_TRANSFORM_2D ? 8 : 12;
	multimesh->uses_custom_data = p_use_custom_data;
	multimesh->custom_data_offset_cache = multimesh->color_offset_cache + (p_use_colors ? 4 : 0);
	multimesh->stride_cache = multimesh->custom_data_offset_cache + (p_use_custom_data ? 4 : 0);
	multimesh->buffer_set = false;

	//print_line("allocate, elements: " + itos(p_instances) + " 2D: " + itos(p_transform_format == RS::MULTIMESH_TRANSFORM_2D) + " colors " + itos(multimesh->uses_colors) + " data " + itos(multimesh->uses_custom_data) + " stride " + itos(multimesh->stride_cache) + " total size " + itos(multimesh->stride_cache * multimesh->instances));
	multimesh->data_cache = Vector<float>();
	multimesh->aabb = AABB();
	multimesh->aabb_dirty = false;
	multimesh->visible_instances = MIN(multimesh->visible_instances, multimesh->instances);

	if (multimesh->instances) {
		multimesh->buffer = RD::get_singleton()->storage_buffer_create(multimesh->instances * multimesh->stride_cache * 4);
	}

	multimesh->dependency.changed_notify(RendererStorage::DEPENDENCY_CHANGED_MULTIMESH);
}

int MeshStorage::multimesh_get_instance_count(RID p_multimesh) const {
	MultiMesh *multimesh = multimesh_owner.get_or_null(p_multimesh);
	ERR_FAIL_COND_V(!multimesh, 0);
	return multimesh->instances;
}

void MeshStorage::multimesh_set_mesh(RID p_multimesh, RID p_mesh) {
	MultiMesh *multimesh = multimesh_owner.get_or_null(p_multimesh);
	ERR_FAIL_COND(!multimesh);
	if (multimesh->mesh == p_mesh) {
		return;
	}
	multimesh->mesh = p_mesh;

	if (multimesh->instances == 0) {
		return;
	}

	if (multimesh->data_cache.size()) {
		//we have a data cache, just mark it dirt
		_multimesh_mark_all_dirty(multimesh, false, true);
	} else if (multimesh->instances) {
		//need to re-create AABB unfortunately, calling this has a penalty
		if (multimesh->buffer_set) {
			Vector<uint8_t> buffer = RD::get_singleton()->buffer_get_data(multimesh->buffer);
			const uint8_t *r = buffer.ptr();
			const float *data = (const float *)r;
			_multimesh_re_create_aabb(multimesh, data, multimesh->instances);
		}
	}

	multimesh->dependency.changed_notify(RendererStorage::DEPENDENCY_CHANGED_MESH);
}

#define MULTIMESH_DIRTY_REGION_SIZE 512

void MeshStorage::_multimesh_make_local(MultiMesh *multimesh) const {
	if (multimesh->data_cache.size() > 0) {
		return; //already local
	}
	ERR_FAIL_COND(multimesh->data_cache.size() > 0);
	// this means that the user wants to load/save individual elements,
	// for this, the data must reside on CPU, so just copy it there.
	multimesh->data_cache.resize(multimesh->instances * multimesh->stride_cache);
	{
		float *w = multimesh->data_cache.ptrw();

		if (multimesh->buffer_set) {
			Vector<uint8_t> buffer = RD::get_singleton()->buffer_get_data(multimesh->buffer);
			{
				const uint8_t *r = buffer.ptr();
				memcpy(w, r, buffer.size());
			}
		} else {
			memset(w, 0, (size_t)multimesh->instances * multimesh->stride_cache * sizeof(float));
		}
	}
	uint32_t data_cache_dirty_region_count = (multimesh->instances - 1) / MULTIMESH_DIRTY_REGION_SIZE + 1;
	multimesh->data_cache_dirty_regions = memnew_arr(bool, data_cache_dirty_region_count);
	for (uint32_t i = 0; i < data_cache_dirty_region_count; i++) {
		multimesh->data_cache_dirty_regions[i] = false;
	}
	multimesh->data_cache_used_dirty_regions = 0;
}

void MeshStorage::_multimesh_mark_dirty(MultiMesh *multimesh, int p_index, bool p_aabb) {
	uint32_t region_index = p_index / MULTIMESH_DIRTY_REGION_SIZE;
#ifdef DEBUG_ENABLED
	uint32_t data_cache_dirty_region_count = (multimesh->instances - 1) / MULTIMESH_DIRTY_REGION_SIZE + 1;
	ERR_FAIL_UNSIGNED_INDEX(region_index, data_cache_dirty_region_count); //bug
#endif
	if (!multimesh->data_cache_dirty_regions[region_index]) {
		multimesh->data_cache_dirty_regions[region_index] = true;
		multimesh->data_cache_used_dirty_regions++;
	}

	if (p_aabb) {
		multimesh->aabb_dirty = true;
	}

	if (!multimesh->dirty) {
		multimesh->dirty_list = multimesh_dirty_list;
		multimesh_dirty_list = multimesh;
		multimesh->dirty = true;
	}
}

void MeshStorage::_multimesh_mark_all_dirty(MultiMesh *multimesh, bool p_data, bool p_aabb) {
	if (p_data) {
		uint32_t data_cache_dirty_region_count = (multimesh->instances - 1) / MULTIMESH_DIRTY_REGION_SIZE + 1;

		for (uint32_t i = 0; i < data_cache_dirty_region_count; i++) {
			if (!multimesh->data_cache_dirty_regions[i]) {
				multimesh->data_cache_dirty_regions[i] = true;
				multimesh->data_cache_used_dirty_regions++;
			}
		}
	}

	if (p_aabb) {
		multimesh->aabb_dirty = true;
	}

	if (!multimesh->dirty) {
		multimesh->dirty_list = multimesh_dirty_list;
		multimesh_dirty_list = multimesh;
		multimesh->dirty = true;
	}
}

void MeshStorage::_multimesh_re_create_aabb(MultiMesh *multimesh, const float *p_data, int p_instances) {
	ERR_FAIL_COND(multimesh->mesh.is_null());
	AABB aabb;
	AABB mesh_aabb = mesh_get_aabb(multimesh->mesh);
	for (int i = 0; i < p_instances; i++) {
		const float *data = p_data + multimesh->stride_cache * i;
		Transform3D t;

		if (multimesh->xform_format == RS::MULTIMESH_TRANSFORM_3D) {
			t.basis.elements[0][0] = data[0];
			t.basis.elements[0][1] = data[1];
			t.basis.elements[0][2] = data[2];
			t.origin.x = data[3];
			t.basis.elements[1][0] = data[4];
			t.basis.elements[1][1] = data[5];
			t.basis.elements[1][2] = data[6];
			t.origin.y = data[7];
			t.basis.elements[2][0] = data[8];
			t.basis.elements[2][1] = data[9];
			t.basis.elements[2][2] = data[10];
			t.origin.z = data[11];

		} else {
			t.basis.elements[0].x = data[0];
			t.basis.elements[1].x = data[1];
			t.origin.x = data[3];

			t.basis.elements[0].y = data[4];
			t.basis.elements[1].y = data[5];
			t.origin.y = data[7];
		}

		if (i == 0) {
			aabb = t.xform(mesh_aabb);
		} else {
			aabb.merge_with(t.xform(mesh_aabb));
		}
	}

	multimesh->aabb = aabb;
}

void MeshStorage::multimesh_instance_set_transform(RID p_multimesh, int p_index, const Transform3D &p_transform) {
	MultiMesh *multimesh = multimesh_owner.get_or_null(p_multimesh);
	ERR_FAIL_COND(!multimesh);
	ERR_FAIL_INDEX(p_index, multimesh->instances);
	ERR_FAIL_COND(multimesh->xform_format != RS::MULTIMESH_TRANSFORM_3D);

	_multimesh_make_local(multimesh);

	{
		float *w = multimesh->data_cache.ptrw();

		float *dataptr = w + p_index * multimesh->stride_cache;

		dataptr[0] = p_transform.basis.elements[0][0];
		dataptr[1] = p_transform.basis.elements[0][1];
		dataptr[2] = p_transform.basis.elements[0][2];
		dataptr[3] = p_transform.origin.x;
		dataptr[4] = p_transform.basis.elements[1][0];
		dataptr[5] = p_transform.basis.elements[1][1];
		dataptr[6] = p_transform.basis.elements[1][2];
		dataptr[7] = p_transform.origin.y;
		dataptr[8] = p_transform.basis.elements[2][0];
		dataptr[9] = p_transform.basis.elements[2][1];
		dataptr[10] = p_transform.basis.elements[2][2];
		dataptr[11] = p_transform.origin.z;
	}

	_multimesh_mark_dirty(multimesh, p_index, true);
}

void MeshStorage::multimesh_instance_set_transform_2d(RID p_multimesh, int p_index, const Transform2D &p_transform) {
	MultiMesh *multimesh = multimesh_owner.get_or_null(p_multimesh);
	ERR_FAIL_COND(!multimesh);
	ERR_FAIL_INDEX(p_index, multimesh->instances);
	ERR_FAIL_COND(multimesh->xform_format != RS::MULTIMESH_TRANSFORM_2D);

	_multimesh_make_local(multimesh);

	{
		float *w = multimesh->data_cache.ptrw();

		float *dataptr = w + p_index * multimesh->stride_cache;

		dataptr[0] = p_transform.elements[0][0];
		dataptr[1] = p_transform.elements[1][0];
		dataptr[2] = 0;
		dataptr[3] = p_transform.elements[2][0];
		dataptr[4] = p_transform.elements[0][1];
		dataptr[5] = p_transform.elements[1][1];
		dataptr[6] = 0;
		dataptr[7] = p_transform.elements[2][1];
	}

	_multimesh_mark_dirty(multimesh, p_index, true);
}

void MeshStorage::multimesh_instance_set_color(RID p_multimesh, int p_index, const Color &p_color) {
	MultiMesh *multimesh = multimesh_owner.get_or_null(p_multimesh);
	ERR_FAIL_COND(!multimesh);
	ERR_FAIL_INDEX(p_index, multimesh->instances);
	ERR_FAIL_COND(!multimesh->uses_colors);

	_multimesh_make_local(multimesh);

	{
		float *w = multimesh->data_cache.ptrw();

		float *dataptr = w + p_index * multimesh->stride_cache + multimesh->color_offset_cache;

		dataptr[0] = p_color.r;
		dataptr[1] = p_color.g;
		dataptr[2] = p_color.b;
		dataptr[3] = p_color.a;
	}

	_multimesh_mark_dirty(multimesh, p_index, false);
}

void MeshStorage::multimesh_instance_set_custom_data(RID p_multimesh, int p_index, const Color &p_color) {
	MultiMesh *multimesh = multimesh_owner.get_or_null(p_multimesh);
	ERR_FAIL_COND(!multimesh);
	ERR_FAIL_INDEX(p_index, multimesh->instances);
	ERR_FAIL_COND(!multimesh->uses_custom_data);

	_multimesh_make_local(multimesh);

	{
		float *w = multimesh->data_cache.ptrw();

		float *dataptr = w + p_index * multimesh->stride_cache + multimesh->custom_data_offset_cache;

		dataptr[0] = p_color.r;
		dataptr[1] = p_color.g;
		dataptr[2] = p_color.b;
		dataptr[3] = p_color.a;
	}

	_multimesh_mark_dirty(multimesh, p_index, false);
}

RID MeshStorage::multimesh_get_mesh(RID p_multimesh) const {
	MultiMesh *multimesh = multimesh_owner.get_or_null(p_multimesh);
	ERR_FAIL_COND_V(!multimesh, RID());

	return multimesh->mesh;
}

Transform3D MeshStorage::multimesh_instance_get_transform(RID p_multimesh, int p_index) const {
	MultiMesh *multimesh = multimesh_owner.get_or_null(p_multimesh);
	ERR_FAIL_COND_V(!multimesh, Transform3D());
	ERR_FAIL_INDEX_V(p_index, multimesh->instances, Transform3D());
	ERR_FAIL_COND_V(multimesh->xform_format != RS::MULTIMESH_TRANSFORM_3D, Transform3D());

	_multimesh_make_local(multimesh);

	Transform3D t;
	{
		const float *r = multimesh->data_cache.ptr();

		const float *dataptr = r + p_index * multimesh->stride_cache;

		t.basis.elements[0][0] = dataptr[0];
		t.basis.elements[0][1] = dataptr[1];
		t.basis.elements[0][2] = dataptr[2];
		t.origin.x = dataptr[3];
		t.basis.elements[1][0] = dataptr[4];
		t.basis.elements[1][1] = dataptr[5];
		t.basis.elements[1][2] = dataptr[6];
		t.origin.y = dataptr[7];
		t.basis.elements[2][0] = dataptr[8];
		t.basis.elements[2][1] = dataptr[9];
		t.basis.elements[2][2] = dataptr[10];
		t.origin.z = dataptr[11];
	}

	return t;
}

Transform2D MeshStorage::multimesh_instance_get_transform_2d(RID p_multimesh, int p_index) const {
	MultiMesh *multimesh = multimesh_owner.get_or_null(p_multimesh);
	ERR_FAIL_COND_V(!multimesh, Transform2D());
	ERR_FAIL_INDEX_V(p_index, multimesh->instances, Transform2D());
	ERR_FAIL_COND_V(multimesh->xform_format != RS::MULTIMESH_TRANSFORM_2D, Transform2D());

	_multimesh_make_local(multimesh);

	Transform2D t;
	{
		const float *r = multimesh->data_cache.ptr();

		const float *dataptr = r + p_index * multimesh->stride_cache;

		t.elements[0][0] = dataptr[0];
		t.elements[1][0] = dataptr[1];
		t.elements[2][0] = dataptr[3];
		t.elements[0][1] = dataptr[4];
		t.elements[1][1] = dataptr[5];
		t.elements[2][1] = dataptr[7];
	}

	return t;
}

Color MeshStorage::multimesh_instance_get_color(RID p_multimesh, int p_index) const {
	MultiMesh *multimesh = multimesh_owner.get_or_null(p_multimesh);
	ERR_FAIL_COND_V(!multimesh, Color());
	ERR_FAIL_INDEX_V(p_index, multimesh->instances, Color());
	ERR_FAIL_COND_V(!multimesh->uses_colors, Color());

	_multimesh_make_local(multimesh);

	Color c;
	{
		const float *r = multimesh->data_cache.ptr();

		const float *dataptr = r + p_index * multimesh->stride_cache + multimesh->color_offset_cache;

		c.r = dataptr[0];
		c.g = dataptr[1];
		c.b = dataptr[2];
		c.a = dataptr[3];
	}

	return c;
}

Color MeshStorage::multimesh_instance_get_custom_data(RID p_multimesh, int p_index) const {
	MultiMesh *multimesh = multimesh_owner.get_or_null(p_multimesh);
	ERR_FAIL_COND_V(!multimesh, Color());
	ERR_FAIL_INDEX_V(p_index, multimesh->instances, Color());
	ERR_FAIL_COND_V(!multimesh->uses_custom_data, Color());

	_multimesh_make_local(multimesh);

	Color c;
	{
		const float *r = multimesh->data_cache.ptr();

		const float *dataptr = r + p_index * multimesh->stride_cache + multimesh->custom_data_offset_cache;

		c.r = dataptr[0];
		c.g = dataptr[1];
		c.b = dataptr[2];
		c.a = dataptr[3];
	}

	return c;
}

void MeshStorage::multimesh_set_buffer(RID p_multimesh, const Vector<float> &p_buffer) {
	MultiMesh *multimesh = multimesh_owner.get_or_null(p_multimesh);
	ERR_FAIL_COND(!multimesh);
	ERR_FAIL_COND(p_buffer.size() != (multimesh->instances * (int)multimesh->stride_cache));

	{
		const float *r = p_buffer.ptr();
		RD::get_singleton()->buffer_update(multimesh->buffer, 0, p_buffer.size() * sizeof(float), r);
		multimesh->buffer_set = true;
	}

	if (multimesh->data_cache.size()) {
		//if we have a data cache, just update it
		multimesh->data_cache = p_buffer;
		{
			//clear dirty since nothing will be dirty anymore
			uint32_t data_cache_dirty_region_count = (multimesh->instances - 1) / MULTIMESH_DIRTY_REGION_SIZE + 1;
			for (uint32_t i = 0; i < data_cache_dirty_region_count; i++) {
				multimesh->data_cache_dirty_regions[i] = false;
			}
			multimesh->data_cache_used_dirty_regions = 0;
		}

		_multimesh_mark_all_dirty(multimesh, false, true); //update AABB
	} else if (multimesh->mesh.is_valid()) {
		//if we have a mesh set, we need to re-generate the AABB from the new data
		const float *data = p_buffer.ptr();

		_multimesh_re_create_aabb(multimesh, data, multimesh->instances);
		multimesh->dependency.changed_notify(RendererStorage::DEPENDENCY_CHANGED_AABB);
	}
}

Vector<float> MeshStorage::multimesh_get_buffer(RID p_multimesh) const {
	MultiMesh *multimesh = multimesh_owner.get_or_null(p_multimesh);
	ERR_FAIL_COND_V(!multimesh, Vector<float>());
	if (multimesh->buffer.is_null()) {
		return Vector<float>();
	} else if (multimesh->data_cache.size()) {
		return multimesh->data_cache;
	} else {
		//get from memory

		Vector<uint8_t> buffer = RD::get_singleton()->buffer_get_data(multimesh->buffer);
		Vector<float> ret;
		ret.resize(multimesh->instances * multimesh->stride_cache);
		{
			float *w = ret.ptrw();
			const uint8_t *r = buffer.ptr();
			memcpy(w, r, buffer.size());
		}

		return ret;
	}
}

void MeshStorage::multimesh_set_visible_instances(RID p_multimesh, int p_visible) {
	MultiMesh *multimesh = multimesh_owner.get_or_null(p_multimesh);
	ERR_FAIL_COND(!multimesh);
	ERR_FAIL_COND(p_visible < -1 || p_visible > multimesh->instances);
	if (multimesh->visible_instances == p_visible) {
		return;
	}

	if (multimesh->data_cache.size()) {
		//there is a data cache..
		_multimesh_mark_all_dirty(multimesh, false, true);
	}

	multimesh->visible_instances = p_visible;

	multimesh->dependency.changed_notify(RendererStorage::DEPENDENCY_CHANGED_MULTIMESH_VISIBLE_INSTANCES);
}

int MeshStorage::multimesh_get_visible_instances(RID p_multimesh) const {
	MultiMesh *multimesh = multimesh_owner.get_or_null(p_multimesh);
	ERR_FAIL_COND_V(!multimesh, 0);
	return multimesh->visible_instances;
}

AABB MeshStorage::multimesh_get_aabb(RID p_multimesh) const {
	MultiMesh *multimesh = multimesh_owner.get_or_null(p_multimesh);
	ERR_FAIL_COND_V(!multimesh, AABB());
	if (multimesh->aabb_dirty) {
		const_cast<MeshStorage *>(this)->_update_dirty_multimeshes();
	}
	return multimesh->aabb;
}

void MeshStorage::_update_dirty_multimeshes() {
	while (multimesh_dirty_list) {
		MultiMesh *multimesh = multimesh_dirty_list;

		if (multimesh->data_cache.size()) { //may have been cleared, so only process if it exists
			const float *data = multimesh->data_cache.ptr();

			uint32_t visible_instances = multimesh->visible_instances >= 0 ? multimesh->visible_instances : multimesh->instances;

			if (multimesh->data_cache_used_dirty_regions) {
				uint32_t data_cache_dirty_region_count = (multimesh->instances - 1) / MULTIMESH_DIRTY_REGION_SIZE + 1;
				uint32_t visible_region_count = visible_instances == 0 ? 0 : (visible_instances - 1) / MULTIMESH_DIRTY_REGION_SIZE + 1;

				uint32_t region_size = multimesh->stride_cache * MULTIMESH_DIRTY_REGION_SIZE * sizeof(float);

				if (multimesh->data_cache_used_dirty_regions > 32 || multimesh->data_cache_used_dirty_regions > visible_region_count / 2) {
					//if there too many dirty regions, or represent the majority of regions, just copy all, else transfer cost piles up too much
					RD::get_singleton()->buffer_update(multimesh->buffer, 0, MIN(visible_region_count * region_size, multimesh->instances * (uint32_t)multimesh->stride_cache * (uint32_t)sizeof(float)), data);
				} else {
					//not that many regions? update them all
					for (uint32_t i = 0; i < visible_region_count; i++) {
						if (multimesh->data_cache_dirty_regions[i]) {
							uint32_t offset = i * region_size;
							uint32_t size = multimesh->stride_cache * (uint32_t)multimesh->instances * (uint32_t)sizeof(float);
							uint32_t region_start_index = multimesh->stride_cache * MULTIMESH_DIRTY_REGION_SIZE * i;
							RD::get_singleton()->buffer_update(multimesh->buffer, offset, MIN(region_size, size - offset), &data[region_start_index]);
						}
					}
				}

				for (uint32_t i = 0; i < data_cache_dirty_region_count; i++) {
					multimesh->data_cache_dirty_regions[i] = false;
				}

				multimesh->data_cache_used_dirty_regions = 0;
			}

			if (multimesh->aabb_dirty) {
				//aabb is dirty..
				_multimesh_re_create_aabb(multimesh, data, visible_instances);
				multimesh->aabb_dirty = false;
				multimesh->dependency.changed_notify(RendererStorage::DEPENDENCY_CHANGED_AABB);
			}
		}

		multimesh_dirty_list = multimesh->dirty_list;

		multimesh->dirty_list = nullptr;
		multimesh->dirty = false;
	}

	multimesh_dirty_list = nullptr;
}

/* SKELETON API */

RID MeshStorage::skeleton_allocate() {
	return skeleton_owner.allocate_rid();
}
void MeshStorage::skeleton_initialize(RID p_rid) {
	skeleton_owner.initialize_rid(p_rid, Skeleton());
}

void MeshStorage::skeleton_free(RID p_rid) {
	_update_dirty_skeletons();
	skeleton_allocate_data(p_rid, 0);
	Skeleton *skeleton = skeleton_owner.get_or_null(p_rid);
	skeleton->dependency.deleted_notify(p_rid);
	skeleton_owner.free(p_rid);
}

void MeshStorage::_skeleton_make_dirty(Skeleton *skeleton) {
	if (!skeleton->dirty) {
		skeleton->dirty = true;
		skeleton->dirty_list = skeleton_dirty_list;
		skeleton_dirty_list = skeleton;
	}
}

void MeshStorage::skeleton_allocate_data(RID p_skeleton, int p_bones, bool p_2d_skeleton) {
	Skeleton *skeleton = skeleton_owner.get_or_null(p_skeleton);
	ERR_FAIL_COND(!skeleton);
	ERR_FAIL_COND(p_bones < 0);

	if (skeleton->size == p_bones && skeleton->use_2d == p_2d_skeleton) {
		return;
	}

	skeleton->size = p_bones;
	skeleton->use_2d = p_2d_skeleton;
	skeleton->uniform_set_3d = RID();

	if (skeleton->buffer.is_valid()) {
		RD::get_singleton()->free(skeleton->buffer);
		skeleton->buffer = RID();
		skeleton->data.clear();
		skeleton->uniform_set_mi = RID();
	}

	if (skeleton->size) {
		skeleton->data.resize(skeleton->size * (skeleton->use_2d ? 8 : 12));
		skeleton->buffer = RD::get_singleton()->storage_buffer_create(skeleton->data.size() * sizeof(float));
		memset(skeleton->data.ptrw(), 0, skeleton->data.size() * sizeof(float));

		_skeleton_make_dirty(skeleton);

		{
			Vector<RD::Uniform> uniforms;
			{
				RD::Uniform u;
				u.binding = 0;
				u.uniform_type = RD::UNIFORM_TYPE_STORAGE_BUFFER;
				u.append_id(skeleton->buffer);
				uniforms.push_back(u);
			}
			skeleton->uniform_set_mi = RD::get_singleton()->uniform_set_create(uniforms, skeleton_shader.version_shader[0], SkeletonShader::UNIFORM_SET_SKELETON);
		}
	}

	skeleton->dependency.changed_notify(RendererStorage::DEPENDENCY_CHANGED_SKELETON_DATA);
}

int MeshStorage::skeleton_get_bone_count(RID p_skeleton) const {
	Skeleton *skeleton = skeleton_owner.get_or_null(p_skeleton);
	ERR_FAIL_COND_V(!skeleton, 0);

	return skeleton->size;
}

void MeshStorage::skeleton_bone_set_transform(RID p_skeleton, int p_bone, const Transform3D &p_transform) {
	Skeleton *skeleton = skeleton_owner.get_or_null(p_skeleton);

	ERR_FAIL_COND(!skeleton);
	ERR_FAIL_INDEX(p_bone, skeleton->size);
	ERR_FAIL_COND(skeleton->use_2d);

	float *dataptr = skeleton->data.ptrw() + p_bone * 12;

	dataptr[0] = p_transform.basis.elements[0][0];
	dataptr[1] = p_transform.basis.elements[0][1];
	dataptr[2] = p_transform.basis.elements[0][2];
	dataptr[3] = p_transform.origin.x;
	dataptr[4] = p_transform.basis.elements[1][0];
	dataptr[5] = p_transform.basis.elements[1][1];
	dataptr[6] = p_transform.basis.elements[1][2];
	dataptr[7] = p_transform.origin.y;
	dataptr[8] = p_transform.basis.elements[2][0];
	dataptr[9] = p_transform.basis.elements[2][1];
	dataptr[10] = p_transform.basis.elements[2][2];
	dataptr[11] = p_transform.origin.z;

	_skeleton_make_dirty(skeleton);
}

Transform3D MeshStorage::skeleton_bone_get_transform(RID p_skeleton, int p_bone) const {
	Skeleton *skeleton = skeleton_owner.get_or_null(p_skeleton);

	ERR_FAIL_COND_V(!skeleton, Transform3D());
	ERR_FAIL_INDEX_V(p_bone, skeleton->size, Transform3D());
	ERR_FAIL_COND_V(skeleton->use_2d, Transform3D());

	const float *dataptr = skeleton->data.ptr() + p_bone * 12;

	Transform3D t;

	t.basis.elements[0][0] = dataptr[0];
	t.basis.elements[0][1] = dataptr[1];
	t.basis.elements[0][2] = dataptr[2];
	t.origin.x = dataptr[3];
	t.basis.elements[1][0] = dataptr[4];
	t.basis.elements[1][1] = dataptr[5];
	t.basis.elements[1][2] = dataptr[6];
	t.origin.y = dataptr[7];
	t.basis.elements[2][0] = dataptr[8];
	t.basis.elements[2][1] = dataptr[9];
	t.basis.elements[2][2] = dataptr[10];
	t.origin.z = dataptr[11];

	return t;
}

void MeshStorage::skeleton_bone_set_transform_2d(RID p_skeleton, int p_bone, const Transform2D &p_transform) {
	Skeleton *skeleton = skeleton_owner.get_or_null(p_skeleton);

	ERR_FAIL_COND(!skeleton);
	ERR_FAIL_INDEX(p_bone, skeleton->size);
	ERR_FAIL_COND(!skeleton->use_2d);

	float *dataptr = skeleton->data.ptrw() + p_bone * 8;

	dataptr[0] = p_transform.elements[0][0];
	dataptr[1] = p_transform.elements[1][0];
	dataptr[2] = 0;
	dataptr[3] = p_transform.elements[2][0];
	dataptr[4] = p_transform.elements[0][1];
	dataptr[5] = p_transform.elements[1][1];
	dataptr[6] = 0;
	dataptr[7] = p_transform.elements[2][1];

	_skeleton_make_dirty(skeleton);
}

Transform2D MeshStorage::skeleton_bone_get_transform_2d(RID p_skeleton, int p_bone) const {
	Skeleton *skeleton = skeleton_owner.get_or_null(p_skeleton);

	ERR_FAIL_COND_V(!skeleton, Transform2D());
	ERR_FAIL_INDEX_V(p_bone, skeleton->size, Transform2D());
	ERR_FAIL_COND_V(!skeleton->use_2d, Transform2D());

	const float *dataptr = skeleton->data.ptr() + p_bone * 8;

	Transform2D t;
	t.elements[0][0] = dataptr[0];
	t.elements[1][0] = dataptr[1];
	t.elements[2][0] = dataptr[3];
	t.elements[0][1] = dataptr[4];
	t.elements[1][1] = dataptr[5];
	t.elements[2][1] = dataptr[7];

	return t;
}

void MeshStorage::skeleton_set_base_transform_2d(RID p_skeleton, const Transform2D &p_base_transform) {
	Skeleton *skeleton = skeleton_owner.get_or_null(p_skeleton);

	ERR_FAIL_COND(!skeleton->use_2d);

	skeleton->base_transform_2d = p_base_transform;
}

void MeshStorage::_update_dirty_skeletons() {
	while (skeleton_dirty_list) {
		Skeleton *skeleton = skeleton_dirty_list;

		if (skeleton->size) {
			RD::get_singleton()->buffer_update(skeleton->buffer, 0, skeleton->data.size() * sizeof(float), skeleton->data.ptr());
		}

		skeleton_dirty_list = skeleton->dirty_list;

		skeleton->dependency.changed_notify(RendererStorage::DEPENDENCY_CHANGED_SKELETON_BONES);

		skeleton->version++;

		skeleton->dirty = false;
		skeleton->dirty_list = nullptr;
	}

	skeleton_dirty_list = nullptr;
}

void MeshStorage::skeleton_update_dependency(RID p_skeleton, RendererStorage::DependencyTracker *p_instance) {
	Skeleton *skeleton = skeleton_owner.get_or_null(p_skeleton);
	ERR_FAIL_COND(!skeleton);

	p_instance->update_dependency(&skeleton->dependency);
}
