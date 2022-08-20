/*************************************************************************/
/*  delaunay_3d.h                                                        */
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

#ifndef DELAUNAY_3D_H
#define DELAUNAY_3D_H

#include "core/io/file_access.h"
#include "core/math/aabb.h"
#include "core/math/projection.h"
#include "core/math/vector3.h"
#include "core/string/print_string.h"
#include "core/templates/local_vector.h"
#include "core/templates/oa_hash_map.h"
#include "core/templates/vector.h"
#include "core/variant/variant.h"

#include "thirdparty/misc/r128.h"

class Delaunay3D {
	struct Simplex;

	enum {
		ACCEL_GRID_SIZE = 16
	};
	struct GridPos {
		Vector3i pos;
		List<Simplex *>::Element *E = nullptr;
	};

	struct Simplex {
		uint32_t points[4];
		R128 circum_center_x;
		R128 circum_center_y;
		R128 circum_center_z;
		R128 circum_r2;
		LocalVector<GridPos> grid_positions;
		List<Simplex *>::Element *SE = nullptr;

		_FORCE_INLINE_ Simplex() {}
		_FORCE_INLINE_ Simplex(uint32_t p_a, uint32_t p_b, uint32_t p_c, uint32_t p_d) {
			points[0] = p_a;
			points[1] = p_b;
			points[2] = p_c;
			points[3] = p_d;
		}
	};

	struct Triangle {
		uint32_t triangle[3];
		bool bad = false;
		_FORCE_INLINE_ bool operator==(const Triangle &p_triangle) const {
			return triangle[0] == p_triangle.triangle[0] && triangle[1] == p_triangle.triangle[1] && triangle[2] == p_triangle.triangle[2];
		}

		_FORCE_INLINE_ Triangle() {}
		_FORCE_INLINE_ Triangle(uint32_t p_a, uint32_t p_b, uint32_t p_c) {
			if (p_a > p_b) {
				SWAP(p_a, p_b);
			}
			if (p_b > p_c) {
				SWAP(p_b, p_c);
			}
			if (p_a > p_b) {
				SWAP(p_a, p_b);
			}

			triangle[0] = p_a;
			triangle[1] = p_b;
			triangle[2] = p_c;
		}
	};

	struct TriangleHasher {
		_FORCE_INLINE_ static uint32_t hash(const Triangle &p_triangle) {
			uint32_t h = hash_djb2_one_32(p_triangle.triangle[0]);
			h = hash_djb2_one_32(p_triangle.triangle[1], h);
			return hash_fmix32(hash_djb2_one_32(p_triangle.triangle[2], h));
		}
	};

	_FORCE_INLINE_ static void circum_sphere_compute(const Vector3 *p_points, Simplex *p_simplex) {
		// the only part in the algorithm where there may be precision errors is this one, so ensure that
		// we do it as maximum precision as possible

		R128 v0_x = p_points[p_simplex->points[0]].x;
		R128 v0_y = p_points[p_simplex->points[0]].y;
		R128 v0_z = p_points[p_simplex->points[0]].z;
		R128 v1_x = p_points[p_simplex->points[1]].x;
		R128 v1_y = p_points[p_simplex->points[1]].y;
		R128 v1_z = p_points[p_simplex->points[1]].z;
		R128 v2_x = p_points[p_simplex->points[2]].x;
		R128 v2_y = p_points[p_simplex->points[2]].y;
		R128 v2_z = p_points[p_simplex->points[2]].z;
		R128 v3_x = p_points[p_simplex->points[3]].x;
		R128 v3_y = p_points[p_simplex->points[3]].y;
		R128 v3_z = p_points[p_simplex->points[3]].z;

		//Create the rows of our "unrolled" 3x3 matrix
		R128 row1_x = v1_x - v0_x;
		R128 row1_y = v1_y - v0_y;
		R128 row1_z = v1_z - v0_z;

		R128 row2_x = v2_x - v0_x;
		R128 row2_y = v2_y - v0_y;
		R128 row2_z = v2_z - v0_z;

		R128 row3_x = v3_x - v0_x;
		R128 row3_y = v3_y - v0_y;
		R128 row3_z = v3_z - v0_z;

		R128 sq_lenght1 = row1_x * row1_x + row1_y * row1_y + row1_z * row1_z;
		R128 sq_lenght2 = row2_x * row2_x + row2_y * row2_y + row2_z * row2_z;
		R128 sq_lenght3 = row3_x * row3_x + row3_y * row3_y + row3_z * row3_z;

		//Compute the determinant of said matrix
		R128 determinant = row1_x * (row2_y * row3_z - row3_y * row2_z) - row2_x * (row1_y * row3_z - row3_y * row1_z) + row3_x * (row1_y * row2_z - row2_y * row1_z);

		// Compute the volume of the tetrahedron, and precompute a scalar quantity for re-use in the formula
		R128 volume = determinant / R128(6.f);
		R128 i12volume = R128(1.f) / (volume * R128(12.f));

		R128 center_x = v0_x + i12volume * ((row2_y * row3_z - row3_y * row2_z) * sq_lenght1 - (row1_y * row3_z - row3_y * row1_z) * sq_lenght2 + (row1_y * row2_z - row2_y * row1_z) * sq_lenght3);
		R128 center_y = v0_y + i12volume * (-(row2_x * row3_z - row3_x * row2_z) * sq_lenght1 + (row1_x * row3_z - row3_x * row1_z) * sq_lenght2 - (row1_x * row2_z - row2_x * row1_z) * sq_lenght3);
		R128 center_z = v0_z + i12volume * ((row2_x * row3_y - row3_x * row2_y) * sq_lenght1 - (row1_x * row3_y - row3_x * row1_y) * sq_lenght2 + (row1_x * row2_y - row2_x * row1_y) * sq_lenght3);

		//Once we know the center, the radius is clearly the distance to any vertex

		R128 rel1_x = center_x - v0_x;
		R128 rel1_y = center_y - v0_y;
		R128 rel1_z = center_z - v0_z;

		R128 radius1 = rel1_x * rel1_x + rel1_y * rel1_y + rel1_z * rel1_z;

		p_simplex->circum_center_x = center_x;
		p_simplex->circum_center_y = center_y;
		p_simplex->circum_center_z = center_z;
		p_simplex->circum_r2 = radius1;
	}

	_FORCE_INLINE_ static bool simplex_contains(const Vector3 *p_points, const Simplex &p_simplex, uint32_t p_vertex) {
		R128 v_x = p_points[p_vertex].x;
		R128 v_y = p_points[p_vertex].y;
		R128 v_z = p_points[p_vertex].z;

		R128 rel2_x = p_simplex.circum_center_x - v_x;
		R128 rel2_y = p_simplex.circum_center_y - v_y;
		R128 rel2_z = p_simplex.circum_center_z - v_z;

		R128 radius2 = rel2_x * rel2_x + rel2_y * rel2_y + rel2_z * rel2_z;

		return radius2 < (p_simplex.circum_r2 - R128(0.00001));
	}

	static bool simplex_is_coplanar(const Vector3 *p_points, const Simplex &p_simplex) {
		Plane p(p_points[p_simplex.points[0]], p_points[p_simplex.points[1]], p_points[p_simplex.points[2]]);
		if (ABS(p.distance_to(p_points[p_simplex.points[3]])) < CMP_EPSILON) {
			return true;
		}

		Projection cm;

		cm.matrix[0][0] = p_points[p_simplex.points[0]].x;
		cm.matrix[0][1] = p_points[p_simplex.points[1]].x;
		cm.matrix[0][2] = p_points[p_simplex.points[2]].x;
		cm.matrix[0][3] = p_points[p_simplex.points[3]].x;

		cm.matrix[1][0] = p_points[p_simplex.points[0]].y;
		cm.matrix[1][1] = p_points[p_simplex.points[1]].y;
		cm.matrix[1][2] = p_points[p_simplex.points[2]].y;
		cm.matrix[1][3] = p_points[p_simplex.points[3]].y;

		cm.matrix[2][0] = p_points[p_simplex.points[0]].z;
		cm.matrix[2][1] = p_points[p_simplex.points[1]].z;
		cm.matrix[2][2] = p_points[p_simplex.points[2]].z;
		cm.matrix[2][3] = p_points[p_simplex.points[3]].z;

		cm.matrix[3][0] = 1.0;
		cm.matrix[3][1] = 1.0;
		cm.matrix[3][2] = 1.0;
		cm.matrix[3][3] = 1.0;

		return ABS(cm.determinant()) <= CMP_EPSILON;
	}

public:
	struct OutputSimplex {
		uint32_t points[4];
	};

	static Vector<OutputSimplex> tetrahedralize(const Vector<Vector3> &p_points) {
		uint32_t point_count = p_points.size();
		Vector3 *points = (Vector3 *)memalloc(sizeof(Vector3) * (point_count + 4));

		{
			const Vector3 *src_points = p_points.ptr();
			AABB rect;
			for (uint32_t i = 0; i < point_count; i++) {
				Vector3 point = src_points[i];
				if (i == 0) {
					rect.position = point;
				} else {
					rect.expand_to(point);
				}
				points[i] = point;
			}

			for (uint32_t i = 0; i < point_count; i++) {
				points[i] = (points[i] - rect.position) / rect.size;
			}

			float delta_max = Math::sqrt(2.0) * 20.0;
			Vector3 center = Vector3(0.5, 0.5, 0.5);

			// any simplex that contains everything is good
			points[point_count + 0] = center + Vector3(0, 1, 0) * delta_max;
			points[point_count + 1] = center + Vector3(0, -1, 1) * delta_max;
			points[point_count + 2] = center + Vector3(1, -1, -1) * delta_max;
			points[point_count + 3] = center + Vector3(-1, -1, -1) * delta_max;
		}

		List<Simplex *> acceleration_grid[ACCEL_GRID_SIZE][ACCEL_GRID_SIZE][ACCEL_GRID_SIZE];

		List<Simplex *> simplex_list;
		{
			//create root simplex
			Simplex *root = memnew(Simplex(point_count + 0, point_count + 1, point_count + 2, point_count + 3));
			root->SE = simplex_list.push_back(root);

			for (uint32_t i = 0; i < ACCEL_GRID_SIZE; i++) {
				for (uint32_t j = 0; j < ACCEL_GRID_SIZE; j++) {
					for (uint32_t k = 0; k < ACCEL_GRID_SIZE; k++) {
						GridPos gp;
						gp.E = acceleration_grid[i][j][k].push_back(root);
						gp.pos = Vector3i(i, j, k);
						root->grid_positions.push_back(gp);
					}
				}
			}

			circum_sphere_compute(points, root);
		}

		OAHashMap<Triangle, uint32_t, TriangleHasher> triangles_inserted;
		LocalVector<Triangle> triangles;

		for (uint32_t i = 0; i < point_count; i++) {
			bool unique = true;
			for (uint32_t j = i + 1; j < point_count; j++) {
				if (points[i].is_equal_approx(points[j])) {
					unique = false;
					break;
				}
			}
			if (!unique) {
				continue;
			}

			Vector3i grid_pos = Vector3i(points[i] * ACCEL_GRID_SIZE);
			grid_pos.x = CLAMP(grid_pos.x, 0, ACCEL_GRID_SIZE - 1);
			grid_pos.y = CLAMP(grid_pos.y, 0, ACCEL_GRID_SIZE - 1);
			grid_pos.z = CLAMP(grid_pos.z, 0, ACCEL_GRID_SIZE - 1);

			for (List<Simplex *>::Element *E = acceleration_grid[grid_pos.x][grid_pos.y][grid_pos.z].front(); E;) {
				List<Simplex *>::Element *N = E->next(); //may be deleted

				Simplex *simplex = E->get();

				if (simplex_contains(points, *simplex, i)) {
					static const uint32_t triangle_order[4][3] = {
						{ 0, 1, 2 },
						{ 0, 1, 3 },
						{ 0, 2, 3 },
						{ 1, 2, 3 },
					};

					for (uint32_t k = 0; k < 4; k++) {
						Triangle t = Triangle(simplex->points[triangle_order[k][0]], simplex->points[triangle_order[k][1]], simplex->points[triangle_order[k][2]]);
						uint32_t *p = triangles_inserted.lookup_ptr(t);
						if (p) {
							triangles[*p].bad = true;
						} else {
							triangles_inserted.insert(t, triangles.size());
							triangles.push_back(t);
						}
					}

					//remove simplex and continue
					simplex_list.erase(simplex->SE);

					for (uint32_t k = 0; k < simplex->grid_positions.size(); k++) {
						Vector3i p = simplex->grid_positions[k].pos;
						acceleration_grid[p.x][p.y][p.z].erase(simplex->grid_positions[k].E);
					}
					memdelete(simplex);
				}
				E = N;
			}

			for (uint32_t j = 0; j < triangles.size(); j++) {
				if (triangles[j].bad) {
					continue;
				}
				Simplex *new_simplex = memnew(Simplex(triangles[j].triangle[0], triangles[j].triangle[1], triangles[j].triangle[2], i));
				circum_sphere_compute(points, new_simplex);
				new_simplex->SE = simplex_list.push_back(new_simplex);
				{
					Vector3 center;
					center.x = double(new_simplex->circum_center_x);
					center.y = double(new_simplex->circum_center_y);
					center.z = double(new_simplex->circum_center_z);

					float radius2 = Math::sqrt(double(new_simplex->circum_r2));
					radius2 += 0.0001; //
					Vector3 extents = Vector3(radius2, radius2, radius2);
					Vector3i from = Vector3i((center - extents) * ACCEL_GRID_SIZE);
					Vector3i to = Vector3i((center + extents) * ACCEL_GRID_SIZE);
					from.x = CLAMP(from.x, 0, ACCEL_GRID_SIZE - 1);
					from.y = CLAMP(from.y, 0, ACCEL_GRID_SIZE - 1);
					from.z = CLAMP(from.z, 0, ACCEL_GRID_SIZE - 1);
					to.x = CLAMP(to.x, 0, ACCEL_GRID_SIZE - 1);
					to.y = CLAMP(to.y, 0, ACCEL_GRID_SIZE - 1);
					to.z = CLAMP(to.z, 0, ACCEL_GRID_SIZE - 1);

					for (int32_t x = from.x; x <= to.x; x++) {
						for (int32_t y = from.y; y <= to.y; y++) {
							for (int32_t z = from.z; z <= to.z; z++) {
								GridPos gp;
								gp.pos = Vector3(x, y, z);
								gp.E = acceleration_grid[x][y][z].push_back(new_simplex);
								new_simplex->grid_positions.push_back(gp);
							}
						}
					}
				}
			}

			triangles.clear();
			triangles_inserted.clear();
		}

		//print_line("end with simplices: " + itos(simplex_list.size()));
		Vector<OutputSimplex> ret_simplices;
		ret_simplices.resize(simplex_list.size());
		OutputSimplex *ret_simplicesw = ret_simplices.ptrw();
		uint32_t simplices_written = 0;

		for (Simplex *simplex : simplex_list) {
			bool invalid = false;
			for (int j = 0; j < 4; j++) {
				if (simplex->points[j] >= point_count) {
					invalid = true;
					break;
				}
			}
			if (invalid || simplex_is_coplanar(points, *simplex)) {
				memdelete(simplex);
				continue;
			}

			ret_simplicesw[simplices_written].points[0] = simplex->points[0];
			ret_simplicesw[simplices_written].points[1] = simplex->points[1];
			ret_simplicesw[simplices_written].points[2] = simplex->points[2];
			ret_simplicesw[simplices_written].points[3] = simplex->points[3];
			simplices_written++;
			memdelete(simplex);
		}

		ret_simplices.resize(simplices_written);

		memfree(points);

		return ret_simplices;
	}
};

#endif // DELAUNAY_3D_H
