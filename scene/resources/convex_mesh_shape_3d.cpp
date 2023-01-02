/*************************************************************************/
/*  convex_mesh_shape_3d.cpp                                             */
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

#include "convex_mesh_shape_3d.h"

#include "core/core_string_names.h"
#include "core/math/convex_hull.h"
#include "servers/physics_server_3d.h"

Vector<Vector3> ConvexMeshShape3D::get_debug_mesh_lines() const {
	Vector<Vector3> poly_points = get_points();

	if (poly_points.size() > 3) {
		Vector<Vector3> varr = Variant(poly_points);
		Geometry3D::MeshData md;
		Error err = ConvexHullComputer::convex_hull(varr, md);
		if (err == OK) {
			Vector<Vector3> lines;
			lines.resize(md.edges.size() * 2);
			for (uint32_t i = 0; i < md.edges.size(); i++) {
				lines.write[i * 2 + 0] = md.vertices[md.edges[i].vertex_a];
				lines.write[i * 2 + 1] = md.vertices[md.edges[i].vertex_b];
			}
			return lines;
		}
	}

	return Vector<Vector3>();
}

real_t ConvexMeshShape3D::get_enclosing_radius() const {
	Vector<Vector3> data = get_points();
	const Vector3 *read = data.ptr();
	real_t r = 0.0;
	for (int i(0); i < data.size(); i++) {
		r = MAX(read[i].length_squared(), r);
	}
	return Math::sqrt(r);
}

void ConvexMeshShape3D::_update_shape() {
	if (!mesh.is_null()) {
		if (mesh->get_surface_count() > 0) {
			set_points(mesh->create_convex_points(clean, simplify));
		}
	} else {
		set_points(Vector<Vector3>());
	}

	PhysicsServer3D::get_singleton()->shape_set_data(get_shape(), points);
	Shape3D::_update_shape();
}

void ConvexMeshShape3D::_update_mesh() {
	_update_shape();
	notify_change_to_owners();
}

void ConvexMeshShape3D::set_mesh(const Ref<Mesh> &p_mesh) {
	if (mesh == p_mesh) {
		return;
	}

	if (mesh.is_valid()) {
		mesh->disconnect(CoreStringNames::get_singleton()->changed, callable_mp(this, &ConvexMeshShape3D::_update_mesh));
	}

	mesh = p_mesh;

	if (!mesh.is_null() && mesh.is_valid()) {
		mesh->connect(CoreStringNames::get_singleton()->changed, callable_mp(this, &ConvexMeshShape3D::_update_mesh));
	}

	_update_mesh();
}

Ref<Mesh> ConvexMeshShape3D::get_mesh() const {
	return mesh;
}

void ConvexMeshShape3D::set_clean(bool p_clean) {
	clean = p_clean;

	_update_shape();
	notify_change_to_owners();
}

bool ConvexMeshShape3D::is_clean() const {
	return clean;
}

void ConvexMeshShape3D::set_simplify(bool p_simplify) {
	simplify = p_simplify;

	_update_shape();
	notify_change_to_owners();
}

bool ConvexMeshShape3D::is_simplify() const {
	return simplify;
}

void ConvexMeshShape3D::set_points(const Vector<Vector3> &p_points) {
	points = p_points;
}

Vector<Vector3> ConvexMeshShape3D::get_points() const {
	return points;
}

void ConvexMeshShape3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_mesh", "mesh"), &ConvexMeshShape3D::set_mesh);
	ClassDB::bind_method(D_METHOD("get_mesh"), &ConvexMeshShape3D::get_mesh);

	ClassDB::bind_method(D_METHOD("set_clean", "clean"), &ConvexMeshShape3D::set_clean);
	ClassDB::bind_method(D_METHOD("is_clean"), &ConvexMeshShape3D::is_clean);

	ClassDB::bind_method(D_METHOD("set_simplify", "simplify"), &ConvexMeshShape3D::set_simplify);
	ClassDB::bind_method(D_METHOD("is_simplify"), &ConvexMeshShape3D::is_simplify);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh", PROPERTY_HINT_RESOURCE_TYPE, "Mesh"), "set_mesh", "get_mesh");

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "clean"), "set_clean", "is_clean");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "simplify"), "set_simplify", "is_simplify");
}

ConvexMeshShape3D::ConvexMeshShape3D() :
		Shape3D(PhysicsServer3D::get_singleton()->shape_create(PhysicsServer3D::SHAPE_CONVEX_POLYGON)) {
}

ConvexMeshShape3D::ConvexMeshShape3D(const Ref<Mesh> &p_mesh) :
		ConvexMeshShape3D() {
	set_mesh(p_mesh);
}
