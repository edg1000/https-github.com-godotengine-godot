/*************************************************************************/
/*  editor_debugger_node.cpp                                             */
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

#include "editor_debugger_node.h"

#include "editor/debugger/editor_debugger_tree.h"
#include "editor/editor_log.h"
#include "editor/editor_node.h"
#include "editor/plugins/script_editor_plugin.h"

template <typename Func>
void _for_all(TabContainer *p_node, const Func &p_func) {
	for (int i = 0; i < p_node->get_tab_count(); i++) {
		ScriptEditorDebugger *dbg = Object::cast_to<ScriptEditorDebugger>(p_node->get_tab_control(i));
		ERR_FAIL_COND(!dbg);
		p_func(dbg);
	}
}

EditorDebuggerNode *EditorDebuggerNode::singleton = NULL;

EditorDebuggerNode::EditorDebuggerNode() {
	if (!singleton)
		singleton = this;
	server.instance();

	add_constant_override("margin_left", -EditorNode::get_singleton()->get_gui_base()->get_stylebox("BottomPanelDebuggerOverride", "EditorStyles")->get_margin(MARGIN_LEFT));
	add_constant_override("margin_right", -EditorNode::get_singleton()->get_gui_base()->get_stylebox("BottomPanelDebuggerOverride", "EditorStyles")->get_margin(MARGIN_RIGHT));

	tabs = memnew(TabContainer);
	tabs->set_tab_align(TabContainer::ALIGN_LEFT);
	tabs->set_tabs_visible(false);
	tabs->connect_compat("tab_changed", this, "_debugger_changed");
	add_child(tabs);

	Ref<StyleBoxEmpty> empty;
	empty.instance();
	tabs->add_style_override("panel", empty);

	auto_switch_remote_scene_tree = EDITOR_DEF("debugger/auto_switch_to_remote_scene_tree", false);
	_add_debugger();

	// Remote scene tree
	remote_scene_tree = memnew(EditorDebuggerTree);
	remote_scene_tree->connect_compat("object_selected", this, "_remote_object_requested");
	remote_scene_tree->connect_compat("save_node", this, "_save_node_requested");
	EditorNode::get_singleton()->get_scene_tree_dock()->add_remote_tree_editor(remote_scene_tree);
	EditorNode::get_singleton()->get_scene_tree_dock()->connect_compat("remote_tree_selected", this, "request_remote_tree");

	remote_scene_tree_timeout = EDITOR_DEF("debugger/remote_scene_tree_refresh_interval", 1.0);
	inspect_edited_object_timeout = EDITOR_DEF("debugger/remote_inspect_refresh_interval", 0.2);

	EditorNode *editor = EditorNode::get_singleton();
	editor->get_undo_redo()->set_method_notify_callback(_method_changeds, this);
	editor->get_undo_redo()->set_property_notify_callback(_property_changeds, this);
	editor->get_pause_button()->connect_compat("pressed", this, "_paused");
}

ScriptEditorDebugger *EditorDebuggerNode::_add_debugger() {
	ScriptEditorDebugger *node = memnew(ScriptEditorDebugger(EditorNode::get_singleton()));

	int id = tabs->get_tab_count();
	node->connect_compat("stop_requested", this, "_debugger_wants_stop", varray(id));
	node->connect_compat("stopped", this, "_debugger_stopped", varray(id));
	node->connect_compat("stack_frame_selected", this, "_stack_frame_selected", varray(id));
	node->connect_compat("error_selected", this, "_error_selected", varray(id));
	node->connect_compat("clear_execution", this, "_clear_execution");
	node->connect_compat("breaked", this, "_breaked", varray(id));
	node->connect_compat("remote_tree_updated", this, "_remote_tree_updated", varray(id));
	node->connect_compat("remote_object_updated", this, "_remote_object_updated", varray(id));
	node->connect_compat("remote_object_property_updated", this, "_remote_object_property_updated", varray(id));
	node->connect_compat("remote_object_requested", this, "_remote_object_requested", varray(id));

	if (tabs->get_tab_count() > 0) {
		get_debugger(0)->clear_style();
	}

	tabs->add_child(node);

	node->set_name("Session " + itos(tabs->get_tab_count()));
	if (tabs->get_tab_count() > 1) {
		node->clear_style();
		tabs->set_tabs_visible(true);
		tabs->add_style_override("panel", EditorNode::get_singleton()->get_gui_base()->get_stylebox("DebuggerPanel", "EditorStyles"));
	}

	return node;
}

void EditorDebuggerNode::_stack_frame_selected(int p_debugger) {
	const ScriptEditorDebugger *dbg = get_debugger(p_debugger);
	ERR_FAIL_COND(!dbg);
	if (dbg != get_current_debugger())
		return;
	_text_editor_stack_goto(dbg);
}

void EditorDebuggerNode::_error_selected(const String &p_file, int p_line, int p_debugger) {
	Ref<Script> s = ResourceLoader::load(p_file);
	emit_signal("goto_script_line", s, p_line - 1);
}

void EditorDebuggerNode::_text_editor_stack_goto(const ScriptEditorDebugger *p_debugger) {
	const String file = p_debugger->get_stack_script_file();
	if (file.empty())
		return;
	stack_script = ResourceLoader::load(file);
	const int line = p_debugger->get_stack_script_line() - 1;
	emit_signal("goto_script_line", stack_script, line);
	emit_signal("set_execution", stack_script, line);
	stack_script.unref(); // Why?!?
}

void EditorDebuggerNode::_bind_methods() {
	ClassDB::bind_method("_menu_option", &EditorDebuggerNode::_menu_option);
	ClassDB::bind_method("_debugger_stopped", &EditorDebuggerNode::_debugger_stopped);
	ClassDB::bind_method("_debugger_wants_stop", &EditorDebuggerNode::_debugger_wants_stop);
	ClassDB::bind_method("_debugger_changed", &EditorDebuggerNode::_debugger_changed);
	ClassDB::bind_method("_stack_frame_selected", &EditorDebuggerNode::_stack_frame_selected);
	ClassDB::bind_method("_error_selected", &EditorDebuggerNode::_error_selected);
	ClassDB::bind_method("_clear_execution", &EditorDebuggerNode::_clear_execution);
	ClassDB::bind_method("_breaked", &EditorDebuggerNode::_breaked);
	ClassDB::bind_method("start", &EditorDebuggerNode::start);
	ClassDB::bind_method("stop", &EditorDebuggerNode::stop);
	ClassDB::bind_method("_paused", &EditorDebuggerNode::_paused);
	ClassDB::bind_method("request_remote_tree", &EditorDebuggerNode::request_remote_tree);
	ClassDB::bind_method("_remote_tree_updated", &EditorDebuggerNode::_remote_tree_updated);
	ClassDB::bind_method("_remote_object_updated", &EditorDebuggerNode::_remote_object_updated);
	ClassDB::bind_method("_remote_object_property_updated", &EditorDebuggerNode::_remote_object_property_updated);
	ClassDB::bind_method("_remote_object_requested", &EditorDebuggerNode::_remote_object_requested);
	ClassDB::bind_method("_save_node_requested", &EditorDebuggerNode::_save_node_requested);

	// LiveDebug.
	ClassDB::bind_method("live_debug_create_node", &EditorDebuggerNode::live_debug_create_node);
	ClassDB::bind_method("live_debug_instance_node", &EditorDebuggerNode::live_debug_instance_node);
	ClassDB::bind_method("live_debug_remove_node", &EditorDebuggerNode::live_debug_remove_node);
	ClassDB::bind_method("live_debug_remove_and_keep_node", &EditorDebuggerNode::live_debug_remove_and_keep_node);
	ClassDB::bind_method("live_debug_restore_node", &EditorDebuggerNode::live_debug_restore_node);
	ClassDB::bind_method("live_debug_duplicate_node", &EditorDebuggerNode::live_debug_duplicate_node);
	ClassDB::bind_method("live_debug_reparent_node", &EditorDebuggerNode::live_debug_reparent_node);

	ADD_SIGNAL(MethodInfo("goto_script_line"));
	ADD_SIGNAL(MethodInfo("set_execution", PropertyInfo("script"), PropertyInfo(Variant::INT, "line")));
	ADD_SIGNAL(MethodInfo("clear_execution", PropertyInfo("script")));
	ADD_SIGNAL(MethodInfo("breaked", PropertyInfo(Variant::BOOL, "reallydid"), PropertyInfo(Variant::BOOL, "can_debug")));
}

EditorDebuggerRemoteObject *EditorDebuggerNode::get_inspected_remote_object() {
	return Object::cast_to<EditorDebuggerRemoteObject>(ObjectDB::get_instance(EditorNode::get_singleton()->get_editor_history()->get_current()));
}

ScriptEditorDebugger *EditorDebuggerNode::get_debugger(int p_id) const {
	return Object::cast_to<ScriptEditorDebugger>(tabs->get_tab_control(p_id));
}

ScriptEditorDebugger *EditorDebuggerNode::get_current_debugger() const {
	return Object::cast_to<ScriptEditorDebugger>(tabs->get_tab_control(tabs->get_current_tab()));
}

ScriptEditorDebugger *EditorDebuggerNode::get_default_debugger() const {
	return Object::cast_to<ScriptEditorDebugger>(tabs->get_tab_control(0));
}

Error EditorDebuggerNode::start() {
	stop();
	if (EDITOR_GET("run/output/always_open_output_on_play")) {
		EditorNode::get_singleton()->make_bottom_panel_item_visible(EditorNode::get_log());
	} else {
		EditorNode::get_singleton()->make_bottom_panel_item_visible(this);
	}

	int remote_port = (int)EditorSettings::get_singleton()->get("network/debug/remote_port");
	const Error err = server->listen(remote_port);
	if (err != OK) {
		EditorNode::get_log()->add_message(String("Error listening on port ") + itos(remote_port), EditorLog::MSG_TYPE_ERROR);
		return err;
	}
	set_process(true);
	EditorNode::get_log()->add_message("--- Debugging process started ---", EditorLog::MSG_TYPE_EDITOR);
	return OK;
}

void EditorDebuggerNode::stop() {
	if (server->is_listening()) {
		server->stop();
		EditorNode::get_log()->add_message("--- Debugging process stopped ---", EditorLog::MSG_TYPE_EDITOR);
	}
	// Also close all debugging sessions.
	_for_all(tabs, [&](ScriptEditorDebugger *dbg) {
		if (dbg->is_session_active())
			dbg->stop();
	});
	_break_state_changed();
	if (hide_on_stop) {
		if (is_visible_in_tree())
			EditorNode::get_singleton()->hide_bottom_panel();
	}
	breakpoints.clear();
	set_process(false);
}

void EditorDebuggerNode::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			EditorNode::get_singleton()->connect_compat("play_pressed", this, "start");
			EditorNode::get_singleton()->connect_compat("stop_pressed", this, "stop");
		} break;
		case NOTIFICATION_EXIT_TREE: {
			EditorNode::get_singleton()->disconnect_compat("play_pressed", this, "start");
			EditorNode::get_singleton()->disconnect_compat("stop_pressed", this, "stop");
		} break;
		case EditorSettings::NOTIFICATION_EDITOR_SETTINGS_CHANGED: {
			if (tabs->get_tab_count() > 1) {
				add_constant_override("margin_left", -EditorNode::get_singleton()->get_gui_base()->get_stylebox("BottomPanelDebuggerOverride", "EditorStyles")->get_margin(MARGIN_LEFT));
				add_constant_override("margin_right", -EditorNode::get_singleton()->get_gui_base()->get_stylebox("BottomPanelDebuggerOverride", "EditorStyles")->get_margin(MARGIN_RIGHT));

				tabs->add_style_override("panel", EditorNode::get_singleton()->get_gui_base()->get_stylebox("DebuggerPanel", "EditorStyles"));
			}
		} break;
		default:
			break;
	}

	if (p_what != NOTIFICATION_PROCESS || !server->is_listening())
		return;

	// Errors and warnings
	int error_count = 0;
	int warning_count = 0;
	_for_all(tabs, [&](ScriptEditorDebugger *dbg) {
		error_count += dbg->get_error_count();
		warning_count += dbg->get_warning_count();
	});

	if (error_count != last_error_count || warning_count != last_warning_count) {

		_for_all(tabs, [&](ScriptEditorDebugger *dbg) {
			dbg->update_tabs();
		});

		if (error_count == 0 && warning_count == 0) {
			debugger_button->set_text(TTR("Debugger"));
			debugger_button->set_icon(Ref<Texture2D>());
		} else {
			debugger_button->set_text(TTR("Debugger") + " (" + itos(error_count + warning_count) + ")");
			if (error_count == 0) {
				debugger_button->set_icon(get_icon("Warning", "EditorIcons"));
			} else {
				debugger_button->set_icon(get_icon("Error", "EditorIcons"));
			}
		}
		last_error_count = error_count;
		last_warning_count = warning_count;
	}

	// Remote scene tree update
	remote_scene_tree_timeout -= get_process_delta_time();
	if (remote_scene_tree_timeout < 0) {
		remote_scene_tree_timeout = EditorSettings::get_singleton()->get("debugger/remote_scene_tree_refresh_interval");
		if (remote_scene_tree->is_visible_in_tree()) {
			get_current_debugger()->request_remote_tree();
		}
	}

	// Remote inspector update
	inspect_edited_object_timeout -= get_process_delta_time();
	if (inspect_edited_object_timeout < 0) {
		inspect_edited_object_timeout = EditorSettings::get_singleton()->get("debugger/remote_inspect_refresh_interval");
		if (EditorDebuggerRemoteObject *obj = get_inspected_remote_object()) {
			get_current_debugger()->request_remote_object(obj->remote_object_id);
		}
	}

	// Take connections.
	if (server->is_connection_available()) {
		ScriptEditorDebugger *debugger = NULL;
		_for_all(tabs, [&](ScriptEditorDebugger *dbg) {
			if (debugger || dbg->is_session_active())
				return;
			debugger = dbg;
		});
		if (debugger == NULL) {
			if (tabs->get_tab_count() <= 4) { // Max 4 debugging sessions active.
				debugger = _add_debugger();
			} else {
				// We already have too many sessions, disconnecting new clients to prevent it from hanging.
				// (Not keeping a reference to the connection will disconnect it)
				server->take_connection();
				return; // Can't add, stop here.
			}
		}

		EditorNode::get_singleton()->get_pause_button()->set_disabled(false);
		// Switch to remote tree view if so desired.
		auto_switch_remote_scene_tree = (bool)EditorSettings::get_singleton()->get("debugger/auto_switch_to_remote_scene_tree");
		if (auto_switch_remote_scene_tree) {
			EditorNode::get_singleton()->get_scene_tree_dock()->show_remote_tree();
		}
		// Good to go.
		EditorNode::get_singleton()->get_scene_tree_dock()->show_tab_buttons();
		debugger->set_editor_remote_tree(remote_scene_tree);
		debugger->start(server->take_connection());
		// Send breakpoints.
		for (Map<Breakpoint, bool>::Element *E = breakpoints.front(); E; E = E->next()) {
			const Breakpoint &bp = E->key();
			debugger->set_breakpoint(bp.source, bp.line, E->get());
		} // Will arrive too late, how does the regular run work?

		debugger->update_live_edit_root();
	}
}

void EditorDebuggerNode::_debugger_stopped(int p_id) {
	ScriptEditorDebugger *dbg = get_debugger(p_id);
	ERR_FAIL_COND(!dbg);

	bool found = false;
	_for_all(tabs, [&](ScriptEditorDebugger *p_debugger) {
		if (p_debugger->is_session_active())
			found = true;
	});
	if (!found) {
		EditorNode::get_singleton()->get_pause_button()->set_pressed(false);
		EditorNode::get_singleton()->get_pause_button()->set_disabled(true);
		EditorNode::get_singleton()->get_scene_tree_dock()->hide_remote_tree();
		EditorNode::get_singleton()->get_scene_tree_dock()->hide_tab_buttons();
		EditorNode::get_singleton()->notify_all_debug_sessions_exited();
	}
}

void EditorDebuggerNode::_debugger_wants_stop(int p_id) {
	// Ask editor to kill PID.
	int pid = get_debugger(p_id)->get_remote_pid();
	if (pid)
		EditorNode::get_singleton()->call_deferred("stop_child_process", pid);
}

void EditorDebuggerNode::_debugger_changed(int p_tab) {
	if (get_inspected_remote_object()) {
		// Clear inspected object, you can only inspect objects in selected debugger.
		// Hopefully, in the future, we will have one inspector per debugger.
		EditorNode::get_singleton()->push_item(NULL);
	}
	if (remote_scene_tree->is_visible_in_tree()) {
		get_current_debugger()->request_remote_tree();
	}
	if (get_current_debugger()->is_breaked()) {
		_text_editor_stack_goto(get_current_debugger());
	}
}

void EditorDebuggerNode::set_script_debug_button(MenuButton *p_button) {
	script_menu = p_button;
	script_menu->set_text(TTR("Debug"));
	script_menu->set_switch_on_hover(true);
	PopupMenu *p = script_menu->get_popup();
	p->set_hide_on_window_lose_focus(true);
	p->add_shortcut(ED_GET_SHORTCUT("debugger/step_into"), DEBUG_STEP);
	p->add_shortcut(ED_GET_SHORTCUT("debugger/step_over"), DEBUG_NEXT);
	p->add_separator();
	p->add_shortcut(ED_GET_SHORTCUT("debugger/break"), DEBUG_BREAK);
	p->add_shortcut(ED_GET_SHORTCUT("debugger/continue"), DEBUG_CONTINUE);
	p->add_separator();
	p->add_check_shortcut(ED_GET_SHORTCUT("debugger/keep_debugger_open"), DEBUG_SHOW_KEEP_OPEN);
	p->add_check_shortcut(ED_GET_SHORTCUT("debugger/debug_with_external_editor"), DEBUG_WITH_EXTERNAL_EDITOR);
	p->connect_compat("id_pressed", this, "_menu_option");

	_break_state_changed();
	script_menu->show();
}

void EditorDebuggerNode::_break_state_changed() {
	const bool breaked = get_current_debugger()->is_breaked();
	const bool can_debug = get_current_debugger()->is_debuggable();
	if (breaked) // Show debugger.
		EditorNode::get_singleton()->make_bottom_panel_item_visible(this);

	// Update script menu.
	if (!script_menu)
		return;
	PopupMenu *p = script_menu->get_popup();
	p->set_item_disabled(p->get_item_index(DEBUG_NEXT), !(breaked && can_debug));
	p->set_item_disabled(p->get_item_index(DEBUG_STEP), !(breaked && can_debug));
	p->set_item_disabled(p->get_item_index(DEBUG_BREAK), breaked);
	p->set_item_disabled(p->get_item_index(DEBUG_CONTINUE), !breaked);
}

void EditorDebuggerNode::_menu_option(int p_id) {
	switch (p_id) {
		case DEBUG_NEXT: {
			debug_next();
		} break;
		case DEBUG_STEP: {
			debug_step();
		} break;
		case DEBUG_BREAK: {
			debug_break();
		} break;
		case DEBUG_CONTINUE: {
			debug_continue();
		} break;

		case DEBUG_SHOW_KEEP_OPEN: {
			bool visible = script_menu->get_popup()->is_item_checked(script_menu->get_popup()->get_item_index(DEBUG_SHOW_KEEP_OPEN));
			hide_on_stop = visible;
			script_menu->get_popup()->set_item_checked(script_menu->get_popup()->get_item_index(DEBUG_SHOW_KEEP_OPEN), !visible);
		} break;
		case DEBUG_WITH_EXTERNAL_EDITOR: {
			bool checked = !script_menu->get_popup()->is_item_checked(script_menu->get_popup()->get_item_index(DEBUG_WITH_EXTERNAL_EDITOR));
			debug_with_external_editor = checked;
			script_menu->get_popup()->set_item_checked(script_menu->get_popup()->get_item_index(DEBUG_WITH_EXTERNAL_EDITOR), checked);
		} break;
	}
}

void EditorDebuggerNode::_paused() {
	const bool paused = EditorNode::get_singleton()->get_pause_button()->is_pressed();
	_for_all(tabs, [&](ScriptEditorDebugger *dbg) {
		if (paused && !dbg->is_breaked()) {
			dbg->debug_break();
		} else if (!paused && dbg->is_breaked()) {
			dbg->debug_continue();
		}
	});
}

void EditorDebuggerNode::_breaked(bool p_breaked, bool p_can_debug, int p_debugger) {
	if (get_current_debugger() != get_debugger(p_debugger)) {
		if (!p_breaked)
			return;
		tabs->set_current_tab(p_debugger);
	}
	_break_state_changed();
	EditorNode::get_singleton()->get_pause_button()->set_pressed(p_breaked);
	emit_signal("breaked", p_breaked, p_can_debug);
}

bool EditorDebuggerNode::is_skip_breakpoints() const {
	return get_default_debugger()->is_skip_breakpoints();
}

void EditorDebuggerNode::set_breakpoint(const String &p_path, int p_line, bool p_enabled) {
	breakpoints[Breakpoint(p_path, p_line)] = p_enabled;
	_for_all(tabs, [&](ScriptEditorDebugger *dbg) {
		dbg->set_breakpoint(p_path, p_line, p_enabled);
	});
}

void EditorDebuggerNode::reload_scripts() {
	_for_all(tabs, [&](ScriptEditorDebugger *dbg) {
		dbg->reload_scripts();
	});
}

// LiveEdit/Inspector
void EditorDebuggerNode::request_remote_tree() {
	get_current_debugger()->request_remote_tree();
}

void EditorDebuggerNode::_remote_tree_updated(int p_debugger) {
	if (p_debugger != tabs->get_current_tab())
		return;
	remote_scene_tree->clear();
	remote_scene_tree->update_scene_tree(get_current_debugger()->get_remote_tree(), p_debugger);
}

void EditorDebuggerNode::_remote_object_updated(ObjectID p_id, int p_debugger) {
	if (p_debugger != tabs->get_current_tab())
		return;
	if (EditorDebuggerRemoteObject *obj = get_inspected_remote_object()) {
		if (obj->remote_object_id == p_id)
			return; // Already being edited
	}

	EditorNode::get_singleton()->push_item(get_current_debugger()->get_remote_object(p_id));
}

void EditorDebuggerNode::_remote_object_property_updated(ObjectID p_id, const String &p_property, int p_debugger) {
	if (p_debugger != tabs->get_current_tab())
		return;
	if (EditorDebuggerRemoteObject *obj = get_inspected_remote_object()) {
		if (obj->remote_object_id != p_id)
			return;
		EditorNode::get_singleton()->get_inspector()->update_property(p_property);
	}
}

void EditorDebuggerNode::_remote_object_requested(ObjectID p_id, int p_debugger) {
	if (p_debugger != tabs->get_current_tab())
		return;
	inspect_edited_object_timeout = 0.7; // Temporarily disable timeout to avoid multiple requests.
	get_current_debugger()->request_remote_object(p_id);
}

void EditorDebuggerNode::_save_node_requested(ObjectID p_id, const String &p_file, int p_debugger) {
	if (p_debugger != tabs->get_current_tab())
		return;
	get_current_debugger()->save_node(p_id, p_file);
}

// Remote inspector/edit.
void EditorDebuggerNode::_method_changeds(void *p_ud, Object *p_base, const StringName &p_name, VARIANT_ARG_DECLARE) {
	if (!singleton)
		return;
	_for_all(singleton->tabs, [&](ScriptEditorDebugger *dbg) {
		dbg->_method_changed(p_base, p_name, VARIANT_ARG_PASS);
	});
}

void EditorDebuggerNode::_property_changeds(void *p_ud, Object *p_base, const StringName &p_property, const Variant &p_value) {
	if (!singleton)
		return;
	_for_all(singleton->tabs, [&](ScriptEditorDebugger *dbg) {
		dbg->_property_changed(p_base, p_property, p_value);
	});
}

// LiveDebug
void EditorDebuggerNode::set_live_debugging(bool p_enabled) {

	_for_all(tabs, [&](ScriptEditorDebugger *dbg) {
		dbg->set_live_debugging(p_enabled);
	});
}
void EditorDebuggerNode::update_live_edit_root() {
	_for_all(tabs, [&](ScriptEditorDebugger *dbg) {
		dbg->update_live_edit_root();
	});
}
void EditorDebuggerNode::live_debug_create_node(const NodePath &p_parent, const String &p_type, const String &p_name) {
	_for_all(tabs, [&](ScriptEditorDebugger *dbg) {
		dbg->live_debug_create_node(p_parent, p_type, p_name);
	});
}
void EditorDebuggerNode::live_debug_instance_node(const NodePath &p_parent, const String &p_path, const String &p_name) {
	_for_all(tabs, [&](ScriptEditorDebugger *dbg) {
		dbg->live_debug_instance_node(p_parent, p_path, p_name);
	});
}
void EditorDebuggerNode::live_debug_remove_node(const NodePath &p_at) {
	_for_all(tabs, [&](ScriptEditorDebugger *dbg) {
		dbg->live_debug_remove_node(p_at);
	});
}
void EditorDebuggerNode::live_debug_remove_and_keep_node(const NodePath &p_at, ObjectID p_keep_id) {
	_for_all(tabs, [&](ScriptEditorDebugger *dbg) {
		dbg->live_debug_remove_and_keep_node(p_at, p_keep_id);
	});
}
void EditorDebuggerNode::live_debug_restore_node(ObjectID p_id, const NodePath &p_at, int p_at_pos) {
	_for_all(tabs, [&](ScriptEditorDebugger *dbg) {
		dbg->live_debug_restore_node(p_id, p_at, p_at_pos);
	});
}
void EditorDebuggerNode::live_debug_duplicate_node(const NodePath &p_at, const String &p_new_name) {
	_for_all(tabs, [&](ScriptEditorDebugger *dbg) {
		dbg->live_debug_duplicate_node(p_at, p_new_name);
	});
}
void EditorDebuggerNode::live_debug_reparent_node(const NodePath &p_at, const NodePath &p_new_place, const String &p_new_name, int p_at_pos) {
	_for_all(tabs, [&](ScriptEditorDebugger *dbg) {
		dbg->live_debug_reparent_node(p_at, p_new_place, p_new_name, p_at_pos);
	});
}
