/*************************************************************************/
/*  tile_atlas_view.cpp                                                  */
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

#include "tile_atlas_view.h"

#include "scene/gui/box_container.h"
#include "scene/gui/center_container.h"
#include "scene/gui/label.h"
#include "scene/gui/texture_rect.h"

#include "core/input/input.h"
#include "core/os/keyboard.h"

void TileAtlasView::_gui_input(const Ref<InputEvent> &p_event) {
	bool ctrl = Input::get_singleton()->is_key_pressed(KEY_CONTROL);

	Ref<InputEventMouseButton> b = p_event;
	if (b.is_valid()) {
		if (ctrl && b->is_pressed() && b->get_button_index() == BUTTON_WHEEL_DOWN) {
			// Zoom out
			zoom *= 0.8;
			_update_zoom();
			accept_event();
		}

		if (ctrl && b->is_pressed() && b->get_button_index() == BUTTON_WHEEL_UP) {
			// Zoom in
			zoom /= 0.8;
			_update_zoom();
			accept_event();
		}
	}
}

Size2i TileAtlasView::_compute_base_tiles_control_size() {
	if (!tile_set || !tile_set->has_atlas_source(source_id)) {
		return Vector2();
	}

	TileAtlasSource *atlas = tile_set->get_atlas_source(source_id);

	// Update the texture.
	Vector2i size;
	Ref<Texture2D> texture = tile_set->get_atlas_source(source_id)->get_texture();
	base_tiles_texture_rect->set_texture(texture);
	if (texture.is_valid()) {
		size = texture->get_size();
	}

	// Extend the size to all existing tiles.
	Size2i grid_size = atlas->get_atlas_grid_size();
	for (int i = 0; i < atlas->get_tiles_count(); i++) {
		Vector2i tile_id = atlas->get_tile_id(i);
		grid_size = grid_size.max(tile_id + Vector2i(1, 1));
	}
	size = size.max(grid_size * (atlas->get_texture_region_size() + atlas->get_separation()) + atlas->get_margins());

	return size;
}

Size2i TileAtlasView::_compute_alternative_tiles_control_size() {
	if (!tile_set || !tile_set->has_atlas_source(source_id)) {
		return Vector2();
	}

	TileAtlasSource *atlas = tile_set->get_atlas_source(source_id);

	Vector2i size;
	for (int i = 0; i < atlas->get_tiles_count(); i++) {
		Vector2i tile_id = atlas->get_tile_id(i);
		int alternatives_count = atlas->get_alternative_tiles_count(tile_id);
		Vector2i line_size;
		Size2i texture_region_size = atlas->get_tile_texture_region(tile_id).size;
		for (int j = 1; j < alternatives_count; j++) {
			int alternative_id = atlas->get_alternative_tile_id(tile_id, j);
			bool transposed = atlas->get_tile_data(tile_id, alternative_id)->tile_get_transpose();
			line_size.x += transposed ? texture_region_size.y : texture_region_size.x;
			line_size.y = MAX(line_size.y, transposed ? texture_region_size.x : texture_region_size.y);
		}
		size.x = MAX(size.x, line_size.x);
		size.y += line_size.y;
	}

	return size;
}

void TileAtlasView::_update_zoom() {
	if (!tile_set || !tile_set->has_atlas_source(source_id)) {
		// Reinitialize everything.
		base_tiles_root_control->set_custom_minimum_size(Vector2());
		alternative_tiles_root_control->set_custom_minimum_size(Vector2());
		base_tiles_texture_rect->set_size(Vector2());
		alternative_tiles_drawing_root->set_scale(Vector2(1, 1));
		return;
	}

	// Compute the minimum sizes.
	Size2i base_tiles_control_size = _compute_base_tiles_control_size();
	base_tiles_root_control->set_custom_minimum_size(Vector2(base_tiles_control_size) * zoom);

	Size2i alternative_tiles_control_size = _compute_alternative_tiles_control_size();
	alternative_tiles_root_control->set_custom_minimum_size(Vector2(alternative_tiles_control_size) * zoom);

	// Set the texture for the base tiles.
	Ref<Texture2D> texture = tile_set->get_atlas_source(source_id)->get_texture();
	if (texture.is_valid()) {
		base_tiles_texture_rect->set_size(Vector2(texture->get_size()) * zoom);
	} else {
		base_tiles_texture_rect->set_size(Vector2());
	}

	// Set the scales.
	if (base_tiles_control_size.x > 0 && base_tiles_control_size.y > 0) {
		base_tiles_drawing_root->set_scale(Vector2(zoom, zoom));
	} else {
		base_tiles_drawing_root->set_scale(Vector2(1, 1));
	}
	if (alternative_tiles_control_size.x > 0 && alternative_tiles_control_size.y > 0) {
		alternative_tiles_drawing_root->set_scale(Vector2(zoom, zoom));
	} else {
		alternative_tiles_drawing_root->set_scale(Vector2(1, 1));
	}

	// Update the margin container's margins.
	const char *constants[] = { "margin_left", "margin_top", "margin_right", "margin_bottom" };
	for (int i = 0; i < 4; i++) {
		margin_container->add_theme_constant_override(constants[i], margin_container_paddings[i] * zoom);
	}

	// Update the backgrounds.
	background_left->update();
	background_right->update();

	emit_signal("zoom_changed");
}

void TileAtlasView::_base_tiles_root_control_gui_input(const Ref<InputEvent> &p_event) {
	if (!tile_set || !tile_set->has_atlas_source(source_id)) {
		return;
	}

	base_tiles_root_control->set_tooltip("");

	Ref<InputEventMouseMotion> mm = p_event;
	if (mm.is_valid()) {
		Transform2D xform = base_tiles_drawing_root->get_transform().affine_inverse();
		Vector2i coords = get_atlas_tile_coords_at_pos(xform.xform(mm->get_position()));
		if (coords != TileAtlasSource::INVALID_ATLAS_COORDS) {
			coords = tile_set->get_atlas_source(source_id)->get_tile_at_coords(coords);
			if (coords != TileAtlasSource::INVALID_ATLAS_COORDS) {
				base_tiles_root_control->set_tooltip(vformat(TTR("Source: %d\nAtlas coordinates: %s\nAlternative: 0"), source_id, coords));
			}
		}
	}
}

void TileAtlasView::_draw_base_tiles_texture_grid() {
	if (!tile_set || !tile_set->has_atlas_source(source_id)) {
		return;
	}

	TileAtlasSource *atlas = tile_set->get_atlas_source(source_id);

	Ref<Texture2D> texture = atlas->get_texture();
	if (texture.is_valid()) {
		Vector2i margins = atlas->get_margins();
		Vector2i separation = atlas->get_separation();
		Vector2i texture_region_size = atlas->get_texture_region_size();

		Size2i grid_size = atlas->get_atlas_grid_size();

		// Draw each tile texture region.
		for (int x = 0; x < grid_size.x; x++) {
			for (int y = 0; y < grid_size.y; y++) {
				Vector2i origin = margins + (Vector2i(x, y) * (texture_region_size + separation));
				Vector2i base_tile_coords = atlas->get_tile_at_coords(Vector2i(x, y));
				if (base_tile_coords != TileAtlasSource::INVALID_ATLAS_COORDS) {
					if (base_tile_coords == Vector2i(x, y)) {
						// Draw existing tile.
						Vector2i size_in_atlas = atlas->get_tile_size_in_atlas(base_tile_coords);
						Vector2 region_size = texture_region_size * size_in_atlas + separation * (size_in_atlas - Vector2i(1, 1));
						base_tiles_texture_grid->draw_rect(Rect2i(origin, region_size), Color(1.0, 1.0, 1.0, 0.8), false);
					}
				} else {
					// Draw the grid.
					base_tiles_texture_grid->draw_rect(Rect2i(origin, texture_region_size), Color(0.7, 0.7, 0.7, 0.1), false);
				}
			}
		}
	}
}

void TileAtlasView::_draw_base_tiles_dark() {
	if (!tile_set || !tile_set->has_atlas_source(source_id)) {
		return;
	}

	TileAtlasSource *atlas = tile_set->get_atlas_source(source_id);

	Ref<Texture2D> texture = atlas->get_texture();
	if (texture.is_valid()) {
		Vector2i margins = atlas->get_margins();
		Vector2i separation = atlas->get_separation();
		Vector2i texture_region_size = atlas->get_texture_region_size();

		Size2i grid_size = atlas->get_atlas_grid_size();

		// Draw each tile texture region.
		for (int x = 0; x < grid_size.x; x++) {
			for (int y = 0; y < grid_size.y; y++) {
				Vector2i origin = margins + (Vector2i(x, y) * (texture_region_size + separation));
				Vector2i base_tile_coords = atlas->get_tile_at_coords(Vector2i(x, y));

				if (base_tile_coords == TileAtlasSource::INVALID_ATLAS_COORDS) {
					// Draw the grid.
					base_tiles_dark->draw_rect(Rect2i(origin, texture_region_size), Color(0.0, 0.0, 0.0, 0.5), true);
				}
			}
		}
	}
}

void TileAtlasView::_draw_base_tiles_shape_grid() {
	if (!tile_set || !tile_set->has_atlas_source(source_id)) {
		return;
	}

	TileAtlasSource *atlas = tile_set->get_atlas_source(source_id);

	// Draw the shapes.
	Vector2i tile_shape_size = tile_set->get_tile_size();
	for (int i = 0; i < atlas->get_tiles_count(); i++) {
		Vector2i tile_id = atlas->get_tile_id(i);
		Vector2 in_tile_base_offset = tile_set->get_tile_effective_texture_offset(source_id, tile_id, 0);
		Rect2i texture_region = atlas->get_tile_texture_region(tile_id);
		Vector2 origin = texture_region.position + (texture_region.size - tile_shape_size) / 2 + in_tile_base_offset;

		// Draw only if the tile shape fits in the texture region
		tile_set->draw_tile_shape(base_tiles_shape_grid, Rect2(origin, tile_shape_size), Color(1.0, 0.5, 0.2, 0.8));
	}
}

void TileAtlasView::_alternative_tiles_root_control_gui_input(const Ref<InputEvent> &p_event) {
	if (!tile_set || !tile_set->has_atlas_source(source_id)) {
		return;
	}

	alternative_tiles_root_control->set_tooltip("");

	Ref<InputEventMouseMotion> mm = p_event;
	if (mm.is_valid()) {
		Transform2D xform = alternative_tiles_drawing_root->get_transform().affine_inverse();
		Vector3i coords3 = get_alternative_tile_at_pos(xform.xform(mm->get_position()));
		Vector2i coords = Vector2i(coords3.x, coords3.y);
		int alternative_id = coords3.z;
		if (coords != TileAtlasSource::INVALID_ATLAS_COORDS && alternative_id != TileAtlasSource::INVALID_TILE_ALTERNATIVE) {
			alternative_tiles_root_control->set_tooltip(vformat(TTR("Source: %d\nAtlas coordinates: %s\nAlternative: %d"), source_id, coords, alternative_id));
		}
	}
}

void TileAtlasView::_draw_alternatives() {
	if (!tile_set || !tile_set->has_atlas_source(source_id)) {
		return;
	}

	TileAtlasSource *atlas = tile_set->get_atlas_source(source_id);

	// Draw the alternative tiles.
	Ref<Texture2D> texture = atlas->get_texture();
	if (texture.is_valid()) {
		Vector2 current_pos;
		for (int i = 0; i < atlas->get_tiles_count(); i++) {
			Vector2i tile_id = atlas->get_tile_id(i);
			current_pos.x = 0;
			int y_increment = 0;
			Rect2i texture_region = atlas->get_tile_texture_region(tile_id);
			int alternatives_count = atlas->get_alternative_tiles_count(tile_id);
			for (int j = 1; j < alternatives_count; j++) {
				int alternative_id = atlas->get_alternative_tile_id(tile_id, j);
				TileData *tile_data = atlas->get_tile_data(tile_id, alternative_id);
				bool transposed = tile_data->tile_get_transpose();

				// Update the y to max value.
				y_increment = MAX(y_increment, transposed ? texture_region.size.x : texture_region.size.y);

				// Flip the region according to the flags.
				Rect2 src_region = texture_region;
				if (tile_data->tile_get_flip_h()) {
					src_region.size.x = -src_region.size.x;
				}
				if (tile_data->tile_get_flip_v()) {
					src_region.size.y = -src_region.size.y;
				}

				// Draw the tile.
				alternatives_draw->draw_texture_rect_region(
						texture,
						Rect2i(current_pos, texture_region.size),
						src_region,
						tile_data->tile_get_modulate(),
						transposed);

				// Increment the x position.
				current_pos.x += transposed ? texture_region.size.y : texture_region.size.x;
			}
			if (alternatives_count > 1) {
				current_pos.y += y_increment;
			}
		}
	}
}

void TileAtlasView::_draw_background_left() {
	Ref<Texture2D> texture = get_theme_icon("Checkerboard", "EditorIcons");
	background_left->set_size(base_tiles_texture_rect->get_size());
	background_left->draw_texture_rect(texture, Rect2(Vector2(), background_left->get_size()), true);
}

void TileAtlasView::_draw_background_right() {
	Ref<Texture2D> texture = get_theme_icon("Checkerboard", "EditorIcons");
	background_right->set_size(alternative_tiles_root_control->get_custom_minimum_size());
	background_right->draw_texture_rect(texture, Rect2(Vector2(), background_right->get_size()), true);
}

void TileAtlasView::set_atlas_source(TileSet *p_tile_set, int p_source_id) {
	tile_set = p_tile_set;
	source_id = p_source_id;

	// Show or hide the view.
	bool valid = tile_set && tile_set->has_atlas_source(p_source_id) && tile_set->get_atlas_source(source_id)->get_texture().is_valid();
	hbox->set_visible(valid);
	missing_source_label->set_visible(!valid);

	// Update the rect cache.
	_update_alternative_tiles_rect_cache();

	// Update everything.
	_update_zoom();

	// Change children control size.
	Size2i base_tiles_control_size = _compute_base_tiles_control_size();
	for (int i = 0; i < base_tiles_drawing_root->get_child_count(); i++) {
		Control *control = Object::cast_to<Control>(base_tiles_drawing_root->get_child(i));
		if (control) {
			control->set_size(base_tiles_control_size);
		}
	}

	Size2i alternative_control_size = _compute_alternative_tiles_control_size();
	for (int i = 0; i < alternative_tiles_drawing_root->get_child_count(); i++) {
		Control *control = Object::cast_to<Control>(alternative_tiles_drawing_root->get_child(i));
		if (control) {
			control->set_size(alternative_control_size);
		}
	}

	// Update.
	base_tiles_texture_grid->update();
	base_tiles_shape_grid->update();
	base_tiles_dark->update();
	alternatives_draw->update();
	background_left->update();
	background_right->update();
}

float TileAtlasView::get_zoom() const {
	return zoom;
};

void TileAtlasView::set_zoom(float p_zoom) {
	if (zoom != p_zoom) {
		zoom = p_zoom;
		_update_zoom();
	}
};

void TileAtlasView::set_padding(Side p_side, int p_padding) {
	ERR_FAIL_COND(p_padding < 0);
	margin_container_paddings[p_side] = p_padding;
}

Vector2i TileAtlasView::get_atlas_tile_coords_at_pos(const Vector2 p_pos) const {
	if (!tile_set || !tile_set->has_atlas_source(source_id)) {
		return TileAtlasSource::INVALID_ATLAS_COORDS;
	}

	TileAtlasSource *atlas = tile_set->get_atlas_source(source_id);

	Ref<Texture2D> texture = atlas->get_texture();
	if (texture.is_valid()) {
		Vector2i margins = atlas->get_margins();
		Vector2i separation = atlas->get_separation();
		Vector2i texture_region_size = atlas->get_texture_region_size();

		// Compute index in atlas
		Vector2 pos = p_pos - margins;
		Vector2i ret = (pos / (texture_region_size + separation)).floor();

		return ret;
	}

	return TileAtlasSource::INVALID_ATLAS_COORDS;
}

void TileAtlasView::_update_alternative_tiles_rect_cache() {
	alternative_tiles_rect_cache.clear();

	if (!tile_set || !tile_set->has_atlas_source(source_id)) {
		return;
	}

	TileAtlasSource *atlas = tile_set->get_atlas_source(source_id);

	Rect2i current;
	for (int i = 0; i < atlas->get_tiles_count(); i++) {
		Vector2i tile_id = atlas->get_tile_id(i);
		int alternatives_count = atlas->get_alternative_tiles_count(tile_id);
		Size2i texture_region_size = atlas->get_tile_texture_region(tile_id).size;
		int line_height = 0;
		for (int j = 1; j < alternatives_count; j++) {
			int alternative_id = atlas->get_alternative_tile_id(tile_id, j);
			TileData *tile_data = atlas->get_tile_data(tile_id, alternative_id);
			bool transposed = tile_data->tile_get_transpose();
			current.size = transposed ? Vector2i(texture_region_size.y, texture_region_size.x) : texture_region_size;

			// Update the rect.
			if (!alternative_tiles_rect_cache.has(tile_id)) {
				alternative_tiles_rect_cache[tile_id] = Map<int, Rect2i>();
			}
			alternative_tiles_rect_cache[tile_id][alternative_id] = current;

			current.position.x += transposed ? texture_region_size.y : texture_region_size.x;
			line_height = MAX(line_height, transposed ? texture_region_size.x : texture_region_size.y);
		}

		current.position.x = 0;
		current.position.y += line_height;
	}
}

Vector3i TileAtlasView::get_alternative_tile_at_pos(const Vector2 p_pos) const {
	if (!tile_set || !tile_set->has_atlas_source(source_id)) {
		return Vector3i(TileAtlasSource::INVALID_ATLAS_COORDS.x, TileAtlasSource::INVALID_ATLAS_COORDS.y, TileAtlasSource::INVALID_TILE_ALTERNATIVE);
	}

	for (Map<Vector2, Map<int, Rect2i>>::Element *E_coords = alternative_tiles_rect_cache.front(); E_coords; E_coords = E_coords->next()) {
		for (Map<int, Rect2i>::Element *E_alternative = E_coords->value().front(); E_alternative; E_alternative = E_alternative->next()) {
			if (E_alternative->value().has_point(p_pos)) {
				return Vector3i(E_coords->key().x, E_coords->key().y, E_alternative->key());
			}
		}
	}

	return Vector3i(TileAtlasSource::INVALID_ATLAS_COORDS.x, TileAtlasSource::INVALID_ATLAS_COORDS.y, TileAtlasSource::INVALID_TILE_ALTERNATIVE);
}

Rect2i TileAtlasView::get_alternative_tile_rect(const Vector2i p_coords, int p_alternative_tile) {
	ERR_FAIL_COND_V_MSG(!alternative_tiles_rect_cache.has(p_coords), Rect2i(), vformat("No cached rect for tile coords:%s", p_coords));
	ERR_FAIL_COND_V_MSG(!alternative_tiles_rect_cache[p_coords].has(p_alternative_tile), Rect2i(), vformat("No cached rect for tile coords:%s alternative_id:%d", p_coords, p_alternative_tile));

	return alternative_tiles_rect_cache[p_coords][p_alternative_tile];
}

void TileAtlasView::update() {
	ScrollContainer::update();
	base_tiles_texture_grid->update();
	base_tiles_shape_grid->update();
	base_tiles_dark->update();
	background_left->update();
	background_right->update();
}

void TileAtlasView::_bind_methods() {
	ADD_SIGNAL(MethodInfo("zoom_changed"));
}

TileAtlasView::TileAtlasView() {
	CenterContainer *center_container = memnew(CenterContainer);
	center_container->set_h_size_flags(SIZE_EXPAND_FILL);
	center_container->set_v_size_flags(SIZE_EXPAND_FILL);
	center_container->connect("gui_input", callable_mp(this, &TileAtlasView::_gui_input));
	add_child(center_container);

	missing_source_label = memnew(Label);
	missing_source_label->set_text(TTR("No atlas source with a valid texture selected."));
	center_container->add_child(missing_source_label);

	margin_container = memnew(MarginContainer);
	center_container->add_child(margin_container);

	hbox = memnew(HBoxContainer);
	hbox->add_theme_constant_override("separation", 10);
	hbox->hide();
	margin_container->add_child(hbox);

	VBoxContainer *left_vbox = memnew(VBoxContainer);
	hbox->add_child(left_vbox);

	VBoxContainer *right_vbox = memnew(VBoxContainer);
	hbox->add_child(right_vbox);

	// Base tiles.
	Label *base_tile_label = memnew(Label);
	base_tile_label->set_text(TTR("Base tiles"));
	left_vbox->add_child(base_tile_label);

	base_tiles_root_control = memnew(Control);
	base_tiles_root_control->connect("gui_input", callable_mp(this, &TileAtlasView::_base_tiles_root_control_gui_input));
	left_vbox->add_child(base_tiles_root_control);

	background_left = memnew(Control);
	background_left->set_texture_repeat(TextureRepeat::TEXTURE_REPEAT_ENABLED);
	background_left->set_mouse_filter(Control::MOUSE_FILTER_IGNORE);
	background_left->connect("draw", callable_mp(this, &TileAtlasView::_draw_background_left));
	base_tiles_root_control->add_child(background_left);

	base_tiles_texture_rect = memnew(TextureRect);
	base_tiles_texture_rect->set_texture_filter(TEXTURE_FILTER_NEAREST);
	base_tiles_texture_rect->set_stretch_mode(TextureRect::STRETCH_KEEP_ASPECT);
	base_tiles_texture_rect->set_expand(true);
	base_tiles_texture_rect->set_mouse_filter(Control::MOUSE_FILTER_IGNORE);
	base_tiles_root_control->add_child(base_tiles_texture_rect);

	base_tiles_drawing_root = memnew(Control);
	base_tiles_drawing_root->set_anchors_and_offsets_preset(Control::PRESET_WIDE);
	base_tiles_drawing_root->set_mouse_filter(Control::MOUSE_FILTER_IGNORE);
	base_tiles_root_control->add_child(base_tiles_drawing_root);

	base_tiles_texture_grid = memnew(Control);
	base_tiles_texture_grid->connect("draw", callable_mp(this, &TileAtlasView::_draw_base_tiles_texture_grid));
	base_tiles_texture_grid->set_mouse_filter(Control::MOUSE_FILTER_IGNORE);
	base_tiles_drawing_root->add_child(base_tiles_texture_grid);

	base_tiles_shape_grid = memnew(Control);
	base_tiles_shape_grid->connect("draw", callable_mp(this, &TileAtlasView::_draw_base_tiles_shape_grid));
	base_tiles_shape_grid->set_mouse_filter(Control::MOUSE_FILTER_IGNORE);
	base_tiles_drawing_root->add_child(base_tiles_shape_grid);

	base_tiles_dark = memnew(Control);
	base_tiles_dark->connect("draw", callable_mp(this, &TileAtlasView::_draw_base_tiles_dark));
	base_tiles_dark->set_mouse_filter(Control::MOUSE_FILTER_IGNORE);
	base_tiles_drawing_root->add_child(base_tiles_dark);

	// Alternative tiles.
	Label *alternative_tiles_label = memnew(Label);
	alternative_tiles_label->set_text(TTR("Alternative tiles"));
	right_vbox->add_child(alternative_tiles_label);

	alternative_tiles_root_control = memnew(Control);
	alternative_tiles_root_control->connect("gui_input", callable_mp(this, &TileAtlasView::_alternative_tiles_root_control_gui_input));
	right_vbox->add_child(alternative_tiles_root_control);

	background_right = memnew(Control);
	background_right->set_texture_repeat(TextureRepeat::TEXTURE_REPEAT_ENABLED);
	background_right->connect("draw", callable_mp(this, &TileAtlasView::_draw_background_right));
	background_right->set_mouse_filter(Control::MOUSE_FILTER_IGNORE);
	alternative_tiles_root_control->add_child(background_right);

	alternative_tiles_drawing_root = memnew(Control);
	alternative_tiles_drawing_root->set_texture_filter(TEXTURE_FILTER_NEAREST);
	alternative_tiles_drawing_root->set_mouse_filter(Control::MOUSE_FILTER_IGNORE);
	alternative_tiles_root_control->add_child(alternative_tiles_drawing_root);

	alternatives_draw = memnew(Control);
	alternatives_draw->connect("draw", callable_mp(this, &TileAtlasView::_draw_alternatives));
	alternatives_draw->set_mouse_filter(Control::MOUSE_FILTER_IGNORE);
	alternative_tiles_drawing_root->add_child(alternatives_draw);
}
