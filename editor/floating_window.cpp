/*************************************************************************/
/*  floating_window.cpp                                                  */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
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

#include "floating_window.h"

void FloatingWindow::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			this->connect("window_input", callable_mp(this, &FloatingWindow::_on_window_event));
		} break;
	}
};

void FloatingWindow::_on_window_event(const Ref<InputEvent> &p_event) {
	bool single_window_mode = EditorSettings::get_singleton()->get_setting(
			"interface/editor/single_window_mode");
	if (single_window_mode) {
		Size2i editor_window_size = get_parent_rect().size;
		Vector2 window_position = this->get_position();
		print_line("Window position:");
		print_line(window_position);
		print_line("Size");
		print_line(editor_window_size);
		Rect2 window_size = this->get_visible_rect();
		if (editor_window_size.x < (window_position.x + window_size.size.x)) {
			this->set_position(Vector2((editor_window_size.x - window_size.size.x), window_position.y));
		} else if (window_position.x < 0) {
			this->set_position(Vector2(0, window_position.y));
		}
		if (window_position.y < 0) {
			this->set_position(Vector2(window_position.x, 20));
		} else if (editor_window_size.y < window_position.y) {
			this->set_position(Vector2(window_position.x, editor_window_size.y));
		}
	}
};
