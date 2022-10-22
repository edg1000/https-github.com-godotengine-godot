/*************************************************************************/
/*  gdnative_interface.h                                                 */
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

#ifndef GDNATIVE_INTERFACE_H
#define GDNATIVE_INTERFACE_H

/* This is a C class header, you can copy it and use it directly in your own binders.
 * Together with the JSON file, you should be able to generate any binder.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifndef __cplusplus
typedef uint32_t char32_t;
typedef uint16_t char16_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* VARIANT TYPES */

typedef enum {
	GDNATIVE_VARIANT_TYPE_NIL,

	/*  atomic types */
	GDNATIVE_VARIANT_TYPE_BOOL,
	GDNATIVE_VARIANT_TYPE_INT,
	GDNATIVE_VARIANT_TYPE_FLOAT,
	GDNATIVE_VARIANT_TYPE_STRING,

	/* math types */
	GDNATIVE_VARIANT_TYPE_VECTOR2,
	GDNATIVE_VARIANT_TYPE_VECTOR2I,
	GDNATIVE_VARIANT_TYPE_RECT2,
	GDNATIVE_VARIANT_TYPE_RECT2I,
	GDNATIVE_VARIANT_TYPE_VECTOR3,
	GDNATIVE_VARIANT_TYPE_VECTOR3I,
	GDNATIVE_VARIANT_TYPE_TRANSFORM2D,
	GDNATIVE_VARIANT_TYPE_VECTOR4,
	GDNATIVE_VARIANT_TYPE_VECTOR4I,
	GDNATIVE_VARIANT_TYPE_PLANE,
	GDNATIVE_VARIANT_TYPE_QUATERNION,
	GDNATIVE_VARIANT_TYPE_AABB,
	GDNATIVE_VARIANT_TYPE_BASIS,
	GDNATIVE_VARIANT_TYPE_TRANSFORM3D,
	GDNATIVE_VARIANT_TYPE_PROJECTION,

	/* misc types */
	GDNATIVE_VARIANT_TYPE_COLOR,
	GDNATIVE_VARIANT_TYPE_STRING_NAME,
	GDNATIVE_VARIANT_TYPE_NODE_PATH,
	GDNATIVE_VARIANT_TYPE_RID,
	GDNATIVE_VARIANT_TYPE_OBJECT,
	GDNATIVE_VARIANT_TYPE_CALLABLE,
	GDNATIVE_VARIANT_TYPE_SIGNAL,
	GDNATIVE_VARIANT_TYPE_DICTIONARY,
	GDNATIVE_VARIANT_TYPE_ARRAY,

	/* typed arrays */
	GDNATIVE_VARIANT_TYPE_PACKED_BYTE_ARRAY,
	GDNATIVE_VARIANT_TYPE_PACKED_INT32_ARRAY,
	GDNATIVE_VARIANT_TYPE_PACKED_INT64_ARRAY,
	GDNATIVE_VARIANT_TYPE_PACKED_FLOAT32_ARRAY,
	GDNATIVE_VARIANT_TYPE_PACKED_FLOAT64_ARRAY,
	GDNATIVE_VARIANT_TYPE_PACKED_STRING_ARRAY,
	GDNATIVE_VARIANT_TYPE_PACKED_VECTOR2_ARRAY,
	GDNATIVE_VARIANT_TYPE_PACKED_VECTOR3_ARRAY,
	GDNATIVE_VARIANT_TYPE_PACKED_COLOR_ARRAY,

	GDNATIVE_VARIANT_TYPE_VARIANT_MAX
} GDNativeVariantType;

typedef enum {
	/* comparison */
	GDNATIVE_VARIANT_OP_EQUAL,
	GDNATIVE_VARIANT_OP_NOT_EQUAL,
	GDNATIVE_VARIANT_OP_LESS,
	GDNATIVE_VARIANT_OP_LESS_EQUAL,
	GDNATIVE_VARIANT_OP_GREATER,
	GDNATIVE_VARIANT_OP_GREATER_EQUAL,

	/* mathematic */
	GDNATIVE_VARIANT_OP_ADD,
	GDNATIVE_VARIANT_OP_SUBTRACT,
	GDNATIVE_VARIANT_OP_MULTIPLY,
	GDNATIVE_VARIANT_OP_DIVIDE,
	GDNATIVE_VARIANT_OP_NEGATE,
	GDNATIVE_VARIANT_OP_POSITIVE,
	GDNATIVE_VARIANT_OP_MODULE,
	GDNATIVE_VARIANT_OP_POWER,

	/* bitwise */
	GDNATIVE_VARIANT_OP_SHIFT_LEFT,
	GDNATIVE_VARIANT_OP_SHIFT_RIGHT,
	GDNATIVE_VARIANT_OP_BIT_AND,
	GDNATIVE_VARIANT_OP_BIT_OR,
	GDNATIVE_VARIANT_OP_BIT_XOR,
	GDNATIVE_VARIANT_OP_BIT_NEGATE,

	/* logic */
	GDNATIVE_VARIANT_OP_AND,
	GDNATIVE_VARIANT_OP_OR,
	GDNATIVE_VARIANT_OP_XOR,
	GDNATIVE_VARIANT_OP_NOT,

	/* containment */
	GDNATIVE_VARIANT_OP_IN,
	GDNATIVE_VARIANT_OP_MAX

} GDNativeVariantOperator;

typedef void *GDNativeVariantPtr;
typedef const void *GDNativeConstVariantPtr;
typedef void *GDNativeStringNamePtr;
typedef const void *GDNativeConstStringNamePtr;
typedef void *GDNativeStringPtr;
typedef const void *GDNativeConstStringPtr;
typedef void *GDNativeObjectPtr;
typedef const void *GDNativeConstObjectPtr;
typedef void *GDNativeTypePtr;
typedef const void *GDNativeConstTypePtr;
typedef const void *GDNativeMethodBindPtr;
typedef int64_t GDNativeInt;
typedef uint8_t GDNativeBool;
typedef uint64_t GDObjectInstanceID;

/* VARIANT DATA I/O */

typedef enum {
	GDNATIVE_CALL_OK,
	GDNATIVE_CALL_ERROR_INVALID_METHOD,
	GDNATIVE_CALL_ERROR_INVALID_ARGUMENT, // Expected a different variant type.
	GDNATIVE_CALL_ERROR_TOO_MANY_ARGUMENTS, // Expected lower number of arguments.
	GDNATIVE_CALL_ERROR_TOO_FEW_ARGUMENTS, // Expected higher number of arguments.
	GDNATIVE_CALL_ERROR_INSTANCE_IS_NULL,
	GDNATIVE_CALL_ERROR_METHOD_NOT_CONST, // Used for const call.
} GDNativeCallErrorType;

typedef struct {
	GDNativeCallErrorType error;
	int32_t argument;
	int32_t expected;
} GDNativeCallError;

typedef void (*GDNativeVariantFromTypeConstructorFunc)(GDNativeVariantPtr, GDNativeTypePtr);
typedef void (*GDNativeTypeFromVariantConstructorFunc)(GDNativeTypePtr, GDNativeVariantPtr);
typedef void (*GDNativePtrOperatorEvaluator)(GDNativeConstTypePtr p_left, GDNativeConstTypePtr p_right, GDNativeTypePtr r_result);
typedef void (*GDNativePtrBuiltInMethod)(GDNativeTypePtr p_base, GDNativeConstTypePtr *p_args, GDNativeTypePtr r_return, int p_argument_count);
typedef void (*GDNativePtrConstructor)(GDNativeTypePtr p_base, GDNativeConstTypePtr *p_args);
typedef void (*GDNativePtrDestructor)(GDNativeTypePtr p_base);
typedef void (*GDNativePtrSetter)(GDNativeTypePtr p_base, GDNativeConstTypePtr p_value);
typedef void (*GDNativePtrGetter)(GDNativeConstTypePtr p_base, GDNativeTypePtr r_value);
typedef void (*GDNativePtrIndexedSetter)(GDNativeTypePtr p_base, GDNativeInt p_index, GDNativeConstTypePtr p_value);
typedef void (*GDNativePtrIndexedGetter)(GDNativeConstTypePtr p_base, GDNativeInt p_index, GDNativeTypePtr r_value);
typedef void (*GDNativePtrKeyedSetter)(GDNativeTypePtr p_base, GDNativeConstTypePtr p_key, GDNativeConstTypePtr p_value);
typedef void (*GDNativePtrKeyedGetter)(GDNativeConstTypePtr p_base, GDNativeConstTypePtr p_key, GDNativeTypePtr r_value);
typedef uint32_t (*GDNativePtrKeyedChecker)(GDNativeConstVariantPtr p_base, GDNativeConstVariantPtr p_key);
typedef void (*GDNativePtrUtilityFunction)(GDNativeTypePtr r_return, GDNativeConstTypePtr *p_arguments, int p_argument_count);

typedef GDNativeObjectPtr (*GDNativeClassConstructor)();

typedef void *(*GDNativeInstanceBindingCreateCallback)(void *p_token, void *p_instance);
typedef void (*GDNativeInstanceBindingFreeCallback)(void *p_token, void *p_instance, void *p_binding);
typedef GDNativeBool (*GDNativeInstanceBindingReferenceCallback)(void *p_token, void *p_binding, GDNativeBool p_reference);

typedef struct {
	GDNativeInstanceBindingCreateCallback create_callback;
	GDNativeInstanceBindingFreeCallback free_callback;
	GDNativeInstanceBindingReferenceCallback reference_callback;
} GDNativeInstanceBindingCallbacks;

/* EXTENSION CLASSES */

typedef void *GDExtensionClassInstancePtr;

typedef GDNativeBool (*GDNativeExtensionClassSet)(GDExtensionClassInstancePtr p_instance, GDNativeConstStringNamePtr p_name, GDNativeConstVariantPtr p_value);
typedef GDNativeBool (*GDNativeExtensionClassGet)(GDExtensionClassInstancePtr p_instance, GDNativeConstStringNamePtr p_name, GDNativeVariantPtr r_ret);
typedef uint64_t (*GDNativeExtensionClassGetRID)(GDExtensionClassInstancePtr p_instance);

typedef struct {
	GDNativeVariantType type;
	GDNativeStringNamePtr name;
	GDNativeStringNamePtr class_name;
	uint32_t hint; // Bitfield of `PropertyHint` (defined in `extension_api.json`).
	GDNativeStringPtr hint_string;
	uint32_t usage; // Bitfield of `PropertyUsageFlags` (defined in `extension_api.json`).
} GDNativePropertyInfo;

typedef struct {
	GDNativeStringNamePtr name;
	GDNativePropertyInfo return_value;
	uint32_t flags; // Bitfield of `GDNativeExtensionClassMethodFlags`.
	int32_t id;

	/* Arguments: `default_arguments` is an array of size `argument_count`. */
	uint32_t argument_count;
	GDNativePropertyInfo *arguments;

	/* Default arguments: `default_arguments` is an array of size `default_argument_count`. */
	uint32_t default_argument_count;
	GDNativeVariantPtr *default_arguments;
} GDNativeMethodInfo;

typedef const GDNativePropertyInfo *(*GDNativeExtensionClassGetPropertyList)(GDExtensionClassInstancePtr p_instance, uint32_t *r_count);
typedef void (*GDNativeExtensionClassFreePropertyList)(GDExtensionClassInstancePtr p_instance, const GDNativePropertyInfo *p_list);
typedef GDNativeBool (*GDNativeExtensionClassPropertyCanRevert)(GDExtensionClassInstancePtr p_instance, GDNativeConstStringNamePtr p_name);
typedef GDNativeBool (*GDNativeExtensionClassPropertyGetRevert)(GDExtensionClassInstancePtr p_instance, GDNativeConstStringNamePtr p_name, GDNativeVariantPtr r_ret);
typedef void (*GDNativeExtensionClassNotification)(GDExtensionClassInstancePtr p_instance, int32_t p_what);
typedef void (*GDNativeExtensionClassToString)(GDExtensionClassInstancePtr p_instance, GDNativeBool *r_is_valid, GDNativeStringPtr p_out);
typedef void (*GDNativeExtensionClassReference)(GDExtensionClassInstancePtr p_instance);
typedef void (*GDNativeExtensionClassUnreference)(GDExtensionClassInstancePtr p_instance);
typedef void (*GDNativeExtensionClassCallVirtual)(GDExtensionClassInstancePtr p_instance, GDNativeConstTypePtr *p_args, GDNativeTypePtr r_ret);
typedef GDNativeObjectPtr (*GDNativeExtensionClassCreateInstance)(void *p_userdata);
typedef void (*GDNativeExtensionClassFreeInstance)(void *p_userdata, GDExtensionClassInstancePtr p_instance);
typedef GDNativeExtensionClassCallVirtual (*GDNativeExtensionClassGetVirtual)(void *p_userdata, GDNativeConstStringNamePtr p_name);

typedef struct {
	GDNativeBool is_virtual;
	GDNativeBool is_abstract;
	GDNativeExtensionClassSet set_func;
	GDNativeExtensionClassGet get_func;
	GDNativeExtensionClassGetPropertyList get_property_list_func;
	GDNativeExtensionClassFreePropertyList free_property_list_func;
	GDNativeExtensionClassPropertyCanRevert property_can_revert_func;
	GDNativeExtensionClassPropertyGetRevert property_get_revert_func;
	GDNativeExtensionClassNotification notification_func;
	GDNativeExtensionClassToString to_string_func;
	GDNativeExtensionClassReference reference_func;
	GDNativeExtensionClassUnreference unreference_func;
	GDNativeExtensionClassCreateInstance create_instance_func; // (Default) constructor; mandatory. If the class is not instantiable, consider making it virtual or abstract.
	GDNativeExtensionClassFreeInstance free_instance_func; // Destructor; mandatory.
	GDNativeExtensionClassGetVirtual get_virtual_func; // Queries a virtual function by name and returns a callback to invoke the requested virtual function.
	GDNativeExtensionClassGetRID get_rid_func;
	void *class_userdata; // Per-class user data, later accessible in instance bindings.
} GDNativeExtensionClassCreationInfo;

typedef void *GDNativeExtensionClassLibraryPtr;

/* Method */

typedef enum {
	GDNATIVE_EXTENSION_METHOD_FLAG_NORMAL = 1,
	GDNATIVE_EXTENSION_METHOD_FLAG_EDITOR = 2,
	GDNATIVE_EXTENSION_METHOD_FLAG_CONST = 4,
	GDNATIVE_EXTENSION_METHOD_FLAG_VIRTUAL = 8,
	GDNATIVE_EXTENSION_METHOD_FLAG_VARARG = 16,
	GDNATIVE_EXTENSION_METHOD_FLAG_STATIC = 32,
	GDNATIVE_EXTENSION_METHOD_FLAGS_DEFAULT = GDNATIVE_EXTENSION_METHOD_FLAG_NORMAL,
} GDNativeExtensionClassMethodFlags;

typedef enum {
	GDNATIVE_EXTENSION_METHOD_ARGUMENT_METADATA_NONE,
	GDNATIVE_EXTENSION_METHOD_ARGUMENT_METADATA_INT_IS_INT8,
	GDNATIVE_EXTENSION_METHOD_ARGUMENT_METADATA_INT_IS_INT16,
	GDNATIVE_EXTENSION_METHOD_ARGUMENT_METADATA_INT_IS_INT32,
	GDNATIVE_EXTENSION_METHOD_ARGUMENT_METADATA_INT_IS_INT64,
	GDNATIVE_EXTENSION_METHOD_ARGUMENT_METADATA_INT_IS_UINT8,
	GDNATIVE_EXTENSION_METHOD_ARGUMENT_METADATA_INT_IS_UINT16,
	GDNATIVE_EXTENSION_METHOD_ARGUMENT_METADATA_INT_IS_UINT32,
	GDNATIVE_EXTENSION_METHOD_ARGUMENT_METADATA_INT_IS_UINT64,
	GDNATIVE_EXTENSION_METHOD_ARGUMENT_METADATA_REAL_IS_FLOAT,
	GDNATIVE_EXTENSION_METHOD_ARGUMENT_METADATA_REAL_IS_DOUBLE
} GDNativeExtensionClassMethodArgumentMetadata;

typedef void (*GDNativeExtensionClassMethodCall)(void *method_userdata, GDExtensionClassInstancePtr p_instance, GDNativeConstVariantPtr *p_args, GDNativeInt p_argument_count, GDNativeVariantPtr r_return, GDNativeCallError *r_error);
typedef void (*GDNativeExtensionClassMethodPtrCall)(void *method_userdata, GDExtensionClassInstancePtr p_instance, GDNativeConstTypePtr *p_args, GDNativeTypePtr r_ret);

typedef struct {
	GDNativeStringNamePtr name;
	void *method_userdata;
	GDNativeExtensionClassMethodCall call_func;
	GDNativeExtensionClassMethodPtrCall ptrcall_func;
	uint32_t method_flags; // Bitfield of `GDNativeExtensionClassMethodFlags`.

	/* If `has_return_value` is false, `return_value_info` and `return_value_metadata` are ignored. */
	GDNativeBool has_return_value;
	GDNativePropertyInfo *return_value_info;
	GDNativeExtensionClassMethodArgumentMetadata return_value_metadata;

	/* Arguments: `arguments_info` and `arguments_metadata` are array of size `argument_count`.
	 * Name and hint information for the argument can be omitted in release builds. Class name should always be present if it applies.
	 */
	uint32_t argument_count;
	GDNativePropertyInfo *arguments_info;
	GDNativeExtensionClassMethodArgumentMetadata *arguments_metadata;

	/* Default arguments: `default_arguments` is an array of size `default_argument_count`. */
	uint32_t default_argument_count;
	GDNativeVariantPtr *default_arguments;
} GDNativeExtensionClassMethodInfo;

/* SCRIPT INSTANCE EXTENSION */

typedef void *GDNativeExtensionScriptInstanceDataPtr; // Pointer to custom ScriptInstance native implementation.

typedef GDNativeBool (*GDNativeExtensionScriptInstanceSet)(GDNativeExtensionScriptInstanceDataPtr p_instance, GDNativeConstStringNamePtr p_name, GDNativeConstVariantPtr p_value);
typedef GDNativeBool (*GDNativeExtensionScriptInstanceGet)(GDNativeExtensionScriptInstanceDataPtr p_instance, GDNativeConstStringNamePtr p_name, GDNativeVariantPtr r_ret);
typedef const GDNativePropertyInfo *(*GDNativeExtensionScriptInstanceGetPropertyList)(GDNativeExtensionScriptInstanceDataPtr p_instance, uint32_t *r_count);
typedef void (*GDNativeExtensionScriptInstanceFreePropertyList)(GDNativeExtensionScriptInstanceDataPtr p_instance, const GDNativePropertyInfo *p_list);
typedef GDNativeVariantType (*GDNativeExtensionScriptInstanceGetPropertyType)(GDNativeExtensionScriptInstanceDataPtr p_instance, GDNativeConstStringNamePtr p_name, GDNativeBool *r_is_valid);

typedef GDNativeBool (*GDNativeExtensionScriptInstancePropertyCanRevert)(GDNativeExtensionScriptInstanceDataPtr p_instance, GDNativeConstStringNamePtr p_name);
typedef GDNativeBool (*GDNativeExtensionScriptInstancePropertyGetRevert)(GDNativeExtensionScriptInstanceDataPtr p_instance, GDNativeConstStringNamePtr p_name, GDNativeVariantPtr r_ret);

typedef GDNativeObjectPtr (*GDNativeExtensionScriptInstanceGetOwner)(GDNativeExtensionScriptInstanceDataPtr p_instance);
typedef void (*GDNativeExtensionScriptInstancePropertyStateAdd)(GDNativeConstStringNamePtr p_name, GDNativeConstVariantPtr p_value, void *p_userdata);
typedef void (*GDNativeExtensionScriptInstanceGetPropertyState)(GDNativeExtensionScriptInstanceDataPtr p_instance, GDNativeExtensionScriptInstancePropertyStateAdd p_add_func, void *p_userdata);

typedef const GDNativeMethodInfo *(*GDNativeExtensionScriptInstanceGetMethodList)(GDNativeExtensionScriptInstanceDataPtr p_instance, uint32_t *r_count);
typedef void (*GDNativeExtensionScriptInstanceFreeMethodList)(GDNativeExtensionScriptInstanceDataPtr p_instance, const GDNativeMethodInfo *p_list);

typedef GDNativeBool (*GDNativeExtensionScriptInstanceHasMethod)(GDNativeExtensionScriptInstanceDataPtr p_instance, GDNativeConstStringNamePtr p_name);

typedef void (*GDNativeExtensionScriptInstanceCall)(GDNativeExtensionScriptInstanceDataPtr p_self, GDNativeConstStringNamePtr p_method, GDNativeConstVariantPtr *p_args, GDNativeInt p_argument_count, GDNativeVariantPtr r_return, GDNativeCallError *r_error);
typedef void (*GDNativeExtensionScriptInstanceNotification)(GDNativeExtensionScriptInstanceDataPtr p_instance, int32_t p_what);
typedef void (*GDNativeExtensionScriptInstanceToString)(GDNativeExtensionScriptInstanceDataPtr p_instance, GDNativeBool *r_is_valid, GDNativeStringPtr r_out);

typedef void (*GDNativeExtensionScriptInstanceRefCountIncremented)(GDNativeExtensionScriptInstanceDataPtr p_instance);
typedef GDNativeBool (*GDNativeExtensionScriptInstanceRefCountDecremented)(GDNativeExtensionScriptInstanceDataPtr p_instance);

typedef GDNativeObjectPtr (*GDNativeExtensionScriptInstanceGetScript)(GDNativeExtensionScriptInstanceDataPtr p_instance);
typedef GDNativeBool (*GDNativeExtensionScriptInstanceIsPlaceholder)(GDNativeExtensionScriptInstanceDataPtr p_instance);

typedef void *GDNativeExtensionScriptLanguagePtr;

typedef GDNativeExtensionScriptLanguagePtr (*GDNativeExtensionScriptInstanceGetLanguage)(GDNativeExtensionScriptInstanceDataPtr p_instance);

typedef void (*GDNativeExtensionScriptInstanceFree)(GDNativeExtensionScriptInstanceDataPtr p_instance);

typedef void *GDNativeScriptInstancePtr; // Pointer to ScriptInstance.

typedef struct {
	GDNativeExtensionScriptInstanceSet set_func;
	GDNativeExtensionScriptInstanceGet get_func;
	GDNativeExtensionScriptInstanceGetPropertyList get_property_list_func;
	GDNativeExtensionScriptInstanceFreePropertyList free_property_list_func;

	GDNativeExtensionScriptInstancePropertyCanRevert property_can_revert_func;
	GDNativeExtensionScriptInstancePropertyGetRevert property_get_revert_func;

	GDNativeExtensionScriptInstanceGetOwner get_owner_func;
	GDNativeExtensionScriptInstanceGetPropertyState get_property_state_func;

	GDNativeExtensionScriptInstanceGetMethodList get_method_list_func;
	GDNativeExtensionScriptInstanceFreeMethodList free_method_list_func;
	GDNativeExtensionScriptInstanceGetPropertyType get_property_type_func;

	GDNativeExtensionScriptInstanceHasMethod has_method_func;

	GDNativeExtensionScriptInstanceCall call_func;
	GDNativeExtensionScriptInstanceNotification notification_func;

	GDNativeExtensionScriptInstanceToString to_string_func;

	GDNativeExtensionScriptInstanceRefCountIncremented refcount_incremented_func;
	GDNativeExtensionScriptInstanceRefCountDecremented refcount_decremented_func;

	GDNativeExtensionScriptInstanceGetScript get_script_func;

	GDNativeExtensionScriptInstanceIsPlaceholder is_placeholder_func;

	GDNativeExtensionScriptInstanceSet set_fallback_func;
	GDNativeExtensionScriptInstanceGet get_fallback_func;

	GDNativeExtensionScriptInstanceGetLanguage get_language_func;

	GDNativeExtensionScriptInstanceFree free_func;

} GDNativeExtensionScriptInstanceInfo;

/* INTERFACE */

typedef struct {
	uint32_t version_major;
	uint32_t version_minor;
	uint32_t version_patch;
	const char *version_string;

	/* GODOT CORE */

	void *(*mem_alloc)(size_t p_bytes);
	void *(*mem_realloc)(void *p_ptr, size_t p_bytes);
	void (*mem_free)(void *p_ptr);

	void (*print_error)(const char *p_description, const char *p_function, const char *p_file, int32_t p_line);
	void (*print_warning)(const char *p_description, const char *p_function, const char *p_file, int32_t p_line);
	void (*print_script_error)(const char *p_description, const char *p_function, const char *p_file, int32_t p_line);

	uint64_t (*get_native_struct_size)(GDNativeConstStringNamePtr p_name);

	/* GODOT VARIANT */

	/* variant general */
	void (*variant_new_copy)(GDNativeVariantPtr r_dest, GDNativeConstVariantPtr p_src);
	void (*variant_new_nil)(GDNativeVariantPtr r_dest);
	void (*variant_destroy)(GDNativeVariantPtr p_self);

	/* variant type */
	void (*variant_call)(GDNativeVariantPtr p_self, GDNativeConstStringNamePtr p_method, GDNativeConstVariantPtr *p_args, GDNativeInt p_argument_count, GDNativeVariantPtr r_return, GDNativeCallError *r_error);
	void (*variant_call_static)(GDNativeVariantType p_type, GDNativeConstStringNamePtr p_method, GDNativeConstVariantPtr *p_args, GDNativeInt p_argument_count, GDNativeVariantPtr r_return, GDNativeCallError *r_error);
	void (*variant_evaluate)(GDNativeVariantOperator p_op, GDNativeConstVariantPtr p_a, GDNativeConstVariantPtr p_b, GDNativeVariantPtr r_return, GDNativeBool *r_valid);
	void (*variant_set)(GDNativeVariantPtr p_self, GDNativeConstVariantPtr p_key, GDNativeConstVariantPtr p_value, GDNativeBool *r_valid);
	void (*variant_set_named)(GDNativeVariantPtr p_self, GDNativeConstStringNamePtr p_key, GDNativeConstVariantPtr p_value, GDNativeBool *r_valid);
	void (*variant_set_keyed)(GDNativeVariantPtr p_self, GDNativeConstVariantPtr p_key, GDNativeConstVariantPtr p_value, GDNativeBool *r_valid);
	void (*variant_set_indexed)(GDNativeVariantPtr p_self, GDNativeInt p_index, GDNativeConstVariantPtr p_value, GDNativeBool *r_valid, GDNativeBool *r_oob);
	void (*variant_get)(GDNativeConstVariantPtr p_self, GDNativeConstVariantPtr p_key, GDNativeVariantPtr r_ret, GDNativeBool *r_valid);
	void (*variant_get_named)(GDNativeConstVariantPtr p_self, GDNativeConstStringNamePtr p_key, GDNativeVariantPtr r_ret, GDNativeBool *r_valid);
	void (*variant_get_keyed)(GDNativeConstVariantPtr p_self, GDNativeConstVariantPtr p_key, GDNativeVariantPtr r_ret, GDNativeBool *r_valid);
	void (*variant_get_indexed)(GDNativeConstVariantPtr p_self, GDNativeInt p_index, GDNativeVariantPtr r_ret, GDNativeBool *r_valid, GDNativeBool *r_oob);
	GDNativeBool (*variant_iter_init)(GDNativeConstVariantPtr p_self, GDNativeVariantPtr r_iter, GDNativeBool *r_valid);
	GDNativeBool (*variant_iter_next)(GDNativeConstVariantPtr p_self, GDNativeVariantPtr r_iter, GDNativeBool *r_valid);
	void (*variant_iter_get)(GDNativeConstVariantPtr p_self, GDNativeVariantPtr r_iter, GDNativeVariantPtr r_ret, GDNativeBool *r_valid);
	GDNativeInt (*variant_hash)(GDNativeConstVariantPtr p_self);
	GDNativeInt (*variant_recursive_hash)(GDNativeConstVariantPtr p_self, GDNativeInt p_recursion_count);
	GDNativeBool (*variant_hash_compare)(GDNativeConstVariantPtr p_self, GDNativeConstVariantPtr p_other);
	GDNativeBool (*variant_booleanize)(GDNativeConstVariantPtr p_self);
	void (*variant_duplicate)(GDNativeConstVariantPtr p_self, GDNativeVariantPtr r_ret, GDNativeBool p_deep);
	void (*variant_stringify)(GDNativeConstVariantPtr p_self, GDNativeStringPtr r_ret);

	GDNativeVariantType (*variant_get_type)(GDNativeConstVariantPtr p_self);
	GDNativeBool (*variant_has_method)(GDNativeConstVariantPtr p_self, GDNativeConstStringNamePtr p_method);
	GDNativeBool (*variant_has_member)(GDNativeVariantType p_type, GDNativeConstStringNamePtr p_member);
	GDNativeBool (*variant_has_key)(GDNativeConstVariantPtr p_self, GDNativeConstVariantPtr p_key, GDNativeBool *r_valid);
	void (*variant_get_type_name)(GDNativeVariantType p_type, GDNativeStringPtr r_name);
	GDNativeBool (*variant_can_convert)(GDNativeVariantType p_from, GDNativeVariantType p_to);
	GDNativeBool (*variant_can_convert_strict)(GDNativeVariantType p_from, GDNativeVariantType p_to);

	/* ptrcalls */
	GDNativeVariantFromTypeConstructorFunc (*get_variant_from_type_constructor)(GDNativeVariantType p_type);
	GDNativeTypeFromVariantConstructorFunc (*get_variant_to_type_constructor)(GDNativeVariantType p_type);
	GDNativePtrOperatorEvaluator (*variant_get_ptr_operator_evaluator)(GDNativeVariantOperator p_operator, GDNativeVariantType p_type_a, GDNativeVariantType p_type_b);
	GDNativePtrBuiltInMethod (*variant_get_ptr_builtin_method)(GDNativeVariantType p_type, GDNativeConstStringNamePtr p_method, GDNativeInt p_hash);
	GDNativePtrConstructor (*variant_get_ptr_constructor)(GDNativeVariantType p_type, int32_t p_constructor);
	GDNativePtrDestructor (*variant_get_ptr_destructor)(GDNativeVariantType p_type);
	void (*variant_construct)(GDNativeVariantType p_type, GDNativeVariantPtr p_base, GDNativeConstVariantPtr *p_args, int32_t p_argument_count, GDNativeCallError *r_error);
	GDNativePtrSetter (*variant_get_ptr_setter)(GDNativeVariantType p_type, GDNativeConstStringNamePtr p_member);
	GDNativePtrGetter (*variant_get_ptr_getter)(GDNativeVariantType p_type, GDNativeConstStringNamePtr p_member);
	GDNativePtrIndexedSetter (*variant_get_ptr_indexed_setter)(GDNativeVariantType p_type);
	GDNativePtrIndexedGetter (*variant_get_ptr_indexed_getter)(GDNativeVariantType p_type);
	GDNativePtrKeyedSetter (*variant_get_ptr_keyed_setter)(GDNativeVariantType p_type);
	GDNativePtrKeyedGetter (*variant_get_ptr_keyed_getter)(GDNativeVariantType p_type);
	GDNativePtrKeyedChecker (*variant_get_ptr_keyed_checker)(GDNativeVariantType p_type);
	void (*variant_get_constant_value)(GDNativeVariantType p_type, GDNativeConstStringNamePtr p_constant, GDNativeVariantPtr r_ret);
	GDNativePtrUtilityFunction (*variant_get_ptr_utility_function)(GDNativeConstStringNamePtr p_function, GDNativeInt p_hash);

	/*  extra utilities */
	void (*string_new_with_latin1_chars)(GDNativeStringPtr r_dest, const char *p_contents);
	void (*string_new_with_utf8_chars)(GDNativeStringPtr r_dest, const char *p_contents);
	void (*string_new_with_utf16_chars)(GDNativeStringPtr r_dest, const char16_t *p_contents);
	void (*string_new_with_utf32_chars)(GDNativeStringPtr r_dest, const char32_t *p_contents);
	void (*string_new_with_wide_chars)(GDNativeStringPtr r_dest, const wchar_t *p_contents);
	void (*string_new_with_latin1_chars_and_len)(GDNativeStringPtr r_dest, const char *p_contents, GDNativeInt p_size);
	void (*string_new_with_utf8_chars_and_len)(GDNativeStringPtr r_dest, const char *p_contents, GDNativeInt p_size);
	void (*string_new_with_utf16_chars_and_len)(GDNativeStringPtr r_dest, const char16_t *p_contents, GDNativeInt p_size);
	void (*string_new_with_utf32_chars_and_len)(GDNativeStringPtr r_dest, const char32_t *p_contents, GDNativeInt p_size);
	void (*string_new_with_wide_chars_and_len)(GDNativeStringPtr r_dest, const wchar_t *p_contents, GDNativeInt p_size);
	/* Information about the following functions:
	 * - The return value is the resulting encoded string length.
	 * - The length returned is in characters, not in bytes. It also does not include a trailing zero.
	 * - These functions also do not write trailing zero, If you need it, write it yourself at the position indicated by the length (and make sure to allocate it).
	 * - Passing NULL in r_text means only the length is computed (again, without including trailing zero).
	 * - p_max_write_length argument is in characters, not bytes. It will be ignored if r_text is NULL.
	 * - p_max_write_length argument does not affect the return value, it's only to cap write length.
	 */
	GDNativeInt (*string_to_latin1_chars)(GDNativeConstStringPtr p_self, char *r_text, GDNativeInt p_max_write_length);
	GDNativeInt (*string_to_utf8_chars)(GDNativeConstStringPtr p_self, char *r_text, GDNativeInt p_max_write_length);
	GDNativeInt (*string_to_utf16_chars)(GDNativeConstStringPtr p_self, char16_t *r_text, GDNativeInt p_max_write_length);
	GDNativeInt (*string_to_utf32_chars)(GDNativeConstStringPtr p_self, char32_t *r_text, GDNativeInt p_max_write_length);
	GDNativeInt (*string_to_wide_chars)(GDNativeConstStringPtr p_self, wchar_t *r_text, GDNativeInt p_max_write_length);
	char32_t *(*string_operator_index)(GDNativeStringPtr p_self, GDNativeInt p_index);
	const char32_t *(*string_operator_index_const)(GDNativeConstStringPtr p_self, GDNativeInt p_index);

	/* Packed array functions */

	uint8_t *(*packed_byte_array_operator_index)(GDNativeTypePtr p_self, GDNativeInt p_index); // p_self should be a PackedByteArray
	const uint8_t *(*packed_byte_array_operator_index_const)(GDNativeConstTypePtr p_self, GDNativeInt p_index); // p_self should be a PackedByteArray

	GDNativeTypePtr (*packed_color_array_operator_index)(GDNativeTypePtr p_self, GDNativeInt p_index); // p_self should be a PackedColorArray, returns Color ptr
	GDNativeTypePtr (*packed_color_array_operator_index_const)(GDNativeConstTypePtr p_self, GDNativeInt p_index); // p_self should be a PackedColorArray, returns Color ptr

	float *(*packed_float32_array_operator_index)(GDNativeTypePtr p_self, GDNativeInt p_index); // p_self should be a PackedFloat32Array
	const float *(*packed_float32_array_operator_index_const)(GDNativeConstTypePtr p_self, GDNativeInt p_index); // p_self should be a PackedFloat32Array
	double *(*packed_float64_array_operator_index)(GDNativeTypePtr p_self, GDNativeInt p_index); // p_self should be a PackedFloat64Array
	const double *(*packed_float64_array_operator_index_const)(GDNativeConstTypePtr p_self, GDNativeInt p_index); // p_self should be a PackedFloat64Array

	int32_t *(*packed_int32_array_operator_index)(GDNativeTypePtr p_self, GDNativeInt p_index); // p_self should be a PackedInt32Array
	const int32_t *(*packed_int32_array_operator_index_const)(GDNativeConstTypePtr p_self, GDNativeInt p_index); // p_self should be a PackedInt32Array
	int64_t *(*packed_int64_array_operator_index)(GDNativeTypePtr p_self, GDNativeInt p_index); // p_self should be a PackedInt32Array
	const int64_t *(*packed_int64_array_operator_index_const)(GDNativeConstTypePtr p_self, GDNativeInt p_index); // p_self should be a PackedInt32Array

	GDNativeStringPtr (*packed_string_array_operator_index)(GDNativeTypePtr p_self, GDNativeInt p_index); // p_self should be a PackedStringArray
	GDNativeStringPtr (*packed_string_array_operator_index_const)(GDNativeConstTypePtr p_self, GDNativeInt p_index); // p_self should be a PackedStringArray

	GDNativeTypePtr (*packed_vector2_array_operator_index)(GDNativeTypePtr p_self, GDNativeInt p_index); // p_self should be a PackedVector2Array, returns Vector2 ptr
	GDNativeTypePtr (*packed_vector2_array_operator_index_const)(GDNativeConstTypePtr p_self, GDNativeInt p_index); // p_self should be a PackedVector2Array, returns Vector2 ptr
	GDNativeTypePtr (*packed_vector3_array_operator_index)(GDNativeTypePtr p_self, GDNativeInt p_index); // p_self should be a PackedVector3Array, returns Vector3 ptr
	GDNativeTypePtr (*packed_vector3_array_operator_index_const)(GDNativeConstTypePtr p_self, GDNativeInt p_index); // p_self should be a PackedVector3Array, returns Vector3 ptr

	GDNativeVariantPtr (*array_operator_index)(GDNativeTypePtr p_self, GDNativeInt p_index); // p_self should be an Array ptr
	GDNativeVariantPtr (*array_operator_index_const)(GDNativeConstTypePtr p_self, GDNativeInt p_index); // p_self should be an Array ptr

	/* Dictionary functions */

	GDNativeVariantPtr (*dictionary_operator_index)(GDNativeTypePtr p_self, GDNativeConstVariantPtr p_key); // p_self should be an Dictionary ptr
	GDNativeVariantPtr (*dictionary_operator_index_const)(GDNativeConstTypePtr p_self, GDNativeConstVariantPtr p_key); // p_self should be an Dictionary ptr

	/* OBJECT */

	void (*object_method_bind_call)(GDNativeMethodBindPtr p_method_bind, GDNativeObjectPtr p_instance, GDNativeConstVariantPtr *p_args, GDNativeInt p_arg_count, GDNativeVariantPtr r_ret, GDNativeCallError *r_error);
	void (*object_method_bind_ptrcall)(GDNativeMethodBindPtr p_method_bind, GDNativeObjectPtr p_instance, GDNativeConstTypePtr *p_args, GDNativeTypePtr r_ret);
	void (*object_destroy)(GDNativeObjectPtr p_o);
	GDNativeObjectPtr (*global_get_singleton)(GDNativeConstStringNamePtr p_name);

	void *(*object_get_instance_binding)(GDNativeObjectPtr p_o, void *p_token, const GDNativeInstanceBindingCallbacks *p_callbacks);
	void (*object_set_instance_binding)(GDNativeObjectPtr p_o, void *p_token, void *p_binding, const GDNativeInstanceBindingCallbacks *p_callbacks);

	void (*object_set_instance)(GDNativeObjectPtr p_o, GDNativeConstStringNamePtr p_classname, GDExtensionClassInstancePtr p_instance); /* p_classname should be a registered extension class and should extend the p_o object's class. */

	GDNativeObjectPtr (*object_cast_to)(GDNativeConstObjectPtr p_object, void *p_class_tag);
	GDNativeObjectPtr (*object_get_instance_from_id)(GDObjectInstanceID p_instance_id);
	GDObjectInstanceID (*object_get_instance_id)(GDNativeConstObjectPtr p_object);

	/* SCRIPT INSTANCE */

	GDNativeScriptInstancePtr (*script_instance_create)(const GDNativeExtensionScriptInstanceInfo *p_info, GDNativeExtensionScriptInstanceDataPtr p_instance_data);

	/* CLASSDB */

	GDNativeObjectPtr (*classdb_construct_object)(GDNativeConstStringNamePtr p_classname); /* The passed class must be a built-in godot class, or an already-registered extension class. In both case, object_set_instance should be called to fully initialize the object. */
	GDNativeMethodBindPtr (*classdb_get_method_bind)(GDNativeConstStringNamePtr p_classname, GDNativeConstStringNamePtr p_methodname, GDNativeInt p_hash);
	void *(*classdb_get_class_tag)(GDNativeConstStringNamePtr p_classname);

	/* CLASSDB EXTENSION */

	/* Provided parameters for `classdb_register_extension_*` can be safely freed once the function returns. */
	void (*classdb_register_extension_class)(GDNativeExtensionClassLibraryPtr p_library, GDNativeConstStringNamePtr p_class_name, GDNativeConstStringNamePtr p_parent_class_name, const GDNativeExtensionClassCreationInfo *p_extension_funcs);
	void (*classdb_register_extension_class_method)(GDNativeExtensionClassLibraryPtr p_library, GDNativeConstStringNamePtr p_class_name, const GDNativeExtensionClassMethodInfo *p_method_info);
	void (*classdb_register_extension_class_integer_constant)(GDNativeExtensionClassLibraryPtr p_library, GDNativeConstStringNamePtr p_class_name, GDNativeConstStringNamePtr p_enum_name, GDNativeConstStringNamePtr p_constant_name, GDNativeInt p_constant_value, GDNativeBool p_is_bitfield);
	void (*classdb_register_extension_class_property)(GDNativeExtensionClassLibraryPtr p_library, GDNativeConstStringNamePtr p_class_name, const GDNativePropertyInfo *p_info, GDNativeConstStringNamePtr p_setter, GDNativeConstStringNamePtr p_getter);
	void (*classdb_register_extension_class_property_group)(GDNativeExtensionClassLibraryPtr p_library, GDNativeConstStringNamePtr p_class_name, GDNativeConstStringPtr p_group_name, GDNativeConstStringPtr p_prefix);
	void (*classdb_register_extension_class_property_subgroup)(GDNativeExtensionClassLibraryPtr p_library, GDNativeConstStringNamePtr p_class_name, GDNativeConstStringPtr p_subgroup_name, GDNativeConstStringPtr p_prefix);
	void (*classdb_register_extension_class_signal)(GDNativeExtensionClassLibraryPtr p_library, GDNativeConstStringNamePtr p_class_name, GDNativeConstStringNamePtr p_signal_name, const GDNativePropertyInfo *p_argument_info, GDNativeInt p_argument_count);
	void (*classdb_unregister_extension_class)(GDNativeExtensionClassLibraryPtr p_library, GDNativeConstStringNamePtr p_class_name); /* Unregistering a parent class before a class that inherits it will result in failure. Inheritors must be unregistered first. */

	void (*get_library_path)(GDNativeExtensionClassLibraryPtr p_library, GDNativeStringPtr r_path);

} GDNativeInterface;

/* INITIALIZATION */

typedef enum {
	GDNATIVE_INITIALIZATION_CORE,
	GDNATIVE_INITIALIZATION_SERVERS,
	GDNATIVE_INITIALIZATION_SCENE,
	GDNATIVE_INITIALIZATION_EDITOR,
	GDNATIVE_MAX_INITIALIZATION_LEVEL,
} GDNativeInitializationLevel;

typedef struct {
	/* Minimum initialization level required.
	 * If Core or Servers, the extension needs editor or game restart to take effect */
	GDNativeInitializationLevel minimum_initialization_level;
	/* Up to the user to supply when initializing */
	void *userdata;
	/* This function will be called multiple times for each initialization level. */
	void (*initialize)(void *userdata, GDNativeInitializationLevel p_level);
	void (*deinitialize)(void *userdata, GDNativeInitializationLevel p_level);
} GDNativeInitialization;

/* Define a C function prototype that implements the function below and expose it to dlopen() (or similar).
 * This is the entry point of the GDExtension library and will be called on initialization.
 * It can be used to set up different init levels, which are called during various stages of initialization/shutdown.
 * The function name must be a unique one specified in the .gdextension config file.
 */
typedef GDNativeBool (*GDNativeInitializationFunction)(const GDNativeInterface *p_interface, GDNativeExtensionClassLibraryPtr p_library, GDNativeInitialization *r_initialization);

#ifdef __cplusplus
}
#endif

#endif // GDNATIVE_INTERFACE_H
