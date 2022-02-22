/*************************************************************************/
/*  vector2i.h                                                           */
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

#ifndef VECTOR2I_H
#define VECTOR2I_H

#include "core/error/error_macros.h"
#include "core/math/math_funcs.h"

class String;
struct Vector2;

struct _NO_DISCARD_ Vector2i {
	enum Axis {
		AXIS_X,
		AXIS_Y,
	};

	union {
		struct {
			union {
				int32_t x;
				int32_t width;
			};
			union {
				int32_t y;
				int32_t height;
			};
		};

		int32_t coord[2] = { 0 };
	};

	_FORCE_INLINE_ int32_t &operator[](int p_idx) {
#ifdef DEBUG_ENABLED
		ERR_FAIL_INDEX_V(p_idx, 2, y);
#endif
		return coord[p_idx];
	}
	_FORCE_INLINE_ const int32_t &operator[](int p_idx) const {
#ifdef DEBUG_ENABLED
		ERR_FAIL_INDEX_V(p_idx, 2, y);
#endif
		return coord[p_idx];
	}

	_FORCE_INLINE_ Vector2i::Axis min_axis_index() const {
		return x < y ? Vector2i::AXIS_X : Vector2i::AXIS_Y;
	}

	_FORCE_INLINE_ Vector2i::Axis max_axis_index() const {
		return x < y ? Vector2i::AXIS_Y : Vector2i::AXIS_X;
	}

	Vector2i min(const Vector2i &p_vector2i) const {
		return Vector2i(MIN(x, p_vector2i.x), MIN(y, p_vector2i.y));
	}

	Vector2i max(const Vector2i &p_vector2i) const {
		return Vector2i(MAX(x, p_vector2i.x), MAX(y, p_vector2i.y));
	}

	Vector2i operator+(const Vector2i &p_v) const;
	void operator+=(const Vector2i &p_v);
	Vector2i operator-(const Vector2i &p_v) const;
	void operator-=(const Vector2i &p_v);
	Vector2i operator*(const Vector2i &p_v1) const;

	Vector2i operator*(const int32_t &rvalue) const;
	void operator*=(const int32_t &rvalue);

	Vector2i operator/(const Vector2i &p_v1) const;
	Vector2i operator/(const int32_t &rvalue) const;
	void operator/=(const int32_t &rvalue);

	Vector2i operator%(const Vector2i &p_v1) const;
	Vector2i operator%(const int32_t &rvalue) const;
	void operator%=(const int32_t &rvalue);

	Vector2i operator-() const;
	bool operator<(const Vector2i &p_vec2) const { return (x == p_vec2.x) ? (y < p_vec2.y) : (x < p_vec2.x); }
	bool operator>(const Vector2i &p_vec2) const { return (x == p_vec2.x) ? (y > p_vec2.y) : (x > p_vec2.x); }

	bool operator<=(const Vector2i &p_vec2) const { return x == p_vec2.x ? (y <= p_vec2.y) : (x < p_vec2.x); }
	bool operator>=(const Vector2i &p_vec2) const { return x == p_vec2.x ? (y >= p_vec2.y) : (x > p_vec2.x); }

	bool operator==(const Vector2i &p_vec2) const;
	bool operator!=(const Vector2i &p_vec2) const;

	int64_t length_squared() const;
	double length() const;

	real_t aspect() const { return width / (real_t)height; }
	Vector2i sign() const { return Vector2i(SIGN(x), SIGN(y)); }
	Vector2i abs() const { return Vector2i(ABS(x), ABS(y)); }
	Vector2i clamp(const Vector2i &p_min, const Vector2i &p_max) const;

	operator String() const;
	operator Vector2() const;

	inline Vector2i() {}
	inline Vector2i(const int32_t p_x, const int32_t p_y) {
		x = p_x;
		y = p_y;
	}
};

// Multiplication operators required to workaround issues with LLVM using implicit conversion.

_FORCE_INLINE_ Vector2i operator*(const int32_t p_scalar, const Vector2i &p_vector) {
	return p_vector * p_scalar;
}

_FORCE_INLINE_ Vector2i operator*(const int64_t p_scalar, const Vector2i &p_vector) {
	return p_vector * p_scalar;
}

_FORCE_INLINE_ Vector2i operator*(const float p_scalar, const Vector2i &p_vector) {
	return p_vector * p_scalar;
}

_FORCE_INLINE_ Vector2i operator*(const double p_scalar, const Vector2i &p_vector) {
	return p_vector * p_scalar;
}

typedef Vector2i Size2i;
typedef Vector2i Point2i;

#endif // VECTOR2I_H
