/**************************************************************************/
/*  saveload_editor_plugin.cpp                                            */
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

#include "saveload_editor_plugin.h"

#include "../saveload_synchronizer.h"
#include "editor_saveload_profiler.h"
#include "saveload_editor.h"

#include "editor/editor_interface.h"
#include "editor/editor_node.h"

void SaveloadEditorDebugger::_bind_methods() {
	ADD_SIGNAL(MethodInfo("open_request", PropertyInfo(Variant::STRING, "path")));
}

bool SaveloadEditorDebugger::has_capture(const String &p_capture) const {
	return p_capture == "multiplayer";
}

void SaveloadEditorDebugger::_open_request(const String &p_path) {
	emit_signal("open_request", p_path);
}

bool SaveloadEditorDebugger::capture(const String &p_message, const Array &p_data, int p_session) {
	ERR_FAIL_COND_V(!profilers.has(p_session), false);
	EditorSaveloadProfiler *profiler = profilers[p_session];
	if (p_message == "saveload:rpc") {
//		SaveloadDebugger::RPCFrame frame;
//		frame.deserialize(p_data);
//		for (int i = 0; i < frame.infos.size(); i++) {
//			profiler->add_rpc_frame_data(frame.infos[i]);
//		}
		return true;
	} else if (p_message == "saveload:syncs") {
		SaveloadDebugger::SaveloadFrame frame;
		frame.deserialize(p_data);
		for (const KeyValue<ObjectID, SaveloadDebugger::SyncInfo> &E : frame.infos) {
			profiler->add_sync_frame_data(E.value);
		}
		Array missing = profiler->pop_missing_node_data();
		if (missing.size()) {
			// Asks for the object information.
			get_session(p_session)->send_message("multiplayer:cache", missing);
		}
		return true;
	} else if (p_message == "saveload:cache") {
		ERR_FAIL_COND_V(p_data.size() % 3, false);
		for (int i = 0; i < p_data.size(); i += 3) {
			EditorSaveloadProfiler::NodeInfo info;
			info.id = p_data[i].operator ObjectID();
			info.type = p_data[i + 1].operator String();
			info.path = p_data[i + 2].operator String();
			profiler->add_node_data(info);
		}
		return true;
	} else if (p_message == "saveload:bandwidth") {
		ERR_FAIL_COND_V(p_data.size() < 2, false);
		profiler->set_bandwidth(p_data[0], p_data[1]);
		return true;
	}
	return false;
}

void SaveloadEditorDebugger::_profiler_activate(bool p_enable, int p_session_id) {
	Ref<EditorDebuggerSession> session = get_session(p_session_id);
	ERR_FAIL_COND(session.is_null());
	session->toggle_profiler("saveload:bandwidth", p_enable);
//	session->toggle_profiler("saveload:rpc", p_enable);
	session->toggle_profiler("saveload:saveload", p_enable);
}

void SaveloadEditorDebugger::setup_session(int p_session_id) {
	Ref<EditorDebuggerSession> session = get_session(p_session_id);
	ERR_FAIL_COND(session.is_null());
	EditorSaveloadProfiler *profiler = memnew(EditorSaveloadProfiler);
	profiler->connect("enable_profiling", callable_mp(this, &SaveloadEditorDebugger::_profiler_activate).bind(p_session_id));
	profiler->connect("open_request", callable_mp(this, &SaveloadEditorDebugger::_open_request));
	profiler->set_name(TTR("Save Lod Profiler"));
	session->add_session_tab(profiler);
	profilers[p_session_id] = profiler;
}

/// MultiplayerEditorPlugin

SaveloadEditorPlugin::SaveloadEditorPlugin() {
	saveload_editor = memnew(SaveloadEditor);
	button = EditorNode::get_singleton()->add_bottom_panel_item(TTR("Save Load"), saveload_editor);
	button->hide();
	saveload_editor->get_pin()->connect("pressed", callable_mp(this, &SaveloadEditorPlugin::_pinned));
	debugger.instantiate();
	debugger->connect("open_request", callable_mp(this, &SaveloadEditorPlugin::_open_request));
}

void SaveloadEditorPlugin::_open_request(const String &p_path) {
	EditorInterface::get_singleton()->open_scene_from_path(p_path);
}

void SaveloadEditorPlugin::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			get_tree()->connect("node_removed", callable_mp(this, &SaveloadEditorPlugin::_node_removed));
			add_debugger_plugin(debugger);
		} break;
		case NOTIFICATION_EXIT_TREE: {
			remove_debugger_plugin(debugger);
		}
	}
}

void SaveloadEditorPlugin::_node_removed(Node *p_node) {
	if (p_node && p_node == saveload_editor->get_current()) {
		saveload_editor->edit(nullptr);
		if (saveload_editor->is_visible_in_tree()) {
			EditorNode::get_singleton()->hide_bottom_panel();
		}
		button->hide();
		saveload_editor->get_pin()->set_pressed(false);
	}
}

void SaveloadEditorPlugin::_pinned() {
	if (!saveload_editor->get_pin()->is_pressed()) {
		if (saveload_editor->is_visible_in_tree()) {
			EditorNode::get_singleton()->hide_bottom_panel();
		}
		button->hide();
	}
}

void SaveloadEditorPlugin::edit(Object *p_object) {
	saveload_editor->edit(Object::cast_to<SaveloadSynchronizer>(p_object));
}

bool SaveloadEditorPlugin::handles(Object *p_object) const {
	return p_object->is_class("SaveloadSynchronizer");
}

void SaveloadEditorPlugin::make_visible(bool p_visible) {
	if (p_visible) {
		button->show();
		EditorNode::get_singleton()->make_bottom_panel_item_visible(saveload_editor);
	} else if (!saveload_editor->get_pin()->is_pressed()) {
		if (saveload_editor->is_visible_in_tree()) {
			EditorNode::get_singleton()->hide_bottom_panel();
		}
		button->hide();
	}
}
