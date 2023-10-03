/**************************************************************************/
/*  gdscript_static_callable.cpp                                          */
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

#include "gdscript_static_callable.h"

#include "gdscript.h"

#include "core/templates/hashfuncs.h"

bool GDScriptStaticCallable::compare_equal(const CallableCustom *p_a, const CallableCustom *p_b) {
	return p_a->hash() == p_b->hash();
}

bool GDScriptStaticCallable::compare_less(const CallableCustom *p_a, const CallableCustom *p_b) {
	return p_a->hash() < p_b->hash();
}

uint32_t GDScriptStaticCallable::hash() const {
	return h;
}

String GDScriptStaticCallable::get_as_text() const {
	String class_name = script->get_class();
	if (script->get_path().is_resource_file()) {
		class_name += "(" + script->get_path().get_file() + ")";
	}
	return class_name + "::" + String(method);
}

CallableCustom::CompareEqualFunc GDScriptStaticCallable::get_compare_equal_func() const {
	return compare_equal;
}

CallableCustom::CompareLessFunc GDScriptStaticCallable::get_compare_less_func() const {
	return compare_less;
}

bool GDScriptStaticCallable::is_valid() const {
	return script.is_valid() && script->has_method(method);
}

ObjectID GDScriptStaticCallable::get_object() const {
	return script->get_instance_id();
}

StringName GDScriptStaticCallable::get_method() const {
	return method;
}

void GDScriptStaticCallable::call(const Variant **p_arguments, int p_argcount, Variant &r_return_value, Callable::CallError &r_call_error) const {
	r_return_value = script->callp(method, p_arguments, p_argcount, r_call_error);
}

GDScriptStaticCallable::GDScriptStaticCallable(const Ref<GDScript> &p_script, const StringName &p_method) {
	script = p_script;
	method = p_method;
	h = method.hash();
	h = hash_murmur3_one_64(script->get_instance_id(), h);
}
