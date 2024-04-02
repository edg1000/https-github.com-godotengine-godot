/**************************************************************************/
/*  capsule_shape_3d.cpp                                                  */
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

#include "capsule_shape_3d.h"

#include "scene/resources/mesh.h"
#include "servers/physics_server_3d.h"

Vector<Vector3> CapsuleShape3D::get_debug_mesh_lines() const {
	float c_radius = get_radius();
	float c_height = get_height();

	Vector<Vector3> points;

	Vector3 d(0, c_height * 0.5 - c_radius, 0);
	for (int i = 0; i < 360; i++) {
		float ra = Math::deg_to_rad((float)i);
		float rb = Math::deg_to_rad((float)i + 1);
		Point2 a = Vector2(Math::sin(ra), Math::cos(ra)) * c_radius;
		Point2 b = Vector2(Math::sin(rb), Math::cos(rb)) * c_radius;

		points.push_back(Vector3(a.x, 0, a.y) + d);
		points.push_back(Vector3(b.x, 0, b.y) + d);

		points.push_back(Vector3(a.x, 0, a.y) - d);
		points.push_back(Vector3(b.x, 0, b.y) - d);

		if (i % 90 == 0) {
			points.push_back(Vector3(a.x, 0, a.y) + d);
			points.push_back(Vector3(a.x, 0, a.y) - d);
		}

		Vector3 dud = i < 180 ? d : -d;

		points.push_back(Vector3(0, a.x, a.y) + dud);
		points.push_back(Vector3(0, b.x, b.y) + dud);
		points.push_back(Vector3(a.y, a.x, 0) + dud);
		points.push_back(Vector3(b.y, b.x, 0) + dud);
	}

	return points;
}

Ref<ArrayMesh> CapsuleShape3D::get_debug_arraymesh_faces(const Color &p_modulate) const {
	int prevrow, thisrow, point;
	float x, y, z, u, v, w;

	const int rings = 32;
	const int radial_segments = 32;

	Vector<Vector3> points;
	Vector<Color> colors;
	Vector<int> indices;
	point = 0;

	/* top hemisphere */
	thisrow = 0;
	prevrow = 0;
	for (int j = 0; j <= (rings + 1); j++) {
		v = j;

		v /= (rings + 1);
		w = sin(0.5 * Math_PI * v);
		y = radius * cos(0.5 * Math_PI * v);

		for (int i = 0; i <= radial_segments; i++) {
			u = i;
			u /= radial_segments;

			x = -sin(u * Math_TAU);
			z = cos(u * Math_TAU);

			Vector3 p = Vector3(x * radius * w, y, -z * radius * w);
			points.push_back(p + Vector3(0.0, 0.5 * height - radius, 0.0));
			colors.push_back(p_modulate);
			point++;

			if (i > 0 && j > 0) {
				indices.push_back(prevrow + i - 1);
				indices.push_back(prevrow + i);
				indices.push_back(thisrow + i - 1);

				indices.push_back(prevrow + i);
				indices.push_back(thisrow + i);
				indices.push_back(thisrow + i - 1);
			}
		}

		prevrow = thisrow;
		thisrow = point;
	}

	/* cylinder */
	thisrow = point;
	prevrow = 0;
	for (int j = 0; j <= (rings + 1); j++) {
		v = j;
		v /= (rings + 1);

		y = (height - 2.0 * radius) * v;
		y = (0.5 * height - radius) - y;

		for (int i = 0; i <= radial_segments; i++) {
			u = i;
			u /= radial_segments;

			x = -sin(u * Math_TAU);
			z = cos(u * Math_TAU);

			Vector3 p = Vector3(x * radius, y, -z * radius);
			points.push_back(p);
			colors.push_back(p_modulate);
			point++;

			if (i > 0 && j > 0) {
				indices.push_back(prevrow + i - 1);
				indices.push_back(prevrow + i);
				indices.push_back(thisrow + i - 1);

				indices.push_back(prevrow + i);
				indices.push_back(thisrow + i);
				indices.push_back(thisrow + i - 1);
			}
		}

		prevrow = thisrow;
		thisrow = point;
	}

	/* bottom hemisphere */
	thisrow = point;
	prevrow = 0;
	for (int j = 0; j <= (rings + 1); j++) {
		v = j;

		v /= (rings + 1);
		v += 1.0;
		w = sin(0.5 * Math_PI * v);
		y = radius * cos(0.5 * Math_PI * v);

		for (int i = 0; i <= radial_segments; i++) {
			u = i;
			u /= radial_segments;

			x = -sin(u * Math_TAU);
			z = cos(u * Math_TAU);

			Vector3 p = Vector3(x * radius * w, y, -z * radius * w);
			points.push_back(p + Vector3(0.0, -0.5 * height + radius, 0.0));
			colors.push_back(p_modulate);
			point++;

			if (i > 0 && j > 0) {
				indices.push_back(prevrow + i - 1);
				indices.push_back(prevrow + i);
				indices.push_back(thisrow + i - 1);

				indices.push_back(prevrow + i);
				indices.push_back(thisrow + i);
				indices.push_back(thisrow + i - 1);
			}
		}

		prevrow = thisrow;
		thisrow = point;
	}

	Ref<ArrayMesh> mesh = memnew(ArrayMesh);
	Array a;
	a.resize(Mesh::ARRAY_MAX);
	a[RS::ARRAY_VERTEX] = points;
	a[RS::ARRAY_COLOR] = colors;
	a[RS::ARRAY_INDEX] = indices;
	mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, a);

	return mesh;
}

real_t CapsuleShape3D::get_enclosing_radius() const {
	return height * 0.5;
}

void CapsuleShape3D::_update_shape() {
	Dictionary d;
	d["radius"] = radius;
	d["height"] = height;
	PhysicsServer3D::get_singleton()->shape_set_data(get_shape(), d);
	Shape3D::_update_shape();
}

void CapsuleShape3D::set_radius(float p_radius) {
	ERR_FAIL_COND_MSG(p_radius < 0, "CapsuleShape3D radius cannot be negative.");
	radius = p_radius;
	if (radius > height * 0.5) {
		height = radius * 2.0;
	}
	_update_shape();
	emit_changed();
}

float CapsuleShape3D::get_radius() const {
	return radius;
}

void CapsuleShape3D::set_height(float p_height) {
	ERR_FAIL_COND_MSG(p_height < 0, "CapsuleShape3D height cannot be negative.");
	height = p_height;
	if (radius > height * 0.5) {
		radius = height * 0.5;
	}
	_update_shape();
	emit_changed();
}

float CapsuleShape3D::get_height() const {
	return height;
}

void CapsuleShape3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_radius", "radius"), &CapsuleShape3D::set_radius);
	ClassDB::bind_method(D_METHOD("get_radius"), &CapsuleShape3D::get_radius);
	ClassDB::bind_method(D_METHOD("set_height", "height"), &CapsuleShape3D::set_height);
	ClassDB::bind_method(D_METHOD("get_height"), &CapsuleShape3D::get_height);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius", PROPERTY_HINT_RANGE, "0.001,100,0.001,or_greater,suffix:m"), "set_radius", "get_radius");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "height", PROPERTY_HINT_RANGE, "0.001,100,0.001,or_greater,suffix:m"), "set_height", "get_height");
	ADD_LINKED_PROPERTY("radius", "height");
	ADD_LINKED_PROPERTY("height", "radius");
}

CapsuleShape3D::CapsuleShape3D() :
		Shape3D(PhysicsServer3D::get_singleton()->shape_create(PhysicsServer3D::SHAPE_CAPSULE)) {
	_update_shape();
}
