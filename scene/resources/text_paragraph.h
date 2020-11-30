/*************************************************************************/
/*  text_paragraph.h                                                     */
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

#ifndef TEXT_PARAGRAPH_H
#define TEXT_PARAGRAPH_H

#include "scene/resources/font.h"
#include "servers/text_server.h"

/*************************************************************************/

class TextParagraph : public Reference {
	GDCLASS(TextParagraph, Reference);

	RID rid;
	Vector<RID> lines;
	int spacing_top = 0;
	int spacing_bottom = 0;

	bool dirty_lines = true;

	float width = -1;
	uint8_t flags = TextServer::BREAK_MANDATORY | TextServer::BREAK_WORD_BOUND | TextServer::JUSTIFICATION_WORD_BOUND | TextServer::JUSTIFICATION_KASHIDA;
	HAlign align = HALIGN_LEFT;

	Vector<float> tab_stops;

protected:
	static void _bind_methods();

	void _shape_lines();

public:
	RID get_rid() const;
	RID get_line_rid(int p_line) const;

	void clear();

	void set_direction(TextServer::Direction p_direction);
	TextServer::Direction get_direction() const;

	void set_orientation(TextServer::Orientation p_orientation);
	TextServer::Orientation get_orientation() const;

	void set_preserve_invalid(bool p_enabled);
	bool get_preserve_invalid() const;

	void set_preserve_control(bool p_enabled);
	bool get_preserve_control() const;

	void set_bidi_override(const Vector<Vector2i> &p_override);

	bool add_string(const String &p_text, const Ref<Font> &p_fonts, int p_size, const Dictionary &p_opentype_features = Dictionary(), const String &p_language = "");
	bool add_object(Variant p_key, const Size2 &p_size, VAlign p_inline_align = VALIGN_CENTER, int p_length = 1);
	bool resize_object(Variant p_key, const Size2 &p_size, VAlign p_inline_align = VALIGN_CENTER);

	void set_align(HAlign p_align);
	HAlign get_align() const;

	void tab_align(const Vector<float> &p_tab_stops);

	void set_flags(uint8_t p_flags);
	uint8_t get_flags() const;

	void set_width(float p_width);
	float get_width() const;

	Size2 get_non_wraped_size() const;

	Size2 get_size() const;

	int get_line_count() const;

	Array get_line_objects(int p_line) const;
	Rect2 get_line_object_rect(int p_line, Variant p_key) const;
	Size2 get_line_size(int p_line) const;
	float get_line_ascent(int p_line) const;
	float get_line_descent(int p_line) const;
	float get_line_width(int p_line) const;
	Vector2i get_line_range(int p_line) const;
	float get_line_underline_position(int p_line) const;
	float get_line_underline_thickness(int p_line) const;

	void draw(RID p_canvas, const Vector2 &p_pos, const Color &p_color = Color(1, 1, 1)) const;
	void draw_outline(RID p_canvas, const Vector2 &p_pos, int p_outline_size = 1, const Color &p_color = Color(1, 1, 1)) const;

	void draw_line(RID p_canvas, const Vector2 &p_pos, int p_line, const Color &p_color = Color(1, 1, 1)) const;
	void draw_line_outline(RID p_canvas, const Vector2 &p_pos, int p_line, int p_outline_size = 1, const Color &p_color = Color(1, 1, 1)) const;

	int hit_test(const Point2 &p_coords) const;

	void _set_bidi_override(const Array &p_override);

	TextParagraph(const String &p_text, const Ref<Font> &p_fonts, int p_size, const Dictionary &p_opentype_features = Dictionary(), const String &p_language = "", float p_width = -1.f, TextServer::Direction p_direction = TextServer::DIRECTION_AUTO, TextServer::Orientation p_orientation = TextServer::ORIENTATION_HORIZONTAL);
	TextParagraph();
	~TextParagraph();
};

#endif // TEXT_PARAGRAPH_H
