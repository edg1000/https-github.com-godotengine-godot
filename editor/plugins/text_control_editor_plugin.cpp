/*************************************************************************/
/*  text_control_editor_plugin.cpp                                       */
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

#include "text_control_editor_plugin.h"

#include "editor/editor_scale.h"

void TextControlEditor::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_ENTER_TREE: {
			if (!EditorFileSystem::get_singleton()->is_connected("filesystem_changed", callable_mp(this, &TextControlEditor::_reload_fonts))) {
				EditorFileSystem::get_singleton()->connect("filesystem_changed", callable_mp(this, &TextControlEditor::_reload_fonts), make_binds(""));
			}
			[[fallthrough]];
		}
		case NOTIFICATION_THEME_CHANGED: {
			clear_formatting->set_icon(get_theme_icon(SNAME("Remove"), SNAME("EditorIcons")));
		} break;
		case NOTIFICATION_EXIT_TREE: {
			if (EditorFileSystem::get_singleton()->is_connected("filesystem_changed", callable_mp(this, &TextControlEditor::_reload_fonts))) {
				EditorFileSystem::get_singleton()->disconnect("filesystem_changed", callable_mp(this, &TextControlEditor::_reload_fonts));
			}
		} break;
		default:
			break;
	}
}

void TextControlEditor::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_update_control"), &TextControlEditor::_update_control);
}

void TextControlEditor::_find_resources(EditorFileSystemDirectory *p_dir) {
	for (int i = 0; i < p_dir->get_subdir_count(); i++) {
		_find_resources(p_dir->get_subdir(i));
	}

	for (int i = 0; i < p_dir->get_file_count(); i++) {
		if (p_dir->get_file_type(i) == "FontData") {
			Ref<FontData> fd = ResourceLoader::load(p_dir->get_file_path(i));
			if (fd.is_valid()) {
				String name = fd->get_font_name();
				String sty = fd->get_font_style_name();
				if (sty.is_empty()) {
					sty = "Default";
				}
				fonts[name][sty] = p_dir->get_file_path(i);
			}
		}
	}
}

void TextControlEditor::_reload_fonts(const String &p_path) {
	fonts.clear();
	_find_resources(EditorFileSystem::get_singleton()->get_filesystem());
	_update_control();
}

void TextControlEditor::_update_fonts_menu() {
	font_list->clear();
	font_list->add_item(TTR("[Theme Default]"), FONT_INFO_THEME_DEFAULT);
	if (custom_font.is_valid()) {
		font_list->add_item(TTR("[Custom Font]"), FONT_INFO_USER_CUSTOM);
	}

	int id = FONT_INFO_ID;
	for (Map<String, Map<String, String>>::Element *E = fonts.front(); E; E = E->next()) {
		font_list->add_item(E->key(), id++);
	}

	if (font_list->get_item_count() > 1) {
		font_list->show();
	} else {
		font_list->hide();
	}
}

void TextControlEditor::_update_styles_menu() {
	font_style_list->clear();
	if ((font_list->get_selected_id() >= FONT_INFO_ID)) {
		const String &name = font_list->get_item_text(font_list->get_selected());
		for (Map<String, String>::Element *E = fonts[name].front(); E; E = E->next()) {
			font_style_list->add_item(E->key());
		}
	} else {
		font_style_list->add_item("Default");
	}

	if (font_style_list->get_item_count() > 1) {
		font_style_list->show();
	} else {
		font_style_list->hide();
	}
}

void TextControlEditor::_update_control() {
	if (edited_control) {
		// Get override names.
		if (edited_control->is_class("RichTextLabel")) {
			edited_color = "default_color";
			edited_font = "normal_font";
			edited_font_size = "normal_font_size";
		} else {
			edited_color = "font_color";
			edited_font = "font";
			edited_font_size = "font_size";
		}

		// Get font override.
		Ref<Font> font;
		if (edited_control->has_theme_font_override(edited_font)) {
			font = edited_control->get_theme_font(edited_font);
		}
		if (font.is_valid()) {
			if (font->get_data_count() != 1) {
				// Composite font, save it to "custom_font" to allow undoing font change.
				custom_font = font;
				_update_fonts_menu();
				font_list->select(FONT_INFO_USER_CUSTOM);
				_update_styles_menu();
				font_style_list->select(0);
			} else {
				// Single face font, search for the font with matching name and style.
				String name = font->get_data(0)->get_font_name();
				String style = font->get_data(0)->get_font_style_name();
				if (fonts.has(name) && fonts[name].has(style)) {
					_update_fonts_menu();
					for (int i = 0; i < font_list->get_item_count(); i++) {
						if (font_list->get_item_text(i) == name) {
							font_list->select(i);
							break;
						}
					}
					_update_styles_menu();
					for (int i = 0; i < font_style_list->get_item_count(); i++) {
						if (font_style_list->get_item_text(i) == style) {
							font_style_list->select(i);
							break;
						}
					}
				} else {
					// Unknown font, save it to "custom_font" to allow undoing font change.
					custom_font = font;
					_update_fonts_menu();
					font_list->select(FONT_INFO_USER_CUSTOM);
					_update_styles_menu();
					font_style_list->select(0);
				}
			}
		} else {
			// No font override, select "Theme Default".
			_update_fonts_menu();
			font_list->select(FONT_INFO_THEME_DEFAULT);
			_update_styles_menu();
			font_style_list->select(0);
		}

		// Get other theme overrides.
		font_size_list->set_block_signals(true);
		font_size_list->set_value(edited_control->get_theme_font_size(edited_font_size));
		font_size_list->set_block_signals(false);

		outline_size_list->set_block_signals(true);
		outline_size_list->set_value(edited_control->get_theme_constant("outline_size"));
		outline_size_list->set_block_signals(false);

		font_color_picker->set_pick_color(edited_control->get_theme_color(edited_color));
		outline_color_picker->set_pick_color(edited_control->get_theme_color("font_outline_color"));
	}
}

void TextControlEditor::_font_selected(int p_id) {
	_set_font();
}

void TextControlEditor::_font_style_selected(int p_id) {
	_set_font();
}

void TextControlEditor::_set_font() {
	if (!edited_control) {
		return;
	}

	UndoRedo *ur = EditorNode::get_singleton()->get_undo_redo();
	ur->create_action(TTR("Set Font"));

	if (font_list->get_selected_id() == FONT_INFO_THEME_DEFAULT) {
		// Remove font override.
		ur->add_do_method(edited_control, "remove_theme_font_override", edited_font);
	} else if (font_list->get_selected_id() == FONT_INFO_USER_CUSTOM) {
		// Restore "custom_font".
		ur->add_do_method(edited_control, "add_theme_font_override", edited_font, custom_font);
	} else {
		// Load new font resource using selected name and style.
		String name = font_list->get_item_text(font_list->get_selected());
		String style = font_style_list->get_item_text(font_style_list->get_selected());
		if (style.is_empty()) {
			style = "Default";
		}

		if (fonts.has(name)) {
			Ref<FontData> fd = ResourceLoader::load(fonts[name][style]);
			if (fd.is_valid()) {
				Ref<Font> font;
				font.instantiate();
				font->add_data(fd);
				ur->add_do_method(edited_control, "add_theme_font_override", edited_font, font);
			}
		}
	}

	if (edited_control->has_theme_font_override(edited_font)) {
		ur->add_undo_method(edited_control, "add_theme_font_override", edited_font, edited_control->get_theme_font(edited_font));
	} else {
		ur->add_undo_method(edited_control, "remove_theme_font_override", edited_font);
	}

	ur->add_do_method(this, "_update_control");
	ur->add_undo_method(this, "_update_control");

	ur->commit_action();
}

void TextControlEditor::_font_size_selected(double p_size) {
	if (!edited_control) {
		return;
	}

	UndoRedo *ur = EditorNode::get_singleton()->get_undo_redo();
	ur->create_action(TTR("Set Font Size"));

	ur->add_do_method(edited_control, "add_theme_font_size_override", edited_font_size, p_size);
	if (edited_control->has_theme_font_size_override(edited_font_size)) {
		ur->add_undo_method(edited_control, "add_theme_font_size_override", edited_font_size, edited_control->get_theme_font_size(edited_font_size));
	} else {
		ur->add_undo_method(edited_control, "remove_theme_font_size_override", edited_font_size);
	}

	ur->add_do_method(this, "_update_control");
	ur->add_undo_method(this, "_update_control");

	ur->commit_action();
}

void TextControlEditor::_outline_size_selected(double p_size) {
	if (!edited_control) {
		return;
	}

	UndoRedo *ur = EditorNode::get_singleton()->get_undo_redo();
	ur->create_action(TTR("Set Font Outline Size"));

	ur->add_do_method(edited_control, "add_theme_constant_override", "outline_size", p_size);
	if (edited_control->has_theme_constant_override("outline_size")) {
		ur->add_undo_method(edited_control, "add_theme_constant_override", "outline_size", edited_control->get_theme_constant("outline_size"));
	} else {
		ur->add_undo_method(edited_control, "remove_theme_constant_override", "outline_size");
	}

	ur->add_do_method(this, "_update_control");
	ur->add_undo_method(this, "_update_control");

	ur->commit_action();
}

void TextControlEditor::_font_color_changed(const Color &p_color) {
	if (!edited_control) {
		return;
	}

	UndoRedo *ur = EditorNode::get_singleton()->get_undo_redo();
	ur->create_action(TTR("Set Font Color"), UndoRedo::MERGE_ENDS);

	ur->add_do_method(edited_control, "add_theme_color_override", edited_color, p_color);
	if (edited_control->has_theme_color_override(edited_color)) {
		ur->add_undo_method(edited_control, "add_theme_color_override", edited_color, edited_control->get_theme_color(edited_color));
	} else {
		ur->add_undo_method(edited_control, "remove_theme_color_override", edited_color);
	}

	ur->add_do_method(this, "_update_control");
	ur->add_undo_method(this, "_update_control");

	ur->commit_action();
}

void TextControlEditor::_font_picker_settings() {
	// get default color picker mode from editor settings
	int default_color_mode = EDITOR_GET("interface/inspector/default_color_picker_mode");
	int picker_shape = EDITOR_GET("interface/inspector/default_color_picker_shape");

	//font color picker settings
	//reset color modes
	font_color_picker->get_picker()->set_hsv_mode(false);
	font_color_picker->get_picker()->set_raw_mode(false);

	if (default_color_mode == 1) {
		font_color_picker->get_picker()->set_hsv_mode(true);
	} else if (default_color_mode == 2) {
		font_color_picker->get_picker()->set_raw_mode(true);
	}

	font_color_picker->get_picker()->set_picker_shape((ColorPicker::PickerShapeType)picker_shape);
}

void TextControlEditor::_outline_color_changed(const Color &p_color) {
	if (!edited_control) {
		return;
	}

	UndoRedo *ur = EditorNode::get_singleton()->get_undo_redo();
	ur->create_action(TTR("Set Font Outline Color"), UndoRedo::MERGE_ENDS);

	ur->add_do_method(edited_control, "add_theme_color_override", "font_outline_color", p_color);
	if (edited_control->has_theme_color_override("font_outline_color")) {
		ur->add_undo_method(edited_control, "add_theme_color_override", "font_outline_color", edited_control->get_theme_color("font_outline_color"));
	} else {
		ur->add_undo_method(edited_control, "remove_theme_color_override", "font_outline_color");
	}

	ur->add_do_method(this, "_update_control");
	ur->add_undo_method(this, "_update_control");

	ur->commit_action();
}

void TextControlEditor::_outline_picker_settings() {
	// get default color picker mode from editor settings
	int default_color_mode = EDITOR_GET("interface/inspector/default_color_picker_mode");
	int picker_shape = EDITOR_GET("interface/inspector/default_color_picker_shape");

	//outline color picker settings
	//reset color modes
	outline_color_picker->get_picker()->set_hsv_mode(false);
	outline_color_picker->get_picker()->set_raw_mode(false);

	if (default_color_mode == 1) {
		outline_color_picker->get_picker()->set_hsv_mode(true);
	} else if (default_color_mode == 2) {
		outline_color_picker->get_picker()->set_raw_mode(true);
	}

	outline_color_picker->get_picker()->set_picker_shape((ColorPicker::PickerShapeType)picker_shape);
}

void TextControlEditor::_clear_formatting() {
	if (!edited_control) {
		return;
	}

	UndoRedo *ur = EditorNode::get_singleton()->get_undo_redo();
	ur->create_action(TTR("Clear Control Formatting"));

	ur->add_do_method(edited_control, "begin_bulk_theme_override");
	ur->add_undo_method(edited_control, "begin_bulk_theme_override");

	ur->add_do_method(edited_control, "remove_theme_font_override", edited_font);
	if (edited_control->has_theme_font_override(edited_font)) {
		ur->add_undo_method(edited_control, "add_theme_font_override", edited_font, edited_control->get_theme_font(edited_font));
	}

	ur->add_do_method(edited_control, "remove_theme_font_size_override", edited_font_size);
	if (edited_control->has_theme_font_size_override(edited_font_size)) {
		ur->add_undo_method(edited_control, "add_theme_font_size_override", edited_font_size, edited_control->get_theme_font_size(edited_font_size));
	}

	ur->add_do_method(edited_control, "remove_theme_color_override", edited_color);
	if (edited_control->has_theme_color_override(edited_color)) {
		ur->add_undo_method(edited_control, "add_theme_color_override", edited_color, edited_control->get_theme_color(edited_color));
	}

	ur->add_do_method(edited_control, "remove_theme_color_override", "font_outline_color");
	if (edited_control->has_theme_color_override("font_outline_color")) {
		ur->add_undo_method(edited_control, "add_theme_color_override", "font_outline_color", edited_control->get_theme_color("font_outline_color"));
	}

	ur->add_do_method(edited_control, "remove_theme_constant_override", "outline_size");
	if (edited_control->has_theme_constant_override("outline_size")) {
		ur->add_undo_method(edited_control, "add_theme_constant_override", "outline_size", edited_control->get_theme_constant("outline_size"));
	}

	ur->add_do_method(edited_control, "end_bulk_theme_override");
	ur->add_undo_method(edited_control, "end_bulk_theme_override");

	ur->add_do_method(this, "_update_control");
	ur->add_undo_method(this, "_update_control");

	ur->commit_action();
}

void TextControlEditor::edit(Object *p_object) {
	Control *ctrl = Object::cast_to<Control>(p_object);
	if (!ctrl) {
		edited_control = nullptr;
		custom_font = Ref<Font>();
	} else {
		edited_control = ctrl;
		custom_font = Ref<Font>();
		_update_control();
	}
}

bool TextControlEditor::handles(Object *p_object) const {
	Control *ctrl = Object::cast_to<Control>(p_object);
	if (!ctrl) {
		return false;
	} else {
		bool valid = false;
		ctrl->get("text", &valid);
		return valid;
	}
}

TextControlEditor::TextControlEditor() {
	add_child(memnew(VSeparator));

	font_list = memnew(OptionButton);
	font_list->set_flat(true);
	font_list->set_tooltip(TTR("Font"));
	add_child(font_list);
	font_list->connect("item_selected", callable_mp(this, &TextControlEditor::_font_selected));

	font_style_list = memnew(OptionButton);
	font_style_list->set_flat(true);
	font_style_list->set_tooltip(TTR("Font style"));
	font_style_list->set_toggle_mode(true);
	add_child(font_style_list);
	font_style_list->connect("item_selected", callable_mp(this, &TextControlEditor::_font_style_selected));

	font_size_list = memnew(SpinBox);
	font_size_list->set_tooltip(TTR("Font Size"));
	font_size_list->get_line_edit()->add_theme_constant_override("minimum_character_width", 2);
	font_size_list->set_min(6);
	font_size_list->set_step(1);
	font_size_list->set_max(96);
	font_size_list->get_line_edit()->set_flat(true);
	add_child(font_size_list);
	font_size_list->connect("value_changed", callable_mp(this, &TextControlEditor::_font_size_selected));

	font_color_picker = memnew(ColorPickerButton);
	font_color_picker->set_custom_minimum_size(Size2(20, 0) * EDSCALE);
	font_color_picker->set_flat(true);
	font_color_picker->set_tooltip(TTR("Text Color"));
	add_child(font_color_picker);
	font_color_picker->connect("color_changed", callable_mp(this, &TextControlEditor::_font_color_changed));
	font_color_picker->get_popup()->connect("about_to_popup", callable_mp(this, &TextControlEditor::_font_picker_settings));

	add_child(memnew(VSeparator));

	outline_size_list = memnew(SpinBox);
	outline_size_list->set_tooltip(TTR("Outline Size"));
	outline_size_list->get_line_edit()->add_theme_constant_override("minimum_character_width", 2);
	outline_size_list->set_min(0);
	outline_size_list->set_step(1);
	outline_size_list->set_max(96);
	outline_size_list->get_line_edit()->set_flat(true);
	add_child(outline_size_list);
	outline_size_list->connect("value_changed", callable_mp(this, &TextControlEditor::_outline_size_selected));

	outline_color_picker = memnew(ColorPickerButton);
	outline_color_picker->set_custom_minimum_size(Size2(20, 0) * EDSCALE);
	outline_color_picker->set_flat(true);
	outline_color_picker->set_tooltip(TTR("Outline Color"));
	add_child(outline_color_picker);
	outline_color_picker->connect("color_changed", callable_mp(this, &TextControlEditor::_outline_color_changed));
	outline_color_picker->get_popup()->connect("about_to_popup", callable_mp(this, &TextControlEditor::_outline_picker_settings));

	add_child(memnew(VSeparator));

	clear_formatting = memnew(Button);
	clear_formatting->set_flat(true);
	clear_formatting->set_tooltip(TTR("Clear Formatting"));
	add_child(clear_formatting);
	clear_formatting->connect("pressed", callable_mp(this, &TextControlEditor::_clear_formatting));
}

/*************************************************************************/

void TextControlEditorPlugin::edit(Object *p_object) {
	text_ctl_editor->edit(p_object);
}

bool TextControlEditorPlugin::handles(Object *p_object) const {
	return text_ctl_editor->handles(p_object);
}

void TextControlEditorPlugin::make_visible(bool p_visible) {
	if (p_visible) {
		text_ctl_editor->show();
	} else {
		text_ctl_editor->hide();
		text_ctl_editor->edit(nullptr);
	}
}

TextControlEditorPlugin::TextControlEditorPlugin(EditorNode *p_node) {
	editor = p_node;
	text_ctl_editor = memnew(TextControlEditor);
	CanvasItemEditor::get_singleton()->add_control_to_menu_panel(text_ctl_editor);

	text_ctl_editor->hide();
}
