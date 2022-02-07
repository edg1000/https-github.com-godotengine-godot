/*************************************************************************/
/*  camera_3d_editor_plugin.cpp                                          */
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

#include "camera_3d_editor_plugin.h"

#include "core/math/math_defs.h"
#include "core/object/callable_method_pointer.h"
#include "core/os/memory.h"
#include "node_3d_editor_plugin.h"
#include "scene/3d/camera_3d.h"
#include "scene/gui/button.h"

class EditorNode;
class Node;

void Camera3DEditor::_node_removed(Node *p_node) {
	if (p_node == node) {
		node = nullptr;
		Node3DEditor::get_singleton()->set_custom_camera(nullptr);
		hide();
	}
}

void Camera3DEditor::_pressed() {
	Node *sn = (node && preview->is_pressed()) ? node : nullptr;
	Node3DEditor::get_singleton()->set_custom_camera(sn);
}

void Camera3DEditor::_bind_methods() {
}

void Camera3DEditor::edit(Node *p_camera) {
	node = p_camera;

	if (!node) {
		preview->set_pressed(false);
		Node3DEditor::get_singleton()->set_custom_camera(nullptr);
	} else {
		if (preview->is_pressed()) {
			Node3DEditor::get_singleton()->set_custom_camera(p_camera);
		} else {
			Node3DEditor::get_singleton()->set_custom_camera(nullptr);
		}
	}
}

Camera3DEditor::Camera3DEditor() {
	preview = memnew(Button);
	add_child(preview);

	preview->set_text(TTR("Preview"));
	preview->set_toggle_mode(true);
	preview->set_anchor(SIDE_LEFT, Control::ANCHOR_END);
	preview->set_anchor(SIDE_RIGHT, Control::ANCHOR_END);
	preview->set_offset(SIDE_LEFT, -60);
	preview->set_offset(SIDE_RIGHT, 0);
	preview->set_offset(SIDE_TOP, 0);
	preview->set_offset(SIDE_BOTTOM, 10);
	preview->connect("pressed", callable_mp(this, &Camera3DEditor::_pressed));
}

void Camera3DEditorPlugin::edit(Object *p_object) {
	Node3DEditor::get_singleton()->set_can_preview(Object::cast_to<Camera3D>(p_object));
	//camera_editor->edit(Object::cast_to<Node>(p_object));
}

bool Camera3DEditorPlugin::handles(Object *p_object) const {
	return p_object->is_class("Camera3D");
}

void Camera3DEditorPlugin::make_visible(bool p_visible) {
	if (p_visible) {
		//Node3DEditor::get_singleton()->set_can_preview(Object::cast_to<Camera3D>(p_object));
	} else {
		Node3DEditor::get_singleton()->set_can_preview(nullptr);
	}
}

Camera3DEditorPlugin::Camera3DEditorPlugin(EditorNode *p_node) {
	editor = p_node;
	/*	camera_editor = memnew( CameraEditor );
	editor->get_main_control()->add_child(camera_editor);

	camera_editor->set_anchor(SIDE_LEFT,Control::ANCHOR_END);
	camera_editor->set_anchor(SIDE_RIGHT,Control::ANCHOR_END);
	camera_editor->set_offset(SIDE_LEFT,60);
	camera_editor->set_offset(SIDE_RIGHT,0);
	camera_editor->set_offset(SIDE_TOP,0);
	camera_editor->set_offset(SIDE_BOTTOM,10);


	camera_editor->hide();
*/
}

Camera3DEditorPlugin::~Camera3DEditorPlugin() {
}
