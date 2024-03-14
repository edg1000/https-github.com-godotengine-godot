/**************************************************************************/
/*  type_info.h                                                           */
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

#ifndef TYPE_INFO_H
#define TYPE_INFO_H

#include "core/typedefs.h"

template <bool C, typename T = void>
struct EnableIf {
	typedef T type;
};

template <typename T>
struct EnableIf<false, T> {
};

template <typename, typename>
inline constexpr bool types_are_same_v = false;

template <typename T>
inline constexpr bool types_are_same_v<T, T> = true;

template <typename B, typename D>
struct TypeInherits {
	static D *get_d();

	static char (&test(B *))[1];
	static char (&test(...))[2];

	static bool const value = sizeof(test(get_d())) == sizeof(char) &&
			!types_are_same_v<B volatile const, void volatile const>;
};

namespace GodotTypeInfo {
enum Metadata {
	METADATA_NONE,
	METADATA_INT_IS_INT8,
	METADATA_INT_IS_INT16,
	METADATA_INT_IS_INT32,
	METADATA_INT_IS_INT64,
	METADATA_INT_IS_UINT8,
	METADATA_INT_IS_UINT16,
	METADATA_INT_IS_UINT32,
	METADATA_INT_IS_UINT64,
	METADATA_REAL_IS_FLOAT,
	METADATA_REAL_IS_DOUBLE
};
}

// If the compiler fails because it's trying to instantiate the primary 'GetTypeInfo' template
// instead of one of the specializations, it's most likely because the type 'T' is not supported.
// If 'T' is a class that inherits 'Object', make sure it can see the actual class declaration
// instead of a forward declaration. You can always forward declare 'T' in a header file, and then
// include the actual declaration of 'T' in the source file where 'GetTypeInfo<T>' is instantiated.
template <class T, typename = void>
struct GetTypeInfo;

#define MAKE_TYPE_INFO(m_type, m_var_type)                                            \
	template <>                                                                       \
	struct GetTypeInfo<m_type> {                                                      \
		static const Variant::Type VARIANT_TYPE = m_var_type;                         \
		static const GodotTypeInfo::Metadata METADATA = GodotTypeInfo::METADATA_NONE; \
		static inline PropertyInfo get_class_info() {                                 \
			return PropertyInfo(VARIANT_TYPE, String());                              \
		}                                                                             \
	};                                                                                \
	template <>                                                                       \
	struct GetTypeInfo<const m_type &> {                                              \
		static const Variant::Type VARIANT_TYPE = m_var_type;                         \
		static const GodotTypeInfo::Metadata METADATA = GodotTypeInfo::METADATA_NONE; \
		static inline PropertyInfo get_class_info() {                                 \
			return PropertyInfo(VARIANT_TYPE, String());                              \
		}                                                                             \
	};

#define MAKE_TYPE_INFO_WITH_META(m_type, m_var_type, m_metadata)    \
	template <>                                                     \
	struct GetTypeInfo<m_type> {                                    \
		static const Variant::Type VARIANT_TYPE = m_var_type;       \
		static const GodotTypeInfo::Metadata METADATA = m_metadata; \
		static inline PropertyInfo get_class_info() {               \
			return PropertyInfo(VARIANT_TYPE, String());            \
		}                                                           \
	};                                                              \
	template <>                                                     \
	struct GetTypeInfo<const m_type &> {                            \
		static const Variant::Type VARIANT_TYPE = m_var_type;       \
		static const GodotTypeInfo::Metadata METADATA = m_metadata; \
		static inline PropertyInfo get_class_info() {               \
			return PropertyInfo(VARIANT_TYPE, String());            \
		}                                                           \
	};

MAKE_TYPE_INFO(bool, Variant::BOOL)
MAKE_TYPE_INFO_WITH_META(uint8_t, Variant::INT, GodotTypeInfo::METADATA_INT_IS_UINT8)
MAKE_TYPE_INFO_WITH_META(int8_t, Variant::INT, GodotTypeInfo::METADATA_INT_IS_INT8)
MAKE_TYPE_INFO_WITH_META(uint16_t, Variant::INT, GodotTypeInfo::METADATA_INT_IS_UINT16)
MAKE_TYPE_INFO_WITH_META(int16_t, Variant::INT, GodotTypeInfo::METADATA_INT_IS_INT16)
MAKE_TYPE_INFO_WITH_META(uint32_t, Variant::INT, GodotTypeInfo::METADATA_INT_IS_UINT32)
MAKE_TYPE_INFO_WITH_META(int32_t, Variant::INT, GodotTypeInfo::METADATA_INT_IS_INT32)
MAKE_TYPE_INFO_WITH_META(uint64_t, Variant::INT, GodotTypeInfo::METADATA_INT_IS_UINT64)
MAKE_TYPE_INFO_WITH_META(int64_t, Variant::INT, GodotTypeInfo::METADATA_INT_IS_INT64)
MAKE_TYPE_INFO(char16_t, Variant::INT)
MAKE_TYPE_INFO(char32_t, Variant::INT)
MAKE_TYPE_INFO_WITH_META(float, Variant::FLOAT, GodotTypeInfo::METADATA_REAL_IS_FLOAT)
MAKE_TYPE_INFO_WITH_META(double, Variant::FLOAT, GodotTypeInfo::METADATA_REAL_IS_DOUBLE)

MAKE_TYPE_INFO(String, Variant::STRING)
MAKE_TYPE_INFO(Vector2, Variant::VECTOR2)
MAKE_TYPE_INFO(Rect2, Variant::RECT2)
MAKE_TYPE_INFO(Vector3, Variant::VECTOR3)
MAKE_TYPE_INFO(Vector2i, Variant::VECTOR2I)
MAKE_TYPE_INFO(Rect2i, Variant::RECT2I)
MAKE_TYPE_INFO(Vector3i, Variant::VECTOR3I)
MAKE_TYPE_INFO(Vector4, Variant::VECTOR4)
MAKE_TYPE_INFO(Vector4i, Variant::VECTOR4I)
MAKE_TYPE_INFO(Transform2D, Variant::TRANSFORM2D)
MAKE_TYPE_INFO(Plane, Variant::PLANE)
MAKE_TYPE_INFO(Quaternion, Variant::QUATERNION)
MAKE_TYPE_INFO(AABB, Variant::AABB)
MAKE_TYPE_INFO(Basis, Variant::BASIS)
MAKE_TYPE_INFO(Transform3D, Variant::TRANSFORM3D)
MAKE_TYPE_INFO(Projection, Variant::PROJECTION)
MAKE_TYPE_INFO(Color, Variant::COLOR)
MAKE_TYPE_INFO(StringName, Variant::STRING_NAME)
MAKE_TYPE_INFO(NodePath, Variant::NODE_PATH)
MAKE_TYPE_INFO(RID, Variant::RID)
MAKE_TYPE_INFO(Callable, Variant::CALLABLE)
MAKE_TYPE_INFO(Signal, Variant::SIGNAL)
MAKE_TYPE_INFO(Dictionary, Variant::DICTIONARY)
MAKE_TYPE_INFO(Array, Variant::ARRAY)
MAKE_TYPE_INFO(PackedByteArray, Variant::PACKED_BYTE_ARRAY)
MAKE_TYPE_INFO(PackedInt32Array, Variant::PACKED_INT32_ARRAY)
MAKE_TYPE_INFO(PackedInt64Array, Variant::PACKED_INT64_ARRAY)
MAKE_TYPE_INFO(PackedFloat32Array, Variant::PACKED_FLOAT32_ARRAY)
MAKE_TYPE_INFO(PackedFloat64Array, Variant::PACKED_FLOAT64_ARRAY)
MAKE_TYPE_INFO(PackedStringArray, Variant::PACKED_STRING_ARRAY)
MAKE_TYPE_INFO(PackedVector2Array, Variant::PACKED_VECTOR2_ARRAY)
MAKE_TYPE_INFO(PackedVector3Array, Variant::PACKED_VECTOR3_ARRAY)
MAKE_TYPE_INFO(PackedColorArray, Variant::PACKED_COLOR_ARRAY)

MAKE_TYPE_INFO(IPAddress, Variant::STRING)

//objectID
template <>
struct GetTypeInfo<ObjectID> {
	static const Variant::Type VARIANT_TYPE = Variant::INT;
	static const GodotTypeInfo::Metadata METADATA = GodotTypeInfo::METADATA_INT_IS_UINT64;
	static inline PropertyInfo get_class_info() {
		return PropertyInfo(Variant::INT, String(), PROPERTY_HINT_INT_IS_OBJECTID);
	}
};

//for variant
template <>
struct GetTypeInfo<Variant> {
	static const Variant::Type VARIANT_TYPE = Variant::NIL;
	static const GodotTypeInfo::Metadata METADATA = GodotTypeInfo::METADATA_NONE;
	static inline PropertyInfo get_class_info() {
		return PropertyInfo(Variant::NIL, String(), PROPERTY_HINT_NONE, String(), PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_NIL_IS_VARIANT);
	}
};

template <>
struct GetTypeInfo<const Variant &> {
	static const Variant::Type VARIANT_TYPE = Variant::NIL;
	static const GodotTypeInfo::Metadata METADATA = GodotTypeInfo::METADATA_NONE;
	static inline PropertyInfo get_class_info() {
		return PropertyInfo(Variant::NIL, String(), PROPERTY_HINT_NONE, String(), PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_NIL_IS_VARIANT);
	}
};

#define MAKE_TEMPLATE_TYPE_INFO(m_template, m_type, m_var_type)                       \
	template <>                                                                       \
	struct GetTypeInfo<m_template<m_type>> {                                          \
		static const Variant::Type VARIANT_TYPE = m_var_type;                         \
		static const GodotTypeInfo::Metadata METADATA = GodotTypeInfo::METADATA_NONE; \
		static inline PropertyInfo get_class_info() {                                 \
			return PropertyInfo(VARIANT_TYPE, String());                              \
		}                                                                             \
	};                                                                                \
	template <>                                                                       \
	struct GetTypeInfo<const m_template<m_type> &> {                                  \
		static const Variant::Type VARIANT_TYPE = m_var_type;                         \
		static const GodotTypeInfo::Metadata METADATA = GodotTypeInfo::METADATA_NONE; \
		static inline PropertyInfo get_class_info() {                                 \
			return PropertyInfo(VARIANT_TYPE, String());                              \
		}                                                                             \
	};

MAKE_TEMPLATE_TYPE_INFO(Vector, Variant, Variant::ARRAY)
MAKE_TEMPLATE_TYPE_INFO(Vector, RID, Variant::ARRAY)
MAKE_TEMPLATE_TYPE_INFO(Vector, Plane, Variant::ARRAY)
MAKE_TEMPLATE_TYPE_INFO(Vector, Face3, Variant::PACKED_VECTOR3_ARRAY)
MAKE_TEMPLATE_TYPE_INFO(Vector, StringName, Variant::PACKED_STRING_ARRAY)

template <typename T>
struct GetTypeInfo<T *, typename EnableIf<TypeInherits<Object, T>::value>::type> {
	static const Variant::Type VARIANT_TYPE = Variant::OBJECT;
	static const GodotTypeInfo::Metadata METADATA = GodotTypeInfo::METADATA_NONE;
	static inline PropertyInfo get_class_info() {
		return PropertyInfo(StringName(T::get_class_static()));
	}
};

template <typename T>
struct GetTypeInfo<const T *, typename EnableIf<TypeInherits<Object, T>::value>::type> {
	static const Variant::Type VARIANT_TYPE = Variant::OBJECT;
	static const GodotTypeInfo::Metadata METADATA = GodotTypeInfo::METADATA_NONE;
	static inline PropertyInfo get_class_info() {
		return PropertyInfo(StringName(T::get_class_static()));
	}
};

namespace godot {
namespace details {
inline String enum_qualified_name_to_class_info_name(const String &p_qualified_name) {
	Vector<String> parts = p_qualified_name.split("::", false);
	if (parts.size() <= 2) {
		return String(".").join(parts);
	}
	// Contains namespace. We only want the class and enum names.
	return parts[parts.size() - 2] + "." + parts[parts.size() - 1];
}
} // namespace details
} // namespace godot

#define TEMPL_MAKE_ENUM_TYPE_INFO(m_enum, m_impl)                                                                                            \
	template <>                                                                                                                              \
	struct GetTypeInfo<m_impl> {                                                                                                             \
		static const Variant::Type VARIANT_TYPE = Variant::INT;                                                                              \
		static const GodotTypeInfo::Metadata METADATA = GodotTypeInfo::METADATA_NONE;                                                        \
		static inline PropertyInfo get_class_info() {                                                                                        \
			return PropertyInfo(Variant::INT, String(), PROPERTY_HINT_NONE, String(), PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_CLASS_IS_ENUM, \
					godot::details::enum_qualified_name_to_class_info_name(String(#m_enum)));                                                \
		}                                                                                                                                    \
	};

#define MAKE_ENUM_TYPE_INFO(m_enum)                 \
	TEMPL_MAKE_ENUM_TYPE_INFO(m_enum, m_enum)       \
	TEMPL_MAKE_ENUM_TYPE_INFO(m_enum, m_enum const) \
	TEMPL_MAKE_ENUM_TYPE_INFO(m_enum, m_enum &)     \
	TEMPL_MAKE_ENUM_TYPE_INFO(m_enum, const m_enum &)

template <typename T>
inline StringName __constant_get_enum_name(T param, const String &p_constant) {
	if constexpr (GetTypeInfo<T>::VARIANT_TYPE == Variant::NIL) {
		ERR_PRINT("Missing VARIANT_ENUM_CAST for constant's enum: " + p_constant);
	}
	return GetTypeInfo<T>::get_class_info().class_name;
}

template <class T>
class BitField {
	int64_t value = 0;

public:
	_FORCE_INLINE_ BitField<T> &set_flag(T p_flag) {
		value |= (int64_t)p_flag;
		return *this;
	}
	_FORCE_INLINE_ bool has_flag(T p_flag) const { return value & (int64_t)p_flag; }
	_FORCE_INLINE_ bool is_empty() const { return value == 0; }
	_FORCE_INLINE_ void clear_flag(T p_flag) { value &= ~(int64_t)p_flag; }
	_FORCE_INLINE_ void clear() { value = 0; }
	_FORCE_INLINE_ constexpr BitField() = default;
	_FORCE_INLINE_ constexpr BitField(int64_t p_value) { value = p_value; }
	_FORCE_INLINE_ constexpr BitField(T p_value) { value = (int64_t)p_value; }
	_FORCE_INLINE_ operator int64_t() const { return value; }
	_FORCE_INLINE_ operator Variant() const { return value; }
};

#define TEMPL_MAKE_BITFIELD_TYPE_INFO(m_enum, m_impl)                                                                                            \
	template <>                                                                                                                                  \
	struct GetTypeInfo<m_impl> {                                                                                                                 \
		static const Variant::Type VARIANT_TYPE = Variant::INT;                                                                                  \
		static const GodotTypeInfo::Metadata METADATA = GodotTypeInfo::METADATA_NONE;                                                            \
		static inline PropertyInfo get_class_info() {                                                                                            \
			return PropertyInfo(Variant::INT, String(), PROPERTY_HINT_NONE, String(), PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_CLASS_IS_BITFIELD, \
					godot::details::enum_qualified_name_to_class_info_name(String(#m_enum)));                                                    \
		}                                                                                                                                        \
	};                                                                                                                                           \
	template <>                                                                                                                                  \
	struct GetTypeInfo<BitField<m_impl>> {                                                                                                       \
		static const Variant::Type VARIANT_TYPE = Variant::INT;                                                                                  \
		static const GodotTypeInfo::Metadata METADATA = GodotTypeInfo::METADATA_NONE;                                                            \
		static inline PropertyInfo get_class_info() {                                                                                            \
			return PropertyInfo(Variant::INT, String(), PROPERTY_HINT_NONE, String(), PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_CLASS_IS_BITFIELD, \
					godot::details::enum_qualified_name_to_class_info_name(String(#m_enum)));                                                    \
		}                                                                                                                                        \
	};

#define MAKE_BITFIELD_TYPE_INFO(m_enum)                 \
	TEMPL_MAKE_BITFIELD_TYPE_INFO(m_enum, m_enum)       \
	TEMPL_MAKE_BITFIELD_TYPE_INFO(m_enum, m_enum const) \
	TEMPL_MAKE_BITFIELD_TYPE_INFO(m_enum, m_enum &)     \
	TEMPL_MAKE_BITFIELD_TYPE_INFO(m_enum, const m_enum &)

template <typename T>
inline StringName __constant_get_bitfield_name(T param, const String &p_constant) {
	if (GetTypeInfo<T>::VARIANT_TYPE == Variant::NIL) {
		ERR_PRINT("Missing VARIANT_ENUM_CAST for constant's bitfield: " + p_constant);
	}
	return GetTypeInfo<BitField<T>>::get_class_info().class_name;
}
#define CLASS_INFO(m_type) (GetTypeInfo<m_type *>::get_class_info())

template <typename T>
struct ZeroInitializer {
	static void initialize(T &value) {} //no initialization by default
};

template <>
struct ZeroInitializer<bool> {
	static void initialize(bool &value) { value = false; }
};

template <typename T>
struct ZeroInitializer<T *> {
	static void initialize(T *&value) { value = nullptr; }
};

#define ZERO_INITIALIZER_NUMBER(m_type)                      \
	template <>                                              \
	struct ZeroInitializer<m_type> {                         \
		static void initialize(m_type &value) { value = 0; } \
	};

ZERO_INITIALIZER_NUMBER(uint8_t)
ZERO_INITIALIZER_NUMBER(int8_t)
ZERO_INITIALIZER_NUMBER(uint16_t)
ZERO_INITIALIZER_NUMBER(int16_t)
ZERO_INITIALIZER_NUMBER(uint32_t)
ZERO_INITIALIZER_NUMBER(int32_t)
ZERO_INITIALIZER_NUMBER(uint64_t)
ZERO_INITIALIZER_NUMBER(int64_t)
ZERO_INITIALIZER_NUMBER(char16_t)
ZERO_INITIALIZER_NUMBER(char32_t)
ZERO_INITIALIZER_NUMBER(float)
ZERO_INITIALIZER_NUMBER(double)

template <typename T>
inline constexpr Variant::Type get_variant_type_v = TypeInherits<Object, T>::value ? Variant::OBJECT : Variant::VARIANT_MAX;

template <typename T>
inline constexpr bool is_variant_type_v = get_variant_type_v<T> != Variant::VARIANT_MAX;

template <typename T>
struct GetObjectClassName {
	static_assert(is_variant_type_v<T>);

	static const String self() {
		if constexpr (get_variant_type_v<T> == Variant::OBJECT) {
			return T::get_class_static();
		} else {
			return String();
		}
	}
	static const String parent() {
		if constexpr (get_variant_type_v<T> == Variant::OBJECT) {
			return T::get_parent_class_static();
		} else {
			return String();
		}
	}
};

#define MAKE_VARIANT_TYPE_CONSTANTS(m_type, m_variant_type) \
	template <>                                             \
	inline constexpr Variant::Type get_variant_type_v<m_type> = m_variant_type;

MAKE_VARIANT_TYPE_CONSTANTS(Variant, Variant::NIL)
MAKE_VARIANT_TYPE_CONSTANTS(bool, Variant::BOOL)
MAKE_VARIANT_TYPE_CONSTANTS(uint8_t, Variant::INT)
MAKE_VARIANT_TYPE_CONSTANTS(int8_t, Variant::INT)
MAKE_VARIANT_TYPE_CONSTANTS(uint16_t, Variant::INT)
MAKE_VARIANT_TYPE_CONSTANTS(int16_t, Variant::INT)
MAKE_VARIANT_TYPE_CONSTANTS(uint32_t, Variant::INT)
MAKE_VARIANT_TYPE_CONSTANTS(int32_t, Variant::INT)
MAKE_VARIANT_TYPE_CONSTANTS(uint64_t, Variant::INT)
MAKE_VARIANT_TYPE_CONSTANTS(int64_t, Variant::INT)
MAKE_VARIANT_TYPE_CONSTANTS(float, Variant::FLOAT)
MAKE_VARIANT_TYPE_CONSTANTS(double, Variant::FLOAT)
MAKE_VARIANT_TYPE_CONSTANTS(String, Variant::STRING)
MAKE_VARIANT_TYPE_CONSTANTS(Vector2, Variant::VECTOR2)
MAKE_VARIANT_TYPE_CONSTANTS(Vector2i, Variant::VECTOR2I)
MAKE_VARIANT_TYPE_CONSTANTS(Rect2, Variant::RECT2)
MAKE_VARIANT_TYPE_CONSTANTS(Rect2i, Variant::RECT2I)
MAKE_VARIANT_TYPE_CONSTANTS(Vector3, Variant::VECTOR3)
MAKE_VARIANT_TYPE_CONSTANTS(Vector3i, Variant::VECTOR3I)
MAKE_VARIANT_TYPE_CONSTANTS(Transform2D, Variant::TRANSFORM2D)
MAKE_VARIANT_TYPE_CONSTANTS(Vector4, Variant::VECTOR4)
MAKE_VARIANT_TYPE_CONSTANTS(Vector4i, Variant::VECTOR4I)
MAKE_VARIANT_TYPE_CONSTANTS(Plane, Variant::PLANE)
MAKE_VARIANT_TYPE_CONSTANTS(Quaternion, Variant::QUATERNION)
MAKE_VARIANT_TYPE_CONSTANTS(AABB, Variant::AABB)
MAKE_VARIANT_TYPE_CONSTANTS(Basis, Variant::BASIS)
MAKE_VARIANT_TYPE_CONSTANTS(Transform3D, Variant::TRANSFORM3D)
MAKE_VARIANT_TYPE_CONSTANTS(Projection, Variant::PROJECTION)
MAKE_VARIANT_TYPE_CONSTANTS(Color, Variant::COLOR)
MAKE_VARIANT_TYPE_CONSTANTS(StringName, Variant::STRING_NAME)
MAKE_VARIANT_TYPE_CONSTANTS(NodePath, Variant::NODE_PATH)
MAKE_VARIANT_TYPE_CONSTANTS(RID, Variant::RID)
MAKE_VARIANT_TYPE_CONSTANTS(Object, Variant::OBJECT)
MAKE_VARIANT_TYPE_CONSTANTS(Callable, Variant::CALLABLE)
MAKE_VARIANT_TYPE_CONSTANTS(Signal, Variant::SIGNAL)
MAKE_VARIANT_TYPE_CONSTANTS(Dictionary, Variant::DICTIONARY)
MAKE_VARIANT_TYPE_CONSTANTS(Array, Variant::ARRAY)
MAKE_VARIANT_TYPE_CONSTANTS(PackedByteArray, Variant::PACKED_BYTE_ARRAY)
MAKE_VARIANT_TYPE_CONSTANTS(PackedInt32Array, Variant::PACKED_INT32_ARRAY)
MAKE_VARIANT_TYPE_CONSTANTS(PackedInt64Array, Variant::PACKED_INT64_ARRAY)
MAKE_VARIANT_TYPE_CONSTANTS(PackedFloat32Array, Variant::PACKED_FLOAT32_ARRAY)
MAKE_VARIANT_TYPE_CONSTANTS(PackedFloat64Array, Variant::PACKED_FLOAT64_ARRAY)
MAKE_VARIANT_TYPE_CONSTANTS(PackedStringArray, Variant::PACKED_STRING_ARRAY)
MAKE_VARIANT_TYPE_CONSTANTS(PackedVector2Array, Variant::PACKED_VECTOR2_ARRAY)
MAKE_VARIANT_TYPE_CONSTANTS(PackedVector3Array, Variant::PACKED_VECTOR3_ARRAY)
MAKE_VARIANT_TYPE_CONSTANTS(PackedColorArray, Variant::PACKED_COLOR_ARRAY)
MAKE_VARIANT_TYPE_CONSTANTS(IPAddress, Variant::STRING)

#undef MAKE_VARIANT_TYPE_CONSTANTS

#endif // TYPE_INFO_H
