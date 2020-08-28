/*************************************************************************/
/*  shader_globals_override.cpp                                          */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
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

#include "shader_globals_override.h"

#include "core/core_string_names.h"
#include "scene/main/window.h"
#include "scene/scene_string_names.h"

StringName *ShaderGlobalsOverride::_remap(const StringName &p_name) const {
	StringName *r = param_remaps.getptr(p_name);
	if (!r) {
		//not cached, do caching
		String p = p_name;
		if (p.begins_with("params/")) {
			String q = p.replace_first("params/", "");
			param_remaps[p] = q;
			r = param_remaps.getptr(q);
		}
	}

	return r;
}

bool ShaderGlobalsOverride::_set(const StringName &p_name, const Variant &p_value) {
	StringName *r = _remap(p_name);

	if (r) {
		Override *o = overrides.getptr(*r);
		if (!o) {
			Override ov;
			ov.in_use = false;
			overrides[*r] = ov;
			o = overrides.getptr(*r);
		}
		if (o) {
			o->override = p_value;
			if (active) {
				RS::get_singleton()->global_variable_set_override(*r, p_value);
			}
			o->in_use = p_value.get_type() != Variant::NIL;
			return true;
		}
	}

	return false;
}

bool ShaderGlobalsOverride::_get(const StringName &p_name, Variant &r_ret) const {
	StringName *r = _remap(p_name);

	if (r) {
		const Override *o = overrides.getptr(*r);
		if (o) {
			r_ret = o->override;
			return true;
		}
	}

	return false;
}

void ShaderGlobalsOverride::_get_property_list(List<PropertyInfo> *p_list) const {
	Vector<StringName> variables;
	variables = RS::get_singleton()->global_variable_get_list();
	for (int i = 0; i < variables.size(); i++) {
		PropertyInfo pinfo;
		pinfo.name = "params/" + variables[i];
		pinfo.usage = PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_CHECKABLE;

		switch (RS::get_singleton()->global_variable_get_type(variables[i])) {
			case RS::GLOBAL_VAR_TYPE_BOOL: {
				pinfo.type = Variant::BOOL;
			} break;
			case RS::GLOBAL_VAR_TYPE_BVEC2: {
				pinfo.type = Variant::INT;
				pinfo.hint = PROPERTY_HINT_FLAGS;
				pinfo.hint_string = "x,y";
			} break;
			case RS::GLOBAL_VAR_TYPE_BVEC3: {
				pinfo.type = Variant::INT;
				pinfo.hint = PROPERTY_HINT_FLAGS;
				pinfo.hint_string = "x,y,z";
			} break;
			case RS::GLOBAL_VAR_TYPE_BVEC4: {
				pinfo.type = Variant::INT;
				pinfo.hint = PROPERTY_HINT_FLAGS;
				pinfo.hint_string = "x,y,z,w";
			} break;
			case RS::GLOBAL_VAR_TYPE_INT: {
				pinfo.type = Variant::INT;
			} break;
			case RS::GLOBAL_VAR_TYPE_IVEC2: {
				pinfo.type = Variant::VECTOR2I;
			} break;
			case RS::GLOBAL_VAR_TYPE_IVEC3: {
				pinfo.type = Variant::VECTOR3I;
			} break;
			case RS::GLOBAL_VAR_TYPE_IVEC4: {
				pinfo.type = Variant::PACKED_INT32_ARRAY;
			} break;
			case RS::GLOBAL_VAR_TYPE_RECT2I: {
				pinfo.type = Variant::RECT2I;
			} break;
			case RS::GLOBAL_VAR_TYPE_UINT: {
				pinfo.type = Variant::INT;
			} break;
			case RS::GLOBAL_VAR_TYPE_UVEC2: {
				pinfo.type = Variant::VECTOR2I;
			} break;
			case RS::GLOBAL_VAR_TYPE_UVEC3: {
				pinfo.type = Variant::VECTOR3I;
			} break;
			case RS::GLOBAL_VAR_TYPE_UVEC4: {
				pinfo.type = Variant::PACKED_INT32_ARRAY;
			} break;
			case RS::GLOBAL_VAR_TYPE_FLOAT: {
				pinfo.type = Variant::FLOAT;
			} break;
			case RS::GLOBAL_VAR_TYPE_VEC2: {
				pinfo.type = Variant::VECTOR2;
			} break;
			case RS::GLOBAL_VAR_TYPE_VEC3: {
				pinfo.type = Variant::VECTOR3;
			} break;
			case RS::GLOBAL_VAR_TYPE_VEC4: {
				pinfo.type = Variant::PLANE;
			} break;
			case RS::GLOBAL_VAR_TYPE_RECT2: {
				pinfo.type = Variant::RECT2;
			} break;
			case RS::GLOBAL_VAR_TYPE_COLOR: {
				pinfo.type = Variant::COLOR;
			} break;
			case RS::GLOBAL_VAR_TYPE_MAT2: {
				pinfo.type = Variant::PACKED_INT32_ARRAY;
			} break;
			case RS::GLOBAL_VAR_TYPE_MAT3: {
				pinfo.type = Variant::BASIS;
			} break;
			case RS::GLOBAL_VAR_TYPE_TRANSFORM_2D: {
				pinfo.type = Variant::TRANSFORM2D;
			} break;
			case RS::GLOBAL_VAR_TYPE_TRANSFORM: {
				pinfo.type = Variant::TRANSFORM;
			} break;
			case RS::GLOBAL_VAR_TYPE_MAT4: {
				pinfo.type = Variant::PACKED_INT32_ARRAY;
			} break;
			case RS::GLOBAL_VAR_TYPE_SAMPLER2D: {
				pinfo.type = Variant::OBJECT;
				pinfo.hint = PROPERTY_HINT_RESOURCE_TYPE;
				pinfo.hint_string = "Texture2D";
			} break;
			case RS::GLOBAL_VAR_TYPE_SAMPLER2DARRAY: {
				pinfo.type = Variant::OBJECT;
				pinfo.hint = PROPERTY_HINT_RESOURCE_TYPE;
				pinfo.hint_string = "Texture2DArray";
			} break;
			case RS::GLOBAL_VAR_TYPE_SAMPLER3D: {
				pinfo.type = Variant::OBJECT;
				pinfo.hint = PROPERTY_HINT_RESOURCE_TYPE;
				pinfo.hint_string = "Texture3D";
			} break;
			case RS::GLOBAL_VAR_TYPE_SAMPLERCUBE: {
				pinfo.type = Variant::OBJECT;
				pinfo.hint = PROPERTY_HINT_RESOURCE_TYPE;
				pinfo.hint_string = "Cubemap";
			} break;
			default: {
			} break;
		}

		if (!overrides.has(variables[i])) {
			Override o;
			o.in_use = false;
			Callable::CallError ce;
			o.override = Variant::construct(pinfo.type, nullptr, 0, ce);
			overrides[variables[i]] = o;
		}

		Override *o = overrides.getptr(variables[i]);
		if (o->in_use && o->override.get_type() != Variant::NIL) {
			pinfo.usage |= PROPERTY_USAGE_CHECKED;
			pinfo.usage |= PROPERTY_USAGE_STORAGE;
		}

		p_list->push_back(pinfo);
	}
}

void ShaderGlobalsOverride::_activate() {
	List<Node *> nodes;
	get_tree()->get_nodes_in_group(SceneStringNames::get_singleton()->shader_overrides_group_active, &nodes);
	if (nodes.size() == 0) {
		//good we are the only override, enable all
		active = true;
		add_to_group(SceneStringNames::get_singleton()->shader_overrides_group_active);

		const StringName *K = nullptr;
		while ((K = overrides.next(K))) {
			Override *o = overrides.getptr(*K);
			if (o->in_use && o->override.get_type() != Variant::NIL) {
				RS::get_singleton()->global_variable_set_override(*K, o->override);
			}
		}

		update_configuration_warning(); //may have activated
	}
}

void ShaderGlobalsOverride::_notification(int p_what) {
	if (p_what == Node3D::NOTIFICATION_ENTER_TREE) {
		add_to_group(SceneStringNames::get_singleton()->shader_overrides_group);
		_activate();

	} else if (p_what == Node3D::NOTIFICATION_EXIT_TREE) {
		if (active) {
			//remove overrides
			const StringName *K = nullptr;
			while ((K = overrides.next(K))) {
				Override *o = overrides.getptr(*K);
				if (o->in_use) {
					RS::get_singleton()->global_variable_set_override(*K, Variant());
				}
			}
		}

		remove_from_group(SceneStringNames::get_singleton()->shader_overrides_group_active);
		remove_from_group(SceneStringNames::get_singleton()->shader_overrides_group);
		get_tree()->call_group(SceneStringNames::get_singleton()->shader_overrides_group, "_activate"); //another may want to activate when this is removed
		active = false;
	}
}

String ShaderGlobalsOverride::get_configuration_warning() const {
	if (!active) {
		return TTR("ShaderGlobalsOverride is not active because another node of the same type is in the scene.");
	}

	return String();
}

void ShaderGlobalsOverride::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_activate"), &ShaderGlobalsOverride::_activate);
}

ShaderGlobalsOverride::ShaderGlobalsOverride() {
	active = false;
}
