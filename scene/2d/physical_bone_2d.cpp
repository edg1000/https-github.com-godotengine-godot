/*************************************************************************/
/*  physical_bone_2d.cpp                                                 */
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

#include "physical_bone_2d.h"

void PhysicalBone2D::_notification(int p_what) {
	if (p_what == NOTIFICATION_ENTER_TREE) {
		_find_skeleton_parent();
		_find_joint_child();

		// Configure joint
		if (child_joint && auto_configure_joint) {
			_auto_configure_joint();
		}

		// Simulate physics if set
		if (simulate_physics) {
			_start_physics_simulation();
		} else {
			_stop_physics_simulation();
		}
	}

	if (p_what == NOTIFICATION_READY) {
		_position_at_bone2d();
		set_physics_process_internal(true);
	}

	if (p_what == NOTIFICATION_INTERNAL_PHYSICS_PROCESS) {
		// Position the RigidBody in the correct position
		if (follow_bone_when_simulating) {
			_position_at_bone2d();
		}

		// Keep the child joint in the correct position.
		if (child_joint && auto_configure_joint) {
			child_joint->set_global_position(get_global_position());
		}

		// Remove any collision layers and masks if we're disabled.
		// We do this so the RigidBody can still have it's layers and masks set like normal, but they will not be applied
		// unless physics are told to simulate, only making them effective when we want them to be.
		if (!_internal_simulate_physics) {
			PhysicsServer2D::get_singleton()->body_set_collision_layer(get_rid(), 0);
			PhysicsServer2D::get_singleton()->body_set_collision_mask(get_rid(), 0);
		}
	}

// Only in the editor: keep the PhysicalBone2D at the Bone2D position (if there is one) at all times.
#ifdef TOOLS_ENABLED
	if (p_what == NOTIFICATION_TRANSFORM_CHANGED) {
		if (Engine::get_singleton()->is_editor_hint()) {
			_position_at_bone2d();
		}
	}
#endif //TOOLS_ENABLED
}

void PhysicalBone2D::_position_at_bone2d() {
	// Reset to Bone2D position
	if (parent_skeleton && bone2d_index > -1) {
		if (bone2d_index <= parent_skeleton->get_bone_count()) {
			Bone2D *bone_to_use = parent_skeleton->get_bone(bone2d_index);
			set_global_transform(bone_to_use->get_global_transform());
		}
	}
}

void PhysicalBone2D::_find_skeleton_parent() {
	Node *current_parent = get_parent();

	while (current_parent != nullptr) {
		Skeleton2D *potential_skeleton = Object::cast_to<Skeleton2D>(current_parent);
		if (potential_skeleton) {
			parent_skeleton = potential_skeleton;
			break;
		} else {
			PhysicalBone2D *potential_parent_bone = Object::cast_to<PhysicalBone2D>(current_parent);
			if (potential_parent_bone) {
				current_parent = potential_parent_bone->get_parent();
			} else {
				current_parent = nullptr;
			}
		}
	}
}

void PhysicalBone2D::_find_joint_child() {
	for (int i = 0; i < get_child_count(); i++) {
		Node *child_node = get_child(i);
		Joint2D *potential_joint = Object::cast_to<Joint2D>(child_node);
		if (potential_joint) {
			child_joint = potential_joint;
			break;
		}
	}
}

String PhysicalBone2D::get_configuration_warning() const {
	String warning = Node2D::get_configuration_warning();

	if (!parent_skeleton) {
		if (warning != String()) {
			warning += "\n\n";
		}
		warning += TTR("A PhysicalBone2D only works with a Skeleton2D or another PhysicalBone2D as a parent node!");
	}

	if (parent_skeleton && bone2d_index <= -1) {
		if (warning != String()) {
			warning += "\n\n";
		}
		warning += TTR("A PhysicalBone2D needs to be assigned to a Bone2D node in order to function! Please set a Bone2D node in the inspector.");
	}

	if (!child_joint) {
		PhysicalBone2D *parent_bone = Object::cast_to<PhysicalBone2D>(get_parent());
		if (parent_bone) {
			if (warning != String()) {
				warning += "\n\n";
			}
			warning += TTR("A PhysicalBone2D node should have a Joint2D-based child node to keep bones connected! Please add a Joint2D-based node as a child to this node!");
		}
	}

	return warning;
}

void PhysicalBone2D::_auto_configure_joint() {
	if (!auto_configure_joint) {
		return;
	}

	if (child_joint) {
		// Node A = parent | Node B = this node
		Node *parent_node = get_parent();
		PhysicalBone2D *potential_parent_bone = Object::cast_to<PhysicalBone2D>(parent_node);

		if (potential_parent_bone) {
			child_joint->set_node_a(child_joint->get_path_to(potential_parent_bone));
			child_joint->set_node_b(child_joint->get_path_to(this));
		} else {
			WARN_PRINT("Cannot setup joint without a parent PhysicalBone2D node.");
		}

		// Place the child joint at this node's position.
		child_joint->set_global_position(get_global_position());
	}
}

void PhysicalBone2D::_start_physics_simulation() {
	if (_internal_simulate_physics) {
		return;
	}

	// Reset to Bone2D position
	_position_at_bone2d();

	// Let the RigidBody executing its force integration.
	if (!follow_bone_when_simulating) {
		PhysicsServer2D::get_singleton()->body_set_force_integration_callback(get_rid(), this, "_direct_state_changed");
	}

	// Apply the layers and masks
	PhysicsServer2D::get_singleton()->body_set_collision_layer(get_rid(), get_collision_layer());
	PhysicsServer2D::get_singleton()->body_set_collision_mask(get_rid(), get_collision_mask());

	_internal_simulate_physics = true;
}

void PhysicalBone2D::_stop_physics_simulation() {
	if (_internal_simulate_physics) {
		// Stop the RigidBody from executing its force integration.
		PhysicsServer2D::get_singleton()->body_set_force_integration_callback(get_rid(), nullptr, "");
		_internal_simulate_physics = false;

		// Reset to Bone2D position
		_position_at_bone2d();
	}
}

Joint2D *PhysicalBone2D::get_joint() const {
	return child_joint;
}

bool PhysicalBone2D::get_auto_configure_joint() const {
	return auto_configure_joint;
}

void PhysicalBone2D::set_auto_configure_joint(bool p_auto_configure) {
	auto_configure_joint = p_auto_configure;
	_auto_configure_joint();
}

void PhysicalBone2D::set_simulate_physics(bool p_simulate) {
	if (p_simulate == simulate_physics) {
		return;
	}
	simulate_physics = p_simulate;

	if (simulate_physics) {
		_start_physics_simulation();
	} else {
		_stop_physics_simulation();
	}
}

bool PhysicalBone2D::get_simulate_physics() const {
	return simulate_physics;
}

bool PhysicalBone2D::is_simulating_physics() const {
	return _internal_simulate_physics;
}

void PhysicalBone2D::set_bone2d_nodepath(const NodePath &p_nodepath) {
	bone2d_nodepath = p_nodepath;
	_change_notify();
}

NodePath PhysicalBone2D::get_bone2d_nodepath() const {
	return bone2d_nodepath;
}

void PhysicalBone2D::set_bone2d_index(int p_bone_idx) {
	ERR_FAIL_COND_MSG(p_bone_idx < 0, "Bone index is out of range: The index is too low!");

	if (!is_inside_tree()) {
		bone2d_index = p_bone_idx;
		return;
	}

	if (parent_skeleton) {
		ERR_FAIL_INDEX_MSG(p_bone_idx, parent_skeleton->get_bone_count(), "Passed-in Bone index is out of range!");
		bone2d_index = p_bone_idx;

		bone2d_nodepath = get_path_to(parent_skeleton->get_bone(bone2d_index));
	} else {
		WARN_PRINT("Cannot verify bone index...");
		bone2d_index = p_bone_idx;
	}

	_change_notify();
}

int PhysicalBone2D::get_bone2d_index() const {
	return bone2d_index;
}

void PhysicalBone2D::set_follow_bone_when_simulating(bool p_follow_bone) {
	follow_bone_when_simulating = p_follow_bone;

	if (_internal_simulate_physics) {
		_stop_physics_simulation();
		_start_physics_simulation();
	}
}

bool PhysicalBone2D::get_follow_bone_when_simulating() const {
	return follow_bone_when_simulating;
}

void PhysicalBone2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_joint"), &PhysicalBone2D::get_joint);
	ClassDB::bind_method(D_METHOD("get_auto_configure_joint"), &PhysicalBone2D::get_auto_configure_joint);
	ClassDB::bind_method(D_METHOD("set_auto_configure_joint", "auto_configure_joint"), &PhysicalBone2D::set_auto_configure_joint);

	ClassDB::bind_method(D_METHOD("set_simulate_physics", "simulate_physics"), &PhysicalBone2D::set_simulate_physics);
	ClassDB::bind_method(D_METHOD("get_simulate_physics"), &PhysicalBone2D::get_simulate_physics);
	ClassDB::bind_method(D_METHOD("is_simulating_physics"), &PhysicalBone2D::is_simulating_physics);

	ClassDB::bind_method(D_METHOD("set_bone2d_nodepath", "nodepath"), &PhysicalBone2D::set_bone2d_nodepath);
	ClassDB::bind_method(D_METHOD("get_bone2d_nodepath"), &PhysicalBone2D::get_bone2d_nodepath);
	ClassDB::bind_method(D_METHOD("set_bone2d_index", "bone_index"), &PhysicalBone2D::set_bone2d_index);
	ClassDB::bind_method(D_METHOD("get_bone2d_index"), &PhysicalBone2D::get_bone2d_index);
	ClassDB::bind_method(D_METHOD("set_follow_bone_when_simulating", "follow_bone"), &PhysicalBone2D::set_follow_bone_when_simulating);
	ClassDB::bind_method(D_METHOD("get_follow_bone_when_simulating"), &PhysicalBone2D::get_follow_bone_when_simulating);

	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "bone2d_nodepath", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Bone2D"), "set_bone2d_nodepath", "get_bone2d_nodepath");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "bone2d_index", PROPERTY_HINT_RANGE, "-1, 1000, 1"), "set_bone2d_index", "get_bone2d_index");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_configure_joint"), "set_auto_configure_joint", "get_auto_configure_joint");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "simulate_physics"), "set_simulate_physics", "get_simulate_physics");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "follow_bone_when_simulating"), "set_follow_bone_when_simulating", "get_follow_bone_when_simulating");
}

PhysicalBone2D::PhysicalBone2D() {
	// Stop the RigidBody from executing its force integration.
	PhysicsServer2D::get_singleton()->body_set_force_integration_callback(get_rid(), nullptr, "");
	child_joint = nullptr;
}

PhysicalBone2D::~PhysicalBone2D() {
}
