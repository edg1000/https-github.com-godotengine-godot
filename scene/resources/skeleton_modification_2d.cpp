/*************************************************************************/
/*  skeleton_modification_2d.cpp                                         */
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

#include "skeleton_modification_2d.h"
#include "scene/2d/skeleton_2d.h"

#include "scene/2d/collision_object_2d.h"
#include "scene/2d/collision_shape_2d.h"
#include "scene/2d/physical_bone_2d.h"

#ifdef TOOLS_ENABLED
#include "editor/editor_settings.h"
#endif // TOOLS_ENABLED

///////////////////////////////////////
// Modification2D
///////////////////////////////////////

void SkeletonModification2D::_execute(float delta) {
	call("_execute", delta);

	if (!enabled)
		return;
}

void SkeletonModification2D::_setup_modification(SkeletonModificationStack2D *p_stack) {
	stack = p_stack;
	if (stack) {
		is_setup = true;
	} else {
		WARN_PRINT("Could not setup modification with name " + get_name());
	}

	call("_setup_modification", p_stack);
}

void SkeletonModification2D::_draw_editor_gizmo() {
	call("_draw_editor_gizmo");
}

bool SkeletonModification2D::_print_execution_error(bool p_condition, String p_message) {
	if (!is_setup) {
		return p_condition;
	}

	if (p_condition) {
		ERR_PRINT_ONCE(p_message);
	}
	return p_condition;
}

void SkeletonModification2D::set_enabled(bool p_enabled) {
	enabled = p_enabled;

#ifdef TOOLS_ENABLED
	if (editor_draw_gizmo) {
		if (stack) {
			stack->set_editor_gizmos_dirty(true);
		}
	}
#endif // TOOLS_ENABLED
}

bool SkeletonModification2D::get_enabled() {
	return enabled;
}

float SkeletonModification2D::clamp_angle(float angle, float min_bound, float max_bound, bool invert) {
	// Map to the 0 to 360 range (in radians though) instead of the -180 to 180 range.
	if (angle < 0) {
		angle = Math_TAU + angle;
	}

	// Make min and max in the range of 0 to 360 (in radians), and make sure they are in the right order
	if (min_bound < 0) {
		min_bound = Math_TAU + min_bound;
	}
	if (max_bound < 0) {
		max_bound = Math_TAU + max_bound;
	}
	if (min_bound > max_bound) {
		float tmp = min_bound;
		min_bound = max_bound;
		max_bound = tmp;
	}

	// Note: May not be the most optimal way to clamp, but it always constraints to the nearest angle.
	if (invert == false) {
		if (angle < min_bound || angle > max_bound) {
			Vector2 min_bound_vec = Vector2(Math::cos(min_bound), Math::sin(min_bound));
			Vector2 max_bound_vec = Vector2(Math::cos(max_bound), Math::sin(max_bound));
			Vector2 angle_vec = Vector2(Math::cos(angle), Math::sin(angle));

			if (angle_vec.distance_squared_to(min_bound_vec) <= angle_vec.distance_squared_to(max_bound_vec)) {
				angle = min_bound;
			} else {
				angle = max_bound;
			}
		}
	} else {
		if (angle > min_bound && angle < max_bound) {
			Vector2 min_bound_vec = Vector2(Math::cos(min_bound), Math::sin(min_bound));
			Vector2 max_bound_vec = Vector2(Math::cos(max_bound), Math::sin(max_bound));
			Vector2 angle_vec = Vector2(Math::cos(angle), Math::sin(angle));

			if (angle_vec.distance_squared_to(min_bound_vec) <= angle_vec.distance_squared_to(max_bound_vec)) {
				angle = min_bound;
			} else {
				angle = max_bound;
			}
		}
	}
	return angle;
}

void SkeletonModification2D::editor_draw_angle_constraints(Bone2D *operation_bone, float min_bound, float max_bound,
		bool constraint_enabled, bool constraint_in_localspace, bool constraint_inverted) {
	if (!operation_bone) {
		return;
	}

	Color bone_ik_color = Color(1.0, 0.65, 0.0, 0.4);
#ifdef TOOLS_ENABLED
	if (Engine::get_singleton()->is_editor_hint()) {
		bone_ik_color = EditorSettings::get_singleton()->get("editors/2d/bone_ik_color");
	}
#endif // TOOLS_ENABLED

	float arc_angle_min = min_bound;
	float arc_angle_max = max_bound;
	if (arc_angle_min < 0) {
		arc_angle_min = (Math_PI * 2) + arc_angle_min;
	}
	if (arc_angle_max < 0) {
		arc_angle_max = (Math_PI * 2) + arc_angle_max;
	}
	if (arc_angle_min > arc_angle_max) {
		float tmp = arc_angle_min;
		arc_angle_min = arc_angle_max;
		arc_angle_max = tmp;
	}
	arc_angle_min += operation_bone->get_bone_angle();
	arc_angle_max += operation_bone->get_bone_angle();

	if (constraint_enabled) {
		if (constraint_in_localspace) {
			Node *operation_bone_parent = operation_bone->get_parent();
			Bone2D *operation_bone_parent_bone = Object::cast_to<Bone2D>(operation_bone_parent);

			if (operation_bone_parent_bone) {
				stack->skeleton->draw_set_transform(
						stack->skeleton->get_global_transform().affine_inverse().xform(operation_bone->get_global_position()),
						operation_bone_parent_bone->get_global_rotation() - stack->skeleton->get_global_rotation());
			} else {
				stack->skeleton->draw_set_transform(stack->skeleton->get_global_transform().affine_inverse().xform(operation_bone->get_global_position()));
			}
		} else {
			stack->skeleton->draw_set_transform(stack->skeleton->get_global_transform().affine_inverse().xform(operation_bone->get_global_position()));
		}

		if (constraint_inverted) {
			stack->skeleton->draw_arc(Vector2(0, 0), operation_bone->get_length(),
					arc_angle_min + (Math_PI * 2), arc_angle_max, 32, bone_ik_color, 1.0);
		} else {
			stack->skeleton->draw_arc(Vector2(0, 0), operation_bone->get_length(),
					arc_angle_min, arc_angle_max, 32, bone_ik_color, 1.0);
		}
		stack->skeleton->draw_line(Vector2(0, 0), Vector2(Math::cos(arc_angle_min), Math::sin(arc_angle_min)) * operation_bone->get_length(), bone_ik_color, 1.0);
		stack->skeleton->draw_line(Vector2(0, 0), Vector2(Math::cos(arc_angle_max), Math::sin(arc_angle_max)) * operation_bone->get_length(), bone_ik_color, 1.0);

	} else {
		stack->skeleton->draw_set_transform(stack->skeleton->get_global_transform().affine_inverse().xform(operation_bone->get_global_position()));
		stack->skeleton->draw_arc(Vector2(0, 0), operation_bone->get_length(), 0, Math_PI * 2, 32, bone_ik_color, 1.0);
		stack->skeleton->draw_line(Vector2(0, 0), Vector2(1, 0) * operation_bone->get_length(), bone_ik_color, 1.0);
	}
}

Ref<SkeletonModificationStack2D> SkeletonModification2D::get_modification_stack() {
	return stack;
}

void SkeletonModification2D::set_is_setup(bool p_setup) {
	is_setup = p_setup;
}

bool SkeletonModification2D::get_is_setup() const {
	return is_setup;
}

void SkeletonModification2D::set_execution_mode(int p_mode) {
	execution_mode = p_mode;
}

int SkeletonModification2D::get_execution_mode() const {
	return execution_mode;
}

void SkeletonModification2D::set_editor_draw_gizmo(bool p_draw_gizmo) {
	editor_draw_gizmo = p_draw_gizmo;
#ifdef TOOLS_ENABLED
	if (is_setup) {
		stack->set_editor_gizmos_dirty(true);
	}
#endif // TOOLS_ENABLED
}

bool SkeletonModification2D::get_editor_draw_gizmo() const {
	return editor_draw_gizmo;
}

void SkeletonModification2D::_bind_methods() {
	BIND_VMETHOD(MethodInfo("_execute", PropertyInfo(Variant::FLOAT, "delta")));
	BIND_VMETHOD(MethodInfo("_setup_modification", PropertyInfo(Variant::OBJECT, "modification_stack", PROPERTY_HINT_RESOURCE_TYPE, "SkeletonModificationStack2D")));
	BIND_VMETHOD(MethodInfo("_draw_editor_gizmo"));

	ClassDB::bind_method(D_METHOD("set_enabled", "enabled"), &SkeletonModification2D::set_enabled);
	ClassDB::bind_method(D_METHOD("get_enabled"), &SkeletonModification2D::get_enabled);
	ClassDB::bind_method(D_METHOD("get_modification_stack"), &SkeletonModification2D::get_modification_stack);
	ClassDB::bind_method(D_METHOD("set_is_setup", "is_setup"), &SkeletonModification2D::set_is_setup);
	ClassDB::bind_method(D_METHOD("get_is_setup"), &SkeletonModification2D::get_is_setup);
	ClassDB::bind_method(D_METHOD("set_execution_mode", "execution_mode"), &SkeletonModification2D::set_execution_mode);
	ClassDB::bind_method(D_METHOD("get_execution_mode"), &SkeletonModification2D::get_execution_mode);
	ClassDB::bind_method(D_METHOD("clamp_angle", "angle", "min", "max", "invert"), &SkeletonModification2D::clamp_angle);
	ClassDB::bind_method(D_METHOD("set_editor_draw_gizmo", "draw_gizmo"), &SkeletonModification2D::set_editor_draw_gizmo);
	ClassDB::bind_method(D_METHOD("get_editor_draw_gizmo"), &SkeletonModification2D::get_editor_draw_gizmo);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enabled"), "set_enabled", "get_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "execution_mode", PROPERTY_HINT_ENUM, "process, physics_process"), "set_execution_mode", "get_execution_mode");
}

SkeletonModification2D::SkeletonModification2D() {
	stack = nullptr;
	is_setup = false;
}
