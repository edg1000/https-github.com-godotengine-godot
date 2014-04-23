/*************************************************************************/
/*  progress_bar.cpp                                                     */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                 */
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
#include "progress_bar.h"


Size2 ProgressBar::get_minimum_size() const {

	Ref<StyleBox> bg = get_stylebox("bg");
	Ref<Font> font = get_font("font");

	Size2 ms=bg->get_minimum_size()+bg->get_center_size();
	ms.height=MAX(ms.height,bg->get_minimum_size().height+font->get_height());
	return ms;
}


void ProgressBar::_notification(int p_what) {


	if (p_what==NOTIFICATION_DRAW) {

		Ref<StyleBox> bg = get_stylebox("bg");
		Ref<StyleBox> fg = get_stylebox("fg");
		Ref<Font> font = get_font("font");
		Color font_color=get_color("font_color");
		Color font_color_shadow=get_color("font_color_shadow");

		draw_style_box(bg,Rect2(Point2(),get_size()));
		float r = get_unit_value();
		int mp = fg->get_minimum_size().width;
		int p = r*get_size().width-mp;
		if (p>0) {

			draw_style_box(fg,Rect2(Point2(),Size2(p+fg->get_minimum_size().width,get_size().height)));
		}

		int fh=font->get_height();
		String txt;
		if (text != "") {
			txt=text;
		} else {
			txt=itos(int(get_unit_value()*100))+"%";
		}
		font->draw_halign(get_canvas_item(),Point2(0,font->get_ascent()+(get_size().height-font->get_height())/2),HALIGN_CENTER,get_size().width,txt,font_color);
	}
}

void ProgressBar::set_text(const String& p_string) {
	text = p_string;
	update();
}

String ProgressBar::get_text() const {
	return text;
}

void ProgressBar::_bind_methods() {
	ObjectTypeDB::bind_method(_MD("set_text","text"),&ProgressBar::set_text);
	ObjectTypeDB::bind_method(_MD("get_text"),&ProgressBar::get_text);
	ADD_PROPERTY( PropertyInfo( Variant::STRING, "text",PROPERTY_HINT_MULTILINE_TEXT,"",PROPERTY_USAGE_DEFAULT_INTL ), _SCS("set_text"),_SCS("get_text")  );
}

ProgressBar::ProgressBar() {

	set_v_size_flags(0);
	text="";
}
