/**************************************************************************/
/*  a_star_grid_2d.cpp                                                    */
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

#include "a_star_grid_2d.h"

#include "core/variant/typed_array.h"

static real_t heuristic_euclidian(const Vector2i &p_from, const Vector2i &p_to) {
	real_t dx = (real_t)ABS(p_to.x - p_from.x);
	real_t dy = (real_t)ABS(p_to.y - p_from.y);
	return (real_t)Math::sqrt(dx * dx + dy * dy);
}

static real_t heuristic_manhattan(const Vector2i &p_from, const Vector2i &p_to) {
	real_t dx = (real_t)ABS(p_to.x - p_from.x);
	real_t dy = (real_t)ABS(p_to.y - p_from.y);
	return dx + dy;
}

static real_t heuristic_octile(const Vector2i &p_from, const Vector2i &p_to) {
	real_t dx = (real_t)ABS(p_to.x - p_from.x);
	real_t dy = (real_t)ABS(p_to.y - p_from.y);
	real_t F = Math_SQRT2 - 1;
	return (dx < dy) ? F * dx + dy : F * dy + dx;
}

static real_t heuristic_chebyshev(const Vector2i &p_from, const Vector2i &p_to) {
	real_t dx = (real_t)ABS(p_to.x - p_from.x);
	real_t dy = (real_t)ABS(p_to.y - p_from.y);
	return MAX(dx, dy);
}

static real_t (*heuristics[AStarGrid2D::HEURISTIC_MAX])(const Vector2i &, const Vector2i &) = { heuristic_euclidian, heuristic_manhattan, heuristic_octile, heuristic_chebyshev };

void AStarGrid2D::set_size(const Size2i &p_size) {
	ERR_FAIL_COND(p_size.x < 0 || p_size.y < 0);
	if (p_size != size) {
		size = p_size;
		dirty = true;
	}
}

Size2i AStarGrid2D::get_size() const {
	return size;
}

void AStarGrid2D::set_offset(const Vector2 &p_offset) {
	if (!offset.is_equal_approx(p_offset)) {
		offset = p_offset;
		dirty = true;
	}
}

Vector2 AStarGrid2D::get_offset() const {
	return offset;
}

void AStarGrid2D::set_cell_size(const Size2 &p_cell_size) {
	if (!cell_size.is_equal_approx(p_cell_size)) {
		cell_size = p_cell_size;
		dirty = true;
	}
}

Size2 AStarGrid2D::get_cell_size() const {
	return cell_size;
}

void AStarGrid2D::update() {
	points.clear();
	for (int64_t y = 0; y < size.y; y++) {
		LocalVector<Point> line;
		for (int64_t x = 0; x < size.x; x++) {
			line.push_back(Point(Vector2i(x, y), offset + Vector2(x, y) * cell_size));
		}
		points.push_back(line);
	}
	dirty = false;
}

bool AStarGrid2D::is_dirty() const {
	return dirty;
}

void AStarGrid2D::set_jumping_enabled(bool p_enabled) {
	jumping_enabled = p_enabled;
}

bool AStarGrid2D::is_jumping_enabled() const {
	return jumping_enabled;
}

void AStarGrid2D::set_diagonal_mode(DiagonalMode p_diagonal_mode) {
	ERR_FAIL_INDEX(p_diagonal_mode, DIAGONAL_MODE_MAX);
	diagonal_mode = p_diagonal_mode;
}

AStarGrid2D::DiagonalMode AStarGrid2D::get_diagonal_mode() const {
	return diagonal_mode;
}

void AStarGrid2D::set_default_compute_heuristic(Heuristic p_heuristic) {
	ERR_FAIL_INDEX(p_heuristic, HEURISTIC_MAX);
	default_compute_heuristic = p_heuristic;
}

AStarGrid2D::Heuristic AStarGrid2D::get_default_compute_heuristic() const {
	return default_compute_heuristic;
}

void AStarGrid2D::set_default_estimate_heuristic(Heuristic p_heuristic) {
	ERR_FAIL_INDEX(p_heuristic, HEURISTIC_MAX);
	default_estimate_heuristic = p_heuristic;
}

AStarGrid2D::Heuristic AStarGrid2D::get_default_estimate_heuristic() const {
	return default_estimate_heuristic;
}

void AStarGrid2D::set_point_solid(const Vector2i &p_id, bool p_solid) {
	ERR_FAIL_COND_MSG(dirty, "Grid is not initialized. Call the update method.");
	ERR_FAIL_COND_MSG(!is_in_boundsv(p_id), vformat("Can't set if point is disabled. Point out of bounds (%s/%s, %s/%s).", p_id.x, size.width, p_id.y, size.height));
	points[p_id.y][p_id.x].solid = p_solid;
}

bool AStarGrid2D::is_point_solid(const Vector2i &p_id) const {
	ERR_FAIL_COND_V_MSG(dirty, false, "Grid is not initialized. Call the update method.");
	ERR_FAIL_COND_V_MSG(!is_in_boundsv(p_id), false, vformat("Can't get if point is disabled. Point out of bounds (%s/%s, %s/%s).", p_id.x, size.width, p_id.y, size.height));
	return points[p_id.y][p_id.x].solid;
}

void AStarGrid2D::set_point_weight_scale(const Vector2i &p_id, real_t p_weight_scale) {
	ERR_FAIL_COND_MSG(dirty, "Grid is not initialized. Call the update method.");
	ERR_FAIL_COND_MSG(!is_in_boundsv(p_id), vformat("Can't set point's weight scale. Point out of bounds (%s/%s, %s/%s).", p_id.x, size.width, p_id.y, size.height));
	ERR_FAIL_COND_MSG(p_weight_scale < 0.0, vformat("Can't set point's weight scale less than 0.0: %f.", p_weight_scale));
	points[p_id.y][p_id.x].weight_scale = p_weight_scale;
}

real_t AStarGrid2D::get_point_weight_scale(const Vector2i &p_id) const {
	ERR_FAIL_COND_V_MSG(dirty, 0, "Grid is not initialized. Call the update method.");
	ERR_FAIL_COND_V_MSG(!is_in_boundsv(p_id), 0, vformat("Can't get point's weight scale. Point out of bounds (%s/%s, %s/%s).", p_id.x, size.width, p_id.y, size.height));
	return points[p_id.y][p_id.x].weight_scale;
}

const AStarGrid2D::Point *AStarGrid2D::_jump(const Point *p_from, const Point *p_to, const Point *p_end) const {
	if (!p_to || p_to->solid) {
		return nullptr;
	}
	if (p_to == p_end) {
		return p_to;
	}

	int64_t from_x = p_from->id.x;
	int64_t from_y = p_from->id.y;

	int64_t to_x = p_to->id.x;
	int64_t to_y = p_to->id.y;

	int64_t dx = to_x - from_x;
	int64_t dy = to_y - from_y;

	if (diagonal_mode == DIAGONAL_MODE_ALWAYS || diagonal_mode == DIAGONAL_MODE_AT_LEAST_ONE_WALKABLE) {
		if (dx != 0 && dy != 0) {
			if ((_is_walkable(to_x - dx, to_y + dy) && !_is_walkable(to_x - dx, to_y)) || (_is_walkable(to_x + dx, to_y - dy) && !_is_walkable(to_x, to_y - dy))) {
				return p_to;
			}
			if (_jump(p_to, _get_point(to_x + dx, to_y), p_end) != nullptr) {
				return p_to;
			}
			if (_jump(p_to, _get_point(to_x, to_y + dy), p_end) != nullptr) {
				return p_to;
			}
		} else {
			if (dx != 0) {
				if ((_is_walkable(to_x + dx, to_y + 1) && !_is_walkable(to_x, to_y + 1)) || (_is_walkable(to_x + dx, to_y - 1) && !_is_walkable(to_x, to_y - 1))) {
					return p_to;
				}
			} else {
				if ((_is_walkable(to_x + 1, to_y + dy) && !_is_walkable(to_x + 1, to_y)) || (_is_walkable(to_x - 1, to_y + dy) && !_is_walkable(to_x - 1, to_y))) {
					return p_to;
				}
			}
		}
		if (_is_walkable(to_x + dx, to_y + dy) && (diagonal_mode == DIAGONAL_MODE_ALWAYS || (_is_walkable(to_x + dx, to_y) || _is_walkable(to_x, to_y + dy)))) {
			return _jump(p_to, _get_point(to_x + dx, to_y + dy), p_end);
		}
	} else if (diagonal_mode == DIAGONAL_MODE_ONLY_IF_NO_OBSTACLES) {
		if (dx != 0 && dy != 0) {
			if ((_is_walkable(to_x + dx, to_y + dy) && !_is_walkable(to_x, to_y + dy)) || !_is_walkable(to_x + dx, to_y)) {
				return p_to;
			}
			if (_jump(p_to, _get_point(to_x + dx, to_y), p_end) != nullptr) {
				return p_to;
			}
			if (_jump(p_to, _get_point(to_x, to_y + dy), p_end) != nullptr) {
				return p_to;
			}
		} else {
			if (dx != 0) {
				if ((_is_walkable(to_x, to_y + 1) && !_is_walkable(to_x - dx, to_y + 1)) || (_is_walkable(to_x, to_y - 1) && !_is_walkable(to_x - dx, to_y - 1))) {
					return p_to;
				}
			} else {
				if ((_is_walkable(to_x + 1, to_y) && !_is_walkable(to_x + 1, to_y - dy)) || (_is_walkable(to_x - 1, to_y) && !_is_walkable(to_x - 1, to_y - dy))) {
					return p_to;
				}
			}
		}
		if (_is_walkable(to_x + dx, to_y + dy) && _is_walkable(to_x + dx, to_y) && _is_walkable(to_x, to_y + dy)) {
			return _jump(p_to, _get_point(to_x + dx, to_y + dy), p_end);
		}
	} else { // DIAGONAL_MODE_NEVER
		if (dx != 0) {
			if ((_is_walkable(to_x, to_y - 1) && !_is_walkable(to_x - dx, to_y - 1)) || (_is_walkable(to_x, to_y + 1) && !_is_walkable(to_x - dx, to_y + 1))) {
				return p_to;
			}
		} else if (dy != 0) {
			if ((_is_walkable(to_x - 1, to_y) && !_is_walkable(to_x - 1, to_y - dy)) || (_is_walkable(to_x + 1, to_y) && !_is_walkable(to_x + 1, to_y - dy))) {
				return p_to;
			}
			if (_jump(p_to, _get_point(to_x + 1, to_y), p_end) != nullptr) {
				return p_to;
			}
			if (_jump(p_to, _get_point(to_x - 1, to_y), p_end) != nullptr) {
				return p_to;
			}
		}
		return _jump(p_to, _get_point(to_x + dx, to_y + dy), p_end);
	}
	return nullptr;
}

void AStarGrid2D::_get_nbors(const Point *p_point, LocalVector<const Point *> &r_nbors) const {
	bool ts0 = false, td0 = false,
		 ts1 = false, td1 = false,
		 ts2 = false, td2 = false,
		 ts3 = false, td3 = false;

	const Point *left = nullptr;
	const Point *right = nullptr;
	const Point *top = nullptr;
	const Point *bottom = nullptr;

	const Point *top_left = nullptr;
	const Point *top_right = nullptr;
	const Point *bottom_left = nullptr;
	const Point *bottom_right = nullptr;

	{
		bool has_left = false;
		bool has_right = false;

		if (p_point->id.x - 1 >= 0) {
			left = _get_point_unchecked(p_point->id.x - 1, p_point->id.y);
			has_left = true;
		}
		if (p_point->id.x + 1 < size.width) {
			right = _get_point_unchecked(p_point->id.x + 1, p_point->id.y);
			has_right = true;
		}
		if (p_point->id.y - 1 >= 0) {
			top = _get_point_unchecked(p_point->id.x, p_point->id.y - 1);
			if (has_left) {
				top_left = _get_point_unchecked(p_point->id.x - 1, p_point->id.y - 1);
			}
			if (has_right) {
				top_right = _get_point_unchecked(p_point->id.x + 1, p_point->id.y - 1);
			}
		}
		if (p_point->id.y + 1 < size.height) {
			bottom = _get_point_unchecked(p_point->id.x, p_point->id.y + 1);
			if (has_left) {
				bottom_left = _get_point_unchecked(p_point->id.x - 1, p_point->id.y + 1);
			}
			if (has_right) {
				bottom_right = _get_point_unchecked(p_point->id.x + 1, p_point->id.y + 1);
			}
		}
	}

	if (top && !top->solid) {
		r_nbors.push_back(top);
		ts0 = true;
	}
	if (right && !right->solid) {
		r_nbors.push_back(right);
		ts1 = true;
	}
	if (bottom && !bottom->solid) {
		r_nbors.push_back(bottom);
		ts2 = true;
	}
	if (left && !left->solid) {
		r_nbors.push_back(left);
		ts3 = true;
	}

	switch (diagonal_mode) {
		case DIAGONAL_MODE_ALWAYS: {
			td0 = true;
			td1 = true;
			td2 = true;
			td3 = true;
		} break;
		case DIAGONAL_MODE_NEVER: {
		} break;
		case DIAGONAL_MODE_AT_LEAST_ONE_WALKABLE: {
			td0 = ts3 || ts0;
			td1 = ts0 || ts1;
			td2 = ts1 || ts2;
			td3 = ts2 || ts3;
		} break;
		case DIAGONAL_MODE_ONLY_IF_NO_OBSTACLES: {
			td0 = ts3 && ts0;
			td1 = ts0 && ts1;
			td2 = ts1 && ts2;
			td3 = ts2 && ts3;
		} break;
		default:
			break;
	}

	if (td0 && (top_left && !top_left->solid)) {
		r_nbors.push_back(top_left);
	}
	if (td1 && (top_right && !top_right->solid)) {
		r_nbors.push_back(top_right);
	}
	if (td2 && (bottom_right && !bottom_right->solid)) {
		r_nbors.push_back(bottom_right);
	}
	if (td3 && (bottom_left && !bottom_left->solid)) {
		r_nbors.push_back(bottom_left);
	}
}

bool AStarGrid2D::_solve(const Point *p_begin_point, const Point *p_end_point, List<const Point *> &r_path) const {
	if (p_end_point->solid) {
		return false;
	}

	LocalVector<Pass *> open_list;
	SortArray<Pass *, SortPasses> sorter;

	RBMap<Vector2i, Pass> data;
	open_list.push_back(&data.insert(p_begin_point->id, Pass(p_begin_point, _estimate_cost(p_begin_point->id, p_end_point->id)))->get());

	while (!open_list.is_empty()) {
		Pass *p_pass = open_list[0];
		const Point *p = p_pass->point; // The currently processed point.

		if (p == p_end_point) {
			// Create a path and return.
			while (p != p_begin_point) {
				r_path.push_front(p);
				p = p_pass->prev_point;
				p_pass = &data[p->id];
			}
			r_path.push_front(p_begin_point);
			return true;
		}

		sorter.pop_heap(0, open_list.size(), open_list.ptr()); // Remove the current point from the open list.
		open_list.remove_at(open_list.size() - 1);
		p_pass->is_closed = true; // Mark the point as closed.

		LocalVector<const Point *> nbors;
		_get_nbors(p, nbors);

		for (const Point *e : nbors) {
			Pass *e_pass = nullptr;
			real_t weight_scale = 1.0;
			bool new_point = false;

			if (jumping_enabled) {
				// TODO: Make it works with weight_scale.
				e = _jump(p, e, p_end_point);
				if (!e) {
					continue;
				}
			} else {
				if (e->solid) {
					continue;
				}
				weight_scale = e->weight_scale;
			}

			if (data.has(e->id)) {
				e_pass = &data[e->id];
				if (e_pass->is_closed) {
					continue;
				}
			} else {
				e_pass = &data.insert(e->id, Pass(e))->get();
				new_point = true;
			}

			real_t tentative_g_score = p_pass->g_score + _compute_cost(p->id, e->id) * weight_scale;

			if (new_point) { // The point wasn't inside the open list.
				open_list.push_back(e_pass);
			} else if (tentative_g_score >= e_pass->g_score) { // The new path is worse than the previous.
				continue;
			}

			e_pass->prev_point = p;
			e_pass->g_score = tentative_g_score;
			e_pass->f_score = e_pass->g_score + _estimate_cost(e->id, p_end_point->id);

			if (new_point) { // The position of the new points is already known.
				sorter.push_heap(0, open_list.size() - 1, 0, e_pass, open_list.ptr());
			} else {
				sorter.push_heap(0, open_list.find(e_pass), 0, e_pass, open_list.ptr());
			}
		}
	}

	return false;
}

real_t AStarGrid2D::_estimate_cost(const Vector2i &p_from_id, const Vector2i &p_to_id) const {
	real_t scost;
	if (GDVIRTUAL_CALL(_estimate_cost, p_from_id, p_to_id, scost)) {
		return scost;
	}
	return heuristics[default_estimate_heuristic](p_from_id, p_to_id);
}

real_t AStarGrid2D::_compute_cost(const Vector2i &p_from_id, const Vector2i &p_to_id) const {
	real_t scost;
	if (GDVIRTUAL_CALL(_compute_cost, p_from_id, p_to_id, scost)) {
		return scost;
	}
	return heuristics[default_compute_heuristic](p_from_id, p_to_id);
}

void AStarGrid2D::clear() {
	points.clear();
	size = Vector2i();
}

Vector2 AStarGrid2D::get_point_position(const Vector2i &p_id) const {
	ERR_FAIL_COND_V_MSG(dirty, Vector2(), "Grid is not initialized. Call the update method.");
	ERR_FAIL_COND_V_MSG(!is_in_boundsv(p_id), Vector2(), vformat("Can't get point's position. Point out of bounds (%s/%s, %s/%s).", p_id.x, size.width, p_id.y, size.height));
	return points[p_id.y][p_id.x].pos;
}

Vector<Vector2> AStarGrid2D::get_point_path(const Vector2i &p_from_id, const Vector2i &p_to_id) const {
	ERR_FAIL_COND_V_MSG(dirty, Vector<Vector2>(), "Grid is not initialized. Call the update method.");
	ERR_FAIL_COND_V_MSG(!is_in_boundsv(p_from_id), Vector<Vector2>(), vformat("Can't get id path. Point out of bounds (%s/%s, %s/%s)", p_from_id.x, size.width, p_from_id.y, size.height));
	ERR_FAIL_COND_V_MSG(!is_in_boundsv(p_to_id), Vector<Vector2>(), vformat("Can't get id path. Point out of bounds (%s/%s, %s/%s)", p_to_id.x, size.width, p_to_id.y, size.height));

	const Point *begin_point = _get_point_unchecked(p_from_id.x, p_from_id.y);
	const Point *end_point = _get_point_unchecked(p_to_id.x, p_to_id.y);

	if (begin_point == end_point) {
		return { begin_point->pos };
	}

	List<const Point *> point_path;
	if (!_solve(begin_point, end_point, point_path)) {
		return Vector<Vector2>();
	}

	Vector<Vector2> path;
	path.resize(point_path.size());
	Vector2 *w = path.ptrw();

	int64_t idx = 0;
	for (const Point *p : point_path) {
		w[idx++] = p->pos;
	}

	return path;
}

TypedArray<Vector2i> AStarGrid2D::get_id_path(const Vector2i &p_from_id, const Vector2i &p_to_id) const {
	ERR_FAIL_COND_V_MSG(dirty, TypedArray<Vector2i>(), "Grid is not initialized. Call the update method.");
	ERR_FAIL_COND_V_MSG(!is_in_boundsv(p_from_id), TypedArray<Vector2i>(), vformat("Can't get id path. Point out of bounds (%s/%s, %s/%s)", p_from_id.x, size.width, p_from_id.y, size.height));
	ERR_FAIL_COND_V_MSG(!is_in_boundsv(p_to_id), TypedArray<Vector2i>(), vformat("Can't get id path. Point out of bounds (%s/%s, %s/%s)", p_to_id.x, size.width, p_to_id.y, size.height));

	const Point *begin_point = _get_point_unchecked(p_from_id.x, p_from_id.y);
	const Point *end_point = _get_point_unchecked(p_to_id.x, p_to_id.y);

	if (begin_point == end_point) {
		return { begin_point->id };
	}

	List<const Point *> point_path;
	if (!_solve(begin_point, end_point, point_path)) {
		return TypedArray<Vector2i>();
	}

	TypedArray<Vector2i> path;
	path.resize(point_path.size());

	int64_t idx = 0;
	for (const Point *p : point_path) {
		path[idx++] = p->id;
	}

	return path;
}

void AStarGrid2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_size", "size"), &AStarGrid2D::set_size);
	ClassDB::bind_method(D_METHOD("get_size"), &AStarGrid2D::get_size);
	ClassDB::bind_method(D_METHOD("set_offset", "offset"), &AStarGrid2D::set_offset);
	ClassDB::bind_method(D_METHOD("get_offset"), &AStarGrid2D::get_offset);
	ClassDB::bind_method(D_METHOD("set_cell_size", "cell_size"), &AStarGrid2D::set_cell_size);
	ClassDB::bind_method(D_METHOD("get_cell_size"), &AStarGrid2D::get_cell_size);
	ClassDB::bind_method(D_METHOD("is_in_bounds", "x", "y"), &AStarGrid2D::is_in_bounds);
	ClassDB::bind_method(D_METHOD("is_in_boundsv", "id"), &AStarGrid2D::is_in_boundsv);
	ClassDB::bind_method(D_METHOD("is_dirty"), &AStarGrid2D::is_dirty);
	ClassDB::bind_method(D_METHOD("update"), &AStarGrid2D::update);
	ClassDB::bind_method(D_METHOD("set_jumping_enabled", "enabled"), &AStarGrid2D::set_jumping_enabled);
	ClassDB::bind_method(D_METHOD("is_jumping_enabled"), &AStarGrid2D::is_jumping_enabled);
	ClassDB::bind_method(D_METHOD("set_diagonal_mode", "mode"), &AStarGrid2D::set_diagonal_mode);
	ClassDB::bind_method(D_METHOD("get_diagonal_mode"), &AStarGrid2D::get_diagonal_mode);
	ClassDB::bind_method(D_METHOD("set_default_compute_heuristic", "heuristic"), &AStarGrid2D::set_default_compute_heuristic);
	ClassDB::bind_method(D_METHOD("get_default_compute_heuristic"), &AStarGrid2D::get_default_compute_heuristic);
	ClassDB::bind_method(D_METHOD("set_default_estimate_heuristic", "heuristic"), &AStarGrid2D::set_default_estimate_heuristic);
	ClassDB::bind_method(D_METHOD("get_default_estimate_heuristic"), &AStarGrid2D::get_default_estimate_heuristic);
	ClassDB::bind_method(D_METHOD("set_point_solid", "id", "solid"), &AStarGrid2D::set_point_solid, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("is_point_solid", "id"), &AStarGrid2D::is_point_solid);
	ClassDB::bind_method(D_METHOD("set_point_weight_scale", "id", "weight_scale"), &AStarGrid2D::set_point_weight_scale);
	ClassDB::bind_method(D_METHOD("get_point_weight_scale", "id"), &AStarGrid2D::get_point_weight_scale);
	ClassDB::bind_method(D_METHOD("clear"), &AStarGrid2D::clear);

	ClassDB::bind_method(D_METHOD("get_point_position", "id"), &AStarGrid2D::get_point_position);
	ClassDB::bind_method(D_METHOD("get_point_path", "from_id", "to_id"), &AStarGrid2D::get_point_path);
	ClassDB::bind_method(D_METHOD("get_id_path", "from_id", "to_id"), &AStarGrid2D::get_id_path);

	GDVIRTUAL_BIND(_estimate_cost, "from_id", "to_id")
	GDVIRTUAL_BIND(_compute_cost, "from_id", "to_id")

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "size"), "set_size", "get_size");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "offset"), "set_offset", "get_offset");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "cell_size"), "set_cell_size", "get_cell_size");

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "jumping_enabled"), "set_jumping_enabled", "is_jumping_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "default_compute_heuristic", PROPERTY_HINT_ENUM, "Euclidean,Manhattan,Octile,Chebyshev"), "set_default_compute_heuristic", "get_default_compute_heuristic");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "default_estimate_heuristic", PROPERTY_HINT_ENUM, "Euclidean,Manhattan,Octile,Chebyshev"), "set_default_estimate_heuristic", "get_default_estimate_heuristic");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "diagonal_mode", PROPERTY_HINT_ENUM, "Never,Always,At Least One Walkable,Only If No Obstacles"), "set_diagonal_mode", "get_diagonal_mode");

	BIND_ENUM_CONSTANT(HEURISTIC_EUCLIDEAN);
	BIND_ENUM_CONSTANT(HEURISTIC_MANHATTAN);
	BIND_ENUM_CONSTANT(HEURISTIC_OCTILE);
	BIND_ENUM_CONSTANT(HEURISTIC_CHEBYSHEV);
	BIND_ENUM_CONSTANT(HEURISTIC_MAX);

	BIND_ENUM_CONSTANT(DIAGONAL_MODE_ALWAYS);
	BIND_ENUM_CONSTANT(DIAGONAL_MODE_NEVER);
	BIND_ENUM_CONSTANT(DIAGONAL_MODE_AT_LEAST_ONE_WALKABLE);
	BIND_ENUM_CONSTANT(DIAGONAL_MODE_ONLY_IF_NO_OBSTACLES);
	BIND_ENUM_CONSTANT(DIAGONAL_MODE_MAX);
}
