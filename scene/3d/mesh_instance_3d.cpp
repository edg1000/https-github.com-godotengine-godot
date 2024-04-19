/**************************************************************************/
/*  mesh_instance_3d.cpp                                                  */
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

#include "mesh_instance_3d.h"

#include "scene/3d/physics/collision_shape_3d.h"
#include "scene/3d/physics/static_body_3d.h"
#include "scene/3d/skeleton_3d.h"
#include "scene/resources/3d/concave_polygon_shape_3d.h"
#include "scene/resources/3d/convex_polygon_shape_3d.h"

bool MeshInstance3D::_set(const StringName &p_name, const Variant &p_value) {
	//this is not _too_ bad performance wise, really. it only arrives here if the property was not set anywhere else.
	//add to it that it's probably found on first call to _set anyway.

	if (!get_instance().is_valid()) {
		return false;
	}

	HashMap<StringName, int>::Iterator E = blend_shape_properties.find(p_name);
	if (E) {
		set_blend_shape_value(E->value, p_value);
		return true;
	}

	if (p_name.operator String().begins_with("surface_material_override/")) {
		int idx = p_name.operator String().get_slicec('/', 1).to_int();

		if (idx >= surface_override_materials.size() || idx < 0) {
			return false;
		}

		set_surface_override_material(idx, p_value);
		return true;
	}

	return false;
}

bool MeshInstance3D::_get(const StringName &p_name, Variant &r_ret) const {
	if (!get_instance().is_valid()) {
		return false;
	}

	HashMap<StringName, int>::ConstIterator E = blend_shape_properties.find(p_name);
	if (E) {
		r_ret = get_blend_shape_value(E->value);
		return true;
	}

	if (p_name.operator String().begins_with("surface_material_override/")) {
		int idx = p_name.operator String().get_slicec('/', 1).to_int();
		if (idx >= surface_override_materials.size() || idx < 0) {
			return false;
		}
		r_ret = surface_override_materials[idx];
		return true;
	}
	return false;
}

void MeshInstance3D::_get_property_list(List<PropertyInfo> *p_list) const {
	List<String> ls;
	for (const KeyValue<StringName, int> &E : blend_shape_properties) {
		ls.push_back(E.key);
	}

	ls.sort();

	for (const String &E : ls) {
		p_list->push_back(PropertyInfo(Variant::FLOAT, E, PROPERTY_HINT_RANGE, "-1,1,0.00001"));
	}

	if (mesh.is_valid()) {
		for (int i = 0; i < mesh->get_surface_count(); i++) {
			p_list->push_back(PropertyInfo(Variant::OBJECT, vformat("%s/%d", PNAME("surface_material_override"), i), PROPERTY_HINT_RESOURCE_TYPE, "BaseMaterial3D,ShaderMaterial", PROPERTY_USAGE_DEFAULT));
		}
	}
}

void MeshInstance3D::set_mesh(const Ref<Mesh> &p_mesh) {
	if (mesh == p_mesh) {
		return;
	}

	if (mesh.is_valid()) {
		mesh->disconnect_changed(callable_mp(this, &MeshInstance3D::_mesh_changed));
	}

	mesh = p_mesh;

	if (mesh.is_valid()) {
		// If mesh is a PrimitiveMesh, calling get_rid on it can trigger a changed callback
		// so do this before connecting _mesh_changed.
		set_base(mesh->get_rid());
		mesh->connect_changed(callable_mp(this, &MeshInstance3D::_mesh_changed));
		_mesh_changed();
	} else {
		blend_shape_tracks.clear();
		blend_shape_properties.clear();
		set_base(RID());
		update_gizmos();
	}

	notify_property_list_changed();
}

Ref<Mesh> MeshInstance3D::get_mesh() const {
	return mesh;
}

int MeshInstance3D::get_blend_shape_count() const {
	if (mesh.is_null()) {
		return 0;
	}
	return mesh->get_blend_shape_count();
}
int MeshInstance3D::find_blend_shape_by_name(const StringName &p_name) {
	if (mesh.is_null()) {
		return -1;
	}
	for (int i = 0; i < mesh->get_blend_shape_count(); i++) {
		if (mesh->get_blend_shape_name(i) == p_name) {
			return i;
		}
	}
	return -1;
}
float MeshInstance3D::get_blend_shape_value(int p_blend_shape) const {
	ERR_FAIL_COND_V(mesh.is_null(), 0.0);
	ERR_FAIL_INDEX_V(p_blend_shape, (int)blend_shape_tracks.size(), 0);
	return blend_shape_tracks[p_blend_shape];
}
void MeshInstance3D::set_blend_shape_value(int p_blend_shape, float p_value) {
	ERR_FAIL_COND(mesh.is_null());
	ERR_FAIL_INDEX(p_blend_shape, (int)blend_shape_tracks.size());
	blend_shape_tracks[p_blend_shape] = p_value;
	RenderingServer::get_singleton()->instance_set_blend_shape_weight(get_instance(), p_blend_shape, p_value);
}

void MeshInstance3D::_resolve_skeleton_path() {
	Ref<SkinReference> new_skin_reference;

	if (!skeleton_path.is_empty()) {
		Skeleton3D *skeleton = Object::cast_to<Skeleton3D>(get_node(skeleton_path));
		if (skeleton) {
			if (skin_internal.is_null()) {
				new_skin_reference = skeleton->register_skin(skeleton->create_skin_from_rest_transforms());
				//a skin was created for us
				skin_internal = new_skin_reference->get_skin();
				notify_property_list_changed();
			} else {
				new_skin_reference = skeleton->register_skin(skin_internal);
			}
		}
	}

	skin_ref = new_skin_reference;

	if (skin_ref.is_valid()) {
		RenderingServer::get_singleton()->instance_attach_skeleton(get_instance(), skin_ref->get_skeleton());
	} else {
		RenderingServer::get_singleton()->instance_attach_skeleton(get_instance(), RID());
	}
}

void MeshInstance3D::set_skin(const Ref<Skin> &p_skin) {
	skin_internal = p_skin;
	skin = p_skin;
	if (!is_inside_tree()) {
		return;
	}
	_resolve_skeleton_path();
}

Ref<Skin> MeshInstance3D::get_skin() const {
	return skin;
}

Ref<SkinReference> MeshInstance3D::get_skin_reference() const {
	return skin_ref;
}

void MeshInstance3D::set_skeleton_path(const NodePath &p_skeleton) {
	skeleton_path = p_skeleton;
	if (!is_inside_tree()) {
		return;
	}
	_resolve_skeleton_path();
}

NodePath MeshInstance3D::get_skeleton_path() {
	return skeleton_path;
}

AABB MeshInstance3D::get_aabb() const {
	if (!mesh.is_null()) {
		return mesh->get_aabb();
	}

	return AABB();
}

Node *MeshInstance3D::create_trimesh_collision_node() {
	if (mesh.is_null()) {
		return nullptr;
	}

	Ref<ConcavePolygonShape3D> shape = mesh->create_trimesh_shape();
	if (shape.is_null()) {
		return nullptr;
	}

	StaticBody3D *static_body = memnew(StaticBody3D);
	CollisionShape3D *cshape = memnew(CollisionShape3D);
	cshape->set_shape(shape);
	static_body->add_child(cshape, true);
	return static_body;
}

void MeshInstance3D::create_trimesh_collision() {
	StaticBody3D *static_body = Object::cast_to<StaticBody3D>(create_trimesh_collision_node());
	ERR_FAIL_NULL(static_body);
	static_body->set_name(String(get_name()) + "_col");

	add_child(static_body, true);
	if (get_owner()) {
		CollisionShape3D *cshape = Object::cast_to<CollisionShape3D>(static_body->get_child(0));
		static_body->set_owner(get_owner());
		cshape->set_owner(get_owner());
	}
}

Node *MeshInstance3D::create_convex_collision_node(bool p_clean, bool p_simplify) {
	if (mesh.is_null()) {
		return nullptr;
	}

	Ref<ConvexPolygonShape3D> shape = mesh->create_convex_shape(p_clean, p_simplify);
	if (shape.is_null()) {
		return nullptr;
	}

	StaticBody3D *static_body = memnew(StaticBody3D);
	CollisionShape3D *cshape = memnew(CollisionShape3D);
	cshape->set_shape(shape);
	static_body->add_child(cshape, true);
	return static_body;
}

void MeshInstance3D::create_convex_collision(bool p_clean, bool p_simplify) {
	StaticBody3D *static_body = Object::cast_to<StaticBody3D>(create_convex_collision_node(p_clean, p_simplify));
	ERR_FAIL_NULL(static_body);
	static_body->set_name(String(get_name()) + "_col");

	add_child(static_body, true);
	if (get_owner()) {
		CollisionShape3D *cshape = Object::cast_to<CollisionShape3D>(static_body->get_child(0));
		static_body->set_owner(get_owner());
		cshape->set_owner(get_owner());
	}
}

Node *MeshInstance3D::create_multiple_convex_collisions_node(const Ref<MeshConvexDecompositionSettings> &p_settings) {
	if (mesh.is_null()) {
		return nullptr;
	}

	Ref<MeshConvexDecompositionSettings> settings;
	if (p_settings.is_valid()) {
		settings = p_settings;
	} else {
		settings.instantiate();
	}

	Vector<Ref<Shape3D>> shapes = mesh->convex_decompose(settings);
	if (!shapes.size()) {
		return nullptr;
	}

	StaticBody3D *static_body = memnew(StaticBody3D);
	for (int i = 0; i < shapes.size(); i++) {
		CollisionShape3D *cshape = memnew(CollisionShape3D);
		cshape->set_shape(shapes[i]);
		static_body->add_child(cshape, true);
	}
	return static_body;
}

void MeshInstance3D::create_multiple_convex_collisions(const Ref<MeshConvexDecompositionSettings> &p_settings) {
	StaticBody3D *static_body = Object::cast_to<StaticBody3D>(create_multiple_convex_collisions_node(p_settings));
	ERR_FAIL_NULL(static_body);
	static_body->set_name(String(get_name()) + "_col");

	add_child(static_body, true);
	if (get_owner()) {
		static_body->set_owner(get_owner());
		int count = static_body->get_child_count();
		for (int i = 0; i < count; i++) {
			CollisionShape3D *cshape = Object::cast_to<CollisionShape3D>(static_body->get_child(i));
			cshape->set_owner(get_owner());
		}
	}
}

void MeshInstance3D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			_resolve_skeleton_path();
		} break;
		case NOTIFICATION_TRANSLATION_CHANGED: {
			if (mesh.is_valid()) {
				mesh->notification(NOTIFICATION_TRANSLATION_CHANGED);
			}
		} break;
	}
}

int MeshInstance3D::get_surface_override_material_count() const {
	return surface_override_materials.size();
}

void MeshInstance3D::set_surface_override_material(int p_surface, const Ref<Material> &p_material) {
	ERR_FAIL_INDEX(p_surface, surface_override_materials.size());

	surface_override_materials.write[p_surface] = p_material;

	if (surface_override_materials[p_surface].is_valid()) {
		RS::get_singleton()->instance_set_surface_override_material(get_instance(), p_surface, surface_override_materials[p_surface]->get_rid());
	} else {
		RS::get_singleton()->instance_set_surface_override_material(get_instance(), p_surface, RID());
	}
}

Ref<Material> MeshInstance3D::get_surface_override_material(int p_surface) const {
	ERR_FAIL_INDEX_V(p_surface, surface_override_materials.size(), Ref<Material>());

	return surface_override_materials[p_surface];
}

Ref<Material> MeshInstance3D::get_active_material(int p_surface) const {
	Ref<Material> mat_override = get_material_override();
	if (mat_override.is_valid()) {
		return mat_override;
	}

	Ref<Material> surface_material = get_surface_override_material(p_surface);
	if (surface_material.is_valid()) {
		return surface_material;
	}

	Ref<Mesh> m = get_mesh();
	if (m.is_valid()) {
		return m->surface_get_material(p_surface);
	}

	return Ref<Material>();
}

void MeshInstance3D::_mesh_changed() {
	ERR_FAIL_COND(mesh.is_null());
	surface_override_materials.resize(mesh->get_surface_count());

	uint32_t initialize_bs_from = blend_shape_tracks.size();
	blend_shape_tracks.resize(mesh->get_blend_shape_count());

	for (uint32_t i = 0; i < blend_shape_tracks.size(); i++) {
		blend_shape_properties["blend_shapes/" + String(mesh->get_blend_shape_name(i))] = i;
		if (i < initialize_bs_from) {
			set_blend_shape_value(i, blend_shape_tracks[i]);
		} else {
			set_blend_shape_value(i, 0);
		}
	}

	int surface_count = mesh->get_surface_count();
	for (int surface_index = 0; surface_index < surface_count; ++surface_index) {
		if (surface_override_materials[surface_index].is_valid()) {
			RS::get_singleton()->instance_set_surface_override_material(get_instance(), surface_index, surface_override_materials[surface_index]->get_rid());
		}
	}

	update_gizmos();
}

MeshInstance3D *MeshInstance3D::create_debug_tangents_node() {
	Vector<Vector3> lines;
	Vector<Color> colors;

	Ref<Mesh> m = get_mesh();
	if (!m.is_valid()) {
		return nullptr;
	}

	for (int i = 0; i < m->get_surface_count(); i++) {
		Array arrays = m->surface_get_arrays(i);
		ERR_CONTINUE(arrays.size() != Mesh::ARRAY_MAX);

		Vector<Vector3> verts = arrays[Mesh::ARRAY_VERTEX];
		Vector<Vector3> norms = arrays[Mesh::ARRAY_NORMAL];
		if (norms.size() == 0) {
			continue;
		}
		Vector<float> tangents = arrays[Mesh::ARRAY_TANGENT];
		if (tangents.size() == 0) {
			continue;
		}

		for (int j = 0; j < verts.size(); j++) {
			Vector3 v = verts[j];
			Vector3 n = norms[j];
			Vector3 t = Vector3(tangents[j * 4 + 0], tangents[j * 4 + 1], tangents[j * 4 + 2]);
			Vector3 b = (n.cross(t)).normalized() * tangents[j * 4 + 3];

			lines.push_back(v); //normal
			colors.push_back(Color(0, 0, 1)); //color
			lines.push_back(v + n * 0.04); //normal
			colors.push_back(Color(0, 0, 1)); //color

			lines.push_back(v); //tangent
			colors.push_back(Color(1, 0, 0)); //color
			lines.push_back(v + t * 0.04); //tangent
			colors.push_back(Color(1, 0, 0)); //color

			lines.push_back(v); //binormal
			colors.push_back(Color(0, 1, 0)); //color
			lines.push_back(v + b * 0.04); //binormal
			colors.push_back(Color(0, 1, 0)); //color
		}
	}

	if (lines.size()) {
		Ref<StandardMaterial3D> sm;
		sm.instantiate();

		sm->set_shading_mode(StandardMaterial3D::SHADING_MODE_UNSHADED);
		sm->set_flag(StandardMaterial3D::FLAG_SRGB_VERTEX_COLOR, true);
		sm->set_flag(StandardMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
		sm->set_flag(StandardMaterial3D::FLAG_DISABLE_FOG, true);

		Ref<ArrayMesh> am;
		am.instantiate();
		Array a;
		a.resize(Mesh::ARRAY_MAX);
		a[Mesh::ARRAY_VERTEX] = lines;
		a[Mesh::ARRAY_COLOR] = colors;

		am->add_surface_from_arrays(Mesh::PRIMITIVE_LINES, a);
		am->surface_set_material(0, sm);

		MeshInstance3D *mi = memnew(MeshInstance3D);
		mi->set_mesh(am);
		mi->set_name("DebugTangents");
		return mi;
	}

	return nullptr;
}

void MeshInstance3D::create_debug_tangents() {
	MeshInstance3D *mi = create_debug_tangents_node();
	if (!mi) {
		return;
	}

	add_child(mi, true);
	if (is_inside_tree() && this == get_tree()->get_edited_scene_root()) {
		mi->set_owner(this);
	} else {
		mi->set_owner(get_owner());
	}
}

bool MeshInstance3D::_property_can_revert(const StringName &p_name) const {
	HashMap<StringName, int>::ConstIterator E = blend_shape_properties.find(p_name);
	if (E) {
		return true;
	}
	return false;
}

bool MeshInstance3D::_property_get_revert(const StringName &p_name, Variant &r_property) const {
	HashMap<StringName, int>::ConstIterator E = blend_shape_properties.find(p_name);
	if (E) {
		r_property = 0.0f;
		return true;
	}
	return false;
}

Ref<ArrayMesh> MeshInstance3D::bake_mesh_from_current_skeleton_pose(Ref<ArrayMesh> p_existing) {
	Ref<ArrayMesh> source_mesh = get_mesh();
	ERR_FAIL_NULL_V_MSG(source_mesh, Ref<ArrayMesh>(), "The source mesh must be a valid ArrayMesh.");

	Ref<ArrayMesh> bake_mesh;

	if (p_existing.is_valid()) {
		ERR_FAIL_NULL_V_MSG(p_existing, Ref<ArrayMesh>(), "The existing mesh must be a valid ArrayMesh.");
		ERR_FAIL_COND_V_MSG(source_mesh == p_existing, Ref<ArrayMesh>(), "The source mesh can not be the same mesh as the existing mesh.");

		bake_mesh = p_existing;
	} else {
		bake_mesh.instantiate();
	}

	ERR_FAIL_NULL_V_MSG(skin_ref, Ref<ArrayMesh>(), "The source mesh must have a valid skin.");
	ERR_FAIL_NULL_V_MSG(skin_internal, Ref<ArrayMesh>(), "The source mesh must have a valid skin.");
	RID skeleton = skin_ref->get_skeleton();
	ERR_FAIL_COND_V_MSG(!skeleton.is_valid(), Ref<ArrayMesh>(), "The source mesh must have its skin registered with a valid skeleton.");

	const int bone_count = RenderingServer::get_singleton()->skeleton_get_bone_count(skeleton);
	ERR_FAIL_COND_V(bone_count <= 0, Ref<ArrayMesh>());
	ERR_FAIL_COND_V(bone_count < skin_internal->get_bind_count(), Ref<ArrayMesh>());

	LocalVector<Transform3D> bone_transforms;
	bone_transforms.resize(bone_count);
	for (int bone_index = 0; bone_index < bone_count; bone_index++) {
		bone_transforms[bone_index] = RenderingServer::get_singleton()->skeleton_bone_get_transform(skeleton, bone_index);
	}

	bake_mesh->clear_surfaces();

	int mesh_surface_count = source_mesh->get_surface_count();

	for (int surface_index = 0; surface_index < mesh_surface_count; surface_index++) {
		ERR_CONTINUE(source_mesh->surface_get_primitive_type(surface_index) != Mesh::PRIMITIVE_TRIANGLES);

		uint32_t surface_format = source_mesh->surface_get_format(surface_index);

		ERR_CONTINUE(0 == (surface_format & Mesh::ARRAY_FORMAT_VERTEX));
		ERR_CONTINUE(0 == (surface_format & Mesh::ARRAY_FORMAT_BONES));
		ERR_CONTINUE(0 == (surface_format & Mesh::ARRAY_FORMAT_WEIGHTS));

		unsigned int bones_per_vertex = surface_format & Mesh::ARRAY_FLAG_USE_8_BONE_WEIGHTS ? 8 : 4;

		surface_format &= ~Mesh::ARRAY_FORMAT_BONES;
		surface_format &= ~Mesh::ARRAY_FORMAT_WEIGHTS;

		const Array &source_mesh_arrays = source_mesh->surface_get_arrays(surface_index);

		ERR_FAIL_COND_V(source_mesh_arrays.size() != RS::ARRAY_MAX, Ref<ArrayMesh>());

		const Vector<Vector3> &source_mesh_vertex_array = source_mesh_arrays[Mesh::ARRAY_VERTEX];
		const Vector<int> &source_mesh_bones_array = source_mesh_arrays[Mesh::ARRAY_BONES];
		const Vector<float> &source_mesh_weights_array = source_mesh_arrays[Mesh::ARRAY_WEIGHTS];

		unsigned int vertex_count = source_mesh_vertex_array.size();
		int expected_bone_array_size = vertex_count * bones_per_vertex;
		ERR_CONTINUE(source_mesh_bones_array.size() != expected_bone_array_size);
		ERR_CONTINUE(source_mesh_weights_array.size() != expected_bone_array_size);

		Array new_mesh_arrays;
		new_mesh_arrays.resize(Mesh::ARRAY_MAX);
		for (int i = 0; i < source_mesh_arrays.size(); i++) {
			if (i == Mesh::ARRAY_VERTEX || i == Mesh::ARRAY_BONES || i == Mesh::ARRAY_WEIGHTS) {
				continue;
			}
			new_mesh_arrays[i] = source_mesh_arrays[i];
		}

		Vector<Vector3> bone_transformed_vertex_array = source_mesh_vertex_array;

		const Vector3 *vertices_ptr = source_mesh_vertex_array.ptr();
		Vector3 *transformed_vertices_ptrw = bone_transformed_vertex_array.ptrw();

		const int *bones_ptr = source_mesh_bones_array.ptr();
		const float *weights_ptr = source_mesh_weights_array.ptr();

		for (unsigned int vertex_index = 0; vertex_index < vertex_count; vertex_index++) {
			Vector3 bone_transformed_vertex;

			for (unsigned int weight_index = 0; weight_index < bones_per_vertex; weight_index++) {
				int vertex_bone_index = bones_ptr[vertex_index * bones_per_vertex + weight_index];
				float bone_weight = weights_ptr[vertex_index * bones_per_vertex + weight_index];
				if (bone_weight < FLT_EPSILON) {
					continue;
				}
				ERR_FAIL_INDEX_V(vertex_bone_index, static_cast<int>(bone_transforms.size()), Ref<ArrayMesh>());
				bone_transformed_vertex += bone_transforms[vertex_bone_index].xform(vertices_ptr[vertex_index]) * bone_weight;
			}

			transformed_vertices_ptrw[vertex_index] = bone_transformed_vertex;
		}

		new_mesh_arrays[Mesh::ARRAY_VERTEX] = bone_transformed_vertex_array;

		bake_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, new_mesh_arrays, Array(), Dictionary(), surface_format);
	}

	return bake_mesh;
}

void MeshInstance3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_mesh", "mesh"), &MeshInstance3D::set_mesh);
	ClassDB::bind_method(D_METHOD("get_mesh"), &MeshInstance3D::get_mesh);
	ClassDB::bind_method(D_METHOD("set_skeleton_path", "skeleton_path"), &MeshInstance3D::set_skeleton_path);
	ClassDB::bind_method(D_METHOD("get_skeleton_path"), &MeshInstance3D::get_skeleton_path);
	ClassDB::bind_method(D_METHOD("set_skin", "skin"), &MeshInstance3D::set_skin);
	ClassDB::bind_method(D_METHOD("get_skin"), &MeshInstance3D::get_skin);
	ClassDB::bind_method(D_METHOD("get_skin_reference"), &MeshInstance3D::get_skin_reference);

	ClassDB::bind_method(D_METHOD("get_surface_override_material_count"), &MeshInstance3D::get_surface_override_material_count);
	ClassDB::bind_method(D_METHOD("set_surface_override_material", "surface", "material"), &MeshInstance3D::set_surface_override_material);
	ClassDB::bind_method(D_METHOD("get_surface_override_material", "surface"), &MeshInstance3D::get_surface_override_material);
	ClassDB::bind_method(D_METHOD("get_active_material", "surface"), &MeshInstance3D::get_active_material);

	ClassDB::bind_method(D_METHOD("create_trimesh_collision"), &MeshInstance3D::create_trimesh_collision);
	ClassDB::set_method_flags("MeshInstance3D", "create_trimesh_collision", METHOD_FLAGS_DEFAULT);
	ClassDB::bind_method(D_METHOD("create_convex_collision", "clean", "simplify"), &MeshInstance3D::create_convex_collision, DEFVAL(true), DEFVAL(false));
	ClassDB::set_method_flags("MeshInstance3D", "create_convex_collision", METHOD_FLAGS_DEFAULT);
	ClassDB::bind_method(D_METHOD("create_multiple_convex_collisions", "settings"), &MeshInstance3D::create_multiple_convex_collisions, DEFVAL(Ref<MeshConvexDecompositionSettings>()));
	ClassDB::set_method_flags("MeshInstance3D", "create_multiple_convex_collisions", METHOD_FLAGS_DEFAULT);

	ClassDB::bind_method(D_METHOD("get_blend_shape_count"), &MeshInstance3D::get_blend_shape_count);
	ClassDB::bind_method(D_METHOD("find_blend_shape_by_name", "name"), &MeshInstance3D::find_blend_shape_by_name);
	ClassDB::bind_method(D_METHOD("get_blend_shape_value", "blend_shape_idx"), &MeshInstance3D::get_blend_shape_value);
	ClassDB::bind_method(D_METHOD("set_blend_shape_value", "blend_shape_idx", "value"), &MeshInstance3D::set_blend_shape_value);

	ClassDB::bind_method(D_METHOD("create_debug_tangents"), &MeshInstance3D::create_debug_tangents);

	ClassDB::bind_method(D_METHOD("bake_mesh_from_current_skeleton_pose", "existing"), &MeshInstance3D::bake_mesh_from_current_skeleton_pose, DEFVAL(Ref<ArrayMesh>()));

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh", PROPERTY_HINT_RESOURCE_TYPE, "Mesh"), "set_mesh", "get_mesh");
	ADD_GROUP("Skeleton", "");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "skin", PROPERTY_HINT_RESOURCE_TYPE, "Skin"), "set_skin", "get_skin");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "skeleton", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Skeleton3D"), "set_skeleton_path", "get_skeleton_path");
	ADD_GROUP("", "");
}

MeshInstance3D::MeshInstance3D() {
}

MeshInstance3D::~MeshInstance3D() {
}
