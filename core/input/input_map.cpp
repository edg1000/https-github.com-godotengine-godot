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

	for (Map<StringName, Action>::Element *E = input_map.front(); E; E = E->next()) {
		actions.push_back(E->key());
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
	const Map<StringName, Action>::Element *E = input_map.find(p_action);
	if (!E) {
		return nullptr;
	}

	return &E->get().inputs;
}

bool InputMap::event_is_action(const Ref<InputEvent> &p_event, const StringName &p_action) const {
	return event_get_action_status(p_event, p_action);
}

bool InputMap::event_get_action_status(const Ref<InputEvent> &p_event, const StringName &p_action, bool *p_pressed, float *p_strength) const {
	Map<StringName, Action>::Element *E = input_map.find(p_action);
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
	List<Ref<InputEvent>>::Element *event = _find_event(E->get(), p_event, &pressed, &strength);
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

const Map<StringName, InputMap::Action> &InputMap::get_action_map() const {
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

void InputMap::load_default() {
	Ref<InputEventKey> key;

	add_action("ui_accept");
	key.instance();
	key->set_keycode(KEY_ENTER);
	action_add_event("ui_accept", key);

	key.instance();
	key->set_keycode(KEY_KP_ENTER);
	action_add_event("ui_accept", key);

	key.instance();
	key->set_keycode(KEY_SPACE);
	action_add_event("ui_accept", key);

	add_action("ui_select");
	key.instance();
	key->set_keycode(KEY_SPACE);
	action_add_event("ui_select", key);

	add_action("ui_cancel");
	key.instance();
	key->set_keycode(KEY_ESCAPE);
	action_add_event("ui_cancel", key);

	add_action("ui_focus_next");
	key.instance();
	key->set_keycode(KEY_TAB);
	action_add_event("ui_focus_next", key);

	add_action("ui_focus_prev");
	key.instance();
	key->set_keycode(KEY_TAB);
	key->set_shift(true);
	action_add_event("ui_focus_prev", key);

	add_action("ui_left");
	key.instance();
	key->set_keycode(KEY_LEFT);
	action_add_event("ui_left", key);

	add_action("ui_right");
	key.instance();
	key->set_keycode(KEY_RIGHT);
	action_add_event("ui_right", key);

	add_action("ui_up");
	key.instance();
	key->set_keycode(KEY_UP);
	action_add_event("ui_up", key);

	add_action("ui_down");
	key.instance();
	key->set_keycode(KEY_DOWN);
	action_add_event("ui_down", key);

	add_action("ui_page_up");
	key.instance();
	key->set_keycode(KEY_PAGEUP);
	action_add_event("ui_page_up", key);

	add_action("ui_page_down");
	key.instance();
	key->set_keycode(KEY_PAGEDOWN);
	action_add_event("ui_page_down", key);

	add_action("ui_home");
	key.instance();
	key->set_keycode(KEY_HOME);
	action_add_event("ui_home", key);

	add_action("ui_end");
	key.instance();
	key->set_keycode(KEY_END);
	action_add_event("ui_end", key);

	//set("display/window/handheld/orientation", "landscape");
}

InputMap::InputMap() {
	ERR_FAIL_COND_MSG(singleton, "Singleton in InputMap already exist.");
	singleton = this;
}
