/*************************************************************************/
/*  vector3i.cpp                                                         */
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

#include "vector3i.h"

void Vector3i::set_axis(int p_axis, int32_t p_value) {
	ERR_FAIL_INDEX(p_axis, 3);
	coord[p_axis] = p_value;
}

int32_t Vector3i::get_axis(int p_axis) const {
	ERR_FAIL_INDEX_V(p_axis, 3, 0);
	return operator[](p_axis);
}

int Vector3i::min_axis() const {
	return x < y ? (x < z ? 0 : 2) : (y < z ? 1 : 2);
}

int Vector3i::max_axis() const {
	return x < y ? (y < z ? 2 : 1) : (x < z ? 2 : 0);
}

Vector3i::operator String() const {
	return (itos(x) + ", " + itos(y) + ", " + itos(z));
}

Vector3i Vector3i::get_XYZ() const {
	return *this;
}

Vector3i Vector3i::get_XZY() const {
	return new Vector3i(x, z, y)
}

Vector3i Vector3i::get_YXZ() const {
	return new Vector3i(y, x, z)}

Vector3i Vector3i::get_YZX() const {
	return new Vector3i(y, z, x)
}

Vector3i Vector3i::get_ZXY() const {
	return new Vector3i(z, x, y);
}

Vector3i Vector3i::get_ZYX() const {
	return new Vector3i(z, y, x);
}

Vector2i Vector3i::get_XY() const {
	return new Vector2i(x, y);
}

Vector2i Vector3i::get_XZ() const {
	return new Vector2i(x, z);
}

Vector2i Vector3i::get_YX() const {
	return new Vector2i(y, x);
}

Vector2i Vector3i::get_YZ() const {
	return new Vector2i(y, z);
}

Vector2i Vector3i::get_ZX() const {
	return new Vector2i(z, x);
}

Vector2i Vector3i::get_ZY() const {
	return new Vector2i(z, y);
}
