/*************************************************************************/
/*  color_rect.cpp                                                       */
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

#include "color_rect.h"

void ColorRect::set_color(const Color &p_color) {
	if (color == p_color) {
		return;
	}
	color = p_color;
	queue_redraw();
}

Color ColorRect::get_color() const {
	return color;
}

void ColorRect::set_filled(bool p_filled) {
	filled = p_filled;
	queue_redraw();
	notify_property_list_changed();
}

bool ColorRect::is_filled() const {
	return filled;
}

void ColorRect::set_border_width(float p_width) {
	border_width = p_width;
	queue_redraw();
}

float ColorRect::get_border_width() const {
	return border_width;
}

void ColorRect::set_editor_only(bool p_enabled) {
	editor_only = p_enabled;
	queue_redraw();
}

bool ColorRect::is_editor_only() const {
	return editor_only;
}

void ColorRect::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			if (Engine::get_singleton()->is_editor_hint() || !editor_only) {
				draw_rect(Rect2(Point2(), get_size()), color, filled, filled ? 1.0 : border_width);
			}
		} break;
	}
}

void ColorRect::_validate_property(PropertyInfo &property) const {
	if (property.name == "border_width") {
		if (filled) {
			property.usage = PROPERTY_USAGE_NO_EDITOR;
		}
	}
}

void ColorRect::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_color", "color"), &ColorRect::set_color);
	ClassDB::bind_method(D_METHOD("get_color"), &ColorRect::get_color);

	ClassDB::bind_method(D_METHOD("set_filled", "filled"), &ColorRect::set_filled);
	ClassDB::bind_method(D_METHOD("is_filled"), &ColorRect::is_filled);

	ClassDB::bind_method(D_METHOD("set_border_width", "width"), &ColorRect::set_border_width);
	ClassDB::bind_method(D_METHOD("get_border_width"), &ColorRect::get_border_width);

	ClassDB::bind_method(D_METHOD("is_editor_only"), &ColorRect::is_editor_only);
	ClassDB::bind_method(D_METHOD("set_editor_only", "enabled"), &ColorRect::set_editor_only);

	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color"), "set_color", "get_color");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "filled"), "set_filled", "is_filled");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "border_width", PROPERTY_HINT_RANGE, "0.0,5.0,0.1,or_greater"), "set_border_width", "get_border_width"); // Should be after `filled`.
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "editor_only"), "set_editor_only", "is_editor_only");
}
