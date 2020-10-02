/*************************************************************************/
/*  input_map.cpp                                                        */
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

#include "input_map.h"

#include "core/os/keyboard.h"
#include "core/project_settings.h"

InputMap *InputMap::singleton = nullptr;

int InputMap::ALL_DEVICES = -1;

void InputMap::_bind_methods() {
	ClassDB::bind_method(D_METHOD("has_action", "action"), &InputMap::has_action);
	ClassDB::bind_method(D_METHOD("get_actions"), &InputMap::_get_actions);
	ClassDB::bind_method(D_METHOD("add_action", "action", "deadzone"), &InputMap::add_action, DEFVAL(0.5f));
	ClassDB::bind_method(D_METHOD("erase_action", "action"), &InputMap::erase_action);

	ClassDB::bind_method(D_METHOD("action_set_deadzone", "action", "deadzone"), &InputMap::action_set_deadzone);
	ClassDB::bind_method(D_METHOD("action_add_event", "action", "event"), &InputMap::action_add_event);
	ClassDB::bind_method(D_METHOD("action_has_event", "action", "event"), &InputMap::action_has_event);
	ClassDB::bind_method(D_METHOD("action_erase_event", "action", "event"), &InputMap::action_erase_event);
	ClassDB::bind_method(D_METHOD("action_erase_events", "action"), &InputMap::action_erase_events);
	ClassDB::bind_method(D_METHOD("action_get_events", "action"), &InputMap::_action_get_events);
	ClassDB::bind_method(D_METHOD("event_is_action", "event", "action"), &InputMap::event_is_action);
	ClassDB::bind_method(D_METHOD("load_from_project_settings"), &InputMap::load_from_project_settings);
}

void InputMap::add_action(const StringName &p_action, float p_deadzone) {
	ERR_FAIL_COND_MSG(input_map.has(p_action), "InputMap already has action '" + String(p_action) + "'.");
	input_map[p_action] = Action();
	static int last_id = 1;
	input_map[p_action].id = last_id;
	input_map[p_action].deadzone = p_deadzone;
	last_id++;
}

void InputMap::erase_action(const StringName &p_action) {
	ERR_FAIL_COND_MSG(!input_map.has(p_action), "Request for nonexistent InputMap action '" + String(p_action) + "'.");
	input_map.erase(p_action);
}

Array InputMap::_get_actions() {
	Array ret;
	List<StringName> actions = get_actions();
	if (actions.empty()) {
		return ret;
	}

	for (const List<StringName>::Element *E = actions.front(); E; E = E->next()) {
		ret.push_back(E->get());
	}

	return ret;
}

List<StringName> InputMap::get_actions() const {
	List<StringName> actions = List<StringName>();
	if (input_map.empty()) {
		return actions;
	}

	for (OrderedHashMap<StringName, Action>::Element E = input_map.front(); E; E = E.next()) {
		actions.push_back(E.key());
	}

	return actions;
}

List<Ref<InputEvent>>::Element *InputMap::_find_event(Action &p_action, const Ref<InputEvent> &p_event, bool *p_pressed, float *p_strength) const {
	ERR_FAIL_COND_V(!p_event.is_valid(), nullptr);

	for (List<Ref<InputEvent>>::Element *E = p_action.inputs.front(); E; E = E->next()) {
		const Ref<InputEvent> e = E->get();

		//if (e.type != Ref<InputEvent>::KEY && e.device != p_event.device) -- unsure about the KEY comparison, why is this here?
		//	continue;

		int device = e->get_device();
		if (device == ALL_DEVICES || device == p_event->get_device()) {
			if (e->action_match(p_event, p_pressed, p_strength, p_action.deadzone)) {
				return E;
			}
		}
	}

	return nullptr;
}

bool InputMap::has_action(const StringName &p_action) const {
	return input_map.has(p_action);
}

void InputMap::action_set_deadzone(const StringName &p_action, float p_deadzone) {
	ERR_FAIL_COND_MSG(!input_map.has(p_action), "Request for nonexistent InputMap action '" + String(p_action) + "'.");

	input_map[p_action].deadzone = p_deadzone;
}

void InputMap::action_add_event(const StringName &p_action, const Ref<InputEvent> &p_event) {
	ERR_FAIL_COND_MSG(p_event.is_null(), "It's not a reference to a valid InputEvent object.");
	ERR_FAIL_COND_MSG(!input_map.has(p_action), "Request for nonexistent InputMap action '" + String(p_action) + "'.");
	if (_find_event(input_map[p_action], p_event)) {
		return; // Already addded.
	}

	input_map[p_action].inputs.push_back(p_event);
}

bool InputMap::action_has_event(const StringName &p_action, const Ref<InputEvent> &p_event) {
	ERR_FAIL_COND_V_MSG(!input_map.has(p_action), false, "Request for nonexistent InputMap action '" + String(p_action) + "'.");
	return (_find_event(input_map[p_action], p_event) != nullptr);
}

void InputMap::action_erase_event(const StringName &p_action, const Ref<InputEvent> &p_event) {
	ERR_FAIL_COND_MSG(!input_map.has(p_action), "Request for nonexistent InputMap action '" + String(p_action) + "'.");

	List<Ref<InputEvent>>::Element *E = _find_event(input_map[p_action], p_event);
	if (E) {
		input_map[p_action].inputs.erase(E);
	}
}

void InputMap::action_erase_events(const StringName &p_action) {
	ERR_FAIL_COND_MSG(!input_map.has(p_action), "Request for nonexistent InputMap action '" + String(p_action) + "'.");

	input_map[p_action].inputs.clear();
}

Array InputMap::_action_get_events(const StringName &p_action) {
	Array ret;
	const List<Ref<InputEvent>> *al = action_get_events(p_action);
	if (al) {
		for (const List<Ref<InputEvent>>::Element *E = al->front(); E; E = E->next()) {
			ret.push_back(E->get());
		}
	}

	return ret;
}

const List<Ref<InputEvent>> *InputMap::action_get_events(const StringName &p_action) {
	const OrderedHashMap<StringName, Action>::Element E = input_map.find(p_action);
	if (!E) {
		return nullptr;
	}

	return &E.get().inputs;
}

bool InputMap::event_is_action(const Ref<InputEvent> &p_event, const StringName &p_action) const {
	return event_get_action_status(p_event, p_action);
}

bool InputMap::event_get_action_status(const Ref<InputEvent> &p_event, const StringName &p_action, bool *p_pressed, float *p_strength) const {
	OrderedHashMap<StringName, Action>::Element E = input_map.find(p_action);
	ERR_FAIL_COND_V_MSG(!E, false, "Request for nonexistent InputMap action '" + String(p_action) + "'.");

	Ref<InputEventAction> input_event_action = p_event;
	if (input_event_action.is_valid()) {
		if (p_pressed != nullptr) {
			*p_pressed = input_event_action->is_pressed();
		}
		if (p_strength != nullptr) {
			*p_strength = (p_pressed != nullptr && *p_pressed) ? input_event_action->get_strength() : 0.0f;
		}
		return input_event_action->get_action() == p_action;
	}

	bool pressed;
	float strength;
	List<Ref<InputEvent>>::Element *event = _find_event(E.get(), p_event, &pressed, &strength);
	if (event != nullptr) {
		if (p_pressed != nullptr) {
			*p_pressed = pressed;
		}
		if (p_strength != nullptr) {
			*p_strength = strength;
		}
		return true;
	} else {
		return false;
	}
}

const OrderedHashMap<StringName, InputMap::Action> &InputMap::get_action_map() const {
	return input_map;
}

void InputMap::load_from_project_settings() {
	input_map.clear();

	List<PropertyInfo> pinfo;
	ProjectSettings::get_singleton()->get_property_list(&pinfo);

	for (List<PropertyInfo>::Element *E = pinfo.front(); E; E = E->next()) {
		const PropertyInfo &pi = E->get();

		if (!pi.name.begins_with("input/")) {
			continue;
		}

		String name = pi.name.substr(pi.name.find("/") + 1, pi.name.length());

		Dictionary action = ProjectSettings::get_singleton()->get(pi.name);
		float deadzone = action.has("deadzone") ? (float)action["deadzone"] : 0.5f;
		Array events = action["events"];

		add_action(name, deadzone);
		for (int i = 0; i < events.size(); i++) {
			Ref<InputEvent> event = events[i];
			if (event.is_null()) {
				continue;
			}
			action_add_event(name, event);
		}
	}
}

struct _BuiltinActionDisplayName {
	const char *name;
	const char *display_name;
};

static const _BuiltinActionDisplayName _builtin_action_display_names[] = {
	/* clang-format off */
	{ "ui_accept", 									"Accept" },
	{ "ui_select", 									"Select" },
	{ "ui_cancel", 									"Cancel" },
	{ "ui_focus_next", 								"Focus Next" },
	{ "ui_focus_prev", 								"Focus Prev" },
	{ "ui_left", 									"Left" },
	{ "ui_right", 									"Right" },
	{ "ui_up", 										"Up" },
	{ "ui_down", 									"Down" },
	{ "ui_page_up",									"Page Up" },
	{ "ui_page_down", 								"Page Down" },
	{ "ui_home", 									"Home" },
	{ "ui_end", 									"End" },
	{ "ui_cut", 									"Cut" },
	{ "ui_copy", 									"Copy" },
	{ "ui_paste", 									"Paste" },
	{ "ui_undo", 									"Undo" },
	{ "ui_redo", 									"Redo" },
	{ "ui_text_completion_query", 					"Completion Query" },
	{ "ui_text_newline", 							"New Line" },
	{ "ui_text_newline_blank",						"New Blank Line" },
	{ "ui_text_newline_above", 						"New Line Above" },
	{ "ui_text_indent", 							"Indent" },
	{ "ui_text_dedent", 							"Dedent" },
	{ "ui_text_backspace", 							"Backspace" },
	{ "ui_text_backspace_word", 					"Backspace Word" },
	{ "ui_text_backspace_word.OSX", 				"Backspace Word" },
	{ "ui_text_backspace_all_to_left", 				"Backspace all to Left" },
	{ "ui_text_backspace_all_to_left.OSX", 			"Backspace all to Left" },
	{ "ui_text_delete", 							"Delete" },
	{ "ui_text_delete_word", 						"Delete Word" },
	{ "ui_text_delete_word.OSX", 					"Delete Word" },
	{ "ui_text_delete_all_to_right", 				"Delete all to Right" },
	{ "ui_text_delete_all_to_right.OSX", 			"Delete all to Right" },
	{ "ui_text_caret_left", 						"Caret Left" },
	{ "ui_text_caret_word_left", 					"Caret Word Left" },
	{ "ui_text_caret_word_left.OSX", 				"Caret Word Left" },
	{ "ui_text_caret_right", 						"Caret Right" },
	{ "ui_text_caret_word_right", 					"Caret Word Right" },
	{ "ui_text_caret_word_right.OSX", 				"Caret Word Right" },
	{ "ui_text_caret_up", 							"Caret Up" },
	{ "ui_text_caret_down", 						"Caret Down" },
	{ "ui_text_caret_line_start", 					"Caret Line Start" },
	{ "ui_text_caret_line_start.OSX", 				"Caret Line Start" },
	{ "ui_text_caret_line_end", 					"Caret Line End" },
	{ "ui_text_caret_line_end.OSX",					"Caret Line End" },
	{ "ui_text_caret_page_up", 						"Caret Page Up" },
	{ "ui_text_caret_page_down", 					"Caret Page Down" },
	{ "ui_text_caret_document_start", 				"Caret Document Start" },
	{ "ui_text_caret_document_start.OSX",			"Caret Document Start" },
	{ "ui_text_caret_document_end", 				"Caret Document End" },
	{ "ui_text_caret_document_end.OSX", 			"Caret Document End" },
	{ "ui_text_scroll_up", 							"Scroll Up" },
	{ "ui_text_scroll_up.OSX", 						"Scroll Up" },
	{ "ui_text_scroll_down", 						"Scroll Down" },
	{ "ui_text_scroll_down.OSX", 					"Scroll Down" },
	{ "ui_text_select_all", 						"Select All" },
	{ "ui_text_toggle_insert_mode", 				"Toggle Insert Mode" },
	{ "ui_graph_duplicate", 						"Duplicate Nodes" },
	{ "ui_graph_delete", 							"Delete Nodes" },
	{ "ui_filedialog_up_one_level", 				"Go Up One Level" },
	{ "ui_filedialog_refresh", 						"Refresh" },
	{ "ui_filedialog_show_hidden", 					"Show Hidden" },
	{ "",											""}
	/* clang-format on */
};

String InputMap::get_builtin_display_name(const String &p_name) const {
	const _BuiltinActionDisplayName *bidn = &_builtin_action_display_names[0];

	while (bidn->name) {
		if (bidn->name == p_name) {
			return bidn->display_name;
		}
		bidn++;
	}

	return "";
}

const OrderedHashMap<String, List<Ref<InputEvent>>> &InputMap::get_builtins() {
	// Return cache if it has already been built.
	if (default_builtin_cache.size()) {
		return default_builtin_cache;
	}

	List<Ref<InputEvent>> inputs;
	inputs.push_back(InputEventKey::create_reference(KEY_ENTER));
	inputs.push_back(InputEventKey::create_reference(KEY_KP_ENTER));
	inputs.push_back(InputEventKey::create_reference(KEY_SPACE));
	default_builtin_cache.insert("ui_accept", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventJoypadButton::create_reference(JOY_BUTTON_Y));
	inputs.push_back(InputEventKey::create_reference(KEY_SPACE));
	default_builtin_cache.insert("ui_select", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_ESCAPE));
	default_builtin_cache.insert("ui_cancel", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_TAB));
	default_builtin_cache.insert("ui_focus_next", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_TAB, KEY_MASK_SHIFT));
	default_builtin_cache.insert("ui_focus_prev", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_LEFT));
	inputs.push_back(InputEventJoypadButton::create_reference(JOY_BUTTON_DPAD_LEFT));
	default_builtin_cache.insert("ui_left", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_RIGHT));
	inputs.push_back(InputEventJoypadButton::create_reference(JOY_BUTTON_DPAD_RIGHT));
	default_builtin_cache.insert("ui_right", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_UP));
	inputs.push_back(InputEventJoypadButton::create_reference(JOY_BUTTON_DPAD_UP));
	default_builtin_cache.insert("ui_up", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_DOWN));
	inputs.push_back(InputEventJoypadButton::create_reference(JOY_BUTTON_DPAD_DOWN));
	default_builtin_cache.insert("ui_down", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_PAGEUP));
	default_builtin_cache.insert("ui_page_up", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_PAGEDOWN));
	default_builtin_cache.insert("ui_page_down", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_HOME));
	default_builtin_cache.insert("ui_home", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_END));
	default_builtin_cache.insert("ui_end", inputs);

	// ///// UI basic Shortcuts /////

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_X, KEY_MASK_CMD));
	inputs.push_back(InputEventKey::create_reference(KEY_DELETE, KEY_MASK_SHIFT));
	default_builtin_cache.insert("ui_cut", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_C, KEY_MASK_CMD));
	inputs.push_back(InputEventKey::create_reference(KEY_INSERT, KEY_MASK_CMD));
	default_builtin_cache.insert("ui_copy", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_V, KEY_MASK_CMD));
	inputs.push_back(InputEventKey::create_reference(KEY_INSERT, KEY_MASK_SHIFT));
	default_builtin_cache.insert("ui_paste", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_Z, KEY_MASK_CMD));
	default_builtin_cache.insert("ui_undo", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_Y, KEY_MASK_CMD));
	inputs.push_back(InputEventKey::create_reference(KEY_Z, KEY_MASK_CMD | KEY_MASK_SHIFT));
	default_builtin_cache.insert("ui_redo", inputs);

	// ///// UI Text Input Shortcuts /////
	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_SPACE, KEY_MASK_CMD));
	default_builtin_cache.insert("ui_text_completion_query", inputs);

	// Newlines
	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_ENTER));
	inputs.push_back(InputEventKey::create_reference(KEY_KP_ENTER));
	default_builtin_cache.insert("ui_text_newline", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_ENTER, KEY_MASK_CMD));
	inputs.push_back(InputEventKey::create_reference(KEY_KP_ENTER, KEY_MASK_CMD));
	default_builtin_cache.insert("ui_text_newline_blank", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_ENTER, KEY_MASK_SHIFT | KEY_MASK_CMD));
	inputs.push_back(InputEventKey::create_reference(KEY_KP_ENTER, KEY_MASK_SHIFT | KEY_MASK_CMD));
	default_builtin_cache.insert("ui_text_newline_above", inputs);

	// Indentation
	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_TAB));
	default_builtin_cache.insert("ui_text_indent", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_TAB, KEY_MASK_SHIFT));
	default_builtin_cache.insert("ui_text_dedent", inputs);

	// Text Backspace and Delete
	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_BACKSPACE));
	default_builtin_cache.insert("ui_text_backspace", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_BACKSPACE, KEY_MASK_CMD));
	default_builtin_cache.insert("ui_text_backspace_word", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_BACKSPACE, KEY_MASK_ALT));
	default_builtin_cache.insert("ui_text_backspace_word.OSX", inputs);

	inputs = List<Ref<InputEvent>>();
	default_builtin_cache.insert("ui_text_backspace_all_to_left", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_BACKSPACE, KEY_MASK_CMD));
	default_builtin_cache.insert("ui_text_backspace_all_to_left.OSX", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_DELETE));
	default_builtin_cache.insert("ui_text_delete", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_DELETE, KEY_MASK_CMD));
	default_builtin_cache.insert("ui_text_delete_word", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_DELETE, KEY_MASK_ALT));
	default_builtin_cache.insert("ui_text_delete_word.OSX", inputs);

	inputs = List<Ref<InputEvent>>();
	default_builtin_cache.insert("ui_text_delete_all_to_right", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_DELETE, KEY_MASK_CMD));
	default_builtin_cache.insert("ui_text_delete_all_to_right.OSX", inputs);

	// Text Caret Movement Left/Right

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_LEFT));
	default_builtin_cache.insert("ui_text_caret_left", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_LEFT, KEY_MASK_CMD));
	default_builtin_cache.insert("ui_text_caret_word_left", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_LEFT, KEY_MASK_ALT));
	default_builtin_cache.insert("ui_text_caret_word_left.OSX", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_RIGHT));
	default_builtin_cache.insert("ui_text_caret_right", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_RIGHT, KEY_MASK_CMD));
	default_builtin_cache.insert("ui_text_caret_word_right", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_RIGHT, KEY_MASK_ALT));
	default_builtin_cache.insert("ui_text_caret_word_right.OSX", inputs);

	// Text Caret Movement Up/Down

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_UP));
	default_builtin_cache.insert("ui_text_caret_up", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_DOWN));
	default_builtin_cache.insert("ui_text_caret_down", inputs);

	// Text Caret Movement Line Start/End

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_HOME));
	default_builtin_cache.insert("ui_text_caret_line_start", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_A, KEY_MASK_CTRL));
	inputs.push_back(InputEventKey::create_reference(KEY_LEFT, KEY_MASK_CMD));
	default_builtin_cache.insert("ui_text_caret_line_start.OSX", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_END));
	default_builtin_cache.insert("ui_text_caret_line_end", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_E, KEY_MASK_CTRL));
	inputs.push_back(InputEventKey::create_reference(KEY_RIGHT, KEY_MASK_CMD));
	default_builtin_cache.insert("ui_text_caret_line_end.OSX", inputs);

	// Text Caret Movement Page Up/Down

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_PAGEUP));
	default_builtin_cache.insert("ui_text_caret_page_up", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_PAGEDOWN));
	default_builtin_cache.insert("ui_text_caret_page_down", inputs);

	// Text Caret Movement Document Start/End

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_HOME, KEY_MASK_CMD));
	default_builtin_cache.insert("ui_text_caret_document_start", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_UP, KEY_MASK_CMD));
	default_builtin_cache.insert("ui_text_caret_document_start.OSX", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_END, KEY_MASK_CMD));
	default_builtin_cache.insert("ui_text_caret_document_end", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_DOWN, KEY_MASK_CMD));
	default_builtin_cache.insert("ui_text_caret_document_end.OSX", inputs);

	// Text Scrolling

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_UP, KEY_MASK_CMD));
	default_builtin_cache.insert("ui_text_scroll_up", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_UP, KEY_MASK_CMD | KEY_MASK_ALT));
	default_builtin_cache.insert("ui_text_scroll_up.OSX", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_DOWN, KEY_MASK_CMD));
	default_builtin_cache.insert("ui_text_scroll_down", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_DOWN, KEY_MASK_CMD | KEY_MASK_ALT));
	default_builtin_cache.insert("ui_text_scroll_down.OSX", inputs);

	// Text Misc

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_A, KEY_MASK_CMD));
	default_builtin_cache.insert("ui_text_select_all", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_INSERT));
	default_builtin_cache.insert("ui_text_toggle_insert_mode", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_MENU));
	default_builtin_cache.insert("ui_menu", inputs);

	// ///// UI Graph Shortcuts /////

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_D, KEY_MASK_CMD));
	default_builtin_cache.insert("ui_graph_duplicate", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_DELETE));
	default_builtin_cache.insert("ui_graph_delete", inputs);

	// ///// UI File Dialog Shortcuts /////
	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_BACKSPACE));
	default_builtin_cache.insert("ui_filedialog_up_one_level", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_F5));
	default_builtin_cache.insert("ui_filedialog_refresh", inputs);

	inputs = List<Ref<InputEvent>>();
	inputs.push_back(InputEventKey::create_reference(KEY_H));
	default_builtin_cache.insert("ui_filedialog_show_hidden", inputs);

	return default_builtin_cache;
}

void InputMap::load_default() {
	OrderedHashMap<String, List<Ref<InputEvent>>> builtins = get_builtins();

	// List of Builtins which have an override for OSX.
	Vector<String> osx_builtins;
	for (OrderedHashMap<String, List<Ref<InputEvent>>>::Element E = builtins.front(); E; E = E.next()) {
		if (String(E.key()).ends_with(".OSX")) {
			// Strip .OSX from name: some_input_name.OSX -> some_input_name
			osx_builtins.push_back(String(E.key()).split(".")[0]);
		}
	}

	for (OrderedHashMap<String, List<Ref<InputEvent>>>::Element E = builtins.front(); E; E = E.next()) {
		String fullname = E.key();
		String name = fullname.split(".")[0];
		String override_for = fullname.split(".").size() > 1 ? fullname.split(".")[1] : "";

#ifdef APPLE_STYLE_KEYS
		if (osx_builtins.has(name) && override_for != "OSX") {
			// Name has osx builtin but this particular one is for non-osx systems - so skip.
			continue;
		}
#else
		if (override_for == "OSX") {
			// Override for OSX - not needed on non-osx platforms.
			continue;
		}
#endif

		add_action(name);

		List<Ref<InputEvent>> inputs = E.get();
		for (List<Ref<InputEvent>>::Element *I = inputs.front(); I; I = I->next()) {
			Ref<InputEventKey> iek = I->get();

			// For the editor, only add keyboard actions.
			if (iek.is_valid()) {
				action_add_event(fullname, I->get());
			}
		}
	}
}

InputMap::InputMap() {
	ERR_FAIL_COND_MSG(singleton, "Singleton in InputMap already exist.");
	singleton = this;
}
