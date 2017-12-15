/*************************************************************************/
/*  input_map.cpp                                                        */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2018 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2018 Godot Engine contributors (cf. AUTHORS.md)    */
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

#include "os/keyboard.h"
#include "project_settings.h"

InputMap *InputMap::singleton = NULL;

void InputMap::_bind_methods() {

	ClassDB::bind_method(D_METHOD("has_action", "action"), &InputMap::has_action);
	ClassDB::bind_method(D_METHOD("get_actions"), &InputMap::_get_actions);
	ClassDB::bind_method(D_METHOD("add_action", "action"), &InputMap::add_action);
	ClassDB::bind_method(D_METHOD("erase_action", "action"), &InputMap::erase_action);

	ClassDB::bind_method(D_METHOD("action_add_event", "action", "event"), &InputMap::action_add_event);
	ClassDB::bind_method(D_METHOD("action_has_event", "action", "event"), &InputMap::action_has_event);
	ClassDB::bind_method(D_METHOD("action_erase_event", "action", "event"), &InputMap::action_erase_event);
	ClassDB::bind_method(D_METHOD("get_action_list", "action"), &InputMap::_get_action_list);
	ClassDB::bind_method(D_METHOD("event_is_action", "event", "action"), &InputMap::event_is_action);

	ClassDB::bind_method(D_METHOD("has_axis", "axis"), &InputMap::has_axis);
	ClassDB::bind_method(D_METHOD("get_axes"), &InputMap::_get_axes);
	ClassDB::bind_method(D_METHOD("add_axis", "axis"), &InputMap::add_axis);
	ClassDB::bind_method(D_METHOD("erase_axis", "axis"), &InputMap::erase_axis);

	ClassDB::bind_method(D_METHOD("axis_add_event", "axis", "event", "value"), &InputMap::axis_add_event);
	ClassDB::bind_method(D_METHOD("axis_has_event", "axis", "event"), &InputMap::axis_has_event);
	ClassDB::bind_method(D_METHOD("axis_erase_event", "axis", "event"), &InputMap::axis_erase_event);
	ClassDB::bind_method(D_METHOD("get_axis_list", "axis"), &InputMap::_get_axis_list);
	ClassDB::bind_method(D_METHOD("event_is_axis", "event", "axis"), &InputMap::event_is_axis);

	ClassDB::bind_method(D_METHOD("load_from_globals"), &InputMap::load_from_globals);
}

void InputMap::add_action(const StringName &p_action) {

	ERR_FAIL_COND(input_map.has(p_action));
	input_map[p_action] = Action();
	static int last_id = 1;
	input_map[p_action].id = last_id;
	last_id++;
}

void InputMap::add_axis(const StringName &p_axis) {
	ERR_FAIL_COND(input_axis_map.has(p_axis));
	input_axis_map[p_axis] = Axis();
	static int last_id = 1;
	input_map[p_axis].id = last_id++;
}

void InputMap::erase_action(const StringName &p_action) {

	ERR_FAIL_COND(!input_map.has(p_action));
	input_map.erase(p_action);
}

void InputMap::erase_axis(const StringName &p_axis) {
	ERR_FAIL_COND(!input_axis_map.has(p_axis));
	input_map.erase(p_axis);
}

Array InputMap::_get_actions() {

	Array ret;
	List<StringName> actions = get_actions();
	if (actions.empty())
		return ret;

	for (const List<StringName>::Element *E = actions.front(); E; E = E->next()) {

		ret.push_back(E->get());
	}

	return ret;
}

Array InputMap::_get_axes() {
	Array ret;
	List<StringName> axes = get_axes();
	if (axes.empty())
		return ret;

	for (const List<StringName>::Element *E = axes.front(); E; E = E->next()) {
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

List<StringName> InputMap::get_axes() const {
	List<StringName> axes = List<StringName>();
	if (input_axis_map.empty())
		return axes;

	for (Map<StringName, Axis>::Element *E = input_axis_map.front(); E; E = E->next()) {
		axes.push_back(E->key());
	}

	return axes;
}

List<Ref<InputEvent> >::Element *InputMap::_find_event(List<Ref<InputEvent> > &p_list, const Ref<InputEvent> &p_event, bool p_action_test) const {

	for (List<Ref<InputEvent> >::Element *E = p_list.front(); E; E = E->next()) {

		const Ref<InputEvent> e = E->get();

		//if (e.type != Ref<InputEvent>::KEY && e.device != p_event.device) -- unsure about the KEY comparison, why is this here?
		//	continue;

		if (e->get_device() != p_event->get_device())
			continue;
		if (e->action_match(p_event))
			return E;
	}

	return NULL;
}

List<Map<Ref<InputEvent>, float> >::Element *InputMap::_find_event(List<Map<Ref<InputEvent>, float> > &p_list,
		const Ref<InputEvent> &p_event, bool p_action_test) const {
	for (List<Map<Ref<InputEvent>, float> >::Element *E = p_list.front(); E; E = E->next()) {
		const Ref<InputEvent> e = E->get().front()->key();

		if (e->get_device() != p_event->get_device())
			continue;
		if (e->action_match(p_event))
			return E;
	}

	return NULL;
}

bool InputMap::has_action(const StringName &p_action) const {

	return input_map.has(p_action);
}

bool InputMap::has_axis(const StringName &p_axis) const {
	return input_axis_map.has(p_axis);
}

void InputMap::action_add_event(const StringName &p_action, const Ref<InputEvent> &p_event) {

	ERR_FAIL_COND(p_event.is_null());
	ERR_FAIL_COND(!input_map.has(p_action));
	if (_find_event(input_map[p_action].inputs, p_event))
		return; //already gots

	input_map[p_action].inputs.push_back(p_event);
}

bool InputMap::action_has_event(const StringName &p_action, const Ref<InputEvent> &p_event) {

	ERR_FAIL_COND_V(!input_map.has(p_action), false);
	return (_find_event(input_map[p_action].inputs, p_event) != NULL);
}

void InputMap::action_erase_event(const StringName &p_action, const Ref<InputEvent> &p_event) {

	ERR_FAIL_COND(!input_map.has(p_action));

	List<Ref<InputEvent> >::Element *E = _find_event(input_map[p_action].inputs, p_event);
	if (E)
		input_map[p_action].inputs.erase(E);
}

void InputMap::axis_add_event(const StringName &p_axis, const Ref<InputEvent> &p_event, float value) {

	ERR_FAIL_COND(p_event.is_null());
	ERR_FAIL_COND(!input_axis_map.has(p_axis));
	if (_find_event(input_axis_map[p_axis].inputs, p_event))
		return; //already gots

	Map<Ref<InputEvent>, float> newMap = Map<Ref<InputEvent>, float>();
	newMap.insert(p_event, value);
	input_axis_map[p_axis].inputs.push_back(newMap);
}

bool InputMap::axis_has_event(const StringName &p_axis, const Ref<InputEvent> &p_event) {

	ERR_FAIL_COND_V(!input_axis_map.has(p_axis), false);
	return (_find_event(input_axis_map[p_axis].inputs, p_event) != NULL);
}

void InputMap::axis_erase_event(const StringName &p_axis, const Ref<InputEvent> &p_event) {

	ERR_FAIL_COND(!input_axis_map.has(p_axis));

	List<Map<Ref<InputEvent>, float> >::Element *E = _find_event(input_axis_map[p_axis].inputs, p_event);
	if (E)
		input_axis_map[p_axis].inputs.erase(E);
}

Array InputMap::_get_action_list(const StringName &p_action) {

	Array ret;
	const List<Ref<InputEvent> > *al = get_action_list(p_action);
	if (al) {
		for (const List<Ref<InputEvent> >::Element *E = al->front(); E; E = E->next()) {

			ret.push_back(E->get());
		}
	}

	return ret;
}

const List<Ref<InputEvent> > *InputMap::get_action_list(const StringName &p_action) {

	const Map<StringName, Action>::Element *E = input_map.find(p_action);
	if (!E)
		return NULL;

	return &E->get().inputs;
}

Array InputMap::_get_axis_list(const StringName &p_axis) {

	Array ret;
	const List<Map<Ref<InputEvent>, float> > *al = get_axis_list(p_axis);
	if (al) {
		for (const List<Map<Ref<InputEvent>, float> >::Element *E = al->front(); E; E = E->next()) {

			Array m;
			Map<Ref<InputEvent>, float>::Element *e = E->get().front();
			m.push_back(e->key());
			m.push_back(e->value());
			ret.push_back(m);
		}
	}

	return ret;
}

const List<Map<Ref<InputEvent>, float> > *InputMap::get_axis_list(const StringName &p_axis) {

	const Map<StringName, Axis>::Element *E = input_axis_map.find(p_axis);
	if (!E)
		return NULL;

	return &E->get().inputs;
}

bool InputMap::event_is_action(const Ref<InputEvent> &p_event, const StringName &p_action) const {

	Map<StringName, Action>::Element *E = input_map.find(p_action);
	if (!E) {
		ERR_EXPLAIN("Request for nonexistent InputMap action: " + String(p_action));
		ERR_FAIL_COND_V(!E, false);
	}

	Ref<InputEventAction> iea = p_event;
	if (iea.is_valid()) {
		return iea->get_action() == p_action;
	}

	return _find_event(E->get().inputs, p_event, true) != NULL;
}

bool InputMap::event_is_axis(const Ref<InputEvent> &p_event, const StringName &p_axis) const {

	Map<StringName, Axis>::Element *E = input_axis_map.find(p_axis);
	if (!E) {
		ERR_EXPLAIN("Request for nonexistent InputMap axis: " + String(p_axis));
		ERR_FAIL_COND_V(!E, false);
	}

	Ref<InputEventAxis> iea = p_event;
	if (iea.is_valid()) {
		return iea->get_axis() == p_axis;
	}

	return _find_event(E->get().inputs, p_event, true) != NULL;
}

float InputMap::event_get_axis_value(const Ref<InputEvent> &p_event, const StringName &p_axis) {
	if (!event_is_axis(p_event, p_axis))
		return 0.0f;

	// If event_is_axis returned true, these will be valid pointers.
	List<Map<Ref<InputEvent>, float> >::Element *E = _find_event(input_axis_map.find(p_axis)->get().inputs, p_event, true);
	return E->get().front()->get();
}

const Map<StringName, InputMap::Action> &InputMap::get_action_map() const {
	return input_map;
}

const Map<StringName, InputMap::Axis> &InputMap::get_axis_map() const {
	return input_axis_map;
}

void InputMap::load_from_globals() {

	input_map.clear();
	// TODO: add input axis map loading.
	// Also TODO: figure out what exactly is going on here.

	List<PropertyInfo> pinfo;
	ProjectSettings::get_singleton()->get_property_list(&pinfo);

	for (List<PropertyInfo>::Element *E = pinfo.front(); E; E = E->next()) {
		const PropertyInfo &pi = E->get();

		if (!pi.name.begins_with("input/"))
			continue;

		String name = pi.name.substr(pi.name.find("/") + 1, pi.name.length());

		add_action(name);

		Array va = ProjectSettings::get_singleton()->get(pi.name);

		for (int i = 0; i < va.size(); i++) {

			Ref<InputEvent> ie = va[i];
			if (ie.is_null())
				continue;
			action_add_event(name, ie);
		}
	}
}

void InputMap::load_default() {

	Ref<InputEventKey> key;

	add_action("ui_accept");
	key.instance();
	key->set_scancode(KEY_ENTER);
	action_add_event("ui_accept", key);

	key.instance();
	key->set_scancode(KEY_KP_ENTER);
	action_add_event("ui_accept", key);

	key.instance();
	key->set_scancode(KEY_SPACE);
	action_add_event("ui_accept", key);

	add_action("ui_select");
	key.instance();
	key->set_scancode(KEY_SPACE);
	action_add_event("ui_select", key);

	add_action("ui_cancel");
	key.instance();
	key->set_scancode(KEY_ESCAPE);
	action_add_event("ui_cancel", key);

	add_action("ui_focus_next");
	key.instance();
	key->set_scancode(KEY_TAB);
	action_add_event("ui_focus_next", key);

	add_action("ui_focus_prev");
	key.instance();
	key->set_scancode(KEY_TAB);
	key->set_shift(true);
	action_add_event("ui_focus_prev", key);

	add_action("ui_left");
	key.instance();
	key->set_scancode(KEY_LEFT);
	action_add_event("ui_left", key);

	add_action("ui_right");
	key.instance();
	key->set_scancode(KEY_RIGHT);
	action_add_event("ui_right", key);

	add_action("ui_up");
	key.instance();
	key->set_scancode(KEY_UP);
	action_add_event("ui_up", key);

	add_action("ui_down");
	key.instance();
	key->set_scancode(KEY_DOWN);
	action_add_event("ui_down", key);

	add_action("ui_page_up");
	key.instance();
	key->set_scancode(KEY_PAGEUP);
	action_add_event("ui_page_up", key);

	add_action("ui_page_down");
	key.instance();
	key->set_scancode(KEY_PAGEDOWN);
	action_add_event("ui_page_down", key);

	//set("display/window/handheld/orientation", "landscape");
}

InputMap::InputMap() {

	ERR_FAIL_COND(singleton);
	singleton = this;
}
