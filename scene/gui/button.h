/*************************************************************************/
/*  button.h                                                             */
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

#ifndef BUTTON_H
#define BUTTON_H

#include "scene/gui/base_button.h"
#include "scene/resources/text_paragraph.h"

class Button : public BaseButton {
	GDCLASS(Button, BaseButton);

private:
	bool flat = false;
	String text;
	String xl_text;
	Ref<TextParagraph> text_buf;

	String language;
	TextDirection text_direction = TEXT_DIRECTION_AUTO;

	Ref<Texture2D> icon;
	bool expand_icon = false;
	bool clip_text = false;
	HorizontalAlignment alignment = HORIZONTAL_ALIGNMENT_CENTER;
	HorizontalAlignment icon_alignment = HORIZONTAL_ALIGNMENT_LEFT;
	float _internal_margin[4] = {};

	void _shape();

protected:
	void _set_internal_margin(Side p_side, float p_value);
	void _notification(int p_what);
	static void _bind_methods();

public:
	virtual Size2 get_minimum_size() const override;

	void set_text(const String &p_text);
	String get_text() const;

	void set_text_direction(TextDirection p_text_direction);
	TextDirection get_text_direction() const;

	void set_language(const String &p_language);
	String get_language() const;

	void set_icon(const Ref<Texture2D> &p_icon);
	Ref<Texture2D> get_icon() const;

	void set_expand_icon(bool p_enabled);
	bool is_expand_icon() const;

	void set_flat(bool p_enabled);
	bool is_flat() const;

	void set_clip_text(bool p_enabled);
	bool get_clip_text() const;

	void set_text_alignment(HorizontalAlignment p_alignment);
	HorizontalAlignment get_text_alignment() const;

	void set_icon_alignment(HorizontalAlignment p_alignment);
	HorizontalAlignment get_icon_alignment() const;

	Button(const String &p_text = String());
	~Button();
};

#endif
