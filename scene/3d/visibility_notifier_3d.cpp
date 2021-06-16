/*************************************************************************/
/*  visibility_notifier_3d.cpp                                           */
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

#include "visibility_notifier_3d.h"

#include "core/config/engine.h"
#include "scene/3d/camera_3d.h"
#include "scene/3d/physics_body_3d.h"
#include "scene/animation/animation_player.h"
#include "scene/scene_string_names.h"

void VisibilityNotifier3D::_visibility_enter() {
	if (!is_inside_tree() || Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	on_screen = true;
	emit_signal(SceneStringNames::get_singleton()->screen_entered);
	_screen_enter();
}
void VisibilityNotifier3D::_visibility_exit() {
	if (!is_inside_tree() || Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	on_screen = false;
	emit_signal(SceneStringNames::get_singleton()->screen_exited);
	_screen_exit();
}

void VisibilityNotifier3D::set_aabb(const AABB &p_aabb) {
	if (aabb == p_aabb) {
		return;
	}
	aabb = p_aabb;

	RS::get_singleton()->visibility_notifier_set_aabb(get_base(), aabb);

	update_gizmo();
}

AABB VisibilityNotifier3D::get_aabb() const {
	return aabb;
}

bool VisibilityNotifier3D::is_on_screen() const {
	return on_screen;
}

void VisibilityNotifier3D::_notification(int p_what) {
	if (p_what == NOTIFICATION_ENTER_TREE || p_what == NOTIFICATION_EXIT_TREE) {
		on_screen = false;
	}
}

void VisibilityNotifier3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_aabb", "rect"), &VisibilityNotifier3D::set_aabb);
	ClassDB::bind_method(D_METHOD("is_on_screen"), &VisibilityNotifier3D::is_on_screen);

	ADD_PROPERTY(PropertyInfo(Variant::AABB, "aabb"), "set_aabb", "get_aabb");

	ADD_SIGNAL(MethodInfo("screen_entered"));
	ADD_SIGNAL(MethodInfo("screen_exited"));
}

Vector<Face3> VisibilityNotifier3D::get_faces(uint32_t p_usage_flags) const {
	return Vector<Face3>();
}

VisibilityNotifier3D::VisibilityNotifier3D() {
	RID notifier = RS::get_singleton()->visibility_notifier_create();
	RS::get_singleton()->visibility_notifier_set_aabb(notifier, aabb);
	RS::get_singleton()->visibility_notifier_set_callbacks(notifier, callable_mp(this, &VisibilityNotifier3D::_visibility_enter), callable_mp(this, &VisibilityNotifier3D::_visibility_exit));
	set_base(notifier);
}

//////////////////////////////////////

void VisibilityEnabler3D::_screen_enter() {
	_update_enable_mode(true);
}

void VisibilityEnabler3D::_screen_exit() {
	_update_enable_mode(false);
}

void VisibilityEnabler3D::set_enable_mode(EnableMode p_mode) {
	enable_mode = p_mode;
	if (is_inside_tree()) {
		_update_enable_mode(is_on_screen());
	}
}
VisibilityEnabler3D::EnableMode VisibilityEnabler3D::get_enable_mode() {
	return enable_mode;
}

void VisibilityEnabler3D::set_enable_node_path(NodePath p_path) {
	if (enable_node_path == p_path) {
		return;
	}
	enable_node_path = p_path;
	if (is_inside_tree()) {
		node_id = ObjectID();
		Node *node = get_node(enable_node_path);
		if (node) {
			node_id = node->get_instance_id();
			_update_enable_mode(is_on_screen());
		}
	}
}
NodePath VisibilityEnabler3D::get_enable_node_path() {
	return enable_node_path;
}

void VisibilityEnabler3D::_update_enable_mode(bool p_enable) {
	Node *node = static_cast<Node *>(ObjectDB::get_instance(node_id));
	if (node) {
		if (p_enable) {
			switch (enable_mode) {
				case ENABLE_MODE_INHERIT: {
					node->set_process_mode(PROCESS_MODE_INHERIT);
				} break;
				case ENABLE_MODE_ALWAYS: {
					node->set_process_mode(PROCESS_MODE_ALWAYS);
				} break;
				case ENABLE_MODE_WHEN_PAUSED: {
					node->set_process_mode(PROCESS_MODE_WHEN_PAUSED);
				} break;
			}
		} else {
			node->set_process_mode(PROCESS_MODE_DISABLED);
		}
	}
}
void VisibilityEnabler3D::_notification(int p_what) {
	if (p_what == NOTIFICATION_ENTER_TREE) {
		if (Engine::get_singleton()->is_editor_hint()) {
			return;
		}

		node_id = ObjectID();
		Node *node = get_node(enable_node_path);
		if (node) {
			node_id = node->get_instance_id();
			node->set_process_mode(PROCESS_MODE_DISABLED);
		}
	}

	if (p_what == NOTIFICATION_EXIT_TREE) {
		node_id = ObjectID();
	}
}

void VisibilityEnabler3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_enable_mode", "mode"), &VisibilityEnabler3D::set_enable_mode);
	ClassDB::bind_method(D_METHOD("get_enable_mode"), &VisibilityEnabler3D::get_enable_mode);

	ClassDB::bind_method(D_METHOD("set_enable_node_path", "path"), &VisibilityEnabler3D::set_enable_node_path);
	ClassDB::bind_method(D_METHOD("get_enable_node_path"), &VisibilityEnabler3D::get_enable_node_path);

	ADD_GROUP("Enabling", "enable_");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "enable_mode", PROPERTY_HINT_ENUM, "Inherit,Always,WhenPaused"), "set_enable_mode", "get_enable_mode");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "enable_node_path"), "set_enable_node_path", "get_enable_node_path");

	BIND_ENUM_CONSTANT(ENABLE_MODE_INHERIT);
	BIND_ENUM_CONSTANT(ENABLE_MODE_ALWAYS);
	BIND_ENUM_CONSTANT(ENABLE_MODE_WHEN_PAUSED);
}

VisibilityEnabler3D::VisibilityEnabler3D() {
}
