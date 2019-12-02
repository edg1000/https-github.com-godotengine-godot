/*************************************************************************/
/*  gd_mono_utils.h                                                      */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md)    */
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

#pragma once

#include <mono/metadata/threads.h>

#include "../mono_gc_handle.h"
#include "../utils/macros.h"
#include "../utils/thread_local.h"
#include "gd_mono_header.h"

#include "core/object.h"
#include "core/reference.h"

#define UNHANDLED_EXCEPTION(m_exc)                     \
	if (unlikely(m_exc != NULL)) {                     \
		GDMonoUtils::debug_unhandled_exception(m_exc); \
		GD_UNREACHABLE();                              \
	}

namespace GDMonoUtils {

namespace Marshal {

bool type_is_generic_array(MonoReflectionType *p_reftype);
bool type_is_generic_dictionary(MonoReflectionType *p_reftype);

void array_get_element_type(MonoReflectionType *p_array_reftype, MonoReflectionType **r_elem_reftype);
void dictionary_get_key_value_types(MonoReflectionType *p_dict_reftype, MonoReflectionType **r_key_reftype, MonoReflectionType **r_value_reftype);

bool generic_ienumerable_is_assignable_from(MonoReflectionType *p_reftype);
bool generic_idictionary_is_assignable_from(MonoReflectionType *p_reftype);
bool generic_ienumerable_is_assignable_from(MonoReflectionType *p_reftype, MonoReflectionType **r_elem_reftype);
bool generic_idictionary_is_assignable_from(MonoReflectionType *p_reftype, MonoReflectionType **r_key_reftype, MonoReflectionType **r_value_reftype);

GDMonoClass *make_generic_array_type(MonoReflectionType *p_elem_reftype);
GDMonoClass *make_generic_dictionary_type(MonoReflectionType *p_key_reftype, MonoReflectionType *p_value_reftype);

Array enumerable_to_array(MonoObject *p_enumerable);
Dictionary idictionary_to_dictionary(MonoObject *p_idictionary);
Dictionary generic_idictionary_to_dictionary(MonoObject *p_generic_idictionary);

} // namespace Marshal

_FORCE_INLINE_ void hash_combine(uint32_t &p_hash, const uint32_t &p_with_hash) {
	p_hash ^= p_with_hash + 0x9e3779b9 + (p_hash << 6) + (p_hash >> 2);
}

/**
 * If the object has a csharp script, returns the target of the gchandle stored in the script instance
 * Otherwise returns a newly constructed MonoObject* which is attached to the object
 * Returns NULL on error
 */
MonoObject *unmanaged_get_managed(Object *unmanaged);

void set_main_thread(MonoThread *p_thread);
void attach_current_thread();
void detach_current_thread();
MonoThread *get_current_thread();

_FORCE_INLINE_ bool is_main_thread() {
	return mono_domain_get() != NULL && mono_thread_get_main() == mono_thread_current();
}

void runtime_object_init(MonoObject *p_this_obj, GDMonoClass *p_class, MonoException **r_exc = NULL);

GDMonoClass *get_object_class(MonoObject *p_object);
GDMonoClass *type_get_proxy_class(const StringName &p_type);
GDMonoClass *get_class_native_base(GDMonoClass *p_class);

MonoObject *create_managed_for_godot_object(GDMonoClass *p_class, const StringName &p_native, Object *p_object);

MonoObject *create_managed_from(const NodePath &p_from);
MonoObject *create_managed_from(const RID &p_from);
MonoObject *create_managed_from(const Array &p_from, GDMonoClass *p_class);
MonoObject *create_managed_from(const Dictionary &p_from, GDMonoClass *p_class);

MonoDomain *create_domain(const String &p_friendly_name);

String get_exception_name_and_message(MonoException *p_exc);
void set_exception_message(MonoException *p_exc, String message);

void debug_print_unhandled_exception(MonoException *p_exc);
void debug_send_unhandled_exception_error(MonoException *p_exc);
void debug_unhandled_exception(MonoException *p_exc);
void print_unhandled_exception(MonoException *p_exc);

/**
 * Sets the exception as pending. The exception will be thrown when returning to managed code.
 * If no managed method is being invoked by the runtime, the exception will be treated as
 * an unhandled exception and the method will not return.
 */
void set_pending_exception(MonoException *p_exc);

extern _THREAD_LOCAL_(int) current_invoke_count;

_FORCE_INLINE_ int get_runtime_invoke_count() {
	return current_invoke_count;
}
_FORCE_INLINE_ int &get_runtime_invoke_count_ref() {
	return current_invoke_count;
}

MonoObject *runtime_invoke(MonoMethod *p_method, void *p_obj, void **p_params, MonoException **r_exc);
MonoObject *runtime_invoke_array(MonoMethod *p_method, void *p_obj, MonoArray *p_params, MonoException **r_exc);

MonoString *object_to_string(MonoObject *p_obj, MonoException **r_exc);

void property_set_value(MonoProperty *p_prop, void *p_obj, void **p_params, MonoException **r_exc);
MonoObject *property_get_value(MonoProperty *p_prop, void *p_obj, void **p_params, MonoException **r_exc);

uint64_t unbox_enum_value(MonoObject *p_boxed, MonoType *p_enum_basetype, bool &r_error);

void dispose(MonoObject *p_mono_object, MonoException **r_exc);

} // namespace GDMonoUtils

#define NATIVE_GDMONOCLASS_NAME(m_class) (GDMonoMarshal::mono_string_to_godot((MonoString *)m_class->get_field(BINDINGS_NATIVE_NAME_FIELD)->get_value(NULL)))

#define GD_MONO_BEGIN_RUNTIME_INVOKE                                              \
	int &_runtime_invoke_count_ref = GDMonoUtils::get_runtime_invoke_count_ref(); \
	_runtime_invoke_count_ref += 1;

#define GD_MONO_END_RUNTIME_INVOKE \
	_runtime_invoke_count_ref -= 1;
