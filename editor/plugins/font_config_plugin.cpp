/*************************************************************************/
/*  font_config_plugin.cpp                                               */
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

#include "font_config_plugin.h"

#include "editor/editor_scale.h"
#include "editor/import/dynamic_font_import_settings.h"

/*************************************************************************/
/*  EditorPropertyFontMetaObject                                         */
/*************************************************************************/

bool EditorPropertyFontMetaObject::_set(const StringName &p_name, const Variant &p_value) {
	String name = p_name;

	if (name.begins_with("keys")) {
		String key = name.get_slicec('/', 1);
		dict[key] = p_value;
		return true;
	}

	return false;
}

bool EditorPropertyFontMetaObject::_get(const StringName &p_name, Variant &r_ret) const {
	String name = p_name;

	if (name.begins_with("keys")) {
		String key = name.get_slicec('/', 1);
		r_ret = dict[key];
		return true;
	}

	return false;
}

void EditorPropertyFontMetaObject::_bind_methods() {
}

void EditorPropertyFontMetaObject::set_dict(const Dictionary &p_dict) {
	dict = p_dict;
}

Dictionary EditorPropertyFontMetaObject::get_dict() {
	return dict;
}

/*************************************************************************/
/*  EditorPropertyFontOTObject                                           */
/*************************************************************************/

bool EditorPropertyFontOTObject::_set(const StringName &p_name, const Variant &p_value) {
	String name = p_name;

	if (name.begins_with("keys")) {
		int key = name.get_slicec('/', 1).to_int();
		dict[key] = p_value;
		return true;
	}

	return false;
}

bool EditorPropertyFontOTObject::_get(const StringName &p_name, Variant &r_ret) const {
	String name = p_name;

	if (name.begins_with("keys")) {
		int key = name.get_slicec('/', 1).to_int();
		r_ret = dict[key];
		return true;
	}

	return false;
}

void EditorPropertyFontOTObject::_bind_methods() {
	ClassDB::bind_method(D_METHOD("property_can_revert", "name"), &EditorPropertyFontOTObject::property_can_revert);
	ClassDB::bind_method(D_METHOD("property_get_revert", "name"), &EditorPropertyFontOTObject::property_get_revert);
}

void EditorPropertyFontOTObject::set_dict(const Dictionary &p_dict) {
	dict = p_dict;
}

Dictionary EditorPropertyFontOTObject::get_dict() {
	return dict;
}

void EditorPropertyFontOTObject::set_defaults(const Dictionary &p_dict) {
	defaults_dict = p_dict;
}

Dictionary EditorPropertyFontOTObject::get_defaults() {
	return defaults_dict;
}

bool EditorPropertyFontOTObject::property_can_revert(const String &p_name) {
	String name = p_name;

	if (name.begins_with("keys")) {
		int key = name.get_slicec('/', 1).to_int();
		if (defaults_dict.has(key) && dict.has(key)) {
			int value = dict[key];
			Vector3i range = defaults_dict[key];
			return range.z != value;
		}
	}

	return false;
}

Variant EditorPropertyFontOTObject::property_get_revert(const String &p_name) {
	String name = p_name;

	if (name.begins_with("keys")) {
		int key = name.get_slicec('/', 1).to_int();
		if (defaults_dict.has(key)) {
			Vector3i range = defaults_dict[key];
			return range.z;
		}
	}

	return Variant();
}

/*************************************************************************/
/* EditorPropertyFontMetaOverride                                        */
/*************************************************************************/

void EditorPropertyFontMetaOverride::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE:
		case NOTIFICATION_THEME_CHANGED: {
			if (Object::cast_to<Button>(button_add)) {
				button_add->set_icon(get_theme_icon(SNAME("Add"), SNAME("EditorIcons")));
			}
		} break;
	}
}

void EditorPropertyFontMetaOverride::_property_changed(const String &p_property, Variant p_value, const String &p_name, bool p_changing) {
	if (p_property.begins_with("keys")) {
		Dictionary dict = object->get_dict();
		String key = p_property.get_slice("/", 1);
		dict[key] = (bool)p_value;

		emit_changed(get_edited_property(), dict, "", true);

		dict = dict.duplicate(); // Duplicate, so undo/redo works better.
		object->set_dict(dict);
	}
}

void EditorPropertyFontMetaOverride::_remove(Object *p_button, const String &p_key) {
	Dictionary dict = object->get_dict();

	dict.erase(p_key);

	emit_changed(get_edited_property(), dict, "", false);

	dict = dict.duplicate(); // Duplicate, so undo/redo works better.
	object->set_dict(dict);
	update_property();
}

void EditorPropertyFontMetaOverride::_add_menu() {
	if (script_editor) {
		Size2 size = get_size();
		menu->set_position(get_screen_position() + Size2(0, size.height * get_global_transform().get_scale().y));
		menu->reset_size();
		menu->popup();
	} else {
		locale_select->popup_locale_dialog();
	}
}

void EditorPropertyFontMetaOverride::_add_script(int p_option) {
	Dictionary dict = object->get_dict();

	dict[script_codes[p_option]] = true;

	emit_changed(get_edited_property(), dict, "", false);

	dict = dict.duplicate(); // Duplicate, so undo/redo works better.
	object->set_dict(dict);
	update_property();
}

void EditorPropertyFontMetaOverride::_add_lang(const String &p_locale) {
	Dictionary dict = object->get_dict();

	dict[p_locale] = true;

	emit_changed(get_edited_property(), dict, "", false);

	dict = dict.duplicate(); // Duplicate, so undo/redo works better.
	object->set_dict(dict);
	update_property();
}

void EditorPropertyFontMetaOverride::_object_id_selected(const StringName &p_property, ObjectID p_id) {
	emit_signal(SNAME("object_id_selected"), p_property, p_id);
}

void EditorPropertyFontMetaOverride::update_property() {
	Variant updated_val = get_edited_object()->get(get_edited_property());

	Dictionary dict = updated_val;

	edit->set_text(vformat(TTR("Overrides (%d)"), dict.size()));

	bool unfolded = get_edited_object()->editor_is_section_unfolded(get_edited_property());
	if (edit->is_pressed() != unfolded) {
		edit->set_pressed(unfolded);
	}

	if (unfolded) {
		updating = true;

		if (!vbox) {
			vbox = memnew(VBoxContainer);
			add_child(vbox);
			set_bottom_editor(vbox);

			property_vbox = memnew(VBoxContainer);
			property_vbox->set_h_size_flags(SIZE_EXPAND_FILL);
			vbox->add_child(property_vbox);

			paginator = memnew(EditorPaginator);
			paginator->connect("page_changed", callable_mp(this, &EditorPropertyFontMetaOverride::_page_changed));
			vbox->add_child(paginator);
		} else {
			// Queue children for deletion, deleting immediately might cause errors.
			for (int i = property_vbox->get_child_count() - 1; i >= 0; i--) {
				property_vbox->get_child(i)->queue_delete();
			}
			button_add = nullptr;
		}

		int size = dict.size();

		int max_page = MAX(0, size - 1) / page_length;
		page_index = MIN(page_index, max_page);

		paginator->update(page_index, max_page);
		paginator->set_visible(max_page > 0);

		int offset = page_index * page_length;

		int amount = MIN(size - offset, page_length);

		dict = dict.duplicate();
		object->set_dict(dict);

		for (int i = 0; i < amount; i++) {
			String name = dict.get_key_at_index(i);
			EditorProperty *prop = memnew(EditorPropertyCheck);
			prop->set_object_and_property(object.ptr(), "keys/" + name);

			if (script_editor) {
				prop->set_label(TranslationServer::get_singleton()->get_script_name(name));
			} else {
				prop->set_label(TranslationServer::get_singleton()->get_locale_name(name));
			}
			prop->set_tooltip(name);
			prop->set_selectable(false);

			prop->connect("property_changed", callable_mp(this, &EditorPropertyFontMetaOverride::_property_changed));
			prop->connect("object_id_selected", callable_mp(this, &EditorPropertyFontMetaOverride::_object_id_selected));

			HBoxContainer *hbox = memnew(HBoxContainer);
			property_vbox->add_child(hbox);
			hbox->add_child(prop);
			prop->set_h_size_flags(SIZE_EXPAND_FILL);
			Button *remove = memnew(Button);
			remove->set_icon(get_theme_icon(SNAME("Remove"), SNAME("EditorIcons")));
			hbox->add_child(remove);
			remove->connect("pressed", callable_mp(this, &EditorPropertyFontMetaOverride::_remove), varray(remove, name));

			prop->update_property();
		}

		PanelContainer *pc = memnew(PanelContainer);
		property_vbox->add_child(pc);
		pc->add_theme_style_override(SNAME("panel"), get_theme_stylebox(SNAME("DictionaryAddItem"), SNAME("EditorStyles")));

		VBoxContainer *add_vbox = memnew(VBoxContainer);
		pc->add_child(add_vbox);

		if (script_editor) {
			button_add = EditorInspector::create_inspector_action_button(TTR("Add Script"));
		} else {
			button_add = EditorInspector::create_inspector_action_button(TTR("Add Locale"));
		}
		button_add->connect("pressed", callable_mp(this, &EditorPropertyFontMetaOverride::_add_menu));
		add_vbox->add_child(button_add);

		updating = false;
	} else {
		if (vbox) {
			set_bottom_editor(nullptr);
			memdelete(vbox);
			button_add = nullptr;
			vbox = nullptr;
		}
	}
}

void EditorPropertyFontMetaOverride::_edit_pressed() {
	Variant prop_val = get_edited_object()->get(get_edited_property());
	if (prop_val.get_type() == Variant::NIL) {
		Callable::CallError ce;
		Variant::construct(Variant::DICTIONARY, prop_val, nullptr, 0, ce);
		get_edited_object()->set(get_edited_property(), prop_val);
	}

	get_edited_object()->editor_set_section_unfold(get_edited_property(), edit->is_pressed());
	update_property();
}

void EditorPropertyFontMetaOverride::_page_changed(int p_page) {
	if (updating) {
		return;
	}
	page_index = p_page;
	update_property();
}

EditorPropertyFontMetaOverride::EditorPropertyFontMetaOverride(bool p_script) {
	script_editor = p_script;

	object.instantiate();
	page_length = int(EDITOR_GET("interface/inspector/max_array_dictionary_items_per_page"));

	edit = memnew(Button);
	edit->set_h_size_flags(SIZE_EXPAND_FILL);
	edit->set_clip_text(true);
	edit->connect("pressed", callable_mp(this, &EditorPropertyFontMetaOverride::_edit_pressed));
	edit->set_toggle_mode(true);
	add_child(edit);
	add_focusable(edit);

	menu = memnew(PopupMenu);
	if (script_editor) {
		script_codes = TranslationServer::get_singleton()->get_all_scripts();
		for (int i = 0; i < script_codes.size(); i++) {
			menu->add_item(TranslationServer::get_singleton()->get_script_name(script_codes[i]) + " (" + script_codes[i] + ")", i);
		}
	}
	add_child(menu);
	menu->connect("id_pressed", callable_mp(this, &EditorPropertyFontMetaOverride::_add_script));

	locale_select = memnew(EditorLocaleDialog);
	locale_select->connect("locale_selected", callable_mp(this, &EditorPropertyFontMetaOverride::_add_lang));
	add_child(locale_select);
}

/*************************************************************************/
/* EditorPropertyOTVariation                                             */
/*************************************************************************/

void EditorPropertyOTVariation::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE:
		case NOTIFICATION_THEME_CHANGED: {
		} break;
	}
}

void EditorPropertyOTVariation::_property_changed(const String &p_property, Variant p_value, const String &p_name, bool p_changing) {
	if (p_property.begins_with("keys")) {
		Dictionary dict = object->get_dict();
		Dictionary defaults_dict = object->get_defaults();
		int key = p_property.get_slice("/", 1).to_int();
		dict[key] = (int)p_value;
		if (defaults_dict.has(key)) {
			Vector3i range = defaults_dict[key];
			if (range.z == (int)p_value) {
				dict.erase(key);
			}
		}

		emit_changed(get_edited_property(), dict, "", true);

		dict = dict.duplicate(); // Duplicate, so undo/redo works better.
		object->set_dict(dict);
	}
}

void EditorPropertyOTVariation::_object_id_selected(const StringName &p_property, ObjectID p_id) {
	emit_signal(SNAME("object_id_selected"), p_property, p_id);
}

void EditorPropertyOTVariation::update_property() {
	Variant updated_val = get_edited_object()->get(get_edited_property());

	Dictionary dict = updated_val;

	Ref<Font> fd;
	if (Object::cast_to<FontConfig>(get_edited_object()) != nullptr) {
		Ref<FontConfig> fc = Object::cast_to<FontConfig>(get_edited_object());
		fd = fc->_get_font_or_default();
	} else if (Object::cast_to<DynamicFontImportSettingsData>(get_edited_object()) != nullptr) {
		Ref<DynamicFontImportSettingsData> imp = Object::cast_to<DynamicFontImportSettingsData>(get_edited_object());
		fd = imp->get_font();
	}

	Vector<String> tokens = get_edited_property().operator String().split("/");
	if (tokens.size() >= 1 && tokens[0] == "variation_fallback") {
		for (int i = 1; i < tokens.size() - 1; i++) {
			ERR_FAIL_COND(fd.is_null());

			int idx = tokens[i].to_int();
			const TypedArray<Font> &fallbacks = fd->get_fallbacks();
			if (idx < fallbacks.size()) {
				fd = fallbacks[idx];
			}
		}
	}

	Dictionary supported = (fd.is_valid()) ? fd->get_supported_variation_list() : Dictionary();

	edit->set_text(vformat(TTR("Variation Coordinates (%d)"), supported.size()));

	bool unfolded = get_edited_object()->editor_is_section_unfolded(get_edited_property());
	if (edit->is_pressed() != unfolded) {
		edit->set_pressed(unfolded);
	}

	if (unfolded) {
		updating = true;

		if (!vbox) {
			vbox = memnew(VBoxContainer);
			add_child(vbox);
			set_bottom_editor(vbox);

			property_vbox = memnew(VBoxContainer);
			property_vbox->set_h_size_flags(SIZE_EXPAND_FILL);
			vbox->add_child(property_vbox);

			paginator = memnew(EditorPaginator);
			paginator->connect("page_changed", callable_mp(this, &EditorPropertyOTVariation::_page_changed));
			vbox->add_child(paginator);
		} else {
			// Queue children for deletion, deleting immediately might cause errors.
			for (int i = property_vbox->get_child_count() - 1; i >= 0; i--) {
				property_vbox->get_child(i)->queue_delete();
			}
		}

		int size = supported.size();

		int max_page = MAX(0, size - 1) / page_length;
		page_index = MIN(page_index, max_page);

		paginator->update(page_index, max_page);
		paginator->set_visible(max_page > 0);

		int offset = page_index * page_length;

		int amount = MIN(size - offset, page_length);

		dict = dict.duplicate();
		object->set_dict(dict);
		object->set_defaults(supported);

		for (int i = 0; i < amount; i++) {
			int name_tag = supported.get_key_at_index(i);
			Vector3i range = supported.get_value_at_index(i);

			EditorPropertyInteger *prop = memnew(EditorPropertyInteger);
			prop->setup(range.x, range.y, 1, false, false);
			prop->set_object_and_property(object.ptr(), "keys/" + itos(name_tag));

			String name = TS->tag_to_name(name_tag);
			prop->set_label(name.capitalize());
			prop->set_tooltip(name);
			prop->set_selectable(false);

			prop->connect("property_changed", callable_mp(this, &EditorPropertyOTVariation::_property_changed));
			prop->connect("object_id_selected", callable_mp(this, &EditorPropertyOTVariation::_object_id_selected));

			property_vbox->add_child(prop);

			prop->update_property();
		}

		updating = false;
	} else {
		if (vbox) {
			set_bottom_editor(nullptr);
			memdelete(vbox);
			vbox = nullptr;
		}
	}
}

void EditorPropertyOTVariation::_edit_pressed() {
	Variant prop_val = get_edited_object()->get(get_edited_property());
	if (prop_val.get_type() == Variant::NIL) {
		Callable::CallError ce;
		Variant::construct(Variant::DICTIONARY, prop_val, nullptr, 0, ce);
		get_edited_object()->set(get_edited_property(), prop_val);
	}

	get_edited_object()->editor_set_section_unfold(get_edited_property(), edit->is_pressed());
	update_property();
}

void EditorPropertyOTVariation::_page_changed(int p_page) {
	if (updating) {
		return;
	}
	page_index = p_page;
	update_property();
}

EditorPropertyOTVariation::EditorPropertyOTVariation() {
	object.instantiate();
	page_length = int(EDITOR_GET("interface/inspector/max_array_dictionary_items_per_page"));

	edit = memnew(Button);
	edit->set_h_size_flags(SIZE_EXPAND_FILL);
	edit->set_clip_text(true);
	edit->connect("pressed", callable_mp(this, &EditorPropertyOTVariation::_edit_pressed));
	edit->set_toggle_mode(true);
	add_child(edit);
	add_focusable(edit);
}

/*************************************************************************/
/* EditorPropertyOTFeatures                                              */
/*************************************************************************/

void EditorPropertyOTFeatures::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE:
		case NOTIFICATION_THEME_CHANGED: {
			if (Object::cast_to<Button>(button_add)) {
				button_add->set_icon(get_theme_icon(SNAME("Add"), SNAME("EditorIcons")));
			}
		} break;
	}
}

void EditorPropertyOTFeatures::_property_changed(const String &p_property, Variant p_value, const String &p_name, bool p_changing) {
	if (p_property.begins_with("keys")) {
		Dictionary dict = object->get_dict();
		int key = p_property.get_slice("/", 1).to_int();
		dict[key] = (int)p_value;

		emit_changed(get_edited_property(), dict, "", true);

		dict = dict.duplicate(); // Duplicate, so undo/redo works better.
		object->set_dict(dict);
	}
}

void EditorPropertyOTFeatures::_remove(Object *p_button, int p_key) {
	Dictionary dict = object->get_dict();

	dict.erase(p_key);

	emit_changed(get_edited_property(), dict, "", false);

	dict = dict.duplicate(); // Duplicate, so undo/redo works better.
	object->set_dict(dict);
	update_property();
}

void EditorPropertyOTFeatures::_add_menu() {
	Size2 size = get_size();
	menu->set_position(get_screen_position() + Size2(0, size.height * get_global_transform().get_scale().y));
	menu->reset_size();
	menu->popup();
}

void EditorPropertyOTFeatures::_add_feature(int p_option) {
	Dictionary dict = object->get_dict();

	dict[p_option] = 1;

	emit_changed(get_edited_property(), dict, "", false);

	dict = dict.duplicate(); // Duplicate, so undo/redo works better.
	object->set_dict(dict);
	update_property();
}

void EditorPropertyOTFeatures::_object_id_selected(const StringName &p_property, ObjectID p_id) {
	emit_signal(SNAME("object_id_selected"), p_property, p_id);
}

void EditorPropertyOTFeatures::_update_supported_fb(const Ref<Font> &p_fb, Dictionary &r_supported) {
	if (p_fb.is_valid()) {
		Dictionary supported = p_fb->get_supported_feature_list();
		for (int i = 0; i < supported.size(); i++) {
			int name_tag = supported.get_key_at_index(i);
			r_supported[name_tag] = supported[name_tag];
		}

		const TypedArray<Font> &fallbacks = p_fb->get_fallbacks();
		for (int i = 0; i < fallbacks.size(); i++) {
			_update_supported_fb(fallbacks[i], r_supported);
		}
	}
}

void EditorPropertyOTFeatures::update_property() {
	Variant updated_val = get_edited_object()->get(get_edited_property());

	Dictionary dict = updated_val;

	Ref<Font> fd;
	if (Object::cast_to<FontConfig>(get_edited_object()) != nullptr) {
		Ref<FontConfig> fc = Object::cast_to<FontConfig>(get_edited_object());
		fd = fc->_get_font_or_default();
	} else if (Object::cast_to<DynamicFontImportSettingsData>(get_edited_object()) != nullptr) {
		Ref<DynamicFontImportSettingsData> imp = Object::cast_to<DynamicFontImportSettingsData>(get_edited_object());
		fd = imp->get_font();
	}

	Dictionary supported;
	_update_supported_fb(fd, supported);

	edit->set_text(vformat(TTR("Features (%d of %d set)"), dict.size(), supported.size()));

	bool unfolded = get_edited_object()->editor_is_section_unfolded(get_edited_property());
	if (edit->is_pressed() != unfolded) {
		edit->set_pressed(unfolded);
	}

	if (unfolded) {
		updating = true;

		if (!vbox) {
			vbox = memnew(VBoxContainer);
			add_child(vbox);
			set_bottom_editor(vbox);

			property_vbox = memnew(VBoxContainer);
			property_vbox->set_h_size_flags(SIZE_EXPAND_FILL);
			vbox->add_child(property_vbox);

			paginator = memnew(EditorPaginator);
			paginator->connect("page_changed", callable_mp(this, &EditorPropertyOTFeatures::_page_changed));
			vbox->add_child(paginator);
		} else {
			// Queue children for deletion, deleting immediately might cause errors.
			for (int i = property_vbox->get_child_count() - 1; i >= 0; i--) {
				property_vbox->get_child(i)->queue_delete();
			}
			button_add = nullptr;
		}

		// Update add menu items.
		menu->clear();
		bool have_sub[FGRP_MAX];
		for (int i = 0; i < FGRP_MAX; i++) {
			menu_sub[i]->clear();
			have_sub[i] = false;
		}

		for (int i = 0; i < supported.size(); i++) {
			int name_tag = supported.get_key_at_index(i);
			Dictionary info = supported.get_value_at_index(i);
			String name = TS->tag_to_name(name_tag);
			FeatureGroups grp = FGRP_MAX;
			if (name.begins_with("stylistic_set_")) {
				grp = FGRP_STYLISTIC_SET;
			} else if (name.begins_with("character_variant_")) {
				grp = FGRP_CHARACTER_VARIANT;
			} else if (name.ends_with("_capitals")) {
				grp = FGRP_CAPITLS;
			} else if (name.ends_with("_ligatures")) {
				grp = FGRP_LIGATURES;
			} else if (name.ends_with("_alternates")) {
				grp = FGRP_ALTERNATES;
			} else if (name.ends_with("_kanji_forms") || name.begins_with("jis") || name == "simplified_forms" || name == "traditional_name_forms" || name == "traditional_forms") {
				grp = FGRP_EAL;
			} else if (name.ends_with("_widths")) {
				grp = FGRP_EAW;
			} else if (name == "tabular_figures" || name == "proportional_figures") {
				grp = FGRP_NUMAL;
			} else if (name.begins_with("custom_")) {
				grp = FGRP_CUSTOM;
			}
			String disp_name = name.capitalize();
			if (info.has("label")) {
				disp_name = vformat("%s (%s)", disp_name, info["label"].operator String());
			}

			if (grp == FGRP_MAX) {
				menu->add_item(disp_name, name_tag);
			} else {
				menu_sub[grp]->add_item(disp_name, name_tag);
				have_sub[grp] = true;
			}
		}
		for (int i = 0; i < FGRP_MAX; i++) {
			if (have_sub[i]) {
				menu->add_submenu_item(RTR(group_names[i]), "FTRMenu_" + itos(i));
			}
		}

		int size = dict.size();

		int max_page = MAX(0, size - 1) / page_length;
		page_index = MIN(page_index, max_page);

		paginator->update(page_index, max_page);
		paginator->set_visible(max_page > 0);

		int offset = page_index * page_length;

		int amount = MIN(size - offset, page_length);

		dict = dict.duplicate();
		object->set_dict(dict);

		bool show_hidden = EDITOR_GET("interface/inspector/show_low_level_opentype_features");

		for (int i = 0; i < amount; i++) {
			int name_tag = dict.get_key_at_index(i);

			if (supported.has(name_tag)) {
				Dictionary info = supported[name_tag];
				Variant::Type vtype = Variant::Type(info["type"].operator int());
				bool hidden = info["hidden"].operator bool();
				if (hidden && !show_hidden) {
					continue;
				}

				EditorProperty *prop = nullptr;
				switch (vtype) {
					case Variant::NIL: {
						prop = memnew(EditorPropertyNil);
					} break;
					case Variant::BOOL: {
						prop = memnew(EditorPropertyCheck);
					} break;
					case Variant::INT: {
						EditorPropertyInteger *editor = memnew(EditorPropertyInteger);
						editor->setup(0, 255, 1, false, false);
						prop = editor;
					} break;
					default: {
						ERR_CONTINUE_MSG(true, vformat("Unsupported OT feature data type %s", Variant::get_type_name(vtype)));
					}
				}
				prop->set_object_and_property(object.ptr(), "keys/" + itos(name_tag));

				String name = TS->tag_to_name(name_tag);
				String disp_name = name.capitalize();
				if (info.has("label")) {
					disp_name = vformat("%s (%s)", disp_name, info["label"].operator String());
				}
				prop->set_label(disp_name);
				prop->set_tooltip(name);
				prop->set_selectable(false);

				prop->connect("property_changed", callable_mp(this, &EditorPropertyOTFeatures::_property_changed));
				prop->connect("object_id_selected", callable_mp(this, &EditorPropertyOTFeatures::_object_id_selected));

				HBoxContainer *hbox = memnew(HBoxContainer);
				property_vbox->add_child(hbox);
				hbox->add_child(prop);
				prop->set_h_size_flags(SIZE_EXPAND_FILL);
				Button *remove = memnew(Button);
				remove->set_icon(get_theme_icon(SNAME("Remove"), SNAME("EditorIcons")));
				hbox->add_child(remove);
				remove->connect("pressed", callable_mp(this, &EditorPropertyOTFeatures::_remove), varray(remove, name_tag));

				prop->update_property();
			}
		}

		PanelContainer *pc = memnew(PanelContainer);
		property_vbox->add_child(pc);
		pc->add_theme_style_override(SNAME("panel"), get_theme_stylebox(SNAME("DictionaryAddItem"), SNAME("EditorStyles")));

		VBoxContainer *add_vbox = memnew(VBoxContainer);
		pc->add_child(add_vbox);

		button_add = EditorInspector::create_inspector_action_button(TTR("Add Feature"));
		button_add->set_icon(get_theme_icon(SNAME("Add"), SNAME("EditorIcons")));
		button_add->connect("pressed", callable_mp(this, &EditorPropertyOTFeatures::_add_menu));
		add_vbox->add_child(button_add);

		updating = false;
	} else {
		if (vbox) {
			set_bottom_editor(nullptr);
			memdelete(vbox);
			button_add = nullptr;
			vbox = nullptr;
		}
	}
}

void EditorPropertyOTFeatures::_edit_pressed() {
	Variant prop_val = get_edited_object()->get(get_edited_property());
	if (prop_val.get_type() == Variant::NIL) {
		Callable::CallError ce;
		Variant::construct(Variant::DICTIONARY, prop_val, nullptr, 0, ce);
		get_edited_object()->set(get_edited_property(), prop_val);
	}

	get_edited_object()->editor_set_section_unfold(get_edited_property(), edit->is_pressed());
	update_property();
}

void EditorPropertyOTFeatures::_page_changed(int p_page) {
	if (updating) {
		return;
	}
	page_index = p_page;
	update_property();
}

EditorPropertyOTFeatures::EditorPropertyOTFeatures() {
	object.instantiate();
	page_length = int(EDITOR_GET("interface/inspector/max_array_dictionary_items_per_page"));

	edit = memnew(Button);
	edit->set_h_size_flags(SIZE_EXPAND_FILL);
	edit->set_clip_text(true);
	edit->connect("pressed", callable_mp(this, &EditorPropertyOTFeatures::_edit_pressed));
	edit->set_toggle_mode(true);
	add_child(edit);
	add_focusable(edit);

	menu = memnew(PopupMenu);
	add_child(menu);
	menu->connect("id_pressed", callable_mp(this, &EditorPropertyOTFeatures::_add_feature));

	for (int i = 0; i < FGRP_MAX; i++) {
		menu_sub[i] = memnew(PopupMenu);
		menu_sub[i]->set_name("FTRMenu_" + itos(i));
		menu->add_child(menu_sub[i]);
		menu_sub[i]->connect("id_pressed", callable_mp(this, &EditorPropertyOTFeatures::_add_feature));
	}

	group_names[FGRP_STYLISTIC_SET] = "Stylistic Sets";
	group_names[FGRP_CHARACTER_VARIANT] = "Character Variants";
	group_names[FGRP_CAPITLS] = "Capitals";
	group_names[FGRP_LIGATURES] = "Ligatures";
	group_names[FGRP_ALTERNATES] = "Alternates";
	group_names[FGRP_EAL] = "East Asian Language";
	group_names[FGRP_EAW] = "East Asian Widths";
	group_names[FGRP_NUMAL] = "Numeral Alignment";
	group_names[FGRP_CUSTOM] = "Custom";
}

/*************************************************************************/
/* EditorInspectorPluginFontConfig                                       */
/*************************************************************************/

bool EditorInspectorPluginFontConfig::can_handle(Object *p_object) {
	return (Object::cast_to<FontConfig>(p_object) != nullptr) || (Object::cast_to<DynamicFontImportSettingsData>(p_object) != nullptr);
}

bool EditorInspectorPluginFontConfig::parse_property(Object *p_object, const Variant::Type p_type, const String &p_path, const PropertyHint p_hint, const String &p_hint_text, const uint32_t p_usage, const bool p_wide) {
	if (p_path.ends_with("variation_opentype")) {
		add_property_editor(p_path, memnew(EditorPropertyOTVariation));
		return true;
	} else if (p_path == "opentype_features") {
		add_property_editor(p_path, memnew(EditorPropertyOTFeatures));
		return true;
	} else if (p_path == "language_support") {
		add_property_editor(p_path, memnew(EditorPropertyFontMetaOverride(false)));
		return true;
	} else if (p_path == "script_support") {
		add_property_editor(p_path, memnew(EditorPropertyFontMetaOverride(true)));
		return true;
	}
	return false;
}

/*************************************************************************/
/* FontPreview                                                           */
/*************************************************************************/

void FontPreview::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			// Draw font name (style).
			Ref<FontConfig> font = get_theme_font(SNAME("font"), SNAME("Label"));
			Color text_color = get_theme_color(SNAME("font_color"), SNAME("Label"));
			font->draw_string(get_canvas_item(), Point2(0, font->get_height() + 2 * EDSCALE), name, HORIZONTAL_ALIGNMENT_CENTER, get_size().x);

			// Draw font preview.
			Vector2 pos = Vector2(0, font->get_height()) + (get_size() - Vector2(0, font->get_height()) - line->get_size()) / 2;
			line->draw(get_canvas_item(), pos, text_color);

			// Draw font baseline.
			Color line_color = text_color;
			line_color.a *= 0.6;
			draw_line(Vector2(0, pos.y + line->get_line_ascent()), Vector2(pos.x - 5, pos.y + line->get_line_ascent()), line_color);
			draw_line(Vector2(pos.x + line->get_size().x + 5, pos.y + line->get_line_ascent()), Vector2(get_size().x, pos.y + line->get_line_ascent()), line_color);
		} break;
	}
}

void FontPreview::_bind_methods() {}

Size2 FontPreview::get_minimum_size() const {
	return Vector2(64, 64) * EDSCALE;
}

void FontPreview::set_data(const Ref<Font> &p_data) {
	Ref<FontConfig> f;
	f.instantiate();
	f->set_font(p_data);
	f->set_size(50);

	line->clear();
	name = vformat("%s (%s)", p_data->get_font_name(), p_data->get_font_style_name());
	if (p_data.is_valid()) {
		String sample;
		static const String sample_base = U"12漢字ԱբΑαАбΑαאבابܐܒހށआআਆઆଆஆఆಆആආกิກິༀကႠა한글ሀᎣᐁᚁᚠᜀᜠᝀᝠកᠠᤁᥐAb😀";
		for (int i = 0; i < sample_base.length(); i++) {
			if (p_data->has_char(sample_base[i])) {
				sample += sample_base[i];
			}
		}
		if (sample.is_empty()) {
			sample = p_data->get_supported_chars().substr(0, 6);
		}
		line->add_string(sample, f);
	}

	update();
}

FontPreview::FontPreview() {
	line.instantiate();
}

/*************************************************************************/
/* EditorInspectorPluginFontPreview                                      */
/*************************************************************************/

bool EditorInspectorPluginFontPreview::can_handle(Object *p_object) {
	return Object::cast_to<Font>(p_object) != nullptr;
}

void EditorInspectorPluginFontPreview::parse_begin(Object *p_object) {
	Font *fd = Object::cast_to<Font>(p_object);
	ERR_FAIL_COND(!fd);

	FontPreview *editor = memnew(FontPreview);
	editor->set_data(fd);
	add_custom_control(editor);
}

bool EditorInspectorPluginFontPreview::parse_property(Object *p_object, const Variant::Type p_type, const String &p_path, const PropertyHint p_hint, const String &p_hint_text, const uint32_t p_usage, const bool p_wide) {
	return false;
}

/*************************************************************************/
/* FontConfigEditorPlugin                                                */
/*************************************************************************/

FontConfigEditorPlugin::FontConfigEditorPlugin() {
	Ref<EditorInspectorPluginFontConfig> fc_plugin;
	fc_plugin.instantiate();
	EditorInspector::add_inspector_plugin(fc_plugin);

	Ref<EditorInspectorPluginFontPreview> fp_plugin;
	fp_plugin.instantiate();
	EditorInspector::add_inspector_plugin(fp_plugin);
}
