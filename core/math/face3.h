/*************************************************************************/
/*  face3.h                                                              */
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

#ifndef FACE3_H
#define FACE3_H

#include "core/math/aabb.h"
#include "core/math/plane.h"
#include "core/math/transform_3d.h"
#include "core/math/vector3.h"

struct _NO_DISCARD_ Face3 {
	enum Side {
		SIDE_OVER,
		SIDE_UNDER,
		SIDE_SPANNING,
		SIDE_COPLANAR
	};

	Vector3 vertex[3];

	/**
	 * @param p_plane plane used to split the face
	 * @param p_res array of at least 3 faces, amount used in function return
	 * @param p_is_point_over array of at least 3 booleans, determining which face is over the plane, amount used in function return
	 * @return amount of faces generated by the split, either 0 (means no split possible), 2 or 3
	 */
	int split_by_plane(const Plane &p_plane, Face3 *p_res, bool *p_is_point_over) const;

	Plane get_plane(ClockDirection p_dir = CLOCKWISE) const;
	Vector3 get_random_point_inside() const;

	Side get_side_of(const Face3 &p_face, ClockDirection p_clock_dir = CLOCKWISE) const;

	bool is_degenerate() const;
	real_t get_area() const;

	Vector3 get_median_point() const;
	Vector3 get_closest_point_to(const Vector3 &p_point) const;

	bool intersects_ray(const Vector3 &p_from, const Vector3 &p_dir, Vector3 *p_intersection = nullptr) const;
	bool intersects_segment(const Vector3 &p_from, const Vector3 &p_dir, Vector3 *p_intersection = nullptr) const;

	ClockDirection get_clock_dir() const; ///< todo, test if this is returning the proper clockwisity

	void get_support(const Vector3 &p_normal, const Transform3D &p_transform, Vector3 *p_vertices, int *p_count, int p_max) const;
	void project_range(const Vector3 &p_normal, const Transform3D &p_transform, real_t &r_min, real_t &r_max) const;

	AABB get_aabb() const {
		AABB aabb(vertex[0], Vector3());
		aabb.expand_to(vertex[1]);
		aabb.expand_to(vertex[2]);
		return aabb;
	}

	bool intersects_aabb(const AABB &p_aabb) const;
	_FORCE_INLINE_ bool intersects_aabb2(const AABB &p_aabb) const;
	operator String() const;

	inline Face3() {}
	inline Face3(const Vector3 &p_v1, const Vector3 &p_v2, const Vector3 &p_v3) {
		vertex[0] = p_v1;
		vertex[1] = p_v2;
		vertex[2] = p_v3;
	}
};

bool Face3::intersects_aabb2(const AABB &p_aabb) const {
	Vector3 perp = (vertex[0] - vertex[2]).cross(vertex[0] - vertex[1]);

	Vector3 half_extents = p_aabb.size * 0.5;
	Vector3 ofs = p_aabb.position + half_extents;

	Vector3 sup = Vector3(
			(perp.x > 0) ? -half_extents.x : half_extents.x,
			(perp.y > 0) ? -half_extents.y : half_extents.y,
			(perp.z > 0) ? -half_extents.z : half_extents.z);

	real_t d = perp.dot(vertex[0]);
	real_t dist_a = perp.dot(ofs + sup) - d;
	real_t dist_b = perp.dot(ofs - sup) - d;

	if (dist_a * dist_b > 0) {
		return false; //does not intersect the plane
	}

#define TEST_AXIS(m_ax)                                            \
	{                                                              \
		real_t aabb_min = p_aabb.position.m_ax;                    \
		real_t aabb_max = p_aabb.position.m_ax + p_aabb.size.m_ax; \
		real_t tri_min, tri_max;                                   \
		for (int i = 0; i < 3; i++) {                              \
			if (i == 0 || vertex[i].m_ax > tri_max)                \
				tri_max = vertex[i].m_ax;                          \
			if (i == 0 || vertex[i].m_ax < tri_min)                \
				tri_min = vertex[i].m_ax;                          \
		}                                                          \
                                                                   \
		if (tri_max < aabb_min || aabb_max < tri_min)              \
			return false;                                          \
	}

	TEST_AXIS(x);
	TEST_AXIS(y);
	TEST_AXIS(z);

#undef TEST_AXIS

	Vector3 edge_norms[3] = {
		vertex[0] - vertex[1],
		vertex[1] - vertex[2],
		vertex[2] - vertex[0],
	};

	for (int i = 0; i < 12; i++) {
		Vector3 from, to;
		switch (i) {
			case 0: {
				from = Vector3(p_aabb.position.x + p_aabb.size.x, p_aabb.position.y, p_aabb.position.z);
				to = Vector3(p_aabb.position.x, p_aabb.position.y, p_aabb.position.z);
			} break;
			case 1: {
				from = Vector3(p_aabb.position.x + p_aabb.size.x, p_aabb.position.y, p_aabb.position.z + p_aabb.size.z);
				to = Vector3(p_aabb.position.x + p_aabb.size.x, p_aabb.position.y, p_aabb.position.z);
			} break;
			case 2: {
				from = Vector3(p_aabb.position.x, p_aabb.position.y, p_aabb.position.z + p_aabb.size.z);
				to = Vector3(p_aabb.position.x + p_aabb.size.x, p_aabb.position.y, p_aabb.position.z + p_aabb.size.z);

			} break;
			case 3: {
				from = Vector3(p_aabb.position.x, p_aabb.position.y, p_aabb.position.z);
				to = Vector3(p_aabb.position.x, p_aabb.position.y, p_aabb.position.z + p_aabb.size.z);

			} break;
			case 4: {
				from = Vector3(p_aabb.position.x, p_aabb.position.y + p_aabb.size.y, p_aabb.position.z);
				to = Vector3(p_aabb.position.x + p_aabb.size.x, p_aabb.position.y + p_aabb.size.y, p_aabb.position.z);
			} break;
			case 5: {
				from = Vector3(p_aabb.position.x + p_aabb.size.x, p_aabb.position.y + p_aabb.size.y, p_aabb.position.z);
				to = Vector3(p_aabb.position.x + p_aabb.size.x, p_aabb.position.y + p_aabb.size.y, p_aabb.position.z + p_aabb.size.z);
			} break;
			case 6: {
				from = Vector3(p_aabb.position.x + p_aabb.size.x, p_aabb.position.y + p_aabb.size.y, p_aabb.position.z + p_aabb.size.z);
				to = Vector3(p_aabb.position.x, p_aabb.position.y + p_aabb.size.y, p_aabb.position.z + p_aabb.size.z);

			} break;
			case 7: {
				from = Vector3(p_aabb.position.x, p_aabb.position.y + p_aabb.size.y, p_aabb.position.z + p_aabb.size.z);
				to = Vector3(p_aabb.position.x, p_aabb.position.y + p_aabb.size.y, p_aabb.position.z);

			} break;
			case 8: {
				from = Vector3(p_aabb.position.x, p_aabb.position.y, p_aabb.position.z + p_aabb.size.z);
				to = Vector3(p_aabb.position.x, p_aabb.position.y + p_aabb.size.y, p_aabb.position.z + p_aabb.size.z);

			} break;
			case 9: {
				from = Vector3(p_aabb.position.x, p_aabb.position.y, p_aabb.position.z);
				to = Vector3(p_aabb.position.x, p_aabb.position.y + p_aabb.size.y, p_aabb.position.z);

			} break;
			case 10: {
				from = Vector3(p_aabb.position.x + p_aabb.size.x, p_aabb.position.y, p_aabb.position.z);
				to = Vector3(p_aabb.position.x + p_aabb.size.x, p_aabb.position.y + p_aabb.size.y, p_aabb.position.z);

			} break;
			case 11: {
				from = Vector3(p_aabb.position.x + p_aabb.size.x, p_aabb.position.y, p_aabb.position.z + p_aabb.size.z);
				to = Vector3(p_aabb.position.x + p_aabb.size.x, p_aabb.position.y + p_aabb.size.y, p_aabb.position.z + p_aabb.size.z);

			} break;
		}

		Vector3 e1 = from - to;
		for (int j = 0; j < 3; j++) {
			Vector3 e2 = edge_norms[j];

			Vector3 axis = vec3_cross(e1, e2);

			if (axis.length_squared() < 0.0001) {
				continue; // coplanar
			}
			//axis.normalize();

			Vector3 sup2 = Vector3(
					(axis.x > 0) ? -half_extents.x : half_extents.x,
					(axis.y > 0) ? -half_extents.y : half_extents.y,
					(axis.z > 0) ? -half_extents.z : half_extents.z);

			real_t maxB = axis.dot(ofs + sup2);
			real_t minB = axis.dot(ofs - sup2);
			if (minB > maxB) {
				SWAP(maxB, minB);
			}

			real_t minT = 1e20, maxT = -1e20;
			for (int k = 0; k < 3; k++) {
				real_t vert_d = axis.dot(vertex[k]);

				if (vert_d > maxT) {
					maxT = vert_d;
				}

				if (vert_d < minT) {
					minT = vert_d;
				}
			}

			if (maxB < minT || maxT < minB) {
				return false;
			}
		}
	}
	return true;
}

#endif // FACE3_H
