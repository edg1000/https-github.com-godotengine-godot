/*************************************************************************/
/*  input_event.cpp                                                      */
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

#include "input_event.h"

#include "core/input/input_map.h"
#include "core/os/keyboard.h"

const int InputEvent::DEVICE_ID_TOUCH_MOUSE = -1;
const int InputEvent::DEVICE_ID_INTERNAL = -2;

void InputEvent::set_device(int p_device) {
	device = p_device;
}

int InputEvent::get_device() const {
	return device;
}

bool InputEvent::is_action(const StringName &p_action) const {
	return InputMap::get_singleton()->event_is_action(Ref<InputEvent>((InputEvent *)this), p_action);
}

bool InputEvent::is_action_pressed(const StringName &p_action, bool p_allow_echo) const {
	bool pressed;
	bool valid = InputMap::get_singleton()->event_get_action_status(Ref<InputEvent>((InputEvent *)this), p_action, &pressed);
	return valid && pressed && (p_allow_echo || !is_echo());
}

bool InputEvent::is_action_released(const StringName &p_action) const {
	bool pressed;
	bool valid = InputMap::get_singleton()->event_get_action_status(Ref<InputEvent>((InputEvent *)this), p_action, &pressed);
	return valid && !pressed;
}

float InputEvent::get_action_strength(const StringName &p_action) const {
	bool pressed;
	float strength;
	bool valid = InputMap::get_singleton()->event_get_action_status(Ref<InputEvent>((InputEvent *)this), p_action, &pressed, &strength);
	return valid ? strength : 0.0f;
}

bool InputEvent::is_pressed() const {
	return false;
}

bool InputEvent::is_echo() const {
	return false;
}

Ref<InputEvent> InputEvent::xformed_by(const Transform2D &p_xform, const Vector2 &p_local_ofs) const {
	return Ref<InputEvent>((InputEvent *)this);
}

String InputEvent::as_text() const {
	return String();
}

String InputEvent::to_string() {
	return String();
}

bool InputEvent::action_match(const Ref<InputEvent> &p_event, bool *p_pressed, float *p_strength, float p_deadzone) const {
	return false;
}

bool InputEvent::shortcut_match(const Ref<InputEvent> &p_event) const {
	return false;
}

bool InputEvent::is_action_type() const {
	return false;
}

void InputEvent::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_device", "device"), &InputEvent::set_device);
	ClassDB::bind_method(D_METHOD("get_device"), &InputEvent::get_device);

	ClassDB::bind_method(D_METHOD("is_action", "action"), &InputEvent::is_action);
	ClassDB::bind_method(D_METHOD("is_action_pressed", "action", "allow_echo"), &InputEvent::is_action_pressed, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("is_action_released", "action"), &InputEvent::is_action_released);
	ClassDB::bind_method(D_METHOD("get_action_strength", "action"), &InputEvent::get_action_strength);

	ClassDB::bind_method(D_METHOD("is_pressed"), &InputEvent::is_pressed);
	ClassDB::bind_method(D_METHOD("is_echo"), &InputEvent::is_echo);

	ClassDB::bind_method(D_METHOD("as_text"), &InputEvent::as_text);

	ClassDB::bind_method(D_METHOD("shortcut_match", "event"), &InputEvent::shortcut_match);

	ClassDB::bind_method(D_METHOD("is_action_type"), &InputEvent::is_action_type);

	ClassDB::bind_method(D_METHOD("accumulate", "with_event"), &InputEvent::accumulate);

	ClassDB::bind_method(D_METHOD("xformed_by", "xform", "local_ofs"), &InputEvent::xformed_by, DEFVAL(Vector2()));

	ADD_PROPERTY(PropertyInfo(Variant::INT, "device"), "set_device", "get_device");
}

///////////////////////////////////

void InputEventFromWindow::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_window_id", "id"), &InputEventFromWindow::set_window_id);
	ClassDB::bind_method(D_METHOD("get_window_id"), &InputEventFromWindow::get_window_id);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "window_id"), "set_window_id", "get_window_id");
}

void InputEventFromWindow::set_window_id(int64_t p_id) {
	window_id = p_id;
}

int64_t InputEventFromWindow::get_window_id() const {
	return window_id;
}

///////////////////////////////////

void InputEventWithModifiers::set_shift(bool p_enabled) {
	shift = p_enabled;
}

bool InputEventWithModifiers::get_shift() const {
	return shift;
}

void InputEventWithModifiers::set_alt(bool p_enabled) {
	alt = p_enabled;
}

bool InputEventWithModifiers::get_alt() const {
	return alt;
}

void InputEventWithModifiers::set_control(bool p_enabled) {
	control = p_enabled;
}

bool InputEventWithModifiers::get_control() const {
	return control;
}

void InputEventWithModifiers::set_metakey(bool p_enabled) {
	meta = p_enabled;
}

bool InputEventWithModifiers::get_metakey() const {
	return meta;
}

void InputEventWithModifiers::set_command(bool p_enabled) {
	command = p_enabled;
}

bool InputEventWithModifiers::get_command() const {
	return command;
}

void InputEventWithModifiers::set_modifiers_from_event(const InputEventWithModifiers *event) {
	set_alt(event->get_alt());
	set_shift(event->get_shift());
	set_control(event->get_control());
	set_metakey(event->get_metakey());
}

String InputEventWithModifiers::as_text() const {
	Vector<String> mod_names;

	if (get_control()) {
		mod_names.push_back(find_keycode_name(KEY_CONTROL));
	}
	if (get_shift()) {
		mod_names.push_back(find_keycode_name(KEY_SHIFT));
	}
	if (get_alt()) {
		mod_names.push_back(find_keycode_name(KEY_ALT));
	}
	if (get_metakey()) {
		mod_names.push_back(find_keycode_name(KEY_META));
	}

	return String("+").join(mod_names);
}

String InputEventWithModifiers::to_string() {
	return as_text();
}

void InputEventWithModifiers::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_alt", "enable"), &InputEventWithModifiers::set_alt);
	ClassDB::bind_method(D_METHOD("get_alt"), &InputEventWithModifiers::get_alt);

	ClassDB::bind_method(D_METHOD("set_shift", "enable"), &InputEventWithModifiers::set_shift);
	ClassDB::bind_method(D_METHOD("get_shift"), &InputEventWithModifiers::get_shift);

	ClassDB::bind_method(D_METHOD("set_control", "enable"), &InputEventWithModifiers::set_control);
	ClassDB::bind_method(D_METHOD("get_control"), &InputEventWithModifiers::get_control);

	ClassDB::bind_method(D_METHOD("set_metakey", "enable"), &InputEventWithModifiers::set_metakey);
	ClassDB::bind_method(D_METHOD("get_metakey"), &InputEventWithModifiers::get_metakey);

	ClassDB::bind_method(D_METHOD("set_command", "enable"), &InputEventWithModifiers::set_command);
	ClassDB::bind_method(D_METHOD("get_command"), &InputEventWithModifiers::get_command);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "alt"), "set_alt", "get_alt");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "shift"), "set_shift", "get_shift");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "control"), "set_control", "get_control");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "meta"), "set_metakey", "get_metakey");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "command"), "set_command", "get_command");
}

///////////////////////////////////

void InputEventKey::set_pressed(bool p_pressed) {
	pressed = p_pressed;
}

bool InputEventKey::is_pressed() const {
	return pressed;
}

void InputEventKey::set_keycode(uint32_t p_keycode) {
	keycode = p_keycode;
}

uint32_t InputEventKey::get_keycode() const {
	return keycode;
}

void InputEventKey::set_physical_keycode(uint32_t p_keycode) {
	physical_keycode = p_keycode;
}

uint32_t InputEventKey::get_physical_keycode() const {
	return physical_keycode;
}

void InputEventKey::set_unicode(uint32_t p_unicode) {
	unicode = p_unicode;
}

uint32_t InputEventKey::get_unicode() const {
	return unicode;
}

void InputEventKey::set_echo(bool p_enable) {
	echo = p_enable;
}

bool InputEventKey::is_echo() const {
	return echo;
}

uint32_t InputEventKey::get_keycode_with_modifiers() const {
	uint32_t sc = keycode;
	if (get_control()) {
		sc |= KEY_MASK_CTRL;
	}
	if (get_alt()) {
		sc |= KEY_MASK_ALT;
	}
	if (get_shift()) {
		sc |= KEY_MASK_SHIFT;
	}
	if (get_metakey()) {
		sc |= KEY_MASK_META;
	}

	return sc;
}

uint32_t InputEventKey::get_physical_keycode_with_modifiers() const {
	uint32_t sc = physical_keycode;
	if (get_control()) {
		sc |= KEY_MASK_CTRL;
	}
	if (get_alt()) {
		sc |= KEY_MASK_ALT;
	}
	if (get_shift()) {
		sc |= KEY_MASK_SHIFT;
	}
	if (get_metakey()) {
		sc |= KEY_MASK_META;
	}

	return sc;
}

String InputEventKey::as_text() const {
	String kc;

	if (keycode == 0) {
		kc = keycode_get_string(physical_keycode) + " (" + RTR("Physical") + ")";
	} else {
		kc = keycode_get_string(keycode);
	}

	if (kc == String()) {
		return kc;
	}

	String mods_text = InputEventWithModifiers::as_text();
	return mods_text == "" ? kc : mods_text + "+" + kc;
}

String InputEventKey::to_string() {
	String p = is_pressed() ? "true" : "false";
	String e = is_echo() ? "true" : "false";

	if (keycode == 0) {
		return vformat("InputEventKey: keycode=%s mods=%s physical=%s pressed=%s echo=%s", itos(physical_keycode) + " " + keycode_get_string(physical_keycode), InputEventWithModifiers::as_text(), "true", p, e);
	} else {
		return vformat("InputEventKey: keycode=%s mods=%s physical=%s pressed=%s echo=%s", itos(keycode) + " " + keycode_get_string(keycode), InputEventWithModifiers::as_text(), "false", p, e);
	}
}

bool InputEventKey::action_match(const Ref<InputEvent> &p_event, bool *p_pressed, float *p_strength, float p_deadzone) const {
	Ref<InputEventKey> key = p_event;
	if (key.is_null()) {
		return false;
	}

	bool match = false;
	if (get_keycode() == 0) {
		uint32_t code = get_physical_keycode_with_modifiers();
		uint32_t event_code = key->get_physical_keycode_with_modifiers();

		match = get_physical_keycode() == key->get_physical_keycode() && (!key->is_pressed() || (code & event_code) == code);
	} else {
		uint32_t code = get_keycode_with_modifiers();
		uint32_t event_code = key->get_keycode_with_modifiers();

		match = get_keycode() == key->get_keycode() && (!key->is_pressed() || (code & event_code) == code);
	}
	if (match) {
		if (p_pressed != nullptr) {
			*p_pressed = key->is_pressed();
		}
		if (p_strength != nullptr) {
			*p_strength = (p_pressed != nullptr && *p_pressed) ? 1.0f : 0.0f;
		}
	}
	return match;
}

bool InputEventKey::shortcut_match(const Ref<InputEvent> &p_event) const {
	Ref<InputEventKey> key = p_event;
	if (key.is_null()) {
		return false;
	}

	uint32_t code = get_keycode_with_modifiers();
	uint32_t event_code = key->get_keycode_with_modifiers();

	return code == event_code;
}

void InputEventKey::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_pressed", "pressed"), &InputEventKey::set_pressed);

	ClassDB::bind_method(D_METHOD("set_keycode", "keycode"), &InputEventKey::set_keycode);
	ClassDB::bind_method(D_METHOD("get_keycode"), &InputEventKey::get_keycode);

	ClassDB::bind_method(D_METHOD("set_physical_keycode", "physical_keycode"), &InputEventKey::set_physical_keycode);
	ClassDB::bind_method(D_METHOD("get_physical_keycode"), &InputEventKey::get_physical_keycode);

	ClassDB::bind_method(D_METHOD("set_unicode", "unicode"), &InputEventKey::set_unicode);
	ClassDB::bind_method(D_METHOD("get_unicode"), &InputEventKey::get_unicode);

	ClassDB::bind_method(D_METHOD("set_echo", "echo"), &InputEventKey::set_echo);

	ClassDB::bind_method(D_METHOD("get_keycode_with_modifiers"), &InputEventKey::get_keycode_with_modifiers);
	ClassDB::bind_method(D_METHOD("get_physical_keycode_with_modifiers"), &InputEventKey::get_physical_keycode_with_modifiers);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "pressed"), "set_pressed", "is_pressed");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "keycode"), "set_keycode", "get_keycode");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "physical_keycode"), "set_physical_keycode", "get_physical_keycode");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "unicode"), "set_unicode", "get_unicode");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "echo"), "set_echo", "is_echo");
}

///////////////////////////////////

void InputEventMouse::set_button_mask(int p_mask) {
	button_mask = p_mask;
}

int InputEventMouse::get_button_mask() const {
	return button_mask;
}

void InputEventMouse::set_position(const Vector2 &p_pos) {
	pos = p_pos;
}

Vector2 InputEventMouse::get_position() const {
	return pos;
}

void InputEventMouse::set_global_position(const Vector2 &p_global_pos) {
	global_pos = p_global_pos;
}

Vector2 InputEventMouse::get_global_position() const {
	return global_pos;
}

void InputEventMouse::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_button_mask", "button_mask"), &InputEventMouse::set_button_mask);
	ClassDB::bind_method(D_METHOD("get_button_mask"), &InputEventMouse::get_button_mask);

	ClassDB::bind_method(D_METHOD("set_position", "position"), &InputEventMouse::set_position);
	ClassDB::bind_method(D_METHOD("get_position"), &InputEventMouse::get_position);

	ClassDB::bind_method(D_METHOD("set_global_position", "global_position"), &InputEventMouse::set_global_position);
	ClassDB::bind_method(D_METHOD("get_global_position"), &InputEventMouse::get_global_position);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "button_mask"), "set_button_mask", "get_button_mask");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "position"), "set_position", "get_position");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "global_position"), "set_global_position", "get_global_position");
}

///////////////////////////////////

void InputEventMouseButton::set_factor(float p_factor) {
	factor = p_factor;
}

float InputEventMouseButton::get_factor() const {
	return factor;
}

void InputEventMouseButton::set_button_index(int p_index) {
	button_index = p_index;
}

int InputEventMouseButton::get_button_index() const {
	return button_index;
}

void InputEventMouseButton::set_pressed(bool p_pressed) {
	pressed = p_pressed;
}

bool InputEventMouseButton::is_pressed() const {
	return pressed;
}

void InputEventMouseButton::set_doubleclick(bool p_doubleclick) {
	doubleclick = p_doubleclick;
}

bool InputEventMouseButton::is_doubleclick() const {
	return doubleclick;
}

Ref<InputEvent> InputEventMouseButton::xformed_by(const Transform2D &p_xform, const Vector2 &p_local_ofs) const {
	Vector2 g = get_global_position();
	Vector2 l = p_xform.xform(get_position() + p_local_ofs);

	Ref<InputEventMouseButton> mb;
	mb.instance();

	mb->set_device(get_device());
	mb->set_window_id(get_window_id());
	mb->set_modifiers_from_event(this);

	mb->set_position(l);
	mb->set_global_position(g);

	mb->set_button_mask(get_button_mask());
	mb->set_pressed(pressed);
	mb->set_doubleclick(doubleclick);
	mb->set_factor(factor);
	mb->set_button_index(button_index);

	return mb;
}

bool InputEventMouseButton::action_match(const Ref<InputEvent> &p_event, bool *p_pressed, float *p_strength, float p_deadzone) const {
	Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_null()) {
		return false;
	}

	bool match = mb->button_index == button_index;
	if (match) {
		if (p_pressed != nullptr) {
			*p_pressed = mb->is_pressed();
		}
		if (p_strength != nullptr) {
			*p_strength = (p_pressed != nullptr && *p_pressed) ? 1.0f : 0.0f;
		}
	}

	return match;
}

static const char *_mouse_button_descriptions[9] = {
	TTRC("Left Mouse Button"),
	TTRC("Right Mouse Button"),
	TTRC("Middle Mouse Button"),
	TTRC("Mouse Wheel Up"),
	TTRC("Mouse Wheel Down"),
	TTRC("Mouse Wheel Left"),
	TTRC("Mouse Wheel Right"),
	TTRC("Mouse Thumb Button 1"),
	TTRC("Mouse Thumb Button 2")
};

String InputEventMouseButton::as_text() const {
	// Modifiers
	String mods_text = InputEventWithModifiers::as_text();
	String full_string = mods_text == "" ? "" : mods_text + "+";

	// Button
	int idx = get_button_index();
	switch (idx) {
		case BUTTON_LEFT:
		case BUTTON_RIGHT:
		case BUTTON_MIDDLE:
		case BUTTON_WHEEL_UP:
		case BUTTON_WHEEL_DOWN:
		case BUTTON_WHEEL_LEFT:
		case BUTTON_WHEEL_RIGHT:
		case BUTTON_XBUTTON1:
		case BUTTON_XBUTTON2:
			full_string += RTR(_mouse_button_descriptions[idx - 1]); // button index starts from 1, array index starts from 0, so subtract 1
			break;
		default:
			full_string += RTR("Button") + " #" + itos(idx);
			break;
	}

	// Double Click
	if (doubleclick) {
		full_string += " (" + RTR("Double Click") + ")";
	}

	return full_string;
}

String InputEventMouseButton::to_string() {
	String p = is_pressed() ? "true" : "false";
	String d = doubleclick ? "true" : "false";

	int idx = get_button_index();
	String button_string = itos(idx);

	switch (idx) {
		case BUTTON_LEFT:
		case BUTTON_RIGHT:
		case BUTTON_MIDDLE:
		case BUTTON_WHEEL_UP:
		case BUTTON_WHEEL_DOWN:
		case BUTTON_WHEEL_LEFT:
		case BUTTON_WHEEL_RIGHT:
		case BUTTON_XBUTTON1:
		case BUTTON_XBUTTON2:
			button_string += " (" + RTR(_mouse_button_descriptions[idx - 1]) + ")"; // button index starts from 1, array index starts from 0, so subtract 1
			break;
		default:
			break;
	}

	return vformat("InputEventMouseButton: button_index=%s pressed=%s position=(%s) button_mask=%s doubleclick=%s", button_index, p, String(get_position()), itos(get_button_mask()), d);
}

void InputEventMouseButton::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_factor", "factor"), &InputEventMouseButton::set_factor);
	ClassDB::bind_method(D_METHOD("get_factor"), &InputEventMouseButton::get_factor);

	ClassDB::bind_method(D_METHOD("set_button_index", "button_index"), &InputEventMouseButton::set_button_index);
	ClassDB::bind_method(D_METHOD("get_button_index"), &InputEventMouseButton::get_button_index);

	ClassDB::bind_method(D_METHOD("set_pressed", "pressed"), &InputEventMouseButton::set_pressed);
	//	ClassDB::bind_method(D_METHOD("is_pressed"), &InputEventMouseButton::is_pressed);

	ClassDB::bind_method(D_METHOD("set_doubleclick", "doubleclick"), &InputEventMouseButton::set_doubleclick);
	ClassDB::bind_method(D_METHOD("is_doubleclick"), &InputEventMouseButton::is_doubleclick);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "factor"), "set_factor", "get_factor");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "button_index"), "set_button_index", "get_button_index");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "pressed"), "set_pressed", "is_pressed");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "doubleclick"), "set_doubleclick", "is_doubleclick");
}

///////////////////////////////////

void InputEventMouseMotion::set_tilt(const Vector2 &p_tilt) {
	tilt = p_tilt;
}

Vector2 InputEventMouseMotion::get_tilt() const {
	return tilt;
}

void InputEventMouseMotion::set_pressure(float p_pressure) {
	pressure = p_pressure;
}

float InputEventMouseMotion::get_pressure() const {
	return pressure;
}

void InputEventMouseMotion::set_relative(const Vector2 &p_relative) {
	relative = p_relative;
}

Vector2 InputEventMouseMotion::get_relative() const {
	return relative;
}

void InputEventMouseMotion::set_speed(const Vector2 &p_speed) {
	speed = p_speed;
}

Vector2 InputEventMouseMotion::get_speed() const {
	return speed;
}

Ref<InputEvent> InputEventMouseMotion::xformed_by(const Transform2D &p_xform, const Vector2 &p_local_ofs) const {
	Vector2 g = get_global_position();
	Vector2 l = p_xform.xform(get_position() + p_local_ofs);
	Vector2 r = p_xform.basis_xform(get_relative());
	Vector2 s = p_xform.basis_xform(get_speed());

	Ref<InputEventMouseMotion> mm;
	mm.instance();

	mm->set_device(get_device());
	mm->set_window_id(get_window_id());

	mm->set_modifiers_from_event(this);

	mm->set_position(l);
	mm->set_pressure(get_pressure());
	mm->set_tilt(get_tilt());
	mm->set_global_position(g);

	mm->set_button_mask(get_button_mask());
	mm->set_relative(r);
	mm->set_speed(s);

	return mm;
}

String InputEventMouseMotion::as_text() const {
	return vformat(RTR("Mouse motion at position (%s) with speed (%s)"), String(get_position()), String(get_speed()));
}

String InputEventMouseMotion::to_string() {
	int button_mask = get_button_mask();
	String button_mask_string = itos(button_mask);
	switch (get_button_mask()) {
		case BUTTON_MASK_LEFT:
			button_mask_string += " (" + RTR(_mouse_button_descriptions[BUTTON_LEFT - 1]) + ")";
			break;
		case BUTTON_MASK_MIDDLE:
			button_mask_string += " (" + RTR(_mouse_button_descriptions[BUTTON_MIDDLE - 1]) + ")";
			break;
		case BUTTON_MASK_RIGHT:
			button_mask_string += " (" + RTR(_mouse_button_descriptions[BUTTON_RIGHT - 1]) + ")";
			break;
		case BUTTON_MASK_XBUTTON1:
			button_mask_string += " (" + RTR(_mouse_button_descriptions[BUTTON_XBUTTON1 - 1]) + ")";
			break;
		case BUTTON_MASK_XBUTTON2:
			button_mask_string += " (" + RTR(_mouse_button_descriptions[BUTTON_XBUTTON2 - 1]) + ")";
			break;
		default:
			break;
	}

	return "InputEventMouseMotion : button_mask=" + button_mask_string + ", position=(" + String(get_position()) + "), relative=(" + String(get_relative()) + "), speed=(" + String(get_speed()) + "), pressure=(" + rtos(get_pressure()) + "), tilt=(" + String(get_tilt()) + ")";
}

bool InputEventMouseMotion::accumulate(const Ref<InputEvent> &p_event) {
	Ref<InputEventMouseMotion> motion = p_event;
	if (motion.is_null()) {
		return false;
	}

	if (get_window_id() != motion->get_window_id()) {
		return false;
	}

	if (is_pressed() != motion->is_pressed()) {
		return false;
	}

	if (get_button_mask() != motion->get_button_mask()) {
		return false;
	}

	if (get_shift() != motion->get_shift()) {
		return false;
	}

	if (get_control() != motion->get_control()) {
		return false;
	}

	if (get_alt() != motion->get_alt()) {
		return false;
	}

	if (get_metakey() != motion->get_metakey()) {
		return false;
	}

	set_position(motion->get_position());
	set_global_position(motion->get_global_position());
	set_speed(motion->get_speed());
	relative += motion->get_relative();

	return true;
}

void InputEventMouseMotion::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_tilt", "tilt"), &InputEventMouseMotion::set_tilt);
	ClassDB::bind_method(D_METHOD("get_tilt"), &InputEventMouseMotion::get_tilt);

	ClassDB::bind_method(D_METHOD("set_pressure", "pressure"), &InputEventMouseMotion::set_pressure);
	ClassDB::bind_method(D_METHOD("get_pressure"), &InputEventMouseMotion::get_pressure);

	ClassDB::bind_method(D_METHOD("set_relative", "relative"), &InputEventMouseMotion::set_relative);
	ClassDB::bind_method(D_METHOD("get_relative"), &InputEventMouseMotion::get_relative);

	ClassDB::bind_method(D_METHOD("set_speed", "speed"), &InputEventMouseMotion::set_speed);
	ClassDB::bind_method(D_METHOD("get_speed"), &InputEventMouseMotion::get_speed);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "tilt"), "set_tilt", "get_tilt");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "pressure"), "set_pressure", "get_pressure");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "relative"), "set_relative", "get_relative");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "speed"), "set_speed", "get_speed");
}

///////////////////////////////////

void InputEventJoypadMotion::set_axis(int p_axis) {
	axis = p_axis;
}

int InputEventJoypadMotion::get_axis() const {
	return axis;
}

void InputEventJoypadMotion::set_axis_value(float p_value) {
	axis_value = p_value;
}

float InputEventJoypadMotion::get_axis_value() const {
	return axis_value;
}

bool InputEventJoypadMotion::is_pressed() const {
	return Math::abs(axis_value) >= 0.5f;
}

bool InputEventJoypadMotion::action_match(const Ref<InputEvent> &p_event, bool *p_pressed, float *p_strength, float p_deadzone) const {
	Ref<InputEventJoypadMotion> jm = p_event;
	if (jm.is_null()) {
		return false;
	}

	bool match = (axis == jm->axis); // Matches even if not in the same direction, but returns a "not pressed" event.
	if (match) {
		bool same_direction = (((axis_value < 0) == (jm->axis_value < 0)) || jm->axis_value == 0);
		bool pressed = same_direction ? Math::abs(jm->get_axis_value()) >= p_deadzone : false;
		if (p_pressed != nullptr) {
			*p_pressed = pressed;
		}
		if (p_strength != nullptr) {
			if (pressed) {
				if (p_deadzone == 1.0f) {
					*p_strength = 1.0f;
				} else {
					*p_strength = CLAMP(Math::inverse_lerp(p_deadzone, 1.0f, Math::abs(jm->get_axis_value())), 0.0f, 1.0f);
				}
			} else {
				*p_strength = 0.0f;
			}
		}
	}
	return match;
}

// Maps to 2*axis if value is neg, or + 1 if value is pos
static const char *_joy_axis_descriptions[JOY_AXIS_MAX * 2] = {
	TTRC("Left Stick Left"),
	TTRC("Left Stick Right"),
	TTRC("Left Stick Up"),
	TTRC("Left Stick Down"),
	TTRC("Right Stick Left"),
	TTRC("Right Stick Right"),
	TTRC("Right Stick Up"),
	TTRC("Right Stick Down"),
	TTRC("Joystick 2 Left"),
	TTRC("Joystick 2 Right, Left Trigger, L2, LT"),
	TTRC("Joystick 2 Up"),
	TTRC("Joystick 2 Down, Right Trigger, R2, RT"),
	TTRC("Joystick 3 Left"),
	TTRC("Joystick 3 Right"),
	TTRC("Joystick 3 Up"),
	TTRC("Joystick 3 Down"),
	TTRC("Joystick 4 Left"),
	TTRC("Joystick 4 Right"),
	TTRC("Joystick 4 Up"),
	TTRC("Joystick 4 Down"),
};

String InputEventJoypadMotion::as_text() const {
	String description = "";
	if (axis < JOY_AXIS_MAX) {
		description = vformat(" (%s)", RTR(_joy_axis_descriptions[2 * axis + (axis_value < 0 ? 0 : 1)]));
	}

	return vformat(RTR("Joypad Motion on Axis %s%s with Value %s"), itos(axis), description, String(Variant(axis_value)));
}

String InputEventJoypadMotion::to_string() {
	return "InputEventJoypadMotion : axis=" + itos(axis) + ", axis_value=" + String(Variant(axis_value));
}

void InputEventJoypadMotion::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_axis", "axis"), &InputEventJoypadMotion::set_axis);
	ClassDB::bind_method(D_METHOD("get_axis"), &InputEventJoypadMotion::get_axis);

	ClassDB::bind_method(D_METHOD("set_axis_value", "axis_value"), &InputEventJoypadMotion::set_axis_value);
	ClassDB::bind_method(D_METHOD("get_axis_value"), &InputEventJoypadMotion::get_axis_value);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "axis"), "set_axis", "get_axis");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "axis_value"), "set_axis_value", "get_axis_value");
}

///////////////////////////////////

void InputEventJoypadButton::set_button_index(int p_index) {
	button_index = p_index;
}

int InputEventJoypadButton::get_button_index() const {
	return button_index;
}

void InputEventJoypadButton::set_pressed(bool p_pressed) {
	pressed = p_pressed;
}

bool InputEventJoypadButton::is_pressed() const {
	return pressed;
}

void InputEventJoypadButton::set_pressure(float p_pressure) {
	pressure = p_pressure;
}

float InputEventJoypadButton::get_pressure() const {
	return pressure;
}

bool InputEventJoypadButton::action_match(const Ref<InputEvent> &p_event, bool *p_pressed, float *p_strength, float p_deadzone) const {
	Ref<InputEventJoypadButton> jb = p_event;
	if (jb.is_null()) {
		return false;
	}

	bool match = button_index == jb->button_index;
	if (match) {
		if (p_pressed != nullptr) {
			*p_pressed = jb->is_pressed();
		}
		if (p_strength != nullptr) {
			*p_strength = (p_pressed != nullptr && *p_pressed) ? 1.0f : 0.0f;
		}
	}

	return match;
}

bool InputEventJoypadButton::shortcut_match(const Ref<InputEvent> &p_event) const {
	Ref<InputEventJoypadButton> button = p_event;
	if (button.is_null()) {
		return false;
	}

	return button_index == button->button_index;
}

static const char *_joy_button_descriptions[JOY_SDL_BUTTONS] = {
	TTRC("Face Bottom, DualShock Cross, Xbox A, Nintendo B"),
	TTRC("Face Right, DualShock Circle, Xbox B, Nintendo A"),
	TTRC("Face Left, DualShock Square, Xbox X, Nintendo Y"),
	TTRC("Face Top, DualShock Triangle, Xbox Y, Nintendo X"),
	TTRC("DualShock Select, Xbox Back, Nintendo -"),
	TTRC("Home, DualShock PS, Guide"),
	TTRC("Start, Nintendo +"),
	TTRC("Left Stick, DualShock L3, Xbox L/LS"),
	TTRC("Right Stick, DualShock R3, Xbox R/RS"),
	TTRC("Left Shoulder, DualShock L1, Xbox LB"),
	TTRC("Right Shoulder, DualShock R1, Xbox RB"),
	TTRC("D-Pad Up"),
	TTRC("D-Pad Down"),
	TTRC("D-Pad Left"),
	TTRC("D-Pad Right")
};

String InputEventJoypadButton::as_text() const {
	String text = "Joypad Button " + itos(button_index);

	if (button_index < JOY_SDL_BUTTONS) {
		text += vformat(" (%s)", _joy_button_descriptions[button_index]);
	}

	if (pressure != 0) {
		text += ", Pressure:" + String(Variant(pressure));
	}

	return text;
}

String InputEventJoypadButton::to_string() {
	return "InputEventJoypadButton : button_index=" + itos(button_index) + ", pressed=" + (pressed ? "true" : "false") + ", pressure=" + String(Variant(pressure));
}

void InputEventJoypadButton::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_button_index", "button_index"), &InputEventJoypadButton::set_button_index);
	ClassDB::bind_method(D_METHOD("get_button_index"), &InputEventJoypadButton::get_button_index);

	ClassDB::bind_method(D_METHOD("set_pressure", "pressure"), &InputEventJoypadButton::set_pressure);
	ClassDB::bind_method(D_METHOD("get_pressure"), &InputEventJoypadButton::get_pressure);

	ClassDB::bind_method(D_METHOD("set_pressed", "pressed"), &InputEventJoypadButton::set_pressed);
	//	ClassDB::bind_method(D_METHOD("is_pressed"), &InputEventJoypadButton::is_pressed);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "button_index"), "set_button_index", "get_button_index");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "pressure"), "set_pressure", "get_pressure");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "pressed"), "set_pressed", "is_pressed");
}

///////////////////////////////////

void InputEventScreenTouch::set_index(int p_index) {
	index = p_index;
}

int InputEventScreenTouch::get_index() const {
	return index;
}

void InputEventScreenTouch::set_position(const Vector2 &p_pos) {
	pos = p_pos;
}

Vector2 InputEventScreenTouch::get_position() const {
	return pos;
}

void InputEventScreenTouch::set_pressed(bool p_pressed) {
	pressed = p_pressed;
}

bool InputEventScreenTouch::is_pressed() const {
	return pressed;
}

Ref<InputEvent> InputEventScreenTouch::xformed_by(const Transform2D &p_xform, const Vector2 &p_local_ofs) const {
	Ref<InputEventScreenTouch> st;
	st.instance();
	st->set_device(get_device());
	st->set_window_id(get_window_id());
	st->set_index(index);
	st->set_position(p_xform.xform(pos + p_local_ofs));
	st->set_pressed(pressed);

	return st;
}

String InputEventScreenTouch::as_text() const {
	String status = pressed ? RTR("touched") : RTR("released");

	return vformat(RTR("Screen %s at (%s) with %s touch points"), status, String(get_position()), itos(index));
}

String InputEventScreenTouch::to_string() {
	return "InputEventScreenTouch : index=" + itos(index) + ", pressed=" + (pressed ? "true" : "false") + ", position=(" + String(get_position()) + ")";
}

void InputEventScreenTouch::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_index", "index"), &InputEventScreenTouch::set_index);
	ClassDB::bind_method(D_METHOD("get_index"), &InputEventScreenTouch::get_index);

	ClassDB::bind_method(D_METHOD("set_position", "position"), &InputEventScreenTouch::set_position);
	ClassDB::bind_method(D_METHOD("get_position"), &InputEventScreenTouch::get_position);

	ClassDB::bind_method(D_METHOD("set_pressed", "pressed"), &InputEventScreenTouch::set_pressed);
	//ClassDB::bind_method(D_METHOD("is_pressed"),&InputEventScreenTouch::is_pressed);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "index"), "set_index", "get_index");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "position"), "set_position", "get_position");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "pressed"), "set_pressed", "is_pressed");
}

///////////////////////////////////

void InputEventScreenDrag::set_index(int p_index) {
	index = p_index;
}

int InputEventScreenDrag::get_index() const {
	return index;
}

void InputEventScreenDrag::set_position(const Vector2 &p_pos) {
	pos = p_pos;
}

Vector2 InputEventScreenDrag::get_position() const {
	return pos;
}

void InputEventScreenDrag::set_relative(const Vector2 &p_relative) {
	relative = p_relative;
}

Vector2 InputEventScreenDrag::get_relative() const {
	return relative;
}

void InputEventScreenDrag::set_speed(const Vector2 &p_speed) {
	speed = p_speed;
}

Vector2 InputEventScreenDrag::get_speed() const {
	return speed;
}

Ref<InputEvent> InputEventScreenDrag::xformed_by(const Transform2D &p_xform, const Vector2 &p_local_ofs) const {
	Ref<InputEventScreenDrag> sd;

	sd.instance();

	sd->set_device(get_device());
	sd->set_window_id(get_window_id());

	sd->set_index(index);
	sd->set_position(p_xform.xform(pos + p_local_ofs));
	sd->set_relative(p_xform.basis_xform(relative));
	sd->set_speed(p_xform.basis_xform(speed));

	return sd;
}

String InputEventScreenDrag::as_text() const {
	return vformat(RTR("Screen dragged with %s touch points at position (%s) with speed of (%s)"), itos(index), String(get_position()), String(get_speed()));
}

String InputEventScreenDrag::to_string() {
	return "InputEventScreenDrag : index=" + itos(index) + ", position=(" + String(get_position()) + "), relative=(" + String(get_relative()) + "), speed=(" + String(get_speed()) + ")";
}

void InputEventScreenDrag::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_index", "index"), &InputEventScreenDrag::set_index);
	ClassDB::bind_method(D_METHOD("get_index"), &InputEventScreenDrag::get_index);

	ClassDB::bind_method(D_METHOD("set_position", "position"), &InputEventScreenDrag::set_position);
	ClassDB::bind_method(D_METHOD("get_position"), &InputEventScreenDrag::get_position);

	ClassDB::bind_method(D_METHOD("set_relative", "relative"), &InputEventScreenDrag::set_relative);
	ClassDB::bind_method(D_METHOD("get_relative"), &InputEventScreenDrag::get_relative);

	ClassDB::bind_method(D_METHOD("set_speed", "speed"), &InputEventScreenDrag::set_speed);
	ClassDB::bind_method(D_METHOD("get_speed"), &InputEventScreenDrag::get_speed);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "index"), "set_index", "get_index");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "position"), "set_position", "get_position");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "relative"), "set_relative", "get_relative");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "speed"), "set_speed", "get_speed");
}

///////////////////////////////////

void InputEventAction::set_action(const StringName &p_action) {
	action = p_action;
}

StringName InputEventAction::get_action() const {
	return action;
}

void InputEventAction::set_pressed(bool p_pressed) {
	pressed = p_pressed;
}

bool InputEventAction::is_pressed() const {
	return pressed;
}

void InputEventAction::set_strength(float p_strength) {
	strength = CLAMP(p_strength, 0.0f, 1.0f);
}

float InputEventAction::get_strength() const {
	return strength;
}

bool InputEventAction::shortcut_match(const Ref<InputEvent> &p_event) const {
	if (p_event.is_null()) {
		return false;
	}

	return p_event->is_action(action);
}

bool InputEventAction::is_action(const StringName &p_action) const {
	return action == p_action;
}

bool InputEventAction::action_match(const Ref<InputEvent> &p_event, bool *p_pressed, float *p_strength, float p_deadzone) const {
	Ref<InputEventAction> act = p_event;
	if (act.is_null()) {
		return false;
	}

	bool match = action == act->action;
	if (match) {
		if (p_pressed != nullptr) {
			*p_pressed = act->pressed;
		}
		if (p_strength != nullptr) {
			*p_strength = (p_pressed != nullptr && *p_pressed) ? 1.0f : 0.0f;
		}
	}
	return match;
}

String InputEventAction::as_text() const {
	return vformat(RTR("Input Action %s was %s"), action, pressed ? "pressed" : "released");
}

String InputEventAction::to_string() {
	return "InputEventAction : action=" + action + ", pressed=(" + (pressed ? "true" : "false");
}

void InputEventAction::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_action", "action"), &InputEventAction::set_action);
	ClassDB::bind_method(D_METHOD("get_action"), &InputEventAction::get_action);

	ClassDB::bind_method(D_METHOD("set_pressed", "pressed"), &InputEventAction::set_pressed);
	//ClassDB::bind_method(D_METHOD("is_pressed"), &InputEventAction::is_pressed);

	ClassDB::bind_method(D_METHOD("set_strength", "strength"), &InputEventAction::set_strength);
	ClassDB::bind_method(D_METHOD("get_strength"), &InputEventAction::get_strength);

	//	ClassDB::bind_method(D_METHOD("is_action", "name"), &InputEventAction::is_action);

	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "action"), "set_action", "get_action");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "pressed"), "set_pressed", "is_pressed");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "strength", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_strength", "get_strength");
}

///////////////////////////////////

void InputEventGesture::set_position(const Vector2 &p_pos) {
	pos = p_pos;
}

void InputEventGesture::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_position", "position"), &InputEventGesture::set_position);
	ClassDB::bind_method(D_METHOD("get_position"), &InputEventGesture::get_position);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "position"), "set_position", "get_position");
}

Vector2 InputEventGesture::get_position() const {
	return pos;
}

///////////////////////////////////

void InputEventMagnifyGesture::set_factor(real_t p_factor) {
	factor = p_factor;
}

real_t InputEventMagnifyGesture::get_factor() const {
	return factor;
}

Ref<InputEvent> InputEventMagnifyGesture::xformed_by(const Transform2D &p_xform, const Vector2 &p_local_ofs) const {
	Ref<InputEventMagnifyGesture> ev;
	ev.instance();

	ev->set_device(get_device());
	ev->set_window_id(get_window_id());

	ev->set_modifiers_from_event(this);

	ev->set_position(p_xform.xform(get_position() + p_local_ofs));
	ev->set_factor(get_factor());

	return ev;
}

String InputEventMagnifyGesture::as_text() const {
	return vformat(RTR("Magnify Gesture at (%s) with factor %s"), String(get_position()), rtos(get_factor()));
}

String InputEventMagnifyGesture::to_string() {
	return "InputEventMagnifyGesture : factor=" + rtos(get_factor()) + ", position=(" + String(get_position()) + ")";
}

void InputEventMagnifyGesture::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_factor", "factor"), &InputEventMagnifyGesture::set_factor);
	ClassDB::bind_method(D_METHOD("get_factor"), &InputEventMagnifyGesture::get_factor);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "factor"), "set_factor", "get_factor");
}

///////////////////////////////////

void InputEventPanGesture::set_delta(const Vector2 &p_delta) {
	delta = p_delta;
}

Vector2 InputEventPanGesture::get_delta() const {
	return delta;
}

Ref<InputEvent> InputEventPanGesture::xformed_by(const Transform2D &p_xform, const Vector2 &p_local_ofs) const {
	Ref<InputEventPanGesture> ev;
	ev.instance();

	ev->set_device(get_device());
	ev->set_window_id(get_window_id());

	ev->set_modifiers_from_event(this);

	ev->set_position(p_xform.xform(get_position() + p_local_ofs));
	ev->set_delta(get_delta());

	return ev;
}

String InputEventPanGesture::as_text() const {
	return vformat(RTR("Pan Gesture at (%s) with delta (%s)"), String(get_position()), String(get_delta()));
}

String InputEventPanGesture::to_string() {
	return "InputEventPanGesture : delta=(" + String(get_delta()) + "), position=(" + String(get_position()) + ")";
}

void InputEventPanGesture::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_delta", "delta"), &InputEventPanGesture::set_delta);
	ClassDB::bind_method(D_METHOD("get_delta"), &InputEventPanGesture::get_delta);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "delta"), "set_delta", "get_delta");
}

///////////////////////////////////

void InputEventMIDI::set_channel(const int p_channel) {
	channel = p_channel;
}

int InputEventMIDI::get_channel() const {
	return channel;
}

void InputEventMIDI::set_message(const int p_message) {
	message = p_message;
}

int InputEventMIDI::get_message() const {
	return message;
}

void InputEventMIDI::set_pitch(const int p_pitch) {
	pitch = p_pitch;
}

int InputEventMIDI::get_pitch() const {
	return pitch;
}

void InputEventMIDI::set_velocity(const int p_velocity) {
	velocity = p_velocity;
}

int InputEventMIDI::get_velocity() const {
	return velocity;
}

void InputEventMIDI::set_instrument(const int p_instrument) {
	instrument = p_instrument;
}

int InputEventMIDI::get_instrument() const {
	return instrument;
}

void InputEventMIDI::set_pressure(const int p_pressure) {
	pressure = p_pressure;
}

int InputEventMIDI::get_pressure() const {
	return pressure;
}

void InputEventMIDI::set_controller_number(const int p_controller_number) {
	controller_number = p_controller_number;
}

int InputEventMIDI::get_controller_number() const {
	return controller_number;
}

void InputEventMIDI::set_controller_value(const int p_controller_value) {
	controller_value = p_controller_value;
}

int InputEventMIDI::get_controller_value() const {
	return controller_value;
}

String InputEventMIDI::as_text() const {
	return vformat(RTR("MIDI Input on Channel=%s Message=%s"), itos(channel), itos(message));
}

String InputEventMIDI::to_string() {
	return vformat("InputEvenMIDI: channel=%s message=%s pitch=%s velocity=%s pressure=%s", itos(channel), itos(message), itos(pitch), itos(velocity), itos(pressure));
}

void InputEventMIDI::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_channel", "channel"), &InputEventMIDI::set_channel);
	ClassDB::bind_method(D_METHOD("get_channel"), &InputEventMIDI::get_channel);
	ClassDB::bind_method(D_METHOD("set_message", "message"), &InputEventMIDI::set_message);
	ClassDB::bind_method(D_METHOD("get_message"), &InputEventMIDI::get_message);
	ClassDB::bind_method(D_METHOD("set_pitch", "pitch"), &InputEventMIDI::set_pitch);
	ClassDB::bind_method(D_METHOD("get_pitch"), &InputEventMIDI::get_pitch);
	ClassDB::bind_method(D_METHOD("set_velocity", "velocity"), &InputEventMIDI::set_velocity);
	ClassDB::bind_method(D_METHOD("get_velocity"), &InputEventMIDI::get_velocity);
	ClassDB::bind_method(D_METHOD("set_instrument", "instrument"), &InputEventMIDI::set_instrument);
	ClassDB::bind_method(D_METHOD("get_instrument"), &InputEventMIDI::get_instrument);
	ClassDB::bind_method(D_METHOD("set_pressure", "pressure"), &InputEventMIDI::set_pressure);
	ClassDB::bind_method(D_METHOD("get_pressure"), &InputEventMIDI::get_pressure);
	ClassDB::bind_method(D_METHOD("set_controller_number", "controller_number"), &InputEventMIDI::set_controller_number);
	ClassDB::bind_method(D_METHOD("get_controller_number"), &InputEventMIDI::get_controller_number);
	ClassDB::bind_method(D_METHOD("set_controller_value", "controller_value"), &InputEventMIDI::set_controller_value);
	ClassDB::bind_method(D_METHOD("get_controller_value"), &InputEventMIDI::get_controller_value);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "channel"), "set_channel", "get_channel");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "message"), "set_message", "get_message");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "pitch"), "set_pitch", "get_pitch");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "velocity"), "set_velocity", "get_velocity");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "instrument"), "set_instrument", "get_instrument");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "pressure"), "set_pressure", "get_pressure");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "controller_number"), "set_controller_number", "get_controller_number");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "controller_value"), "set_controller_value", "get_controller_value");
}
