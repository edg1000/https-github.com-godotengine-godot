/*************************************************************************/
/*  editor_undo_redo_manager.cpp                                         */
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

#include "editor_undo_redo_manager.h"

#include "core/io/resource.h"
#include "core/os/os.h"
#include "core/templates/local_vector.h"
#include "editor/editor_log.h"
#include "editor/editor_node.h"
#include "scene/main/node.h"

EditorUndoRedoManager::History &EditorUndoRedoManager::get_or_create_history(int p_idx) {
	if (!history_map.has(p_idx)) {
		History history;
		history.undo_redo = memnew(UndoRedo);
		history.id = p_idx;
		history_map[p_idx] = history;

		EditorNode::get_singleton()->get_log()->register_undo_redo(history.undo_redo);
		EditorDebuggerNode::get_singleton()->register_undo_redo(history.undo_redo);
	}
	return history_map[p_idx];
}

UndoRedo *EditorUndoRedoManager::get_history_undo_redo(int p_idx) const {
	ERR_FAIL_COND_V(!history_map.has(p_idx), nullptr);
	return history_map[p_idx].undo_redo;
}

int EditorUndoRedoManager::get_history_id_for_object(Object *p_object) const {
	int history_id = INVALID_HISTORY;

	if (Node *node = Object::cast_to<Node>(p_object)) {
		Node *edited_scene = EditorNode::get_singleton()->get_edited_scene();

		if (edited_scene && (node == edited_scene || edited_scene->is_ancestor_of(node))) {
			int idx = EditorNode::get_singleton()->get_editor_data().get_current_edited_scene_history_id();
			if (idx > 0) {
				history_id = idx;
			}
		}
	}

	if (Resource *res = Object::cast_to<Resource>(p_object)) {
		if (res->is_built_in()) {
			if (res->get_path().is_empty()) {
				int idx = EditorNode::get_singleton()->get_editor_data().get_current_edited_scene_history_id();
				if (idx > 0) {
					history_id = idx;
				}
			} else {
				int idx = EditorNode::get_singleton()->get_editor_data().get_scene_history_id_from_path(res->get_path().get_slice("::", 0));
				if (idx > 0) {
					history_id = idx;
				}
			}
		}
	}

	if (history_id == INVALID_HISTORY) {
		if (pending_action.history_id != INVALID_HISTORY) {
			history_id = pending_action.history_id;
		} else {
			history_id = GLOBAL_HISTORY;
		}
	}
	return history_id;
}

EditorUndoRedoManager::History &EditorUndoRedoManager::get_history_for_object(Object *p_object) {
	int history_id = get_history_id_for_object(p_object);
	ERR_FAIL_COND_V_MSG(pending_action.history_id != INVALID_HISTORY && history_id != pending_action.history_id, get_or_create_history(pending_action.history_id), vformat("UndoRedo history mismatch: expected %d, got %d.", pending_action.history_id, history_id));

	History &history = get_or_create_history(history_id);
	if (pending_action.history_id == INVALID_HISTORY) {
		pending_action.history_id = history_id;
		history.undo_redo->create_action(pending_action.action_name, pending_action.merge_mode);
	}

	return history;
}

void EditorUndoRedoManager::create_action_for_history(const String &p_name, int p_history_id, UndoRedo::MergeMode p_mode) {
	pending_action.action_name = p_name;
	pending_action.timestamp = OS::get_singleton()->get_unix_time();
	pending_action.merge_mode = p_mode;

	if (p_history_id != INVALID_HISTORY) {
		pending_action.history_id = p_history_id;
		History &history = get_or_create_history(p_history_id);
		history.undo_redo->create_action(pending_action.action_name, pending_action.merge_mode);
	}
}

void EditorUndoRedoManager::create_action(const String &p_name, UndoRedo::MergeMode p_mode, Object *p_custom_context) {
	create_action_for_history(p_name, INVALID_HISTORY, p_mode);

	if (p_custom_context) {
		// This assigns context to pending action.
		get_history_for_object(p_custom_context);
	}
}

void EditorUndoRedoManager::add_do_methodp(Object *p_object, const StringName &p_method, const Variant **p_args, int p_argcount) {
	UndoRedo *undo_redo = get_history_for_object(p_object).undo_redo;
	undo_redo->add_do_methodp(p_object, p_method, p_args, p_argcount);
}

void EditorUndoRedoManager::add_undo_methodp(Object *p_object, const StringName &p_method, const Variant **p_args, int p_argcount) {
	UndoRedo *undo_redo = get_history_for_object(p_object).undo_redo;
	undo_redo->add_undo_methodp(p_object, p_method, p_args, p_argcount);
}

void EditorUndoRedoManager::_add_do_method(const Variant **p_args, int p_argcount, Callable::CallError &r_error) {
	if (p_argcount < 2) {
		r_error.error = Callable::CallError::CALL_ERROR_TOO_FEW_ARGUMENTS;
		r_error.argument = 0;
		return;
	}

	if (p_args[0]->get_type() != Variant::OBJECT) {
		r_error.error = Callable::CallError::CALL_ERROR_INVALID_ARGUMENT;
		r_error.argument = 0;
		r_error.expected = Variant::OBJECT;
		return;
	}

	if (p_args[1]->get_type() != Variant::STRING_NAME && p_args[1]->get_type() != Variant::STRING) {
		r_error.error = Callable::CallError::CALL_ERROR_INVALID_ARGUMENT;
		r_error.argument = 1;
		r_error.expected = Variant::STRING_NAME;
		return;
	}

	r_error.error = Callable::CallError::CALL_OK;

	Object *object = *p_args[0];
	StringName method = *p_args[1];

	add_do_methodp(object, method, p_args + 2, p_argcount - 2);
}

void EditorUndoRedoManager::_add_undo_method(const Variant **p_args, int p_argcount, Callable::CallError &r_error) {
	if (p_argcount < 2) {
		r_error.error = Callable::CallError::CALL_ERROR_TOO_FEW_ARGUMENTS;
		r_error.argument = 0;
		return;
	}

	if (p_args[0]->get_type() != Variant::OBJECT) {
		r_error.error = Callable::CallError::CALL_ERROR_INVALID_ARGUMENT;
		r_error.argument = 0;
		r_error.expected = Variant::OBJECT;
		return;
	}

	if (p_args[1]->get_type() != Variant::STRING_NAME && p_args[1]->get_type() != Variant::STRING) {
		r_error.error = Callable::CallError::CALL_ERROR_INVALID_ARGUMENT;
		r_error.argument = 1;
		r_error.expected = Variant::STRING_NAME;
		return;
	}

	r_error.error = Callable::CallError::CALL_OK;

	Object *object = *p_args[0];
	StringName method = *p_args[1];

	add_undo_methodp(object, method, p_args + 2, p_argcount - 2);
}

void EditorUndoRedoManager::add_do_property(Object *p_object, const StringName &p_property, const Variant &p_value) {
	UndoRedo *undo_redo = get_history_for_object(p_object).undo_redo;
	undo_redo->add_do_property(p_object, p_property, p_value);
}

void EditorUndoRedoManager::add_undo_property(Object *p_object, const StringName &p_property, const Variant &p_value) {
	UndoRedo *undo_redo = get_history_for_object(p_object).undo_redo;
	undo_redo->add_undo_property(p_object, p_property, p_value);
}

void EditorUndoRedoManager::add_do_reference(Object *p_object) {
	UndoRedo *undo_redo = get_history_for_object(p_object).undo_redo;
	undo_redo->add_do_reference(p_object);
}

void EditorUndoRedoManager::add_undo_reference(Object *p_object) {
	UndoRedo *undo_redo = get_history_for_object(p_object).undo_redo;
	undo_redo->add_undo_reference(p_object);
}

void EditorUndoRedoManager::commit_action(bool p_execute) {
	ERR_FAIL_COND(pending_action.history_id == INVALID_HISTORY);
	is_committing = true;

	History &history = get_or_create_history(pending_action.history_id);
	history.undo_redo->commit_action(p_execute);
	history.redo_stack.clear();

	if (!history.undo_stack.is_empty()) {
		const Action &prev_action = history.undo_stack.back()->get();
		if (pending_action.merge_mode != UndoRedo::MERGE_DISABLE && pending_action.merge_mode == prev_action.merge_mode && pending_action.action_name == prev_action.action_name) {
			// Discard action if it should be merged (UndoRedo handles merging internally).
			pending_action = Action();
			is_committing = false;
			return;
		}
	}

	history.undo_stack.push_back(pending_action);
	pending_action = Action();
	is_committing = false;
}

bool EditorUndoRedoManager::is_committing_action() const {
	return is_committing;
}

bool EditorUndoRedoManager::undo() {
	if (!has_undo()) {
		return false;
	}

	History *selected_history = nullptr;
	double global_timestamp = 0;

	// Pick the history with greatest last action timestamp (either global or current scene).
	{
		History &history = get_or_create_history(GLOBAL_HISTORY);
		if (!history.undo_stack.is_empty()) {
			selected_history = &history;
			global_timestamp = history.undo_stack.back()->get().timestamp;
		}
	}

	{
		History &history = get_or_create_history(EditorNode::get_editor_data().get_current_edited_scene_history_id());
		if (!history.undo_stack.is_empty() && history.undo_stack.back()->get().timestamp > global_timestamp) {
			selected_history = &history;
		}
	}

	if (selected_history) {
		Action action = selected_history->undo_stack.back()->get();
		selected_history->undo_stack.pop_back();
		selected_history->redo_stack.push_back(action);
		return selected_history->undo_redo->undo();
	}
	return false;
}

bool EditorUndoRedoManager::redo() {
	if (!has_redo()) {
		return false;
	}

	History *selected_history = nullptr;
	double global_timestamp = INFINITY;

	// Pick the history with lowest last action timestamp (either global or current scene).
	{
		History &history = get_or_create_history(GLOBAL_HISTORY);
		if (!history.redo_stack.is_empty()) {
			selected_history = &history;
			global_timestamp = history.redo_stack.back()->get().timestamp;
		}
	}

	{
		History &history = get_or_create_history(EditorNode::get_editor_data().get_current_edited_scene_history_id());
		if (!history.redo_stack.is_empty() && history.redo_stack.back()->get().timestamp < global_timestamp) {
			selected_history = &history;
		}
	}

	if (selected_history) {
		Action action = selected_history->redo_stack.back()->get();
		selected_history->redo_stack.pop_back();
		selected_history->undo_stack.push_back(action);
		return selected_history->undo_redo->redo();
	}
	return false;
}

void EditorUndoRedoManager::set_history_as_saved(int p_id) {
	History &history = get_or_create_history(p_id);
	history.saved_version = history.undo_redo->get_version();
}

void EditorUndoRedoManager::set_history_as_unsaved(int p_id) {
	History &history = get_or_create_history(p_id);
	history.saved_version = -1;
}

bool EditorUndoRedoManager::is_history_unsaved(int p_id) {
	History &history = get_or_create_history(p_id);
	return history.undo_redo->get_version() != history.saved_version;
}

bool EditorUndoRedoManager::has_undo() {
	for (const KeyValue<int, History> &E : history_map) {
		if ((E.key == GLOBAL_HISTORY || E.key == EditorNode::get_editor_data().get_current_edited_scene_history_id()) && !E.value.undo_stack.is_empty()) {
			return true;
		}
	}
	return false;
}

bool EditorUndoRedoManager::has_redo() {
	for (const KeyValue<int, History> &E : history_map) {
		if ((E.key == GLOBAL_HISTORY || E.key == EditorNode::get_editor_data().get_current_edited_scene_history_id()) && !E.value.redo_stack.is_empty()) {
			return true;
		}
	}
	return false;
}

void EditorUndoRedoManager::clear_history(bool p_increase_version, int p_idx) {
	if (p_idx != INVALID_HISTORY) {
		get_or_create_history(p_idx).undo_redo->clear_history(p_increase_version);
		if (!p_increase_version) {
			set_history_as_saved(p_idx);
		}
		return;
	}

	for (const KeyValue<int, History> &E : history_map) {
		E.value.undo_redo->clear_history(p_increase_version);
		set_history_as_saved(E.key);
	}
}

String EditorUndoRedoManager::get_current_action_name() {
	if (has_undo()) {
		History *selected_history = nullptr;
		double global_timestamp = 0;

		// Pick the history with greatest last action timestamp (either global or current scene).
		{
			History &history = get_or_create_history(GLOBAL_HISTORY);
			if (!history.undo_stack.is_empty()) {
				selected_history = &history;
				global_timestamp = history.undo_stack.back()->get().timestamp;
			}
		}

		{
			History &history = get_or_create_history(EditorNode::get_editor_data().get_current_edited_scene_history_id());
			if (!history.undo_stack.is_empty() && history.undo_stack.back()->get().timestamp > global_timestamp) {
				selected_history = &history;
			}
		}

		if (selected_history) {
			return selected_history->undo_redo->get_current_action_name();
		}
	}
	return "";
}

void EditorUndoRedoManager::discard_history(int p_idx, bool p_erase_from_map) {
	ERR_FAIL_COND(!history_map.has(p_idx));
	History &history = history_map[p_idx];

	if (history.undo_redo) {
		memdelete(history.undo_redo);
		history.undo_redo = nullptr;
	}

	if (p_erase_from_map) {
		history_map.erase(p_idx);
	}
}

void EditorUndoRedoManager::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create_action", "name", "merge_mode", "custom_context"), &EditorUndoRedoManager::create_action, DEFVAL(UndoRedo::MERGE_DISABLE), DEFVAL((Object *)nullptr));
	ClassDB::bind_method(D_METHOD("commit_action", "execute"), &EditorUndoRedoManager::commit_action, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("is_committing_action"), &EditorUndoRedoManager::is_committing_action);

	{
		MethodInfo mi;
		mi.name = "add_do_method";
		mi.arguments.push_back(PropertyInfo(Variant::OBJECT, "object"));
		mi.arguments.push_back(PropertyInfo(Variant::STRING_NAME, "method"));

		ClassDB::bind_vararg_method(METHOD_FLAGS_DEFAULT, "add_do_method", &EditorUndoRedoManager::_add_do_method, mi, varray(), false);
	}

	{
		MethodInfo mi;
		mi.name = "add_undo_method";
		mi.arguments.push_back(PropertyInfo(Variant::OBJECT, "object"));
		mi.arguments.push_back(PropertyInfo(Variant::STRING_NAME, "method"));

		ClassDB::bind_vararg_method(METHOD_FLAGS_DEFAULT, "add_undo_method", &EditorUndoRedoManager::_add_undo_method, mi, varray(), false);
	}

	ClassDB::bind_method(D_METHOD("add_do_property", "object", "property", "value"), &EditorUndoRedoManager::add_do_property);
	ClassDB::bind_method(D_METHOD("add_undo_property", "object", "property", "value"), &EditorUndoRedoManager::add_undo_property);
	ClassDB::bind_method(D_METHOD("add_do_reference", "object"), &EditorUndoRedoManager::add_do_reference);
	ClassDB::bind_method(D_METHOD("add_undo_reference", "object"), &EditorUndoRedoManager::add_undo_reference);

	ClassDB::bind_method(D_METHOD("get_object_history_id", "object"), &EditorUndoRedoManager::get_history_id_for_object);
	ClassDB::bind_method(D_METHOD("get_history_undo_redo", "id"), &EditorUndoRedoManager::get_history_undo_redo);

	BIND_ENUM_CONSTANT(GLOBAL_HISTORY);
	BIND_ENUM_CONSTANT(INVALID_HISTORY);
}

EditorUndoRedoManager::~EditorUndoRedoManager() {
	for (const KeyValue<int, History> &E : history_map) {
		discard_history(E.key, false);
	}
}
