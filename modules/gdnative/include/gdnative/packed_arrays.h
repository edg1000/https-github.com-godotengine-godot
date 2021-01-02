/*************************************************************************/
/*  packed_arrays.h                                                      */
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

#ifndef GODOT_PACKED_ARRAYS_H
#define GODOT_PACKED_ARRAYS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/////// PackedByteArray

#define GODOT_PACKED_BYTE_ARRAY_SIZE (2 * sizeof(void *))

#ifndef GODOT_CORE_API_GODOT_PACKED_BYTE_ARRAY_TYPE_DEFINED
#define GODOT_CORE_API_GODOT_PACKED_BYTE_ARRAY_TYPE_DEFINED
typedef struct {
	uint8_t _dont_touch_that[GODOT_PACKED_BYTE_ARRAY_SIZE];
} godot_packed_byte_array;
#endif

/////// PackedInt32Array

#define GODOT_PACKED_INT32_ARRAY_SIZE (2 * sizeof(void *))

#ifndef GODOT_CORE_API_GODOT_PACKED_INT32_ARRAY_TYPE_DEFINED
#define GODOT_CORE_API_GODOT_PACKED_INT32_ARRAY_TYPE_DEFINED
typedef struct {
	uint8_t _dont_touch_that[GODOT_PACKED_INT32_ARRAY_SIZE];
} godot_packed_int32_array;
#endif

/////// PackedInt64Array

#define GODOT_PACKED_INT64_ARRAY_SIZE (2 * sizeof(void *))

#ifndef GODOT_CORE_API_GODOT_PACKED_INT64_ARRAY_TYPE_DEFINED
#define GODOT_CORE_API_GODOT_PACKED_INT64_ARRAY_TYPE_DEFINED
typedef struct {
	uint8_t _dont_touch_that[GODOT_PACKED_INT64_ARRAY_SIZE];
} godot_packed_int64_array;
#endif

/////// PackedFloat32Array

#define GODOT_PACKED_FLOAT32_ARRAY_SIZE (2 * sizeof(void *))

#ifndef GODOT_CORE_API_GODOT_PACKED_FLOAT32_ARRAY_TYPE_DEFINED
#define GODOT_CORE_API_GODOT_PACKED_FLOAT32_ARRAY_TYPE_DEFINED
typedef struct {
	uint8_t _dont_touch_that[GODOT_PACKED_FLOAT32_ARRAY_SIZE];
} godot_packed_float32_array;
#endif

/////// PackedFloat64Array

#define GODOT_PACKED_FLOAT64_ARRAY_SIZE (2 * sizeof(void *))

#ifndef GODOT_CORE_API_GODOT_PACKED_FLOAT64_ARRAY_TYPE_DEFINED
#define GODOT_CORE_API_GODOT_PACKED_FLOAT64_ARRAY_TYPE_DEFINED
typedef struct {
	uint8_t _dont_touch_that[GODOT_PACKED_FLOAT64_ARRAY_SIZE];
} godot_packed_float64_array;
#endif

/////// PackedStringArray

#define GODOT_PACKED_STRING_ARRAY_SIZE (2 * sizeof(void *))

#ifndef GODOT_CORE_API_GODOT_PACKED_STRING_ARRAY_TYPE_DEFINED
#define GODOT_CORE_API_GODOT_PACKED_STRING_ARRAY_TYPE_DEFINED
typedef struct {
	uint8_t _dont_touch_that[GODOT_PACKED_STRING_ARRAY_SIZE];
} godot_packed_string_array;
#endif

/////// PackedVector2Array

#define GODOT_PACKED_VECTOR2_ARRAY_SIZE (2 * sizeof(void *))

#ifndef GODOT_CORE_API_GODOT_PACKED_VECTOR2_ARRAY_TYPE_DEFINED
#define GODOT_CORE_API_GODOT_PACKED_VECTOR2_ARRAY_TYPE_DEFINED
typedef struct {
	uint8_t _dont_touch_that[GODOT_PACKED_VECTOR2_ARRAY_SIZE];
} godot_packed_vector2_array;
#endif

/////// PackedVector2iArray

#define GODOT_PACKED_VECTOR2I_ARRAY_SIZE (2 * sizeof(void *))

#ifndef GODOT_CORE_API_GODOT_PACKED_VECTOR2I_ARRAY_TYPE_DEFINED
#define GODOT_CORE_API_GODOT_PACKED_VECTOR2I_ARRAY_TYPE_DEFINED
typedef struct {
	uint8_t _dont_touch_that[GODOT_PACKED_VECTOR2I_ARRAY_SIZE];
} godot_packed_vector2i_array;
#endif

/////// PackedVector3Array

#define GODOT_PACKED_VECTOR3_ARRAY_SIZE (2 * sizeof(void *))

#ifndef GODOT_CORE_API_GODOT_PACKED_VECTOR3_ARRAY_TYPE_DEFINED
#define GODOT_CORE_API_GODOT_PACKED_VECTOR3_ARRAY_TYPE_DEFINED
typedef struct {
	uint8_t _dont_touch_that[GODOT_PACKED_VECTOR3_ARRAY_SIZE];
} godot_packed_vector3_array;
#endif

/////// PackedVector3iArray

#define GODOT_PACKED_VECTOR3I_ARRAY_SIZE (2 * sizeof(void *))

#ifndef GODOT_CORE_API_GODOT_PACKED_VECTOR3I_ARRAY_TYPE_DEFINED
#define GODOT_CORE_API_GODOT_PACKED_VECTOR3I_ARRAY_TYPE_DEFINED
typedef struct {
	uint8_t _dont_touch_that[GODOT_PACKED_VECTOR3I_ARRAY_SIZE];
} godot_packed_vector3i_array;
#endif

/////// PackedColorArray

#define GODOT_PACKED_COLOR_ARRAY_SIZE (2 * sizeof(void *))

#ifndef GODOT_CORE_API_GODOT_PACKED_COLOR_ARRAY_TYPE_DEFINED
#define GODOT_CORE_API_GODOT_PACKED_COLOR_ARRAY_TYPE_DEFINED
typedef struct {
	uint8_t _dont_touch_that[GODOT_PACKED_COLOR_ARRAY_SIZE];
} godot_packed_color_array;
#endif

#include <gdnative/gdnative.h>

#ifdef __cplusplus
extern "C" {
#endif

#if __STDC_VERSION__ >= 201112L

#define godot_packed_array_new(r_dest)                                  \
    _Generic((r_dest),                                                  \
        godot_packed_byte_array *:godot_packed_byte_array_new,          \
        godot_packed_int32_array *:godot_packed_int32_array_new,        \
        godot_packed_int64_array *:godot_packed_int64_array_new,        \
        godot_packed_float32_array *:godot_packed_float32_array_new,    \
        godot_packed_float64_array *:godot_packed_float64_array_new,    \
        godot_packed_string_array *:godot_packed_string_array_new,      \
        godot_packed_vector2_array *:godot_packed_vector2_array_new,    \
        godot_packed_vector2i_array *:godot_packed_vector2i_array_new,  \
        godot_packed_vector3_array *:godot_packed_vector3_array_new,    \
        godot_packed_color_array *:godot_packed_color_array_new)        \
    ((r_dest))

#define godot_packed_array_new_copy(r_dest, p_src)                          \
    _Generic((p_src),                                                       \
        godot_packed_byte_array *:godot_packed_byte_array_new_copy,         \
        godot_packed_int32_array *:godot_packed_int32_array_new_copy,       \
        godot_packed_int64_array *:godot_packed_int64_array_new_copy,       \
        godot_packed_float32_array *:godot_packed_float32_array_new_copy,   \
        godot_packed_float64_array *:godot_packed_float64_array_new_copy,   \
        godot_packed_string_array *:godot_packed_string_array_new_copy,     \
        godot_packed_vector2_array *:godot_packed_vector2_array_new_copy,   \
        godot_packed_vector2i_array *:godot_packed_vector2i_array_new_copy, \
        godot_packed_vector3_array *:godot_packed_vector3_array_new_copy,   \
        godot_packed_color_array *:godot_packed_color_array_new_copy)       \
    ((r_dest), (p_src))

#define godot_packed_array_new_with_array(r_dest, p_a)                              \
    _Generic((r_dest),                                                              \
        godot_packed_byte_array *:godot_packed_byte_array_new_with_array,           \
        godot_packed_int32_array *:godot_packed_int32_array_new_with_array,         \
        godot_packed_int64_array *:godot_packed_int64_array_new_with_array,         \
        godot_packed_float32_array *:godot_packed_float32_array_new_with_array,     \
        godot_packed_float64_array *:godot_packed_float64_array_new_with_array,     \
        godot_packed_string_array *:godot_packed_string_array_new_with_array,       \
        godot_packed_vector2_array *:godot_packed_vector2_array_new_with_array,     \
        godot_packed_vector2i_array *:godot_packed_vector2i_array_new_with_array,   \
        godot_packed_vector3_array *:godot_packed_vector3_array_new_with_array,     \
        godot_packed_color_array *:godot_packed_color_array_new_with_array)         \
    ((r_dest), (p_a))

#define godot_packed_array_ptr(p_self)                                  \
    _Generic((p_self),                                                  \
        godot_packed_byte_array *:godot_packed_byte_array_ptr,          \
        godot_packed_int32_array *:godot_packed_int32_array_ptr,        \
        godot_packed_int64_array *:godot_packed_int64_array_ptr,        \
        godot_packed_float32_array *:godot_packed_float32_array_ptr,    \
        godot_packed_float64_array *:godot_packed_float64_array_ptr,    \
        godot_packed_string_array *:godot_packed_string_array_ptr,      \
        godot_packed_vector2_array *:godot_packed_vector2_array_ptr,    \
        godot_packed_vector2i_array *:godot_packed_vector2i_array_ptr,  \
        godot_packed_vector3_array *:godot_packed_vector3_array_ptr,    \
        godot_packed_color_array *:godot_packed_color_array_ptr)        \
    ((p_self))

#define godot_packed_array_ptrw(p_self)                                 \
    _Generic((pself),                                                   \
        godot_packed_byte_array *:godot_packed_byte_array_ptrw,         \
        godot_packed_int32_array *:godot_packed_int32_array_ptrw,       \
        godot_packed_int64_array *:godot_packed_int64_array_ptrw,       \
        godot_packed_float32_array *:godot_packed_float32_array_ptrw,   \
        godot_packed_float64_array *:godot_packed_float64_array_ptrw,   \
        godot_packed_string_array *:godot_packed_string_array_ptrw,     \
        godot_packed_vector2_array *:godot_packed_vector2_array_ptrw,   \
        godot_packed_vector2i_array *:godot_packed_vector2i_array_ptrw, \
        godot_packed_vector3_array *:godot_packed_vector3_array_ptrw,   \
        godot_packed_color_array *:godot_packed_color_array_ptrw)       \
    ((p_self))

#define godot_packed_array_append(p_self, p_data)                           \
    _Generic((p_self),                                                      \
        godot_packed_byte_array *:godot_packed_byte_array_append,           \
        godot_packed_int32_array *:godot_packed_int32_array_append,         \
        godot_packed_int64_array *:godot_packed_int64_array_append,         \
        godot_packed_float32_array *:godot_packed_float32_array_append,     \
        godot_packed_float64_array *:godot_packed_float64_array_append,     \
        godot_packed_string_array *:godot_packed_string_array_append,       \
        godot_packed_vector2_array *:godot_packed_vector2_array_append,     \
        godot_packed_vector2i_array *:godot_packed_vector2i_array_append,   \
        godot_packed_vector3_array *:godot_packed_vector3_array_append,     \
        godot_packed_color_array *:godot_packed_color_array_append)         \
    ((p_self), (p_data))

#define godot_packed_array_append_array(p_self, p_array)                        \
    _Generic((p_self),                                                          \
        godot_packed_byte_array *:godot_packed_byte_array_append_array,         \
        godot_packed_int32_array *:godot_packed_int32_array_append_array,       \
        godot_packed_int64_array *:godot_packed_int64_array_append_array,       \
        godot_packed_float32_array *:godot_packed_float32_array_append_array,   \
        godot_packed_float64_array *:godot_packed_float64_array_append_array,   \
        godot_packed_string_array *:godot_packed_string_array_append_array,     \
        godot_packed_vector2_array *:godot_packed_vector2_array_append_array,   \
        godot_packed_vector2i_array *:godot_packed_vector2i_array_append_array, \
        godot_packed_vector3_array *:godot_packed_vector3_array_append_array,   \
        godot_packed_color_array *:godot_packed_color_array_append_array)       \
    ((p_self), (p_array))

#define godot_packed_array_insert(p_self, p_data)                           \
    _Generic((p_self),                                                      \
        godot_packed_byte_array *:godot_packed_byte_array_insert,           \
        godot_packed_int32_array *:godot_packed_int32_array_insert,         \
        godot_packed_int64_array *:godot_packed_int64_array_insert,         \
        godot_packed_float32_array *:godot_packed_float32_array_insert,     \
        godot_packed_float64_array *:godot_packed_float64_array_insert,     \
        godot_packed_string_array *:godot_packed_string_array_insert,       \
        godot_packed_vector2_array *:godot_packed_vector2_array_insert,     \
        godot_packed_vector2i_array *:godot_packed_vector2i_array_insert,   \
        godot_packed_vector3_array *:godot_packed_vector3_array_insert,     \
        godot_packed_color_array *:godot_packed_color_array_insert)         \
    ((p_self), (p_data))

#define godot_packed_array_has(p_self, p_value)                         \
    _Generic((p_self),                                                  \
        godot_packed_byte_array *:godot_packed_byte_array_has,          \
        godot_packed_int32_array *:godot_packed_int32_array_has,        \
        godot_packed_int64_array *:godot_packed_int64_array_has,        \
        godot_packed_float32_array *:godot_packed_float32_array_has,    \
        godot_packed_float64_array *:godot_packed_float64_array_has,    \
        godot_packed_string_array *:godot_packed_string_array_has,      \
        godot_packed_vector2_array *:godot_packed_vector2_array_has,    \
        godot_packed_vector2i_array *:godot_packed_vector2i_array_has,  \
        godot_packed_vector3_array *:godot_packed_vector3_array_has,    \
        godot_packed_color_array *:godot_packed_color_array_has)        \
    ((p_self), (p_value))

#define godot_packed_array_sort(p_self)                                 \
    _Generic((p_self),                                                  \
        godot_packed_byte_array *:godot_packed_byte_array_sort,         \
        godot_packed_int32_array *:godot_packed_int32_array_sort,       \
        godot_packed_int64_array *:godot_packed_int64_array_sort,       \
        godot_packed_float32_array *:godot_packed_float32_array_sort,   \
        godot_packed_float64_array *:godot_packed_float64_array_sort,   \
        godot_packed_string_array *:godot_packed_string_array_sort,     \
        godot_packed_vector2_array *:godot_packed_vector2_array_sort,   \
        godot_packed_vector2i_array *:godot_packed_vector2i_array_sort, \
        godot_packed_vector3_array *:godot_packed_vector3_array_sort,   \
        godot_packed_color_array *:godot_packed_color_array_sort)       \
    ((p_self))

#define godot_packed_array_invert(p_self)                                   \
    _Generic((p_self),                                                      \
        godot_packed_byte_array *:godot_packed_byte_array_invert,           \
        godot_packed_int32_array *:godot_packed_int32_array_invert,         \
        godot_packed_int64_array *:godot_packed_int64_array_invert,         \
        godot_packed_float32_array *:godot_packed_float32_array_invert,     \
        godot_packed_float64_array *:godot_packed_float64_array_invert,     \
        godot_packed_string_array *:godot_packed_string_array_invert,       \
        godot_packed_vector2_array *:godot_packed_vector2_array_invert,     \
        godot_packed_vector2i_array *:godot_packed_vector2i_array_invert,   \
        godot_packed_vector3_array *:godot_packed_vector3_array_invert,     \
        godot_packed_color_array *:godot_packed_color_array_invert)         \
    ((p_self))

#define godot_packed_array_push_back(p_self, p_data) \
    _Generic((p_self),\
        godot_packed_byte_array *:godot_packed_byte_array_push_back,\
        godot_packed_int32_array *:godot_packed_int32_array_push_back,\
        godot_packed_int64_array *:godot_packed_int64_array_push_back,\
        godot_packed_float32_array *:godot_packed_float32_array_push_back,\
        godot_packed_float64_array *:godot_packed_float64_array_push_back,\
        godot_packed_string_array *:godot_packed_string_array_push_back,\
        godot_packed_vector2_array *:godot_packed_vector2_array_push_back,\
        godot_packed_vector2i_array *:godot_packed_vector2i_array_push_back,\
        godot_packed_vector3_array *:godot_packed_vector3_array_push_back,\
        godot_packed_color_array *:godot_packed_color_array_push_back)\
    (p_self)

#define godot_packed_array_remove(p_self, p_idx)                            \
    _Generic((p_self),                                                      \
        godot_packed_byte_array *:godot_packed_byte_array_remove,           \
        godot_packed_int32_array *:godot_packed_int32_array_remove,         \
        godot_packed_int64_array *:godot_packed_int64_array_remove,         \
        godot_packed_float32_array *:godot_packed_float32_array_remove,     \
        godot_packed_float64_array *:godot_packed_float64_array_remove,     \
        godot_packed_string_array *:godot_packed_string_array_remove,       \
        godot_packed_vector2_array *:godot_packed_vector2_array_remove,     \
        godot_packed_vector2i_array *:godot_packed_vector2i_array_remove,   \
        godot_packed_vector3_array *:godot_packed_vector3_array_remove,     \
        godot_packed_color_array *:godot_packed_color_array_remove)         \
    ((p_self))

#define godot_packed_array_resize(p_self, p_size)                           \
    _Generic((p_self),                                                      \
        godot_packed_byte_array *:godot_packed_byte_array_resize,           \
        godot_packed_int32_array *:godot_packed_int32_array_resize,         \
        godot_packed_int64_array *:godot_packed_int64_array_resize,         \
        godot_packed_float32_array *:godot_packed_float32_array_resize,     \
        godot_packed_float64_array *:godot_packed_float64_array_resize,     \
        godot_packed_string_array *:godot_packed_string_array_resize,       \
        godot_packed_vector2_array *:godot_packed_vector2_array_resize,     \
        godot_packed_vector2i_array *:godot_packed_vector2i_array_resize,   \
        godot_packed_vector3_array *:godot_packed_vector3_array_resize,     \
        godot_packed_color_array *:godot_packed_color_array_resize)         \
    ((p_self))

#define godot_packed_array_set(p_self, p_idx, p_data)                   \
    _Generic((p_self),                                                  \
        godot_packed_byte_array *:godot_packed_byte_array_set,          \
        godot_packed_int32_array *:godot_packed_int32_array_set,        \
        godot_packed_int64_array *:godot_packed_int64_array_set,        \
        godot_packed_float32_array *:godot_packed_float32_array_set,    \
        godot_packed_float64_array *:godot_packed_float64_array_set,    \
        godot_packed_string_array *:godot_packed_string_array_set,      \
        godot_packed_vector2_array *:godot_packed_vector2_array_set,    \
        godot_packed_vector2i_array *:godot_packed_vector2i_array_set,  \
        godot_packed_vector3_array *:godot_packed_vector3_array_set,    \
        godot_packed_color_array *:godot_packed_color_array_set)        \
    ((p_self), (p_idx), (p_data))

#define godot_packed_array_get(p_self, p_idx)                           \
    _Generic((p_self),                                                  \
        godot_packed_byte_array *:godot_packed_byte_array_get,          \
        godot_packed_int32_array *:godot_packed_int32_array_get,        \
        godot_packed_int64_array *:godot_packed_int64_array_get,        \
        godot_packed_float32_array *:godot_packed_float32_array_get,    \
        godot_packed_float64_array *:godot_packed_float64_array_get,    \
        godot_packed_string_array *:godot_packed_string_array_get,      \
        godot_packed_vector2_array *:godot_packed_vector2_array_get,    \
        godot_packed_vector2i_array *:godot_packed_vector2i_array_get,  \
        godot_packed_vector3_array *:godot_packed_vector3_array_get,    \
        godot_packed_color_array *:godot_packed_color_array_get)        \
    ((p_self), (p_idx))

#define godot_packed_array_size(p_self)                                 \
    _Generic((p_self),                                                  \
        godot_packed_byte_array *:godot_packed_byte_array_size,         \
        godot_packed_int32_array *:godot_packed_int32_array_size,       \
        godot_packed_int64_array *:godot_packed_int64_array_size,       \
        godot_packed_float32_array *:godot_packed_float32_array_size,   \
        godot_packed_float64_array *:godot_packed_float64_array_size,   \
        godot_packed_string_array *:godot_packed_string_array_size,     \
        godot_packed_vector2_array *:godot_packed_vector2_array_size,   \
        godot_packed_vector2i_array *:godot_packed_vector2i_array_size, \
        godot_packed_vector3_array *:godot_packed_vector3_array_size,   \
        godot_packed_color_array *:godot_packed_color_array_size)       \
    ((p_self))

#define godot_packed_array_is_empty(p_self)                                 \
    _Generic((p_self),                                                      \
        godot_packed_byte_array *:godot_packed_byte_array_is_empty,         \
        godot_packed_int32_array *:godot_packed_int32_array_is_empty,       \
        godot_packed_int64_array *:godot_packed_int64_array_is_empty,       \
        godot_packed_float32_array *:godot_packed_float32_array_is_empty,   \
        godot_packed_float64_array *:godot_packed_float64_array_is_empty,   \
        godot_packed_string_array *:godot_packed_string_array_is_empty,     \
        godot_packed_vector2_array *:godot_packed_vector2_array_is_empty,   \
        godot_packed_vector2i_array *:godot_packed_vector2i_array_is_empty, \
        godot_packed_vector3_array *:godot_packed_vector3_array_is_empty,   \
        godot_packed_color_array *:godot_packed_color_array_is_empty)       \
    ((p_self))

#define godot_packed_array_destroy(p_self)                                  \
    _Generic((p_self),                                                      \
        godot_packed_byte_array *:godot_packed_byte_array_destroy,          \
        godot_packed_int32_array *:godot_packed_int32_array_destroy,        \
        godot_packed_int64_array *:godot_packed_int64_array_destroy,        \
        godot_packed_float32_array *:godot_packed_float32_array_destroy,    \
        godot_packed_float64_array *:godot_packed_float64_array_destroy,    \
        godot_packed_string_array *:godot_packed_string_array_destroy,      \
        godot_packed_vector2_array *:godot_packed_vector2_array_destroy,    \
        godot_packed_vector2i_array *:godot_packed_vector2i_array_destroy,  \
        godot_packed_vector3_array *:godot_packed_vector3_array_destroy,    \
        godot_packed_color_array *:godot_packed_color_array_destroy)        \
    ((p_self))

#endif

// Byte.
void GDAPI godot_packed_byte_array_new(godot_packed_byte_array *r_dest);
void GDAPI godot_packed_byte_array_new_copy(godot_packed_byte_array *r_dest, const godot_packed_byte_array *p_src);
void GDAPI godot_packed_byte_array_new_with_array(godot_packed_byte_array *r_dest, const godot_array *p_a);

const uint8_t GDAPI *godot_packed_byte_array_ptr(const godot_packed_byte_array *p_self);
uint8_t GDAPI *godot_packed_byte_array_ptrw(godot_packed_byte_array *p_self);

void GDAPI godot_packed_byte_array_append(godot_packed_byte_array *p_self, const uint8_t p_data);

void GDAPI godot_packed_byte_array_append_array(godot_packed_byte_array *p_self, const godot_packed_byte_array *p_array);

godot_error GDAPI godot_packed_byte_array_insert(godot_packed_byte_array *p_self, const godot_int p_idx, const uint8_t p_data);

godot_bool GDAPI godot_packed_byte_array_has(godot_packed_byte_array *p_self, const uint8_t p_value);

void GDAPI godot_packed_byte_array_sort(godot_packed_byte_array *p_self);

void GDAPI godot_packed_byte_array_invert(godot_packed_byte_array *p_self);

void GDAPI godot_packed_byte_array_push_back(godot_packed_byte_array *p_self, const uint8_t p_data);

void GDAPI godot_packed_byte_array_remove(godot_packed_byte_array *p_self, const godot_int p_idx);

void GDAPI godot_packed_byte_array_resize(godot_packed_byte_array *p_self, const godot_int p_size);

void GDAPI godot_packed_byte_array_set(godot_packed_byte_array *p_self, const godot_int p_idx, const uint8_t p_data);
uint8_t GDAPI godot_packed_byte_array_get(const godot_packed_byte_array *p_self, const godot_int p_idx);

godot_int GDAPI godot_packed_byte_array_size(const godot_packed_byte_array *p_self);

godot_bool GDAPI godot_packed_byte_array_is_empty(const godot_packed_byte_array *p_self);

void GDAPI godot_packed_byte_array_new(godot_packed_byte_array *p_self);
void GDAPI godot_packed_byte_array_destroy(godot_packed_byte_array *p_self);
uint8_t GDAPI *godot_packed_byte_array_operator_index(godot_packed_byte_array *p_self, godot_int p_index);
const uint8_t GDAPI *godot_packed_byte_array_operator_index_const(const godot_packed_byte_array *p_self, godot_int p_index);

// Int32.

void GDAPI godot_packed_int32_array_new(godot_packed_int32_array *p_self);
void GDAPI godot_packed_int32_array_destroy(godot_packed_int32_array *p_self);
int32_t GDAPI *godot_packed_int32_array_operator_index(godot_packed_int32_array *p_self, godot_int p_index);
const int32_t GDAPI *godot_packed_int32_array_operator_index_const(const godot_packed_int32_array *p_self, godot_int p_index);

// Int64.

void GDAPI godot_packed_int64_array_new(godot_packed_int64_array *p_self);
void GDAPI godot_packed_int64_array_destroy(godot_packed_int64_array *p_self);
int64_t GDAPI *godot_packed_int64_array_operator_index(godot_packed_int64_array *p_self, godot_int p_index);
const int64_t GDAPI *godot_packed_int64_array_operator_index_const(const godot_packed_int64_array *p_self, godot_int p_index);

// Float32.

void GDAPI godot_packed_float32_array_new(godot_packed_float32_array *p_self);
void GDAPI godot_packed_float32_array_destroy(godot_packed_float32_array *p_self);
float GDAPI *godot_packed_float32_array_operator_index(godot_packed_float32_array *p_self, godot_int p_index);
const float GDAPI *godot_packed_float32_array_operator_index_const(const godot_packed_float32_array *p_self, godot_int p_index);

// Float64.

void GDAPI godot_packed_float64_array_new(godot_packed_float64_array *p_self);
void GDAPI godot_packed_float64_array_destroy(godot_packed_float64_array *p_self);
double GDAPI *godot_packed_float64_array_operator_index(godot_packed_float64_array *p_self, godot_int p_index);
const double GDAPI *godot_packed_float64_array_operator_index_const(const godot_packed_float64_array *p_self, godot_int p_index);

// String.

void GDAPI godot_packed_string_array_new(godot_packed_string_array *p_self);
void GDAPI godot_packed_string_array_destroy(godot_packed_string_array *p_self);
godot_string GDAPI *godot_packed_string_array_operator_index(godot_packed_string_array *p_self, godot_int p_index);
const godot_string GDAPI *godot_packed_string_array_operator_index_const(const godot_packed_string_array *p_self, godot_int p_index);

// Vector2.

void GDAPI godot_packed_vector2_array_new(godot_packed_vector2_array *p_self);
void GDAPI godot_packed_vector2_array_destroy(godot_packed_vector2_array *p_self);
godot_vector2 GDAPI *godot_packed_vector2_array_operator_index(godot_packed_vector2_array *p_self, godot_int p_index);
const godot_vector2 GDAPI *godot_packed_vector2_array_operator_index_const(const godot_packed_vector2_array *p_self, godot_int p_index);

// Vector2i.

void GDAPI godot_packed_vector2i_array_new(godot_packed_vector2i_array *p_self);
void GDAPI godot_packed_vector2i_array_destroy(godot_packed_vector2i_array *p_self);
godot_vector2i GDAPI *godot_packed_vector2i_array_operator_index(godot_packed_vector2i_array *p_self, godot_int p_index);
const godot_vector2i GDAPI *godot_packed_vector2i_array_operator_index_const(const godot_packed_vector2i_array *p_self, godot_int p_index);

// Vector3.

void GDAPI godot_packed_vector3_array_new(godot_packed_vector3_array *p_self);
void GDAPI godot_packed_vector3_array_destroy(godot_packed_vector3_array *p_self);
godot_vector3 GDAPI *godot_packed_vector3_array_operator_index(godot_packed_vector3_array *p_self, godot_int p_index);
const godot_vector3 GDAPI *godot_packed_vector3_array_operator_index_const(const godot_packed_vector3_array *p_self, godot_int p_index);

// Vector3i.

void GDAPI godot_packed_vector3i_array_new(godot_packed_vector3i_array *p_self);
void GDAPI godot_packed_vector3i_array_destroy(godot_packed_vector3i_array *p_self);
godot_vector3i GDAPI *godot_packed_vector3i_array_operator_index(godot_packed_vector3i_array *p_self, godot_int p_index);
const godot_vector3i GDAPI *godot_packed_vector3i_array_operator_index_const(const godot_packed_vector3i_array *p_self, godot_int p_index);

// Color.

void GDAPI godot_packed_color_array_new(godot_packed_color_array *p_self);
void GDAPI godot_packed_color_array_destroy(godot_packed_color_array *p_self);
godot_color GDAPI *godot_packed_color_array_operator_index(godot_packed_color_array *p_self, godot_int p_index);
const godot_color GDAPI *godot_packed_color_array_operator_index_const(const godot_packed_color_array *p_self, godot_int p_index);

#ifdef __cplusplus
}
#endif

#endif // GODOT_PACKED_ARRAYS_H
