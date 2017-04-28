/*************************************************************************/
/*  button.cpp                                                           */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2017 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2017 Godot Engine contributors (cf. AUTHORS.md)    */
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
#include "button.h"
#include "print_string.h"
#include "servers/visual_server.h"
#include "translation.h"

Size2 Button::get_minimum_size() const {

	Size2 minsize = get_font("font")->get_string_size(xl_text);
	if (clip_text)
		minsize.width = 0;

	Ref<Texture> _icon;
	if (icon.is_null() && has_icon("icon"))
		_icon = Control::get_icon("icon");
	else
		_icon = icon;

	if (!_icon.is_null()) {

		minsize.height = MAX(minsize.height, _icon->get_height());
		minsize.width += _icon->get_width();
		if (xl_text != "")
			minsize.width += get_constant("hseparation");
	}

	return get_stylebox("normal")->get_minimum_size() + minsize;
}

void Button::_notification(int p_what) {

	if (p_what == NOTIFICATION_TRANSLATION_CHANGED) {

		xl_text = XL_MESSAGE(text);
		minimum_size_changed();
		update();
	}

	if (p_what == NOTIFICATION_DRAW) {

		RID ci = get_canvas_item();
		Size2 size = get_size();
		Color color;
		Color icon_color = Color(1, 1, 1);

		//print_line(get_text()+": "+itos(is_flat())+" hover "+itos(get_draw_mode()));

		String style_name = "";
		switch (get_draw_mode()) {

			case DRAW_NORMAL: {

				style_name = "normal";
				color = get_color("font_color");

			} break;
			case DRAW_PRESSED: {

				style_name = "pressed";
				if (has_color("font_color_pressed")) {
					color = get_color("font_color_pressed");
					icon_color = color;
				} else
					color = get_color("font_color");

			} break;
			case DRAW_HOVER: {

				style_name = "hover";
				color = get_color("font_color_hover");

			} break;
			case DRAW_DISABLED: {

				style_name = "disabled";
				color = get_color("font_color_disabled");

			} break;
		}

		if (style_name != "" && !flat) get_stylebox(style_name)->draw(ci, Rect2(Point2(0, 0), size));

		Ref<StyleBox> style = get_stylebox(style_name);
		if (has_focus()) {

			style->draw(ci, Rect2(Point2(), size));
		}

		Ref<Font> font = get_font("font");
		Ref<Texture> _icon;
		if (icon.is_null() && has_icon("icon"))
			_icon = Control::get_icon("icon");
		else
			_icon = icon;

		Point2 icon_ofs = (!_icon.is_null()) ? Point2(_icon->get_width() + get_constant("hseparation"), 0) : Point2();
		int text_clip = size.width - style->get_minimum_size().width - icon_ofs.width;
		Point2 text_ofs = (size - style->get_minimum_size() - icon_ofs - font->get_string_size(xl_text)) / 2.0;

		switch (align) {
			case ALIGN_LEFT: {
				text_ofs.x = style->get_margin(MARGIN_LEFT) + icon_ofs.x;
				text_ofs.y += style->get_offset().y;
			} break;
			case ALIGN_CENTER: {
				if (text_ofs.x < 0)
					text_ofs.x = 0;
				text_ofs += icon_ofs;
				text_ofs += style->get_offset();
			} break;
			case ALIGN_RIGHT: {
				text_ofs.x = size.x - style->get_margin(MARGIN_RIGHT) - font->get_string_size(xl_text).x;
				text_ofs.y += style->get_offset().y;
			} break;
		}

		text_ofs.y += font->get_ascent();
		font->draw(ci, text_ofs.floor(), xl_text, color, clip_text ? text_clip : -1);
		if (!_icon.is_null()) {

			int valign = size.height - style->get_minimum_size().y;

			_icon->draw(ci, style->get_offset() + Point2(0, Math::floor((valign - _icon->get_height()) / 2.0)), is_disabled() ? Color(1, 1, 1, 0.4) : icon_color);
		}
	}
}

void Button::set_text(const String &p_text) {

	if (text == p_text)
		return;
	text = p_text;
	xl_text = XL_MESSAGE(p_text);
	update();
	_change_notify("text");
	minimum_size_changed();
}
String Button::get_text() const {

	return text;
}

void Button::set_icon(const Ref<Texture> &p_icon) {

	if (icon == p_icon)
		return;
	icon = p_icon;
	update();
	_change_notify("icon");
	minimum_size_changed();
}

Ref<Texture> Button::get_icon() const {

	return icon;
}

void Button::set_flat(bool p_flat) {

	flat = p_flat;
	update();
	_change_notify("flat");
}

bool Button::is_flat() const {

	return flat;
}

void Button::set_clip_text(bool p_clip_text) {

	clip_text = p_clip_text;
	update();
	minimum_size_changed();
}

bool Button::get_clip_text() const {

	return clip_text;
}

void Button::set_text_align(TextAlign p_align) {

	align = p_align;
	update();
}

Button::TextAlign Button::get_text_align() const {

	return align;
}

void Button::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_text", "text"), &Button::set_text);
	ClassDB::bind_method(D_METHOD("get_text"), &Button::get_text);
	ClassDB::bind_method(D_METHOD("set_button_icon", "texture:Texture"), &Button::set_icon);
	ClassDB::bind_method(D_METHOD("get_button_icon:Texture"), &Button::get_icon);
	ClassDB::bind_method(D_METHOD("set_flat", "enabled"), &Button::set_flat);
	ClassDB::bind_method(D_METHOD("set_clip_text", "enabled"), &Button::set_clip_text);
	ClassDB::bind_method(D_METHOD("get_clip_text"), &Button::get_clip_text);
	ClassDB::bind_method(D_METHOD("set_text_align", "align"), &Button::set_text_align);
	ClassDB::bind_method(D_METHOD("get_text_align"), &Button::get_text_align);
	ClassDB::bind_method(D_METHOD("is_flat"), &Button::is_flat);

	BIND_CONSTANT(ALIGN_LEFT);
	BIND_CONSTANT(ALIGN_CENTER);
	BIND_CONSTANT(ALIGN_RIGHT);

	ADD_PROPERTYNZ(PropertyInfo(Variant::STRING, "text", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT_INTL), "set_text", "get_text");
	ADD_PROPERTYNZ(PropertyInfo(Variant::OBJECT, "icon", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_button_icon", "get_button_icon");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "flat"), "set_flat", "is_flat");
	ADD_PROPERTYNZ(PropertyInfo(Variant::BOOL, "clip_text"), "set_clip_text", "get_clip_text");
	ADD_PROPERTYNO(PropertyInfo(Variant::INT, "align", PROPERTY_HINT_ENUM, "Left,Center,Right"), "set_text_align", "get_text_align");
}

Button::Button(const String &p_text) {

	flat = false;
	clip_text = false;
	set_mouse_filter(MOUSE_FILTER_STOP);
	set_text(p_text);
	align = ALIGN_CENTER;
}

Button::~Button() {
}
