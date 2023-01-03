/*************************************************************************/
/*  node_3d_editor_plugin.cpp                                            */
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

#include "node_3d_editor_plugin.h"

#include "editor/editor_node.h"

void Node3DEditorPlugin::make_visible(bool p_visible) {
	if (p_visible) {
		spatial_editor->show();
		spatial_editor->set_process(true);
		spatial_editor->set_physics_process(true);

	} else {
		spatial_editor->hide();
		spatial_editor->set_process(false);
		spatial_editor->set_physics_process(false);
	}
}

void Node3DEditorPlugin::edit(Object *p_object) {
	spatial_editor->edit(Object::cast_to<Node3D>(p_object));
}

bool Node3DEditorPlugin::handles(Object *p_object) const {
	if (p_object->is_class("Node3D")) {
		return true;
	} else {
		// This ensures that gizmos are cleared when selecting a non-Node3D node.
		const_cast<Node3DEditorPlugin *>(this)->edit((Object *)nullptr);
		return false;
	}
}

Dictionary Node3DEditorPlugin::get_state() const {
	return spatial_editor->get_state();
}

void Node3DEditorPlugin::set_state(const Dictionary &p_state) {
	spatial_editor->set_state(p_state);
}

void Node3DEditorPlugin::edited_scene_changed() {
	for (uint32_t i = 0; i < Node3DEditor::VIEWPORTS_COUNT; i++) {
		Node3DEditorViewport *viewport = Node3DEditor::get_singleton()->get_editor_viewport(i);
		if (viewport->is_visible()) {
			viewport->notification(Control::NOTIFICATION_VISIBILITY_CHANGED);
		}
	}
}

Node3DEditorPlugin::Node3DEditorPlugin() {
	spatial_editor = memnew(Node3DEditor);
	spatial_editor->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	EditorNode::get_singleton()->get_main_screen_control()->add_child(spatial_editor);

	spatial_editor->hide();
}

Node3DEditorPlugin::~Node3DEditorPlugin() {
}
