/**************************************************************************/
/*  credits_roll.cpp                                                      */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "credits_roll.h"

#include "core/authors.gen.h"
#include "core/donors.gen.h"
#include "core/input/input.h"
#include "core/license.gen.h"
#include "core/string/string_builder.h"
#include "editor/themes/editor_scale.h"
#include "editor_string_names.h"
#include "scene/gui/box_container.h"
#include "scene/gui/color_rect.h"
#include "scene/gui/label.h"
#include "scene/gui/texture_rect.h"

Label *CreditsRoll::_create_label(const String &p_with_text, LabelSize p_size) {
	Label *label = memnew(Label);
	label->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
	label->set_h_size_flags(Control::SIZE_SHRINK_CENTER);
	label->set_text(p_with_text);

	switch (p_size) {
		case LabelSize::NORMAL:
			label->add_theme_font_size_override(SNAME("font_size"), font_size_normal);
			break;

		case LabelSize::HEADER:
			label->add_theme_font_size_override(SNAME("font_size"), font_size_header);
			break;

		case LabelSize::BIG_HEADER:
			label->add_theme_font_size_override(SNAME("font_size"), font_size_big_header);
			break;
	}
	content->add_child(label);
	return label;
}

void CreditsRoll::_create_nothing(int p_size) {
	if (p_size == -1) {
		p_size = 30 * EDSCALE;
	}
	Control *c = memnew(Control);
	c->set_custom_minimum_size(Vector2(0, p_size));
	content->add_child(c);
}

String CreditsRoll::_build_string(const char *const *p_from) const {
	StringBuilder sb;

	while (*p_from) {
		sb.append(String::utf8(*p_from));
		sb.append("\n");
		p_from++;
	}
	return sb.as_string();
}

void CreditsRoll::_notification(int p_what) {
	if (p_what == NOTIFICATION_VISIBILITY_CHANGED && !is_visible()) {
		set_process_internal(false);
	}

	if (p_what != NOTIFICATION_INTERNAL_PROCESS) {
		return;
	}

	const Vector2 pos = content->get_position();
	if (pos.y < -content->get_size().y - 30) {
		hide();
		return;
	}

	if (Input::get_singleton()->is_action_pressed(SNAME("ui_accept"))) {
		content->set_position(Vector2(pos.x, pos.y - 2000 * get_process_delta_time()));
	} else {
		content->set_position(Vector2(pos.x, pos.y - 100 * get_process_delta_time()));
	}
}

void CreditsRoll::roll_credits() {
	if (content->get_child_count() == 0) {
		font_size_normal = get_theme_font_size("main_size", EditorStringName(EditorFonts)) * 2;
		font_size_header = font_size_normal + 10 * EDSCALE;
		font_size_big_header = font_size_header + 20 * EDSCALE;

		{
			const Ref<Texture2D> logo_texture = get_editor_theme_icon("Logo");

			TextureRect *logo = memnew(TextureRect);
			logo->set_custom_minimum_size(Vector2(0, logo_texture->get_height() * 3));
			logo->set_stretch_mode(TextureRect::STRETCH_KEEP_ASPECT_CENTERED);
			logo->set_texture(logo_texture);
			content->add_child(logo);
		}

		_create_label(TTR("Credits"), LabelSize::BIG_HEADER);

		_create_nothing();

		_create_label(TTR("Project Founders"), LabelSize::HEADER);
		_create_label(_build_string(AUTHORS_FOUNDERS));

		_create_nothing();

		_create_label(TTR("Lead Developer"), LabelSize::HEADER);
		_create_label(_build_string(AUTHORS_LEAD_DEVELOPERS));

		_create_nothing();

		_create_label(TTR("Project Manager", "Job Title"), LabelSize::HEADER);
		_create_label(_build_string(AUTHORS_PROJECT_MANAGERS));

		_create_nothing();

		_create_label(TTR("Developers"), LabelSize::HEADER);
		_create_label(_build_string(AUTHORS_DEVELOPERS));

		_create_nothing();

		_create_label(TTR("Patrons"), LabelSize::HEADER);
		_create_label(_build_string(DONORS_PATRONS));

		_create_nothing();

		_create_label(TTR("Platinum Sponsors"), LabelSize::HEADER);
		_create_label(_build_string(DONORS_SPONSORS_PLATINUM));

		_create_nothing();

		_create_label(TTR("Gold Sponsors"), LabelSize::HEADER);
		_create_label(_build_string(DONORS_SPONSORS_GOLD));

		_create_nothing();

		_create_label(TTR("Silver Sponsors"), LabelSize::HEADER);
		_create_label(_build_string(DONORS_SPONSORS_SILVER));

		_create_nothing();

		_create_label(TTR("Diamond Members"), LabelSize::HEADER);
		_create_label(_build_string(DONORS_MEMBERS_DIAMOND));

		_create_nothing();

		_create_label(TTR("Titanium Members"), LabelSize::HEADER);
		_create_label(_build_string(DONORS_MEMBERS_TITANIUM));

		_create_nothing();

		_create_label(TTR("Platinum Members"), LabelSize::HEADER);
		_create_label(_build_string(DONORS_MEMBERS_PLATINUM));

		_create_nothing();

		_create_label(TTR("Gold Members"), LabelSize::HEADER);
		_create_label(_build_string(DONORS_MEMBERS_GOLD));

		_create_nothing();
		_create_label(String::utf8(GODOT_LICENSE_TEXT));

		_create_nothing(400 * EDSCALE);
		_create_label(TTR("Thank you for choosing Godot Engine!"), LabelSize::BIG_HEADER);
	}

	content->set_position(Vector2(content->get_position().x, DisplayServer::get_singleton()->screen_get_size().y + 30));
	popup();
	set_process_internal(true);
}

CreditsRoll::CreditsRoll() {
	set_mode(MODE_FULLSCREEN);

	{
		ColorRect *background = memnew(ColorRect);
		background->set_color(Color(0, 0, 0, 1));
		background->set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);
		add_child(background);
	}

	content = memnew(VBoxContainer);
	content->set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);
	add_child(content);
}
