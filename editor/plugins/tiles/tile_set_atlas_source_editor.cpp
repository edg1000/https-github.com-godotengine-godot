/*************************************************************************/
/*  tile_set_atlas_source_editor.cpp                                     */
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

#include "tile_set_atlas_source_editor.h"

#include "tiles_editor_plugin.h"

#include "editor/editor_inspector.h"
#include "editor/editor_scale.h"
#include "editor/progress_dialog.h"

#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/control.h"
#include "scene/gui/item_list.h"
#include "scene/gui/separator.h"
#include "scene/gui/split_container.h"
#include "scene/gui/tab_container.h"

#include "core/core_string_names.h"
#include "core/math/geometry_2d.h"
#include "core/os/keyboard.h"

void TileSetAtlasSourceEditor::TileSetAtlasSourceProxyObject::set_id(int p_id) {
	ERR_FAIL_COND(p_id < 0);
	if (source_id == p_id) {
		return;
	}
	ERR_FAIL_COND_MSG(tile_set->has_source(p_id), vformat("Cannot change TileSet atlas source ID. Another atlas source exists with id %d.", p_id));

	int previous_source = source_id;
	source_id = p_id; // source_id must be updated before, because it's used by the atlas source list update.
	tile_set->set_source_id(previous_source, p_id);
	emit_signal("changed", "id");
}

int TileSetAtlasSourceEditor::TileSetAtlasSourceProxyObject::get_id() {
	return source_id;
}

bool TileSetAtlasSourceEditor::TileSetAtlasSourceProxyObject::_set(const StringName &p_name, const Variant &p_value) {
	bool valid = false;
	tile_set_atlas_source->set(p_name, p_value, &valid);
	if (valid) {
		emit_signal("changed", String(p_name).utf8().get_data());
	}
	return valid;
}

bool TileSetAtlasSourceEditor::TileSetAtlasSourceProxyObject::_get(const StringName &p_name, Variant &r_ret) const {
	if (!tile_set_atlas_source) {
		return false;
	}
	bool valid = false;
	r_ret = tile_set_atlas_source->get(p_name, &valid);
	return valid;
}

void TileSetAtlasSourceEditor::TileSetAtlasSourceProxyObject::_get_property_list(List<PropertyInfo> *p_list) const {
	p_list->push_back(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"));
	p_list->push_back(PropertyInfo(Variant::VECTOR2I, "margins", PROPERTY_HINT_NONE, ""));
	p_list->push_back(PropertyInfo(Variant::VECTOR2I, "separation", PROPERTY_HINT_NONE, ""));
	p_list->push_back(PropertyInfo(Variant::VECTOR2I, "tile_size", PROPERTY_HINT_NONE, ""));
}

void TileSetAtlasSourceEditor::TileSetAtlasSourceProxyObject::_bind_methods() {
	// -- Shape and layout --
	ClassDB::bind_method(D_METHOD("set_id", "id"), &TileSetAtlasSourceEditor::TileSetAtlasSourceProxyObject::set_id);
	ClassDB::bind_method(D_METHOD("get_id"), &TileSetAtlasSourceEditor::TileSetAtlasSourceProxyObject::get_id);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "id"), "set_id", "get_id");

	ADD_SIGNAL(MethodInfo("changed", PropertyInfo(Variant::STRING, "what")));
}

void TileSetAtlasSourceEditor::TileSetAtlasSourceProxyObject::edit(Ref<TileSet> p_tile_set, TileSetAtlasSource *p_tile_set_atlas_source, int p_source_id) {
	ERR_FAIL_COND(!p_tile_set.is_valid());
	ERR_FAIL_COND(!p_tile_set_atlas_source);
	ERR_FAIL_COND(p_source_id < 0);
	ERR_FAIL_COND(p_tile_set->get_source(p_source_id) != p_tile_set_atlas_source);

	// Disconnect to changes.
	if (tile_set_atlas_source) {
		tile_set_atlas_source->disconnect(CoreStringNames::get_singleton()->property_list_changed, callable_mp((Object *)this, &Object::notify_property_list_changed));
	}

	tile_set = p_tile_set;
	tile_set_atlas_source = p_tile_set_atlas_source;
	source_id = p_source_id;

	// Connect to changes.
	if (tile_set_atlas_source) {
		if (!tile_set_atlas_source->is_connected(CoreStringNames::get_singleton()->property_list_changed, callable_mp((Object *)this, &Object::notify_property_list_changed))) {
			tile_set_atlas_source->connect(CoreStringNames::get_singleton()->property_list_changed, callable_mp((Object *)this, &Object::notify_property_list_changed));
		}
	}

	notify_property_list_changed();
}

// -- Proxy object used by the tile inspector --
bool TileSetAtlasSourceEditor::TileProxyObject::_set(const StringName &p_name, const Variant &p_value) {
	if (!tile_set_atlas_source) {
		return false;
	}

	if (tiles.size() == 1) {
		const Vector2i &coords = tiles.front()->get().tile;
		const int &alternative = tiles.front()->get().alternative;

		if (alternative == 0 && p_name == "atlas_coords") {
			Vector2i as_vector2i = Vector2i(p_value);
			ERR_FAIL_COND_V(!tile_set_atlas_source->can_move_tile_in_atlas(coords, as_vector2i), false);

			if (tiles_set_atlas_source_editor->selection.front()->get().tile == coords) {
				tiles_set_atlas_source_editor->selection.clear();
				tiles_set_atlas_source_editor->selection.insert({ as_vector2i, 0 });
				tiles_set_atlas_source_editor->_update_tile_id_label();
			}

			tile_set_atlas_source->move_tile_in_atlas(coords, as_vector2i);
			tiles.clear();
			tiles.insert({ as_vector2i, 0 });
			emit_signal("changed", "atlas_coords");
			return true;
		} else if (alternative == 0 && p_name == "size_in_atlas") {
			Vector2i as_vector2i = Vector2i(p_value);
			ERR_FAIL_COND_V(!tile_set_atlas_source->can_move_tile_in_atlas(coords, TileSetAtlasSource::INVALID_ATLAS_COORDS, as_vector2i), false);

			tile_set_atlas_source->move_tile_in_atlas(coords, TileSetAtlasSource::INVALID_ATLAS_COORDS, as_vector2i);
			emit_signal("changed", "size_in_atlas");
			return true;
		} else if (alternative > 0 && p_name == "alternative_id") {
			int as_int = int(p_value);
			ERR_FAIL_COND_V(as_int < 0, false);
			ERR_FAIL_COND_V_MSG(tile_set_atlas_source->has_alternative_tile(coords, as_int), false, vformat("Cannot change alternative tile ID. Another alternative exists with id %d for tile at coords %s.", as_int, coords));

			if (tiles_set_atlas_source_editor->selection.front()->get().alternative == alternative) {
				tiles_set_atlas_source_editor->selection.clear();
				tiles_set_atlas_source_editor->selection.insert({ coords, as_int });
			}

			int previous_alternative_tile = alternative;
			tiles.clear();
			tiles.insert({ coords, as_int }); // tiles must be updated before.
			tile_set_atlas_source->set_alternative_tile_id(coords, previous_alternative_tile, as_int);

			emit_signal("changed", "alternative_id");
			return true;
		}
	}

	bool any_valid = false;
	for (Set<TileSelection>::Element *E = tiles.front(); E; E = E->next()) {
		const Vector2i &coords = E->get().tile;
		const int &alternative = E->get().alternative;

		bool valid = false;
		TileData *tile_data = Object::cast_to<TileData>(tile_set_atlas_source->get_tile_data(coords, alternative));
		ERR_FAIL_COND_V(!tile_data, false);
		tile_data->set(p_name, p_value, &valid);

		any_valid |= valid;
	}

	if (any_valid) {
		emit_signal("changed", String(p_name).utf8().get_data());
	}

	return any_valid;
}

bool TileSetAtlasSourceEditor::TileProxyObject::_get(const StringName &p_name, Variant &r_ret) const {
	if (!tile_set_atlas_source) {
		return false;
	}

	if (tiles.size() == 1) {
		const Vector2i &coords = tiles.front()->get().tile;
		const int &alternative = tiles.front()->get().alternative;

		if (alternative == 0 && p_name == "atlas_coords") {
			r_ret = coords;
			return true;
		} else if (alternative == 0 && p_name == "size_in_atlas") {
			r_ret = tile_set_atlas_source->get_tile_size_in_atlas(coords);
			return true;
		} else if (alternative > 0 && p_name == "alternative_id") {
			r_ret = alternative;
			return true;
		}
	}

	for (Set<TileSelection>::Element *E = tiles.front(); E; E = E->next()) {
		// Return the first tile with a property matching the name.
		// Note: It's a little bit annoying, but the behavior is the same the one in MultiNodeEdit.
		const Vector2i &coords = E->get().tile;
		const int &alternative = E->get().alternative;

		TileData *tile_data = Object::cast_to<TileData>(tile_set_atlas_source->get_tile_data(coords, alternative));
		ERR_FAIL_COND_V(!tile_data, false);

		bool valid = false;
		r_ret = tile_data->get(p_name, &valid);
		if (valid) {
			return true;
		}
	}

	return false;
}

void TileSetAtlasSourceEditor::TileProxyObject::_get_property_list(List<PropertyInfo> *p_list) const {
	if (!tile_set_atlas_source) {
		return;
	}

	if (tiles.size() == 1) {
		if (tiles.front()->get().alternative == 0) {
			p_list->push_back(PropertyInfo(Variant::VECTOR2I, "atlas_coords", PROPERTY_HINT_NONE, ""));
			p_list->push_back(PropertyInfo(Variant::VECTOR2I, "size_in_atlas", PROPERTY_HINT_NONE, ""));
		} else {
			p_list->push_back(PropertyInfo(Variant::INT, "alternative_id", PROPERTY_HINT_NONE, ""));
		}
	}

	// Get the list of properties common to all tiles (similar to what's done in MultiNodeEdit).
	struct PropertyId {
		int occurence_id = 0;
		String property;
		bool operator<(const PropertyId &p_other) const {
			return occurence_id == p_other.occurence_id ? property < p_other.property : occurence_id < p_other.occurence_id;
		}
	};
	struct PLData {
		int uses = 0;
		PropertyInfo property_info;
	};
	Map<PropertyId, PLData> usage;

	List<PLData *> data_list;
	for (Set<TileSelection>::Element *E = tiles.front(); E; E = E->next()) {
		const Vector2i &coords = E->get().tile;
		const int &alternative = E->get().alternative;

		TileData *tile_data = Object::cast_to<TileData>(tile_set_atlas_source->get_tile_data(coords, alternative));
		ERR_FAIL_COND(!tile_data);

		List<PropertyInfo> list;
		tile_data->get_property_list(&list);

		Map<String, int> counts; // Counts the number of time a property appears (useful for groups that may appear more than once)
		for (List<PropertyInfo>::Element *E_property = list.front(); E_property; E_property = E_property->next()) {
			const String &property_string = E_property->get().name;
			if (!tile_data->is_allowing_transform() && (property_string == "flip_h" || property_string == "flip_v" || property_string == "transpose")) {
				continue;
			}

			if (!counts.has(property_string)) {
				counts[property_string] = 1;
			} else {
				counts[property_string] += 1;
			}

			PropertyInfo stored_property_info = E_property->get();
			stored_property_info.usage |= PROPERTY_USAGE_STORAGE; // Ignore the storage flag in comparing properties.

			PropertyId id = { counts[property_string], property_string };
			if (!usage.has(id)) {
				usage[id] = { 1, stored_property_info };
				data_list.push_back(&usage[id]);
			} else if (usage[id].property_info == stored_property_info) {
				usage[id].uses += 1;
			}
		}
	}

	// Add only properties that are common to all tiles.
	for (List<PLData *>::Element *E = data_list.front(); E; E = E->next()) {
		if (E->get()->uses == tiles.size()) {
			p_list->push_back(E->get()->property_info);
		}
	}
}

void TileSetAtlasSourceEditor::TileProxyObject::edit(TileSetAtlasSource *p_tile_set_atlas_source, int p_source_id, Set<TileSelection> p_tiles) {
	ERR_FAIL_COND(!p_tile_set_atlas_source);
	ERR_FAIL_COND(p_source_id < 0);
	ERR_FAIL_COND(p_tiles.is_empty());
	for (Set<TileSelection>::Element *E = p_tiles.front(); E; E = E->next()) {
		ERR_FAIL_COND(E->get().tile == TileSetAtlasSource::INVALID_ATLAS_COORDS);
		ERR_FAIL_COND(E->get().alternative < 0);
	}

	// Disconnect to changes.
	for (Set<TileSelection>::Element *E = tiles.front(); E; E = E->next()) {
		const Vector2i &coords = E->get().tile;
		const int &alternative = E->get().alternative;

		if (tile_set_atlas_source && tile_set_atlas_source->has_tile(coords) && tile_set_atlas_source->has_alternative_tile(coords, alternative)) {
			TileData *tile_data = Object::cast_to<TileData>(tile_set_atlas_source->get_tile_data(coords, alternative));
			if (tile_data->is_connected(CoreStringNames::get_singleton()->property_list_changed, callable_mp((Object *)this, &Object::notify_property_list_changed))) {
				tile_data->disconnect(CoreStringNames::get_singleton()->property_list_changed, callable_mp((Object *)this, &Object::notify_property_list_changed));
			}
		}
	}

	tile_set_atlas_source = p_tile_set_atlas_source;
	source_id = p_source_id;
	tiles = Set<TileSelection>(p_tiles);

	// Connect to changes.
	for (Set<TileSelection>::Element *E = p_tiles.front(); E; E = E->next()) {
		const Vector2i &coords = E->get().tile;
		const int &alternative = E->get().alternative;

		if (tile_set_atlas_source->has_tile(coords) && tile_set_atlas_source->has_alternative_tile(coords, alternative)) {
			TileData *tile_data = Object::cast_to<TileData>(tile_set_atlas_source->get_tile_data(coords, alternative));
			if (!tile_data->is_connected(CoreStringNames::get_singleton()->property_list_changed, callable_mp((Object *)this, &Object::notify_property_list_changed))) {
				tile_data->connect(CoreStringNames::get_singleton()->property_list_changed, callable_mp((Object *)this, &Object::notify_property_list_changed));
			}
		}
	}

	notify_property_list_changed();
}

void TileSetAtlasSourceEditor::TileProxyObject::_bind_methods() {
	ADD_SIGNAL(MethodInfo("changed", PropertyInfo(Variant::STRING, "what")));
}

void TileSetAtlasSourceEditor::_inspector_property_selected(String p_property) {
	selected_property = p_property;
	_update_atlas_view();
}

void TileSetAtlasSourceEditor::_update_tile_id_label() {
	if (selection.size() == 1) {
		TileSelection selected = selection.front()->get();
		tool_tile_id_label->set_text(vformat("%d, %s, %d", tile_set_atlas_source_id, selected.tile, selected.alternative));
		tool_tile_id_label->set_tooltip(vformat(TTR("Selected tile:\nSource: %d\nAtlas coordinates: %s\nAlternative: %d"), tile_set_atlas_source_id, selected.tile, selected.alternative));
		tool_tile_id_label->show();
	} else {
		tool_tile_id_label->hide();
	}
}

void TileSetAtlasSourceEditor::_update_source_inspector() {
	// Update the proxy object.
	atlas_source_proxy_object->edit(tile_set, tile_set_atlas_source, tile_set_atlas_source_id);

	// Update the "clear outside texture" button.
	tool_advanced_menu_buttom->get_popup()->set_item_disabled(0, !tile_set_atlas_source->has_tiles_outside_texture());
}

void TileSetAtlasSourceEditor::_update_fix_selected_and_hovered_tiles() {
	// Fix selected.
	for (Set<TileSelection>::Element *E = selection.front(); E; E = E->next()) {
		TileSelection selected = E->get();
		if (!tile_set_atlas_source->has_tile(selected.tile) || !tile_set_atlas_source->has_alternative_tile(selected.tile, selected.alternative)) {
			selection.erase(E);
		}
	}

	// Fix hovered.
	if (!tile_set_atlas_source->has_tile(hovered_base_tile_coords)) {
		hovered_base_tile_coords = TileSetAtlasSource::INVALID_ATLAS_COORDS;
	}
	Vector2i coords = Vector2i(hovered_alternative_tile_coords.x, hovered_alternative_tile_coords.y);
	int alternative = hovered_alternative_tile_coords.z;
	if (!tile_set_atlas_source->has_tile(coords) || !tile_set_atlas_source->has_alternative_tile(coords, alternative)) {
		hovered_alternative_tile_coords = Vector3i(TileSetAtlasSource::INVALID_ATLAS_COORDS.x, TileSetAtlasSource::INVALID_ATLAS_COORDS.y, TileSetAtlasSource::INVALID_TILE_ALTERNATIVE);
	}
}

void TileSetAtlasSourceEditor::_update_tile_inspector() {
	bool has_atlas_tile_selected = (tools_button_group->get_pressed_button() == tool_select_button) && !selection.is_empty();

	// Update the proxy object.
	if (has_atlas_tile_selected) {
		tile_proxy_object->edit(tile_set_atlas_source, tile_set_atlas_source_id, selection);
	}

	// Update visibility.
	tile_inspector_label->set_visible(has_atlas_tile_selected);
	tile_inspector->set_visible(has_atlas_tile_selected);
}

void TileSetAtlasSourceEditor::_update_atlas_view() {
	// Update the atlas display.
	tile_atlas_view->set_atlas_source(*tile_set, tile_set_atlas_source, tile_set_atlas_source_id);

	// Create a bunch of buttons to add alternative tiles.
	for (int i = 0; i < alternative_tiles_control->get_child_count(); i++) {
		alternative_tiles_control->get_child(i)->queue_delete();
	}

	Vector2i pos;
	Vector2 texture_region_base_size = tile_set_atlas_source->get_texture_region_size();
	int texture_region_base_size_min = MIN(texture_region_base_size.x, texture_region_base_size.y);
	for (int i = 0; i < tile_set_atlas_source->get_tiles_count(); i++) {
		Vector2i tile_id = tile_set_atlas_source->get_tile_id(i);
		int alternative_count = tile_set_atlas_source->get_alternative_tiles_count(tile_id);
		if (alternative_count > 1) {
			// Compute the right extremity of alternative.
			int y_increment = 0;
			pos.x = 0;
			for (int j = 1; j < alternative_count; j++) {
				int alternative_id = tile_set_atlas_source->get_alternative_tile_id(tile_id, j);
				Rect2i rect = tile_atlas_view->get_alternative_tile_rect(tile_id, alternative_id);
				pos.x = MAX(pos.x, rect.get_end().x);
				y_increment = MAX(y_increment, rect.size.y);
			}

			// Create and position the button.
			Button *button = memnew(Button);
			alternative_tiles_control->add_child(button);
			button->set_flat(true);
			button->set_icon(get_theme_icon("Add", "EditorIcons"));
			button->add_theme_style_override("normal", memnew(StyleBoxEmpty));
			button->add_theme_style_override("hover", memnew(StyleBoxEmpty));
			button->add_theme_style_override("focus", memnew(StyleBoxEmpty));
			button->add_theme_style_override("pressed", memnew(StyleBoxEmpty));
			button->connect("pressed", callable_mp(tile_set_atlas_source, &TileSetAtlasSource::create_alternative_tile), varray(tile_id, -1));
			button->set_rect(Rect2(Vector2(pos.x, pos.y + (y_increment - texture_region_base_size.y) / 2.0), Vector2(texture_region_base_size_min, texture_region_base_size_min)));
			button->set_expand_icon(true);

			pos.y += y_increment;
		}
	}
	tile_atlas_view->set_padding(Side::SIDE_RIGHT, texture_region_base_size_min);

	// Redraw everything.
	tile_atlas_control->update();
	tile_atlas_control_unscaled->update();
	alternative_tiles_control->update();
	alternative_tiles_control_unscaled->update();
	tile_atlas_view->update();

	// Synchronize atlas view.
	TilesEditor::get_singleton()->synchronize_atlas_view(tile_atlas_view);
}

void TileSetAtlasSourceEditor::_update_toolbar() {
	// Hide all settings.
	for (int i = 0; i < tool_settings->get_child_count(); i++) {
		Object::cast_to<CanvasItem>(tool_settings->get_child(i))->hide();
	}

	// SHow only the correct settings.
	if (tools_button_group->get_pressed_button() == tool_select_button) {
	} else if (tools_button_group->get_pressed_button() == tool_add_remove_button) {
		tool_settings_vsep->show();
		tools_settings_erase_button->show();
	} else if (tools_button_group->get_pressed_button() == tool_add_remove_rect_button) {
		tool_settings_vsep->show();
		tools_settings_erase_button->show();
	}
}

void TileSetAtlasSourceEditor::_tile_atlas_control_mouse_exited() {
	hovered_base_tile_coords = TileSetAtlasSource::INVALID_ATLAS_COORDS;
	tile_atlas_control->update();
	tile_atlas_control_unscaled->update();
	tile_atlas_view->update();
}

void TileSetAtlasSourceEditor::_tile_atlas_view_transform_changed() {
	tile_atlas_control->update();
	tile_atlas_control_unscaled->update();
}

void TileSetAtlasSourceEditor::_tile_atlas_control_gui_input(const Ref<InputEvent> &p_event) {
	// Update the hovered coords.
	hovered_base_tile_coords = tile_atlas_view->get_atlas_tile_coords_at_pos(tile_atlas_control->get_local_mouse_position());

	// Handle the event.
	Ref<InputEventMouseMotion> mm = p_event;
	if (mm.is_valid()) {
		Vector2i start_base_tiles_coords = tile_atlas_view->get_atlas_tile_coords_at_pos(drag_start_mouse_pos);
		Vector2i last_base_tiles_coords = tile_atlas_view->get_atlas_tile_coords_at_pos(drag_last_mouse_pos);
		Vector2i new_base_tiles_coords = tile_atlas_view->get_atlas_tile_coords_at_pos(tile_atlas_control->get_local_mouse_position());

		Vector2i grid_size = tile_set_atlas_source->get_atlas_grid_size();

		if (drag_type == DRAG_TYPE_NONE) {
			if (selection.size() == 1) {
				// Change the cursor depending on the hovered thing.
				TileSelection selected = selection.front()->get();
				if (selected.tile != TileSetAtlasSource::INVALID_ATLAS_COORDS && selected.alternative == 0) {
					Vector2 mouse_local_pos = tile_atlas_control->get_local_mouse_position();
					Vector2i size_in_atlas = tile_set_atlas_source->get_tile_size_in_atlas(selected.tile);
					Rect2 region = tile_set_atlas_source->get_tile_texture_region(selected.tile);
					Size2 zoomed_size = resize_handle->get_size() / tile_atlas_view->get_zoom();
					Rect2 rect = region.grow_individual(zoomed_size.x, zoomed_size.y, 0, 0);
					const Vector2i coords[] = { Vector2i(0, 0), Vector2i(1, 0), Vector2i(1, 1), Vector2i(0, 1) };
					const Vector2i directions[] = { Vector2i(0, -1), Vector2i(1, 0), Vector2i(0, 1), Vector2i(-1, 0) };
					CursorShape cursor_shape = CURSOR_ARROW;
					bool can_grow[4];
					for (int i = 0; i < 4; i++) {
						can_grow[i] = tile_set_atlas_source->can_move_tile_in_atlas(selected.tile, selected.tile + directions[i]);
						can_grow[i] |= (i % 2 == 0) ? size_in_atlas.y > 1 : size_in_atlas.x > 1;
					}
					for (int i = 0; i < 4; i++) {
						Vector2 pos = rect.position + Vector2(rect.size.x, rect.size.y) * coords[i];
						if (can_grow[i] && can_grow[(i + 3) % 4] && Rect2(pos, zoomed_size).has_point(mouse_local_pos)) {
							cursor_shape = (i % 2) ? CURSOR_BDIAGSIZE : CURSOR_FDIAGSIZE;
						}
						Vector2 next_pos = rect.position + Vector2(rect.size.x, rect.size.y) * coords[(i + 1) % 4];
						if (can_grow[i] && Rect2((pos + next_pos) / 2.0, zoomed_size).has_point(mouse_local_pos)) {
							cursor_shape = (i % 2) ? CURSOR_HSIZE : CURSOR_VSIZE;
						}
					}
					tile_atlas_control->set_default_cursor_shape(cursor_shape);
				}
			}
		} else if (drag_type == DRAG_TYPE_CREATE_BIG_TILE) {
			// Create big tile.
			new_base_tiles_coords = new_base_tiles_coords.max(Vector2i(0, 0)).min(grid_size - Vector2i(1, 1));

			Rect2i new_rect = Rect2i(start_base_tiles_coords, new_base_tiles_coords - start_base_tiles_coords).abs();
			new_rect.size += Vector2i(1, 1);
			// Check if the new tile can fit in the new rect.
			if (tile_set_atlas_source->can_move_tile_in_atlas(drag_current_tile, new_rect.position, new_rect.size)) {
				// Move and resize the tile.
				tile_set_atlas_source->move_tile_in_atlas(drag_current_tile, new_rect.position, new_rect.size);
				drag_current_tile = new_rect.position;
			}
		} else if (drag_type == DRAG_TYPE_CREATE_TILES) {
			// Create tiles.
			last_base_tiles_coords = last_base_tiles_coords.max(Vector2i(0, 0)).min(grid_size - Vector2i(1, 1));
			new_base_tiles_coords = new_base_tiles_coords.max(Vector2i(0, 0)).min(grid_size - Vector2i(1, 1));

			Vector<Point2i> line = Geometry2D::bresenham_line(last_base_tiles_coords, new_base_tiles_coords);
			for (int i = 0; i < line.size(); i++) {
				if (tile_set_atlas_source->get_tile_at_coords(line[i]) == TileSetAtlasSource::INVALID_ATLAS_COORDS) {
					tile_set_atlas_source->create_tile(line[i]);
					drag_modified_tiles.insert(line[i]);
				}
			}

			drag_last_mouse_pos = tile_atlas_control->get_local_mouse_position();

		} else if (drag_type == DRAG_TYPE_REMOVE_TILES) {
			// Remove tiles.
			last_base_tiles_coords = last_base_tiles_coords.max(Vector2i(0, 0)).min(grid_size - Vector2i(1, 1));
			new_base_tiles_coords = new_base_tiles_coords.max(Vector2i(0, 0)).min(grid_size - Vector2i(1, 1));

			Vector<Point2i> line = Geometry2D::bresenham_line(last_base_tiles_coords, new_base_tiles_coords);
			for (int i = 0; i < line.size(); i++) {
				Vector2i base_tile_coords = tile_set_atlas_source->get_tile_at_coords(line[i]);
				if (base_tile_coords != TileSetAtlasSource::INVALID_ATLAS_COORDS) {
					drag_modified_tiles.insert(base_tile_coords);
				}
			}

			drag_last_mouse_pos = tile_atlas_control->get_local_mouse_position();
		} else if (drag_type == DRAG_TYPE_MOVE_TILE) {
			// Move tile.
			Vector2 mouse_offset = (Vector2(tile_set_atlas_source->get_tile_size_in_atlas(drag_current_tile)) / 2.0 - Vector2(0.5, 0.5)) * tile_set->get_tile_size();
			Vector2i coords = tile_atlas_view->get_atlas_tile_coords_at_pos(tile_atlas_control->get_local_mouse_position() - mouse_offset);
			coords = coords.max(Vector2i(0, 0)).min(grid_size - Vector2i(1, 1));
			if (drag_current_tile != coords && tile_set_atlas_source->can_move_tile_in_atlas(drag_current_tile, coords)) {
				tile_set_atlas_source->move_tile_in_atlas(drag_current_tile, coords);
				selection.clear();
				selection.insert({ coords, 0 });
				drag_current_tile = coords;

				// Update only what's needed.
				tile_set_atlas_source_changed_needs_update = false;
				_update_tile_inspector();
				_update_atlas_view();
				_update_tile_id_label();
			}
		} else if (drag_type >= DRAG_TYPE_RESIZE_TOP_LEFT && drag_type <= DRAG_TYPE_RESIZE_LEFT) {
			// Resizing a tile.
			new_base_tiles_coords = new_base_tiles_coords.max(Vector2i(-1, -1)).min(grid_size);

			Rect2i old_rect = Rect2i(drag_current_tile, tile_set_atlas_source->get_tile_size_in_atlas(drag_current_tile));
			Rect2i new_rect = old_rect;

			if (drag_type == DRAG_TYPE_RESIZE_LEFT || drag_type == DRAG_TYPE_RESIZE_TOP_LEFT || drag_type == DRAG_TYPE_RESIZE_BOTTOM_LEFT) {
				new_rect.position.x = MIN(new_base_tiles_coords.x + 1, old_rect.get_end().x - 1);
				new_rect.size.x = old_rect.get_end().x - new_rect.position.x;
			}
			if (drag_type == DRAG_TYPE_RESIZE_TOP || drag_type == DRAG_TYPE_RESIZE_TOP_LEFT || drag_type == DRAG_TYPE_RESIZE_TOP_RIGHT) {
				new_rect.position.y = MIN(new_base_tiles_coords.y + 1, old_rect.get_end().y - 1);
				new_rect.size.y = old_rect.get_end().y - new_rect.position.y;
			}

			if (drag_type == DRAG_TYPE_RESIZE_RIGHT || drag_type == DRAG_TYPE_RESIZE_TOP_RIGHT || drag_type == DRAG_TYPE_RESIZE_BOTTOM_RIGHT) {
				new_rect.set_end(Vector2i(MAX(new_base_tiles_coords.x, old_rect.position.x + 1), new_rect.get_end().y));
			}
			if (drag_type == DRAG_TYPE_RESIZE_BOTTOM || drag_type == DRAG_TYPE_RESIZE_BOTTOM_LEFT || drag_type == DRAG_TYPE_RESIZE_BOTTOM_RIGHT) {
				new_rect.set_end(Vector2i(new_rect.get_end().x, MAX(new_base_tiles_coords.y, old_rect.position.y + 1)));
			}

			if (tile_set_atlas_source->can_move_tile_in_atlas(drag_current_tile, new_rect.position, new_rect.size)) {
				tile_set_atlas_source->move_tile_in_atlas(drag_current_tile, new_rect.position, new_rect.size);
				selection.clear();
				selection.insert({ new_rect.position, 0 });
				drag_current_tile = new_rect.position;

				// Update only what's needed.
				tile_set_atlas_source_changed_needs_update = false;
				_update_tile_inspector();
				_update_atlas_view();
				_update_tile_id_label();
			}
		}

		// Redraw for the hovered tile.
		tile_atlas_control->update();
		tile_atlas_control_unscaled->update();
		alternative_tiles_control->update();
		alternative_tiles_control_unscaled->update();
		tile_atlas_view->update();
		return;
	}

	Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid()) {
		Vector2 mouse_local_pos = tile_atlas_control->get_local_mouse_position();
		if (mb->get_button_index() == MOUSE_BUTTON_LEFT) {
			if (mb->is_pressed()) {
				// Left click pressed.
				if (tools_button_group->get_pressed_button() == tool_add_remove_button) {
					if (tools_settings_erase_button->is_pressed()) {
						// Remove tiles.

						// Setup the dragging info.
						drag_type = DRAG_TYPE_REMOVE_TILES;
						drag_start_mouse_pos = mouse_local_pos;
						drag_last_mouse_pos = drag_start_mouse_pos;

						// Remove a first tile.
						Vector2i coords = tile_atlas_view->get_atlas_tile_coords_at_pos(drag_start_mouse_pos);
						if (coords != TileSetAtlasSource::INVALID_ATLAS_COORDS) {
							coords = tile_set_atlas_source->get_tile_at_coords(coords);
						}
						if (coords != TileSetAtlasSource::INVALID_ATLAS_COORDS) {
							drag_modified_tiles.insert(coords);
						}
					} else {
						if (mb->is_shift_pressed()) {
							// Create a big tile.
							Vector2i coords = tile_atlas_view->get_atlas_tile_coords_at_pos(mouse_local_pos);
							if (coords != TileSetAtlasSource::INVALID_ATLAS_COORDS && tile_set_atlas_source->get_tile_at_coords(coords) == TileSetAtlasSource::INVALID_ATLAS_COORDS) {
								// Setup the dragging info, only if we start on an empty tile.
								drag_type = DRAG_TYPE_CREATE_BIG_TILE;
								drag_start_mouse_pos = mouse_local_pos;
								drag_last_mouse_pos = drag_start_mouse_pos;
								drag_current_tile = coords;

								// Create a tile.
								tile_set_atlas_source->create_tile(coords);
							}
						} else {
							// Create tiles.

							// Setup the dragging info.
							drag_type = DRAG_TYPE_CREATE_TILES;
							drag_start_mouse_pos = mouse_local_pos;
							drag_last_mouse_pos = drag_start_mouse_pos;

							// Create a first tile if needed.
							Vector2i coords = tile_atlas_view->get_atlas_tile_coords_at_pos(drag_start_mouse_pos);
							if (coords != TileSetAtlasSource::INVALID_ATLAS_COORDS && tile_set_atlas_source->get_tile_at_coords(coords) == TileSetAtlasSource::INVALID_ATLAS_COORDS) {
								tile_set_atlas_source->create_tile(coords);
								drag_modified_tiles.insert(coords);
							}
						}
					}
				} else if (tools_button_group->get_pressed_button() == tool_add_remove_rect_button) {
					if (tools_settings_erase_button->is_pressed()) {
						// Remove tiles using rect.

						// Setup the dragging info.
						drag_type = DRAG_TYPE_REMOVE_TILES_USING_RECT;
						drag_start_mouse_pos = mouse_local_pos;
						drag_last_mouse_pos = drag_start_mouse_pos;
					} else {
						if (mb->is_shift_pressed()) {
							// Create a big tile.
							Vector2i coords = tile_atlas_view->get_atlas_tile_coords_at_pos(mouse_local_pos);
							if (coords != TileSetAtlasSource::INVALID_ATLAS_COORDS && tile_set_atlas_source->get_tile_at_coords(coords) == TileSetAtlasSource::INVALID_ATLAS_COORDS) {
								// Setup the dragging info, only if we start on an empty tile.
								drag_type = DRAG_TYPE_CREATE_BIG_TILE;
								drag_start_mouse_pos = mouse_local_pos;
								drag_last_mouse_pos = drag_start_mouse_pos;
								drag_current_tile = coords;

								// Create a tile.
								tile_set_atlas_source->create_tile(coords);
							}
						} else {
							// Create tiles using rect.
							drag_type = DRAG_TYPE_CREATE_TILES_USING_RECT;
							drag_start_mouse_pos = mouse_local_pos;
							drag_last_mouse_pos = drag_start_mouse_pos;
						}
					}
				} else if (tools_button_group->get_pressed_button() == tool_select_button) {
					// Dragging a handle.
					drag_type = DRAG_TYPE_NONE;
					if (selection.size() == 1) {
						TileSelection selected = selection.front()->get();
						if (selected.tile != TileSetAtlasSource::INVALID_ATLAS_COORDS && selected.alternative == 0) {
							Vector2i size_in_atlas = tile_set_atlas_source->get_tile_size_in_atlas(selected.tile);
							Rect2 region = tile_set_atlas_source->get_tile_texture_region(selected.tile);
							Size2 zoomed_size = resize_handle->get_size() / tile_atlas_view->get_zoom();
							Rect2 rect = region.grow_individual(zoomed_size.x, zoomed_size.y, 0, 0);
							const Vector2i coords[] = { Vector2i(0, 0), Vector2i(1, 0), Vector2i(1, 1), Vector2i(0, 1) };
							const Vector2i directions[] = { Vector2i(0, -1), Vector2i(1, 0), Vector2i(0, 1), Vector2i(-1, 0) };
							CursorShape cursor_shape = CURSOR_ARROW;
							bool can_grow[4];
							for (int i = 0; i < 4; i++) {
								can_grow[i] = tile_set_atlas_source->can_move_tile_in_atlas(selected.tile, selected.tile + directions[i]);
								can_grow[i] |= (i % 2 == 0) ? size_in_atlas.y > 1 : size_in_atlas.x > 1;
							}
							for (int i = 0; i < 4; i++) {
								Vector2 pos = rect.position + Vector2(rect.size.x, rect.size.y) * coords[i];
								if (can_grow[i] && can_grow[(i + 3) % 4] && Rect2(pos, zoomed_size).has_point(mouse_local_pos)) {
									drag_type = (DragType)((int)DRAG_TYPE_RESIZE_TOP_LEFT + i * 2);
									drag_start_mouse_pos = mouse_local_pos;
									drag_last_mouse_pos = drag_start_mouse_pos;
									drag_current_tile = selected.tile;
									drag_start_tile_shape = Rect2i(selected.tile, tile_set_atlas_source->get_tile_size_in_atlas(selected.tile));
									cursor_shape = (i % 2) ? CURSOR_BDIAGSIZE : CURSOR_FDIAGSIZE;
								}
								Vector2 next_pos = rect.position + Vector2(rect.size.x, rect.size.y) * coords[(i + 1) % 4];
								if (can_grow[i] && Rect2((pos + next_pos) / 2.0, zoomed_size).has_point(mouse_local_pos)) {
									drag_type = (DragType)((int)DRAG_TYPE_RESIZE_TOP + i * 2);
									drag_start_mouse_pos = mouse_local_pos;
									drag_last_mouse_pos = drag_start_mouse_pos;
									drag_current_tile = selected.tile;
									drag_start_tile_shape = Rect2i(selected.tile, tile_set_atlas_source->get_tile_size_in_atlas(selected.tile));
									cursor_shape = (i % 2) ? CURSOR_HSIZE : CURSOR_VSIZE;
								}
							}
							tile_atlas_control->set_default_cursor_shape(cursor_shape);
						}
					}

					// Selecting then dragging a tile.
					if (drag_type == DRAG_TYPE_NONE) {
						TileSelection selected = { TileSetAtlasSource::INVALID_ATLAS_COORDS, TileSetAtlasSource::INVALID_TILE_ALTERNATIVE };
						Vector2i coords = tile_atlas_view->get_atlas_tile_coords_at_pos(mouse_local_pos);
						if (coords != TileSetAtlasSource::INVALID_ATLAS_COORDS) {
							coords = tile_set_atlas_source->get_tile_at_coords(coords);
							if (coords != TileSetAtlasSource::INVALID_ATLAS_COORDS) {
								selected = { coords, 0 };
							}
						}

						bool shift = mb->is_shift_pressed();
						if (!shift && selection.size() == 1 && selected.tile != TileSetAtlasSource::INVALID_ATLAS_COORDS && selection.has(selected)) {
							// Start move dragging.
							drag_type = DRAG_TYPE_MOVE_TILE;
							drag_start_mouse_pos = mouse_local_pos;
							drag_last_mouse_pos = drag_start_mouse_pos;
							drag_current_tile = selected.tile;
							drag_start_tile_shape = Rect2i(selected.tile, tile_set_atlas_source->get_tile_size_in_atlas(selected.tile));
							tile_atlas_control->set_default_cursor_shape(CURSOR_MOVE);
						} else {
							// Start selection dragging.
							drag_type = DRAG_TYPE_RECT_SELECT;
							drag_start_mouse_pos = mouse_local_pos;
							drag_last_mouse_pos = drag_start_mouse_pos;
						}
					}
				}
			} else {
				// Left click released.
				_end_dragging();
			}
			tile_atlas_control->update();
			tile_atlas_control_unscaled->update();
			alternative_tiles_control->update();
			alternative_tiles_control_unscaled->update();
			tile_atlas_view->update();
			return;
		} else if (mb->get_button_index() == MOUSE_BUTTON_RIGHT) {
			if (mb->is_pressed()) {
				// Right click pressed.

				TileSelection selected = { tile_atlas_view->get_atlas_tile_coords_at_pos(mouse_local_pos), 0 };
				if (selected.tile != TileSetAtlasSource::INVALID_ATLAS_COORDS) {
					selected.tile = tile_set_atlas_source->get_tile_at_coords(selected.tile);
				}

				// Set the selection if needed.
				if (selection.size() <= 1) {
					if (selected.tile != TileSetAtlasSource::INVALID_ATLAS_COORDS) {
						undo_redo->create_action(TTR("Select tiles"));
						undo_redo->add_undo_method(this, "_set_selection_from_array", _get_selection_as_array());
						selection.clear();
						selection.insert(selected);
						undo_redo->add_do_method(this, "_set_selection_from_array", _get_selection_as_array());
						undo_redo->commit_action(false);
						_update_tile_inspector();
						_update_tile_id_label();
					}
				}

				// Pops up the correct menu, depending on whether we have a tile or not.
				if (selected.tile != TileSetAtlasSource::INVALID_ATLAS_COORDS && selection.has(selected)) {
					// We have a tile.
					menu_option_coords = selected.tile;
					menu_option_alternative = 0;
					base_tile_popup_menu->popup(Rect2i(get_global_mouse_position(), Size2i()));
				} else if (hovered_base_tile_coords != TileSetAtlasSource::INVALID_ATLAS_COORDS) {
					// We don't have a tile, but can create one.
					menu_option_coords = hovered_base_tile_coords;
					menu_option_alternative = TileSetAtlasSource::INVALID_TILE_ALTERNATIVE;
					empty_base_tile_popup_menu->popup(Rect2i(get_global_mouse_position(), Size2i()));
				}
			} else {
				// Right click released.
				_end_dragging();
			}
			tile_atlas_control->update();
			tile_atlas_control_unscaled->update();
			alternative_tiles_control->update();
			alternative_tiles_control_unscaled->update();
			tile_atlas_view->update();
			return;
		}
	}
}

void TileSetAtlasSourceEditor::_end_dragging() {
	switch (drag_type) {
		case DRAG_TYPE_CREATE_TILES:
			undo_redo->create_action(TTR("Create tiles"));
			for (Set<Vector2i>::Element *E = drag_modified_tiles.front(); E; E = E->next()) {
				undo_redo->add_do_method(tile_set_atlas_source, "create_tile", E->get());
				undo_redo->add_undo_method(tile_set_atlas_source, "remove_tile", E->get());
			}
			undo_redo->commit_action(false);
			break;
		case DRAG_TYPE_CREATE_BIG_TILE:
			undo_redo->create_action(TTR("Create a tile"));
			undo_redo->add_do_method(tile_set_atlas_source, "create_tile", drag_current_tile, tile_set_atlas_source->get_tile_size_in_atlas(drag_current_tile));
			undo_redo->add_undo_method(tile_set_atlas_source, "remove_tile", drag_current_tile);
			undo_redo->commit_action(false);
			break;
		case DRAG_TYPE_REMOVE_TILES: {
			List<PropertyInfo> list;
			tile_set_atlas_source->get_property_list(&list);
			Map<Vector2i, List<const PropertyInfo *>> per_tile = _group_properties_per_tiles(list, tile_set_atlas_source);
			undo_redo->create_action(TTR("Remove tiles"));
			for (Set<Vector2i>::Element *E = drag_modified_tiles.front(); E; E = E->next()) {
				Vector2i coords = E->get();
				undo_redo->add_do_method(tile_set_atlas_source, "remove_tile", coords);
				undo_redo->add_undo_method(tile_set_atlas_source, "create_tile", coords);
				if (per_tile.has(coords)) {
					for (List<const PropertyInfo *>::Element *E_property = per_tile[coords].front(); E_property; E_property = E_property->next()) {
						String property = E_property->get()->name;
						Variant value = tile_set_atlas_source->get(property);
						if (value.get_type() != Variant::NIL) {
							undo_redo->add_undo_method(tile_set_atlas_source, "set", E_property->get()->name, value);
						}
					}
				}
			}
			undo_redo->commit_action();
		} break;
		case DRAG_TYPE_CREATE_TILES_USING_RECT: {
			Vector2i start_base_tiles_coords = tile_atlas_view->get_atlas_tile_coords_at_pos(drag_start_mouse_pos);
			Vector2i new_base_tiles_coords = tile_atlas_view->get_atlas_tile_coords_at_pos(tile_atlas_control->get_local_mouse_position());
			Rect2i area = Rect2i(start_base_tiles_coords, new_base_tiles_coords - start_base_tiles_coords).abs();
			area.set_end((area.get_end() + Vector2i(1, 1)).min(tile_set_atlas_source->get_atlas_grid_size()));
			undo_redo->create_action(TTR("Create tiles"));
			for (int x = area.get_position().x; x < area.get_end().x; x++) {
				for (int y = area.get_position().y; y < area.get_end().y; y++) {
					Vector2i coords = Vector2i(x, y);
					if (tile_set_atlas_source->get_tile_at_coords(coords) == TileSetAtlasSource::INVALID_ATLAS_COORDS) {
						undo_redo->add_do_method(tile_set_atlas_source, "create_tile", coords);
						undo_redo->add_undo_method(tile_set_atlas_source, "remove_tile", coords);
					}
				}
			}
			undo_redo->commit_action();
		} break;
		case DRAG_TYPE_REMOVE_TILES_USING_RECT: {
			Vector2i start_base_tiles_coords = tile_atlas_view->get_atlas_tile_coords_at_pos(drag_start_mouse_pos);
			Vector2i new_base_tiles_coords = tile_atlas_view->get_atlas_tile_coords_at_pos(tile_atlas_control->get_local_mouse_position());
			Rect2i area = Rect2i(start_base_tiles_coords, new_base_tiles_coords - start_base_tiles_coords).abs();
			area.set_end((area.get_end() + Vector2i(1, 1)).min(tile_set_atlas_source->get_atlas_grid_size()));
			List<PropertyInfo> list;
			tile_set_atlas_source->get_property_list(&list);
			Map<Vector2i, List<const PropertyInfo *>> per_tile = _group_properties_per_tiles(list, tile_set_atlas_source);

			Set<Vector2i> to_delete;
			for (int x = area.get_position().x; x < area.get_end().x; x++) {
				for (int y = area.get_position().y; y < area.get_end().y; y++) {
					Vector2i coords = tile_set_atlas_source->get_tile_at_coords(Vector2i(x, y));
					if (coords != TileSetAtlasSource::INVALID_ATLAS_COORDS) {
						to_delete.insert(coords);
					}
				}
			}

			undo_redo->create_action(TTR("Remove tiles"));
			undo_redo->add_do_method(this, "_set_selection_from_array", Array());
			for (Set<Vector2i>::Element *E = to_delete.front(); E; E = E->next()) {
				Vector2i coords = E->get();
				undo_redo->add_do_method(tile_set_atlas_source, "remove_tile", coords);
				undo_redo->add_undo_method(tile_set_atlas_source, "create_tile", coords);
				if (per_tile.has(coords)) {
					for (List<const PropertyInfo *>::Element *E_property = per_tile[coords].front(); E_property; E_property = E_property->next()) {
						String property = E_property->get()->name;
						Variant value = tile_set_atlas_source->get(property);
						if (value.get_type() != Variant::NIL) {
							undo_redo->add_undo_method(tile_set_atlas_source, "set", E_property->get()->name, value);
						}
					}
				}
			}
			undo_redo->add_undo_method(this, "_set_selection_from_array", _get_selection_as_array());
			undo_redo->commit_action();
		} break;
		case DRAG_TYPE_MOVE_TILE:
			if (drag_current_tile != drag_start_tile_shape.position) {
				undo_redo->create_action(TTR("Move a tile"));
				undo_redo->add_do_method(tile_set_atlas_source, "move_tile_in_atlas", drag_start_tile_shape.position, drag_current_tile, tile_set_atlas_source->get_tile_size_in_atlas(drag_current_tile));
				undo_redo->add_do_method(this, "_set_selection_from_array", _get_selection_as_array());
				undo_redo->add_undo_method(tile_set_atlas_source, "move_tile_in_atlas", drag_current_tile, drag_start_tile_shape.position, drag_start_tile_shape.size);
				Array array;
				array.push_back(drag_start_tile_shape.position);
				array.push_back(0);
				undo_redo->add_undo_method(this, "_set_selection_from_array", array);
				undo_redo->commit_action(false);
			}
			break;
		case DRAG_TYPE_RECT_SELECT: {
			Vector2i start_base_tiles_coords = tile_atlas_view->get_atlas_tile_coords_at_pos(drag_start_mouse_pos);
			Vector2i new_base_tiles_coords = tile_atlas_view->get_atlas_tile_coords_at_pos(tile_atlas_control->get_local_mouse_position());
			ERR_FAIL_COND(start_base_tiles_coords == TileSetAtlasSource::INVALID_ATLAS_COORDS);
			ERR_FAIL_COND(new_base_tiles_coords == TileSetAtlasSource::INVALID_ATLAS_COORDS);

			Rect2i region = Rect2i(start_base_tiles_coords, new_base_tiles_coords - start_base_tiles_coords).abs();
			region.size += Vector2i(1, 1);

			undo_redo->create_action(TTR("Select tiles"));
			undo_redo->add_undo_method(this, "_set_selection_from_array", _get_selection_as_array());

			// Determine if we clear, then add or remove to the selection.
			bool add_to_selection = true;
			if (Input::get_singleton()->is_key_pressed(KEY_SHIFT)) {
				Vector2i coords = tile_set_atlas_source->get_tile_at_coords(start_base_tiles_coords);
				if (coords != TileSetAtlasSource::INVALID_ATLAS_COORDS) {
					if (selection.has({ coords, 0 })) {
						add_to_selection = false;
					}
				}
			} else {
				selection.clear();
			}

			// Modify the selection.
			for (int x = region.position.x; x < region.get_end().x; x++) {
				for (int y = region.position.y; y < region.get_end().y; y++) {
					Vector2i coords = Vector2i(x, y);
					coords = tile_set_atlas_source->get_tile_at_coords(coords);
					if (coords != TileSetAtlasSource::INVALID_ATLAS_COORDS) {
						if (add_to_selection && !selection.has({ coords, 0 })) {
							selection.insert({ coords, 0 });
						} else if (!add_to_selection && selection.has({ coords, 0 })) {
							selection.erase({ coords, 0 });
						}
					}
				}
			}
			_update_tile_inspector();
			_update_tile_id_label();
			undo_redo->add_do_method(this, "_set_selection_from_array", _get_selection_as_array());
			undo_redo->commit_action(false);
			break;
		}
		case DRAG_TYPE_RESIZE_TOP_LEFT:
		case DRAG_TYPE_RESIZE_TOP:
		case DRAG_TYPE_RESIZE_TOP_RIGHT:
		case DRAG_TYPE_RESIZE_RIGHT:
		case DRAG_TYPE_RESIZE_BOTTOM_RIGHT:
		case DRAG_TYPE_RESIZE_BOTTOM:
		case DRAG_TYPE_RESIZE_BOTTOM_LEFT:
		case DRAG_TYPE_RESIZE_LEFT:
			if (drag_start_tile_shape != Rect2i(drag_current_tile, tile_set_atlas_source->get_tile_size_in_atlas(drag_current_tile))) {
				undo_redo->create_action(TTR("Resize a tile"));
				undo_redo->add_do_method(tile_set_atlas_source, "move_tile_in_atlas", drag_start_tile_shape.position, drag_current_tile, tile_set_atlas_source->get_tile_size_in_atlas(drag_current_tile));
				undo_redo->add_do_method(this, "_set_selection_from_array", _get_selection_as_array());
				undo_redo->add_undo_method(tile_set_atlas_source, "move_tile_in_atlas", drag_current_tile, drag_start_tile_shape.position, drag_start_tile_shape.size);
				Array array;
				array.push_back(drag_start_tile_shape.position);
				array.push_back(0);
				undo_redo->add_undo_method(this, "_set_selection_from_array", array);
				undo_redo->commit_action(false);
			}
			break;
		default:
			break;
	}

	drag_modified_tiles.clear();
	drag_type = DRAG_TYPE_NONE;
	tile_atlas_control->set_default_cursor_shape(CURSOR_ARROW);
}

Map<Vector2i, List<const PropertyInfo *>> TileSetAtlasSourceEditor::_group_properties_per_tiles(const List<PropertyInfo> &r_list, const TileSetAtlasSource *p_atlas) {
	// Group properties per tile.
	Map<Vector2i, List<const PropertyInfo *>> per_tile;
	for (const List<PropertyInfo>::Element *E_property = r_list.front(); E_property; E_property = E_property->next()) {
		Vector<String> components = String(E_property->get().name).split("/", true, 1);
		if (components.size() >= 1) {
			Vector<String> coord_arr = components[0].split(":");
			if (coord_arr.size() == 2 && coord_arr[0].is_valid_integer() && coord_arr[1].is_valid_integer()) {
				Vector2i coords = Vector2i(coord_arr[0].to_int(), coord_arr[1].to_int());
				per_tile[coords].push_back(&(E_property->get()));
			}
		}
	}
	return per_tile;
}

void TileSetAtlasSourceEditor::_menu_option(int p_option) {
	switch (p_option) {
		case TILE_DELETE: {
			List<PropertyInfo> list;
			tile_set_atlas_source->get_property_list(&list);
			Map<Vector2i, List<const PropertyInfo *>> per_tile = _group_properties_per_tiles(list, tile_set_atlas_source);
			undo_redo->create_action(TTR("Remove tile"));

			// Remove tiles
			Set<Vector2i> removed;
			for (Set<TileSelection>::Element *E = selection.front(); E; E = E->next()) {
				TileSelection selected = E->get();
				if (selected.alternative == 0) {
					// Remove a tile.
					undo_redo->add_do_method(tile_set_atlas_source, "remove_tile", selected.tile);
					undo_redo->add_undo_method(tile_set_atlas_source, "create_tile", selected.tile);
					removed.insert(selected.tile);
					if (per_tile.has(selected.tile)) {
						for (List<const PropertyInfo *>::Element *E_property = per_tile[selected.tile].front(); E_property; E_property = E_property->next()) {
							String property = E_property->get()->name;
							Variant value = tile_set_atlas_source->get(property);
							if (value.get_type() != Variant::NIL) {
								undo_redo->add_undo_method(tile_set_atlas_source, "set", E_property->get()->name, value);
							}
						}
					}
				}
			}

			// Remove alternatives
			for (Set<TileSelection>::Element *E = selection.front(); E; E = E->next()) {
				TileSelection selected = E->get();
				if (selected.alternative > 0 && !removed.has(selected.tile)) {
					// Remove an alternative tile.
					undo_redo->add_do_method(tile_set_atlas_source, "remove_alternative_tile", selected.tile, selected.alternative);
					undo_redo->add_undo_method(tile_set_atlas_source, "create_alternative_tile", selected.tile, selected.alternative);
					if (per_tile.has(selected.tile)) {
						for (List<const PropertyInfo *>::Element *E_property = per_tile[selected.tile].front(); E_property; E_property = E_property->next()) {
							Vector<String> components = E_property->get()->name.split("/", true, 2);
							if (components.size() >= 2 && components[1].is_valid_integer() && components[1].to_int() == selected.alternative) {
								String property = E_property->get()->name;
								Variant value = tile_set_atlas_source->get(property);
								if (value.get_type() != Variant::NIL) {
									undo_redo->add_undo_method(tile_set_atlas_source, "set", E_property->get()->name, value);
								}
							}
						}
					}
				}
			}
			undo_redo->commit_action();
			_update_fix_selected_and_hovered_tiles();
			_update_tile_id_label();
		} break;
		case TILE_CREATE: {
			undo_redo->create_action(TTR("Create a tile"));
			undo_redo->add_do_method(tile_set_atlas_source, "create_tile", menu_option_coords);
			Array array;
			array.push_back(menu_option_coords);
			array.push_back(0);
			undo_redo->add_do_method(this, "_set_selection_from_array", array);
			undo_redo->add_undo_method(tile_set_atlas_source, "remove_tile", menu_option_coords);
			undo_redo->add_undo_method(this, "_set_selection_from_array", _get_selection_as_array());
			undo_redo->commit_action();
			_update_tile_id_label();
		} break;
		case TILE_CREATE_ALTERNATIVE: {
			undo_redo->create_action(TTR("Create tile alternatives"));
			Array array;
			for (Set<TileSelection>::Element *E = selection.front(); E; E = E->next()) {
				if (E->get().alternative == 0) {
					int next_id = tile_set_atlas_source->get_next_alternative_tile_id(E->get().tile);
					undo_redo->add_do_method(tile_set_atlas_source, "create_alternative_tile", E->get().tile, next_id);
					array.push_back(E->get().tile);
					array.push_back(next_id);
					undo_redo->add_undo_method(tile_set_atlas_source, "remove_alternative_tile", E->get().tile, next_id);
				}
			}
			undo_redo->add_do_method(this, "_set_selection_from_array", array);
			undo_redo->add_undo_method(this, "_set_selection_from_array", _get_selection_as_array());
			undo_redo->commit_action();
			_update_tile_id_label();
		} break;
		case ADVANCED_CLEANUP_TILES_OUTSIDE_TEXTURE: {
			tile_set_atlas_source->clear_tiles_outside_texture();
		} break;
		case ADVANCED_AUTO_CREATE_TILES: {
			_auto_create_tiles();
		} break;
		case ADVANCED_AUTO_REMOVE_TILES: {
			_auto_remove_tiles();
		} break;
	}
}

void TileSetAtlasSourceEditor::_unhandled_key_input(const Ref<InputEvent> &p_event) {
	// Check for shortcuts.
	if (ED_IS_SHORTCUT("tiles_editor/delete_tile", p_event)) {
		if (tools_button_group->get_pressed_button() == tool_select_button && !selection.is_empty()) {
			_menu_option(TILE_DELETE);
			accept_event();
		}
	}
}

void TileSetAtlasSourceEditor::_set_selection_from_array(Array p_selection) {
	ERR_FAIL_COND((p_selection.size() % 2) != 0);
	selection.clear();
	for (int i = 0; i < p_selection.size() / 2; i++) {
		TileSelection selected = { p_selection[i * 2], p_selection[i * 2 + 1] };
		if (tile_set_atlas_source->has_tile(selected.tile) && tile_set_atlas_source->has_alternative_tile(selected.tile, selected.alternative)) {
			selection.insert(selected);
		}
	}
	_update_tile_inspector();
	_update_tile_id_label();
	_update_atlas_view();
}

Array TileSetAtlasSourceEditor::_get_selection_as_array() {
	Array output;
	for (Set<TileSelection>::Element *E = selection.front(); E; E = E->next()) {
		output.push_back(E->get().tile);
		output.push_back(E->get().alternative);
	}
	return output;
}

void TileSetAtlasSourceEditor::_tile_atlas_control_draw() {
	// Draw the selected tile.
	if (tools_button_group->get_pressed_button() == tool_select_button) {
		for (Set<TileSelection>::Element *E = selection.front(); E; E = E->next()) {
			TileSelection selected = E->get();
			if (selected.alternative == 0) {
				// Draw the rect.
				Rect2 region = tile_set_atlas_source->get_tile_texture_region(selected.tile);
				tile_atlas_control->draw_rect(region, Color(0.2, 0.2, 1.0), false);
			}
		}

		if (selection.size() == 1) {
			// Draw the resize handles (only when it's possible to expand).
			TileSelection selected = selection.front()->get();
			Vector2i size_in_atlas = tile_set_atlas_source->get_tile_size_in_atlas(selected.tile);
			Size2 zoomed_size = resize_handle->get_size() / tile_atlas_view->get_zoom();
			Rect2 region = tile_set_atlas_source->get_tile_texture_region(selected.tile);
			Rect2 rect = region.grow_individual(zoomed_size.x, zoomed_size.y, 0, 0);
			Vector2i coords[] = { Vector2i(0, 0), Vector2i(1, 0), Vector2i(1, 1), Vector2i(0, 1) };
			Vector2i directions[] = { Vector2i(0, -1), Vector2i(1, 0), Vector2i(0, 1), Vector2i(-1, 0) };
			bool can_grow[4];
			for (int i = 0; i < 4; i++) {
				can_grow[i] = tile_set_atlas_source->can_move_tile_in_atlas(selected.tile, selected.tile + directions[i]);
				can_grow[i] |= (i % 2 == 0) ? size_in_atlas.y > 1 : size_in_atlas.x > 1;
			}
			for (int i = 0; i < 4; i++) {
				Vector2 pos = rect.position + Vector2(rect.size.x, rect.size.y) * coords[i];
				if (can_grow[i] && can_grow[(i + 3) % 4]) {
					tile_atlas_control->draw_texture_rect(resize_handle, Rect2(pos, zoomed_size), false);
				} else {
					tile_atlas_control->draw_texture_rect(resize_handle_disabled, Rect2(pos, zoomed_size), false);
				}
				Vector2 next_pos = rect.position + Vector2(rect.size.x, rect.size.y) * coords[(i + 1) % 4];
				if (can_grow[i]) {
					tile_atlas_control->draw_texture_rect(resize_handle, Rect2((pos + next_pos) / 2.0, zoomed_size), false);
				} else {
					tile_atlas_control->draw_texture_rect(resize_handle_disabled, Rect2((pos + next_pos) / 2.0, zoomed_size), false);
				}
			}
		}
	}

	if (drag_type == DRAG_TYPE_REMOVE_TILES) {
		// Draw the tiles to be removed.
		for (Set<Vector2i>::Element *E = drag_modified_tiles.front(); E; E = E->next()) {
			tile_atlas_control->draw_rect(tile_set_atlas_source->get_tile_texture_region(E->get()), Color(0.0, 0.0, 0.0), false);
		}
	} else if (drag_type == DRAG_TYPE_RECT_SELECT || drag_type == DRAG_TYPE_REMOVE_TILES_USING_RECT) {
		// Draw tiles to be removed.
		Vector2i start_base_tiles_coords = tile_atlas_view->get_atlas_tile_coords_at_pos(drag_start_mouse_pos);
		Vector2i new_base_tiles_coords = tile_atlas_view->get_atlas_tile_coords_at_pos(tile_atlas_control->get_local_mouse_position());
		Rect2i area = Rect2i(start_base_tiles_coords, new_base_tiles_coords - start_base_tiles_coords).abs();
		area.set_end((area.get_end() + Vector2i(1, 1)).min(tile_set_atlas_source->get_atlas_grid_size()));

		Color color = Color(0.0, 0.0, 0.0);
		if (drag_type == DRAG_TYPE_RECT_SELECT) {
			color = Color(0.5, 0.5, 1.0);
		}

		Set<Vector2i> to_paint;
		for (int x = area.get_position().x; x < area.get_end().x; x++) {
			for (int y = area.get_position().y; y < area.get_end().y; y++) {
				Vector2i coords = tile_set_atlas_source->get_tile_at_coords(Vector2i(x, y));
				if (coords != TileSetAtlasSource::INVALID_ATLAS_COORDS) {
					to_paint.insert(coords);
				}
			}
		}

		for (Set<Vector2i>::Element *E = to_paint.front(); E; E = E->next()) {
			Vector2i coords = E->get();
			tile_atlas_control->draw_rect(tile_set_atlas_source->get_tile_texture_region(coords), color, false);
		}
	} else if (drag_type == DRAG_TYPE_CREATE_TILES_USING_RECT) {
		// Draw tiles to be created.
		Vector2i margins = tile_set_atlas_source->get_margins();
		Vector2i separation = tile_set_atlas_source->get_separation();
		Vector2i tile_size = tile_set_atlas_source->get_texture_region_size();

		Vector2i start_base_tiles_coords = tile_atlas_view->get_atlas_tile_coords_at_pos(drag_start_mouse_pos);
		Vector2i new_base_tiles_coords = tile_atlas_view->get_atlas_tile_coords_at_pos(tile_atlas_control->get_local_mouse_position());
		Rect2i area = Rect2i(start_base_tiles_coords, new_base_tiles_coords - start_base_tiles_coords).abs();
		area.set_end((area.get_end() + Vector2i(1, 1)).min(tile_set_atlas_source->get_atlas_grid_size()));
		for (int x = area.get_position().x; x < area.get_end().x; x++) {
			for (int y = area.get_position().y; y < area.get_end().y; y++) {
				Vector2i coords = Vector2i(x, y);
				if (tile_set_atlas_source->get_tile_at_coords(coords) == TileSetAtlasSource::INVALID_ATLAS_COORDS) {
					Vector2i origin = margins + (coords * (tile_size + separation));
					tile_atlas_control->draw_rect(Rect2i(origin, tile_size), Color(1.0, 1.0, 1.0), false);
				}
			}
		}
	}

	// Draw the hovered tile.
	if (drag_type == DRAG_TYPE_REMOVE_TILES_USING_RECT || drag_type == DRAG_TYPE_CREATE_TILES_USING_RECT) {
		// Draw the rect.
		Vector2i start_base_tiles_coords = tile_atlas_view->get_atlas_tile_coords_at_pos(drag_start_mouse_pos);
		Vector2i new_base_tiles_coords = tile_atlas_view->get_atlas_tile_coords_at_pos(tile_atlas_control->get_local_mouse_position());
		Rect2i area = Rect2i(start_base_tiles_coords, new_base_tiles_coords - start_base_tiles_coords).abs();
		area.set_end((area.get_end() + Vector2i(1, 1)).min(tile_set_atlas_source->get_atlas_grid_size()));
		Vector2i margins = tile_set_atlas_source->get_margins();
		Vector2i separation = tile_set_atlas_source->get_separation();
		Vector2i tile_size = tile_set_atlas_source->get_texture_region_size();
		Vector2i origin = margins + (area.position * (tile_size + separation));
		tile_atlas_control->draw_rect(Rect2i(origin, area.size * tile_size), Color(1.0, 1.0, 1.0), false);
	} else {
		Vector2i grid_size = tile_set_atlas_source->get_atlas_grid_size();
		if (hovered_base_tile_coords.x >= 0 && hovered_base_tile_coords.y >= 0 && hovered_base_tile_coords.x < grid_size.x && hovered_base_tile_coords.y < grid_size.y) {
			Vector2i hovered_tile = tile_set_atlas_source->get_tile_at_coords(hovered_base_tile_coords);
			if (hovered_tile != TileSetAtlasSource::INVALID_ATLAS_COORDS) {
				// Draw existing hovered tile.
				tile_atlas_control->draw_rect(tile_set_atlas_source->get_tile_texture_region(hovered_tile), Color(1.0, 1.0, 1.0), false);
			} else {
				// Draw empty tile, only in add/remove tiles mode.
				if (tools_button_group->get_pressed_button() == tool_add_remove_button || tools_button_group->get_pressed_button() == tool_add_remove_rect_button) {
					Vector2i margins = tile_set_atlas_source->get_margins();
					Vector2i separation = tile_set_atlas_source->get_separation();
					Vector2i tile_size = tile_set_atlas_source->get_texture_region_size();
					Vector2i origin = margins + (hovered_base_tile_coords * (tile_size + separation));
					tile_atlas_control->draw_rect(Rect2i(origin, tile_size), Color(1.0, 1.0, 1.0), false);
				}
			}
		}
	}
}

void TileSetAtlasSourceEditor::_tile_atlas_control_unscaled_draw() {
	// Draw the preview of the selected property.
	TileDataEditor *tile_data_editor = TileSetEditor::get_singleton()->get_tile_data_editor(selected_property);
	if (tile_data_editor && tile_inspector->is_visible_in_tree()) {
		for (int i = 0; i < tile_set_atlas_source->get_tiles_count(); i++) {
			Vector2i coords = tile_set_atlas_source->get_tile_id(i);
			Rect2i texture_region = tile_set_atlas_source->get_tile_texture_region(coords);
			Vector2i position = (texture_region.position + texture_region.get_end()) / 2 + tile_set_atlas_source->get_tile_effective_texture_offset(coords, 0);

			Transform2D xform = tile_atlas_control->get_parent_control()->get_transform();
			xform.translate(position);

			tile_data_editor->draw_over_tile(tile_atlas_control_unscaled, xform, *tile_set, tile_set_atlas_source_id, coords, 0, selected_property);
		}
	}
}

void TileSetAtlasSourceEditor::_tile_alternatives_control_gui_input(const Ref<InputEvent> &p_event) {
	// Update the hovered alternative tile.
	hovered_alternative_tile_coords = tile_atlas_view->get_alternative_tile_at_pos(alternative_tiles_control->get_local_mouse_position());

	Ref<InputEventMouseMotion> mm = p_event;
	if (mm.is_valid()) {
		tile_atlas_control->update();
		tile_atlas_control_unscaled->update();
		alternative_tiles_control->update();
		alternative_tiles_control_unscaled->update();
	}

	Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid()) {
		drag_type = DRAG_TYPE_NONE;

		Vector2 mouse_local_pos = alternative_tiles_control->get_local_mouse_position();
		if (mb->get_button_index() == MOUSE_BUTTON_LEFT) {
			if (mb->is_pressed()) {
				// Left click pressed.
				if (tools_button_group->get_pressed_button() == tool_select_button) {
					Vector3 tile = tile_atlas_view->get_alternative_tile_at_pos(mouse_local_pos);

					selection.clear();
					TileSelection selected = { Vector2i(tile.x, tile.y), int(tile.z) };
					if (selected.tile != TileSetAtlasSource::INVALID_ATLAS_COORDS) {
						selection.insert(selected);
					}

					_update_tile_inspector();
					_update_tile_id_label();
				}
			}
		} else if (mb->get_button_index() == MOUSE_BUTTON_RIGHT) {
			if (mb->is_pressed()) {
				// Right click pressed
				Vector3 tile = tile_atlas_view->get_alternative_tile_at_pos(mouse_local_pos);

				selection.clear();
				TileSelection selected = { Vector2i(tile.x, tile.y), int(tile.z) };
				if (selected.tile != TileSetAtlasSource::INVALID_ATLAS_COORDS) {
					selection.insert(selected);
				}

				_update_tile_inspector();
				_update_tile_id_label();

				if (selection.size() == 1) {
					selected = selection.front()->get();
					menu_option_coords = selected.tile;
					menu_option_alternative = selected.alternative;
					alternative_tile_popup_menu->popup(Rect2i(get_global_mouse_position(), Size2i()));
				}
			}
		}
		tile_atlas_control->update();
		tile_atlas_control_unscaled->update();
		alternative_tiles_control->update();
		alternative_tiles_control_unscaled->update();
	}
}

void TileSetAtlasSourceEditor::_tile_alternatives_control_mouse_exited() {
	hovered_alternative_tile_coords = Vector3i(TileSetAtlasSource::INVALID_ATLAS_COORDS.x, TileSetAtlasSource::INVALID_ATLAS_COORDS.y, TileSetAtlasSource::INVALID_TILE_ALTERNATIVE);
	tile_atlas_control->update();
	tile_atlas_control_unscaled->update();
	alternative_tiles_control->update();
	alternative_tiles_control_unscaled->update();
}

void TileSetAtlasSourceEditor::_tile_alternatives_control_draw() {
	// Update the hovered alternative tile.
	if (tools_button_group->get_pressed_button() == tool_select_button) {
		// Draw hovered tile.
		Vector2i coords = Vector2(hovered_alternative_tile_coords.x, hovered_alternative_tile_coords.y);
		if (coords != TileSetAtlasSource::INVALID_ATLAS_COORDS) {
			Rect2i rect = tile_atlas_view->get_alternative_tile_rect(coords, hovered_alternative_tile_coords.z);
			if (rect != Rect2i()) {
				alternative_tiles_control->draw_rect(rect, Color(1.0, 1.0, 1.0), false);
			}
		}

		// Draw selected tile.
		for (Set<TileSelection>::Element *E = selection.front(); E; E = E->next()) {
			TileSelection selected = E->get();
			if (selected.alternative >= 1) {
				Rect2i rect = tile_atlas_view->get_alternative_tile_rect(selected.tile, selected.alternative);
				if (rect != Rect2i()) {
					alternative_tiles_control->draw_rect(rect, Color(0.2, 0.2, 1.0), false);
				}
			}
		}
	}
}

void TileSetAtlasSourceEditor::_tile_alternatives_control_unscaled_draw() {
	//TODO
}

void TileSetAtlasSourceEditor::_tile_set_atlas_source_changed() {
	tile_set_atlas_source_changed_needs_update = true;
}

void TileSetAtlasSourceEditor::_atlas_source_proxy_object_changed(String p_what) {
	if (p_what == "texture" && !atlas_source_proxy_object->get("texture").is_null()) {
		confirm_auto_create_tiles->popup_centered();
	} else if (p_what == "id") {
		emit_signal("source_id_changed", atlas_source_proxy_object->get_id());
	}
}

void TileSetAtlasSourceEditor::_undo_redo_inspector_callback(Object *p_undo_redo, Object *p_edited, String p_property, Variant p_new_value) {
	UndoRedo *undo_redo = Object::cast_to<UndoRedo>(p_undo_redo);
	ERR_FAIL_COND(!undo_redo);

#define ADD_UNDO(obj, property) undo_redo->add_undo_property(obj, property, tile_data->get(property));

	TileProxyObject *tile_data = Object::cast_to<TileProxyObject>(p_edited);
	if (tile_data) {
		Vector<String> components = String(p_property).split("/", true, 2);
		if (components.size() == 2 && components[1] == "shapes_count") {
			int layer_index = components[0].trim_prefix("physics_layer_").to_int();
			int new_shapes_count = p_new_value;
			int old_shapes_count = tile_data->get(vformat("physics_layer_%d/shapes_count", layer_index));
			if (new_shapes_count < old_shapes_count) {
				for (int i = new_shapes_count - 1; i < old_shapes_count; i++) {
					ADD_UNDO(tile_data, vformat("physics_layer_%d/shape_%d/shape", layer_index, i));
					ADD_UNDO(tile_data, vformat("physics_layer_%d/shape_%d/one_way", layer_index, i));
					ADD_UNDO(tile_data, vformat("physics_layer_%d/shape_%d/one_way_margin", layer_index, i));
				}
			}
		}
	}
#undef ADD_UNDO
}

void TileSetAtlasSourceEditor::edit(Ref<TileSet> p_tile_set, TileSetAtlasSource *p_tile_set_atlas_source, int p_source_id) {
	ERR_FAIL_COND(!p_tile_set.is_valid());
	ERR_FAIL_COND(!p_tile_set_atlas_source);
	ERR_FAIL_COND(p_source_id < 0);
	ERR_FAIL_COND(p_tile_set->get_source(p_source_id) != p_tile_set_atlas_source);

	if (p_tile_set == tile_set && p_tile_set_atlas_source == tile_set_atlas_source && p_source_id == tile_set_atlas_source_id) {
		return;
	}

	// Remove listener for old objects.
	if (tile_set_atlas_source) {
		tile_set_atlas_source->disconnect("changed", callable_mp(this, &TileSetAtlasSourceEditor::_tile_set_atlas_source_changed));
	}

	// Clear the selection.
	selection.clear();

	// Change the edited object.
	tile_set = p_tile_set;
	tile_set_atlas_source = p_tile_set_atlas_source;
	tile_set_atlas_source_id = p_source_id;

	// Add the listener again.
	if (tile_set_atlas_source) {
		tile_set_atlas_source->connect("changed", callable_mp(this, &TileSetAtlasSourceEditor::_tile_set_atlas_source_changed));
	}

	// Update everything.
	_update_source_inspector();

	// Update the selected tile.
	_update_fix_selected_and_hovered_tiles();
	_update_tile_id_label();
	_update_atlas_view();
	_update_tile_inspector();
}

void TileSetAtlasSourceEditor::init_source() {
	confirm_auto_create_tiles->popup_centered();
}

void TileSetAtlasSourceEditor::_auto_create_tiles() {
	if (!tile_set_atlas_source) {
		return;
	}

	Ref<Texture2D> texture = tile_set_atlas_source->get_texture();
	if (texture.is_valid()) {
		Vector2i margins = tile_set_atlas_source->get_margins();
		Vector2i separation = tile_set_atlas_source->get_separation();
		Vector2i texture_region_size = tile_set_atlas_source->get_texture_region_size();
		Size2i grid_size = tile_set_atlas_source->get_atlas_grid_size();
		undo_redo->create_action(TTR("Create tiles in non-transparent texture regions"));
		for (int y = 0; y < grid_size.y; y++) {
			for (int x = 0; x < grid_size.x; x++) {
				// Check if we have a tile at the coord
				Vector2i coords = Vector2i(x, y);
				if (tile_set_atlas_source->get_tile_at_coords(coords) == TileSetAtlasSource::INVALID_ATLAS_COORDS) {
					// Check if the texture is empty at the given coords.
					Rect2i region = Rect2i(margins + (coords * (texture_region_size + separation)), texture_region_size);
					bool is_opaque = false;
					for (int region_x = region.get_position().x; region_x < region.get_end().x; region_x++) {
						for (int region_y = region.get_position().y; region_y < region.get_end().y; region_y++) {
							if (texture->is_pixel_opaque(region_x, region_y)) {
								is_opaque = true;
								break;
							}
						}
						if (is_opaque) {
							break;
						}
					}

					// If we do have opaque pixels, create a tile.
					if (is_opaque) {
						undo_redo->add_do_method(tile_set_atlas_source, "create_tile", coords);
						undo_redo->add_undo_method(tile_set_atlas_source, "remove_tile", coords);
					}
				}
			}
		}
		undo_redo->commit_action();
	}
}

void TileSetAtlasSourceEditor::_auto_remove_tiles() {
	if (!tile_set_atlas_source) {
		return;
	}

	Ref<Texture2D> texture = tile_set_atlas_source->get_texture();
	if (texture.is_valid()) {
		Vector2i margins = tile_set_atlas_source->get_margins();
		Vector2i separation = tile_set_atlas_source->get_separation();
		Vector2i texture_region_size = tile_set_atlas_source->get_texture_region_size();
		Vector2i grid_size = tile_set_atlas_source->get_atlas_grid_size();

		undo_redo->create_action(TTR("Remove tiles in fully transparent texture regions"));

		List<PropertyInfo> list;
		tile_set_atlas_source->get_property_list(&list);
		Map<Vector2i, List<const PropertyInfo *>> per_tile = _group_properties_per_tiles(list, tile_set_atlas_source);

		for (int i = 0; i < tile_set_atlas_source->get_tiles_count(); i++) {
			Vector2i coords = tile_set_atlas_source->get_tile_id(i);
			Vector2i size_in_atlas = tile_set_atlas_source->get_tile_size_in_atlas(coords);

			// Skip tiles outside texture.
			if ((coords.x + size_in_atlas.x) > grid_size.x || (coords.y + size_in_atlas.y) > grid_size.y) {
				continue;
			}

			// Check if the texture is empty at the given coords.
			Rect2i region = Rect2i(margins + (coords * (texture_region_size + separation)), texture_region_size * size_in_atlas);
			bool is_opaque = false;
			for (int region_x = region.get_position().x; region_x < region.get_end().x; region_x++) {
				for (int region_y = region.get_position().y; region_y < region.get_end().y; region_y++) {
					if (texture->is_pixel_opaque(region_x, region_y)) {
						is_opaque = true;
						break;
					}
				}
				if (is_opaque) {
					break;
				}
			}

			// If we do have opaque pixels, create a tile.
			if (!is_opaque) {
				undo_redo->add_do_method(tile_set_atlas_source, "remove_tile", coords);
				undo_redo->add_undo_method(tile_set_atlas_source, "create_tile", coords);
				if (per_tile.has(coords)) {
					for (List<const PropertyInfo *>::Element *E_property = per_tile[coords].front(); E_property; E_property = E_property->next()) {
						String property = E_property->get()->name;
						Variant value = tile_set_atlas_source->get(property);
						if (value.get_type() != Variant::NIL) {
							undo_redo->add_undo_method(tile_set_atlas_source, "set", E_property->get()->name, value);
						}
					}
				}
			}
		}
		undo_redo->commit_action();
	}
}

void TileSetAtlasSourceEditor::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE:
		case NOTIFICATION_THEME_CHANGED:
			tool_select_button->set_icon(get_theme_icon("ToolSelect", "EditorIcons"));
			tool_add_remove_button->set_icon(get_theme_icon("EditAddRemove", "EditorIcons"));
			tool_add_remove_rect_button->set_icon(get_theme_icon("RectangleAddRemove", "EditorIcons"));

			tools_settings_erase_button->set_icon(get_theme_icon("Eraser", "EditorIcons"));

			tool_advanced_menu_buttom->set_icon(get_theme_icon("Tools", "EditorIcons"));

			resize_handle = get_theme_icon("EditorHandle", "EditorIcons");
			resize_handle_disabled = get_theme_icon("EditorHandleDisabled", "EditorIcons");
			break;
		case NOTIFICATION_INTERNAL_PROCESS:
			if (tile_set_atlas_source_changed_needs_update) {
				// Update everything.
				_update_source_inspector();

				// Update the selected tile.
				_update_fix_selected_and_hovered_tiles();
				_update_tile_id_label();
				_update_atlas_view();
				_update_tile_inspector();

				tile_set_atlas_source_changed_needs_update = false;
			}
			break;
		default:
			break;
	}
}

void TileSetAtlasSourceEditor::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_unhandled_key_input"), &TileSetAtlasSourceEditor::_unhandled_key_input);
	ClassDB::bind_method(D_METHOD("_set_selection_from_array"), &TileSetAtlasSourceEditor::_set_selection_from_array);

	ADD_SIGNAL(MethodInfo("source_id_changed", PropertyInfo(Variant::INT, "source_id")));
}

TileSetAtlasSourceEditor::TileSetAtlasSourceEditor() {
	set_process_unhandled_key_input(true);
	set_process_internal(true);

	// -- Right side --
	HSplitContainer *split_container_right_side = memnew(HSplitContainer);
	split_container_right_side->set_h_size_flags(SIZE_EXPAND_FILL);
	add_child(split_container_right_side);

	// Middle panel.
	ScrollContainer *middle_panel = memnew(ScrollContainer);
	middle_panel->set_enable_h_scroll(false);
	middle_panel->set_custom_minimum_size(Size2i(200, 0) * EDSCALE);
	split_container_right_side->add_child(middle_panel);

	VBoxContainer *middle_vbox_container = memnew(VBoxContainer);
	middle_vbox_container->set_h_size_flags(SIZE_EXPAND_FILL);
	middle_panel->add_child(middle_vbox_container);

	// Tile inspector.
	tile_inspector_label = memnew(Label);
	tile_inspector_label->set_text(TTR("Tile Properties:"));
	tile_inspector_label->hide();
	middle_vbox_container->add_child(tile_inspector_label);

	tile_proxy_object = memnew(TileProxyObject(this));
	tile_proxy_object->connect("changed", callable_mp(this, &TileSetAtlasSourceEditor::_update_atlas_view).unbind(1));

	tile_inspector = memnew(EditorInspector);
	tile_inspector->set_undo_redo(undo_redo);
	tile_inspector->set_enable_v_scroll(false);
	tile_inspector->edit(tile_proxy_object);
	tile_inspector->set_use_folding(true);
	tile_inspector->connect("property_selected", callable_mp(this, &TileSetAtlasSourceEditor::_inspector_property_selected));
	middle_vbox_container->add_child(tile_inspector);

	// Atlas source inspector.
	atlas_source_inspector_label = memnew(Label);
	atlas_source_inspector_label->set_text(TTR("Atlas Properties:"));
	middle_vbox_container->add_child(atlas_source_inspector_label);

	atlas_source_proxy_object = memnew(TileSetAtlasSourceProxyObject());
	atlas_source_proxy_object->connect("changed", callable_mp(this, &TileSetAtlasSourceEditor::_atlas_source_proxy_object_changed));

	atlas_source_inspector = memnew(EditorInspector);
	atlas_source_inspector->set_undo_redo(undo_redo);
	atlas_source_inspector->set_enable_v_scroll(false);
	atlas_source_inspector->edit(atlas_source_proxy_object);
	middle_vbox_container->add_child(atlas_source_inspector);

	// Right panel.
	VBoxContainer *right_panel = memnew(VBoxContainer);
	right_panel->set_h_size_flags(SIZE_EXPAND_FILL);
	right_panel->set_v_size_flags(SIZE_EXPAND_FILL);
	split_container_right_side->add_child(right_panel);

	// -- Dialogs --
	confirm_auto_create_tiles = memnew(AcceptDialog);
	confirm_auto_create_tiles->set_title(TTR("Create tiles automatically in non-transparent texture regions?"));
	confirm_auto_create_tiles->set_text(TTR("The atlas's texture was modified.\nWould you like to automatically create tiles in the atlas?"));
	confirm_auto_create_tiles->get_ok_button()->set_text(TTR("Yes"));
	confirm_auto_create_tiles->add_cancel_button()->set_text(TTR("No"));
	confirm_auto_create_tiles->connect("confirmed", callable_mp(this, &TileSetAtlasSourceEditor::_auto_create_tiles));
	add_child(confirm_auto_create_tiles);

	// -- Toolbox --
	tools_button_group.instance();

	toolbox = memnew(HBoxContainer);
	right_panel->add_child(toolbox);

	tool_select_button = memnew(Button);
	tool_select_button->set_flat(true);
	tool_select_button->set_toggle_mode(true);
	tool_select_button->set_pressed(true);
	tool_select_button->set_button_group(tools_button_group);
	tool_select_button->set_tooltip(TTR("Select tiles."));
	tool_select_button->connect("pressed", callable_mp(this, &TileSetAtlasSourceEditor::_update_fix_selected_and_hovered_tiles));
	tool_select_button->connect("pressed", callable_mp(this, &TileSetAtlasSourceEditor::_update_tile_id_label));
	tool_select_button->connect("pressed", callable_mp(this, &TileSetAtlasSourceEditor::_update_tile_inspector));
	tool_select_button->connect("pressed", callable_mp(this, &TileSetAtlasSourceEditor::_update_atlas_view));
	tool_select_button->connect("pressed", callable_mp(this, &TileSetAtlasSourceEditor::_update_toolbar));
	toolbox->add_child(tool_select_button);

	tool_add_remove_button = memnew(Button);
	tool_add_remove_button->set_flat(true);
	tool_add_remove_button->set_toggle_mode(true);
	tool_add_remove_button->set_button_group(tools_button_group);
	tool_add_remove_button->set_tooltip(TTR("Add/Remove tiles tool (use the shift key to create big tiles)."));
	tool_add_remove_button->connect("pressed", callable_mp(this, &TileSetAtlasSourceEditor::_update_fix_selected_and_hovered_tiles));
	tool_add_remove_button->connect("pressed", callable_mp(this, &TileSetAtlasSourceEditor::_update_tile_id_label));
	tool_add_remove_button->connect("pressed", callable_mp(this, &TileSetAtlasSourceEditor::_update_tile_inspector));
	tool_add_remove_button->connect("pressed", callable_mp(this, &TileSetAtlasSourceEditor::_update_atlas_view));
	tool_add_remove_button->connect("pressed", callable_mp(this, &TileSetAtlasSourceEditor::_update_toolbar));
	toolbox->add_child(tool_add_remove_button);

	tool_add_remove_rect_button = memnew(Button);
	tool_add_remove_rect_button->set_flat(true);
	tool_add_remove_rect_button->set_toggle_mode(true);
	tool_add_remove_rect_button->set_button_group(tools_button_group);
	tool_add_remove_rect_button->set_tooltip(TTR("Add/Remove tiles rectangle tool (use the shift key to create big tiles)."));
	tool_add_remove_rect_button->connect("pressed", callable_mp(this, &TileSetAtlasSourceEditor::_update_fix_selected_and_hovered_tiles));
	tool_add_remove_rect_button->connect("pressed", callable_mp(this, &TileSetAtlasSourceEditor::_update_tile_id_label));
	tool_add_remove_rect_button->connect("pressed", callable_mp(this, &TileSetAtlasSourceEditor::_update_tile_inspector));
	tool_add_remove_rect_button->connect("pressed", callable_mp(this, &TileSetAtlasSourceEditor::_update_atlas_view));
	tool_add_remove_rect_button->connect("pressed", callable_mp(this, &TileSetAtlasSourceEditor::_update_toolbar));
	toolbox->add_child(tool_add_remove_rect_button);

	// Tool settings.
	tool_settings = memnew(HBoxContainer);
	toolbox->add_child(tool_settings);

	tool_settings_vsep = memnew(VSeparator);
	tool_settings->add_child(tool_settings_vsep);

	tools_settings_erase_button = memnew(Button);
	tools_settings_erase_button->set_flat(true);
	tools_settings_erase_button->set_toggle_mode(true);
	tools_settings_erase_button->set_shortcut(ED_SHORTCUT("tiles_editor/eraser", "Eraser", KEY_E));
	tools_settings_erase_button->set_shortcut_context(this);
	tool_settings->add_child(tools_settings_erase_button);

	VSeparator *tool_advanced_vsep = memnew(VSeparator);
	toolbox->add_child(tool_advanced_vsep);

	tool_advanced_menu_buttom = memnew(MenuButton);
	tool_advanced_menu_buttom->set_flat(true);
	tool_advanced_menu_buttom->get_popup()->add_item(TTR("Cleanup Tiles Outside Texture"), ADVANCED_CLEANUP_TILES_OUTSIDE_TEXTURE);
	tool_advanced_menu_buttom->get_popup()->set_item_disabled(0, true);
	tool_advanced_menu_buttom->get_popup()->add_item(TTR("Create Tiles in Non-Transparent Texture Regions"), ADVANCED_AUTO_CREATE_TILES);
	tool_advanced_menu_buttom->get_popup()->add_item(TTR("Remove Tiles in Fully Transparent Texture Regions"), ADVANCED_AUTO_REMOVE_TILES);
	tool_advanced_menu_buttom->get_popup()->connect("id_pressed", callable_mp(this, &TileSetAtlasSourceEditor::_menu_option));
	toolbox->add_child(tool_advanced_menu_buttom);

	_update_toolbar();

	// Right side of toolbar.
	Control *middle_space = memnew(Control);
	middle_space->set_h_size_flags(SIZE_EXPAND_FILL);
	toolbox->add_child(middle_space);

	tool_tile_id_label = memnew(Label);
	tool_tile_id_label->set_mouse_filter(Control::MOUSE_FILTER_STOP);
	toolbox->add_child(tool_tile_id_label);
	_update_tile_id_label();

	// Tile atlas view.
	tile_atlas_view = memnew(TileAtlasView);
	tile_atlas_view->set_h_size_flags(SIZE_EXPAND_FILL);
	tile_atlas_view->set_v_size_flags(SIZE_EXPAND_FILL);
	tile_atlas_view->connect("transform_changed", callable_mp(TilesEditor::get_singleton(), &TilesEditor::set_atlas_view_transform));
	tile_atlas_view->connect("transform_changed", callable_mp(this, &TileSetAtlasSourceEditor::_tile_atlas_view_transform_changed).unbind(2));
	right_panel->add_child(tile_atlas_view);

	base_tile_popup_menu = memnew(PopupMenu);
	base_tile_popup_menu->add_shortcut(ED_SHORTCUT("tiles_editor/delete", TTR("Delete"), KEY_DELETE), TILE_DELETE);
	base_tile_popup_menu->add_item(TTR("Create an Alternative Tile"), TILE_CREATE_ALTERNATIVE);
	base_tile_popup_menu->connect("id_pressed", callable_mp(this, &TileSetAtlasSourceEditor::_menu_option));
	tile_atlas_view->add_child(base_tile_popup_menu);

	empty_base_tile_popup_menu = memnew(PopupMenu);
	empty_base_tile_popup_menu->add_item(TTR("Create a Tile"), TILE_CREATE);
	empty_base_tile_popup_menu->connect("id_pressed", callable_mp(this, &TileSetAtlasSourceEditor::_menu_option));
	tile_atlas_view->add_child(empty_base_tile_popup_menu);

	tile_atlas_control = memnew(Control);
	tile_atlas_control->connect("draw", callable_mp(this, &TileSetAtlasSourceEditor::_tile_atlas_control_draw));
	tile_atlas_control->connect("mouse_exited", callable_mp(this, &TileSetAtlasSourceEditor::_tile_atlas_control_mouse_exited));
	tile_atlas_control->connect("gui_input", callable_mp(this, &TileSetAtlasSourceEditor::_tile_atlas_control_gui_input));
	tile_atlas_view->add_control_over_atlas_tiles(tile_atlas_control);

	tile_atlas_control_unscaled = memnew(Control);
	tile_atlas_control_unscaled->set_anchors_and_offsets_preset(Control::PRESET_WIDE);
	tile_atlas_control_unscaled->connect("draw", callable_mp(this, &TileSetAtlasSourceEditor::_tile_atlas_control_unscaled_draw));
	tile_atlas_view->add_control_over_atlas_tiles(tile_atlas_control_unscaled, false);
	tile_atlas_control_unscaled->set_mouse_filter(Control::MOUSE_FILTER_IGNORE);

	alternative_tile_popup_menu = memnew(PopupMenu);
	alternative_tile_popup_menu->add_shortcut(ED_SHORTCUT("tiles_editor/delete_tile", TTR("Delete"), KEY_DELETE), TILE_DELETE);
	alternative_tile_popup_menu->connect("id_pressed", callable_mp(this, &TileSetAtlasSourceEditor::_menu_option));
	tile_atlas_view->add_child(alternative_tile_popup_menu);

	alternative_tiles_control = memnew(Control);
	alternative_tiles_control->connect("draw", callable_mp(this, &TileSetAtlasSourceEditor::_tile_alternatives_control_draw));
	alternative_tiles_control->connect("mouse_exited", callable_mp(this, &TileSetAtlasSourceEditor::_tile_alternatives_control_mouse_exited));
	alternative_tiles_control->connect("gui_input", callable_mp(this, &TileSetAtlasSourceEditor::_tile_alternatives_control_gui_input));
	tile_atlas_view->add_control_over_alternative_tiles(alternative_tiles_control);

	alternative_tiles_control_unscaled = memnew(Control);
	alternative_tiles_control_unscaled->set_anchors_and_offsets_preset(Control::PRESET_WIDE);
	alternative_tiles_control_unscaled->connect("draw", callable_mp(this, &TileSetAtlasSourceEditor::_tile_alternatives_control_unscaled_draw));
	tile_atlas_view->add_control_over_atlas_tiles(alternative_tiles_control_unscaled, false);
	alternative_tiles_control_unscaled->set_mouse_filter(Control::MOUSE_FILTER_IGNORE);

	tile_atlas_view_missing_source_label = memnew(Label);
	tile_atlas_view_missing_source_label->set_text(TTR("Add or select an atlas texture to the left panel."));
	tile_atlas_view_missing_source_label->set_align(Label::ALIGN_CENTER);
	tile_atlas_view_missing_source_label->set_valign(Label::VALIGN_CENTER);
	tile_atlas_view_missing_source_label->set_h_size_flags(SIZE_EXPAND_FILL);
	tile_atlas_view_missing_source_label->set_v_size_flags(SIZE_EXPAND_FILL);
	tile_atlas_view_missing_source_label->hide();
	right_panel->add_child(tile_atlas_view_missing_source_label);
}

TileSetAtlasSourceEditor::~TileSetAtlasSourceEditor() {
	memdelete(tile_proxy_object);
	memdelete(atlas_source_proxy_object);
}
