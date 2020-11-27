/*************************************************************************/
/*  tile_set.cpp                                                         */
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

#include "tile_set.h"

#include "scene/gui/control.h"

bool TileData::_set(const StringName &p_name, const Variant &p_value) {
	Vector<String> components = String(p_name).split("/", true, 1);

	if (components[0] == TileSetAtlasPluginRendering::ID) {
		if (components.size() < 2) {
			return false;
		}
		return rendering.set(components[1], p_value);
		// TODO: handle other plugins.
	} else {
		return false;
	}
	return true;
}

bool TileData::_get(const StringName &p_name, Variant &r_ret) const {
	Vector<String> components = String(p_name).split("/", true, 1);

	if (components[0] == TileSetAtlasPluginRendering::ID) {
		if (components.size() < 2) {
			return false;
		}
		return rendering.get(components[1], r_ret);
		// TODO: handle other plugins.
	} else {
		return false;
	}
	return true;
}

void TileData::_get_property_list(List<PropertyInfo> *p_list) const {
	// Atlases data.
	List<PropertyInfo> plugin_property_list;
	rendering.get_property_list(&plugin_property_list);
	TileSet::_append_property_list_with_prefix(TileSetAtlasPluginRendering::ID, &plugin_property_list, p_list);

	// TODO: handle other plugins, like this:
	/*
	plugin_property_list.clear();
	rendering.get_property_list(&plugin_property_list);
	for (List<PropertyInfo>::Element * E_property = plugin_property_list.front(); E_property; E_property = E_property->next()) {
		E_property->get().name = vformat("%s/%s", TileSetAtlasPluginRendering::ID, E_property->get().name);
		p_list->push_back(E_property->get());
	}
*/
}

// Base properties

void TileData::tile_set_flip_h(bool p_flip_h) {
	flip_h = p_flip_h;
	emit_signal("changed");
}
bool TileData::tile_get_flip_h() const {
	return flip_h;
}

void TileData::tile_set_flip_v(bool p_flip_v) {
	flip_v = p_flip_v;
	emit_signal("changed");
}

bool TileData::tile_get_flip_v() const {
	return flip_v;
}

void TileData::tile_set_transpose(bool p_transpose) {
	transpose = p_transpose;
	emit_signal("changed");
}
bool TileData::tile_get_transpose() const {
	return transpose;
}

// Misc
void TileData::tile_set_probability(float p_probability) {
	probability = p_probability;
	emit_signal("changed");
}
float TileData::tile_get_probability() const {
	return probability;
}

// Rendering
void TileData::tile_set_texture_offset(Vector2i p_texture_offset) {
	rendering.tex_offset = p_texture_offset;
	emit_signal("changed");
}

Vector2i TileData::tile_get_texture_offset() const {
	return rendering.tex_offset;
}

void TileData::tile_set_material(Ref<ShaderMaterial> p_material) {
	rendering.material = p_material;
	emit_signal("changed");
}
Ref<ShaderMaterial> TileData::tile_get_material() const {
	return rendering.material;
}

void TileData::tile_set_modulate(Color p_modulate) {
	rendering.modulate = p_modulate;
	emit_signal("changed");
}
Color TileData::tile_get_modulate() const {
	return rendering.modulate;
}

void TileData::tile_set_z_index(int p_z_index) {
	rendering.z_index = p_z_index;
	emit_signal("changed");
}
int TileData::tile_get_z_index() const {
	return rendering.z_index;
}

void TileData::tile_set_y_sort_origin(Vector2i p_y_sort_origin) {
	rendering.y_sort_origin = p_y_sort_origin;
	emit_signal("changed");
}
Vector2i TileData::tile_get_y_sort_origin() const {
	return rendering.y_sort_origin;
}

void tile_set_occluder(int p_layer_id, Ref<OccluderPolygon2D> p_occluder_polygon) {
}

Ref<OccluderPolygon2D> TileData::tile_get_occluder(int p_layer_id) const {
	// TODO
	return Ref<OccluderPolygon2D>();
}

void TileData::_bind_methods() {
	// -- Tile properties --
	// Base properties
	ClassDB::bind_method(D_METHOD("tile_set_flip_h", "flip_h"), &TileData::tile_set_flip_h);
	ClassDB::bind_method(D_METHOD("tile_get_flip_h"), &TileData::tile_get_flip_h);
	ClassDB::bind_method(D_METHOD("tile_set_flip_v", "flip_v"), &TileData::tile_set_flip_v);
	ClassDB::bind_method(D_METHOD("tile_get_flip_v"), &TileData::tile_get_flip_v);
	ClassDB::bind_method(D_METHOD("tile_set_transpose", "transpose"), &TileData::tile_set_transpose);
	ClassDB::bind_method(D_METHOD("tile_get_transpose"), &TileData::tile_get_transpose);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "flip_h"), "tile_set_flip_h", "tile_get_flip_h");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "flip_v"), "tile_set_flip_v", "tile_get_flip_v");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "transpose"), "tile_set_transpose", "tile_get_transpose");

	// Rendering.
	ClassDB::bind_method(D_METHOD("tile_set_texture_offset", "source_id", "atlas_coords", "texture_offset"), &TileData::tile_set_texture_offset);
	ClassDB::bind_method(D_METHOD("tile_get_texture_offset", "source_id", "atlas_coords"), &TileData::tile_get_texture_offset);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "texture_offset"), "tile_set_texture_offset", "tile_get_texture_offset");

	ADD_SIGNAL(MethodInfo("changed", PropertyInfo(Variant::STRING, "what")));
}

void TileAtlasSource::set_texture(Ref<Texture2D> p_texture) {
	texture = p_texture;

	emit_signal("changed", "texture");
}

Ref<Texture2D> TileAtlasSource::get_texture() const {
	return texture;
}

void TileAtlasSource::set_margins(Vector2i p_margins) {
	if (p_margins.x < 0 || p_margins.y < 0) {
		WARN_PRINT("Atlas source margins should be positive.");
		margins = Vector2i(MAX(0, p_margins.x), MAX(0, p_margins.y));
	} else {
		margins = p_margins;
	}

	emit_signal("changed", "margins");
}
Vector2i TileAtlasSource::get_margins() const {
	return margins;
}

void TileAtlasSource::set_separation(Vector2i p_separation) {
	if (p_separation.x < 0 || p_separation.y < 0) {
		WARN_PRINT("Atlas source separation should be positive.");
		separation = Vector2i(MAX(0, p_separation.x), MAX(0, p_separation.y));
	} else {
		separation = p_separation;
	}

	emit_signal("changed", "separation");
}
Vector2i TileAtlasSource::get_separation() const {
	return separation;
}

void TileAtlasSource::set_texture_region_size(Vector2i p_tile_size) {
	if (p_tile_size.x <= 0 || p_tile_size.y <= 0) {
		WARN_PRINT("Atlas source tile_size should be strictly positive.");
		texture_region_size = Vector2i(MAX(1, p_tile_size.x), MAX(1, p_tile_size.y));
	} else {
		texture_region_size = p_tile_size;
	}

	emit_signal("changed", "texture_region_size");
}
Vector2i TileAtlasSource::get_texture_region_size() const {
	return texture_region_size;
}

void TileAtlasSource::set_base_texture_offset(Vector2i p_base_texture_offset) {
	base_texture_offset = p_base_texture_offset;

	emit_signal("changed", "base_texture_offset");
}
Vector2i TileAtlasSource::get_base_texture_offset() const {
	return base_texture_offset;
}

Vector2i TileAtlasSource::get_atlas_grid_size() const {
	Ref<Texture2D> texture = get_texture();
	if (!texture.is_valid()) {
		return Vector2i();
	}

	ERR_FAIL_COND_V(texture_region_size.x <= 0 || texture_region_size.y <= 0, Vector2i());

	Size2i valid_area = texture->get_size() - margins;

	// Compute the number of valid tiles in the tiles atlas
	Size2i grid_size = Size2i();
	if (valid_area.x >= texture_region_size.x && valid_area.y >= texture_region_size.y) {
		valid_area -= texture_region_size;
		grid_size = Size2i(1, 1) + valid_area / (texture_region_size + separation);
	}
	return grid_size;
}

bool TileAtlasSource::_set(const StringName &p_name, const Variant &p_value) {
	Vector<String> components = String(p_name).split("/", true, 2);

	// Compute the vector2i if we have coordinates.
	Vector<String> coords_split = components[0].split(":");
	Vector2i coords = TileAtlasSource::INVALID_ATLAS_COORDS;
	if (coords_split.size() == 2 && coords_split[0].is_valid_integer() && coords_split[1].is_valid_integer()) {
		coords = Vector2i(coords_split[0].to_int(), coords_split[1].to_int());
	}

	// Properties.
	if (coords != TileAtlasSource::INVALID_ATLAS_COORDS) {
		// Create the tile if needed.
		if (!has_tile(coords)) {
			create_tile(coords);
		}
		if (components.size() >= 2) {
			// Properties.
			if (components[1] == "size_in_atlas") {
				move_tile_in_atlas(coords, coords, p_value);
			} else if (components[1] == "next_alternative_id") {
				tiles[coords].next_alternative_id = p_value;
			} else if (components[1].is_valid_integer()) {
				int alternative_id = components[1].to_int();
				if (alternative_id != TileAtlasSource::INVALID_TILE_ALTERNATIVE) {
					// Create the alternative if needed ?
					if (!has_alternative_tile(coords, alternative_id)) {
						create_alternative_tile(coords, alternative_id);
					}
					if (!tiles[coords].alternatives.has(alternative_id)) {
						tiles[coords].alternatives[alternative_id] = memnew(TileData);
						tiles[coords].alternatives_ids.append(alternative_id);
					}
					if (components.size() >= 3) {
						bool valid;
						tiles[coords].alternatives[alternative_id]->set(components[2], p_value, &valid);
						return valid;
					} else {
						// Only create the alternative if it did not exist yet.
						return true;
					}
				}
			}
		}
	}

	return false;
}

bool TileAtlasSource::_get(const StringName &p_name, Variant &r_ret) const {
	Vector<String> components = String(p_name).split("/", true, 2);

	// Properties.
	Vector<String> coords_split = components[0].split(":");
	if (coords_split.size() == 2 && coords_split[0].is_valid_integer() && coords_split[1].is_valid_integer()) {
		Vector2i coords = Vector2i(coords_split[0].to_int(), coords_split[1].to_int());
		if (tiles.has(coords)) {
			if (components.size() >= 2) {
				// Properties.
				if (components[1] == "size_in_atlas") {
					r_ret = tiles[coords].size_in_atlas;
					return true;
				} else if (components[1] == "next_alternative_id") {
					r_ret = tiles[coords].next_alternative_id;
					return true;
				} else if (components[1].is_valid_integer()) {
					int alternative_id = components[1].to_int();
					if (alternative_id != TileAtlasSource::INVALID_TILE_ALTERNATIVE && tiles[coords].alternatives.has(alternative_id)) {
						if (components.size() >= 3) {
							bool valid;
							r_ret = tiles[coords].alternatives[alternative_id]->get(components[2], &valid);
							return valid;
						} else {
							// Only to notify the tile alternative exists.
							r_ret = alternative_id;
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}

void TileAtlasSource::_get_property_list(List<PropertyInfo> *p_list) const {
	// Atlases data.
	for (Map<Vector2i, TileAlternativesData>::Element *E_tile = tiles.front(); E_tile; E_tile = E_tile->next()) {
		List<PropertyInfo> tile_property_list;
		if (E_tile->get().size_in_atlas != Vector2i(1, 1)) {
			tile_property_list.push_back(PropertyInfo(Variant::VECTOR2I, "size_in_atlas", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR));
		}
		if (E_tile->get().next_alternative_id != 1) {
			tile_property_list.push_back(PropertyInfo(Variant::INT, "next_alternative_id", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR));
		}

		// Atlases data.
		for (Map<int, TileData *>::Element *E_alternative = E_tile->get().alternatives.front(); E_alternative; E_alternative = E_alternative->next()) {
			// Add a dummy property to show the tile exists.
			tile_property_list.push_back(PropertyInfo(Variant::INT, vformat("%d", E_alternative->key()), PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR));

			List<PropertyInfo> alternative_property_list;
			E_alternative->get()->get_property_list(&alternative_property_list);
			TileSet::_append_property_list_with_prefix(vformat("%d", E_alternative->key()), &alternative_property_list, &tile_property_list);
		}

		TileSet::_append_property_list_with_prefix(vformat("%d:%d", E_tile->key().x, E_tile->key().y), &tile_property_list, p_list);
	}
}

// -- Tiles management --
// Base tiles
void TileAtlasSource::create_tile(const Vector2i p_atlas_coords, const Vector2i p_size) {
	// Create a tile if it does not exists.
	ERR_FAIL_COND(p_atlas_coords.x < 0 || p_atlas_coords.y < 0);
	ERR_FAIL_COND(p_size.x <= 0 || p_size.y <= 0);
	for (int x = 0; x < p_size.x; x++) {
		for (int y = 0; y < p_size.y; y++) {
			Vector2i coords = p_atlas_coords + Vector2i(x, y);
			ERR_FAIL_COND_MSG(tiles.has(coords), vformat("Cannot create tile at position %s with size %s. Already a tile present at %s.", p_atlas_coords, p_size, coords));
		}
	}

	// Create and resize the tile.
	tiles.insert(p_atlas_coords, TileAtlasSource::TileAlternativesData());
	tiles_ids.append(p_atlas_coords);
	tiles_ids.sort();

	tiles[p_atlas_coords].alternatives[0] = memnew(TileData);
	tiles[p_atlas_coords].alternatives_ids.append(0);

	// Add all covered positions to the mapping cache
	for (int x = 0; x < p_size.x; x++) {
		for (int y = 0; y < p_size.y; y++) {
			Vector2i coords = p_atlas_coords + Vector2i(x, y);
			_coords_mapping_cache[coords] = p_atlas_coords;
		}
	}

	emit_signal("changed");
}

void TileAtlasSource::remove_tile(Vector2i p_atlas_coords) {
	ERR_FAIL_COND_MSG(!tiles.has(p_atlas_coords), vformat("TileAtlasSource has no tile at %s.", String(p_atlas_coords)));

	// Remove all covered positions from the mapping cache
	Size2i size = tiles[p_atlas_coords].size_in_atlas;

	for (int x = 0; x < size.x; x++) {
		for (int y = 0; y < size.y; y++) {
			Vector2i coords = p_atlas_coords + Vector2i(x, y);
			_coords_mapping_cache.erase(coords);
		}
	}

	// Free tile data.
	for (Map<int, TileData *>::Element *E_tile_data = tiles[p_atlas_coords].alternatives.front(); E_tile_data; E_tile_data = E_tile_data->next()) {
		memdelete(E_tile_data->get());
	}

	// Delete the tile
	tiles.erase(p_atlas_coords);
	tiles_ids.erase(p_atlas_coords);
	tiles_ids.sort();

	emit_signal("changed");
}

bool TileAtlasSource::has_tile(Vector2i p_atlas_coords) const {
	return tiles.has(p_atlas_coords);
}

Vector2i TileAtlasSource::get_tile_at_coords(Vector2i p_atlas_coords) const {
	if (!_coords_mapping_cache.has(p_atlas_coords)) {
		return INVALID_ATLAS_COORDS;
	}

	return _coords_mapping_cache[p_atlas_coords];
}

Vector2i TileAtlasSource::get_tile_size_in_atlas(Vector2i p_atlas_coords) const {
	ERR_FAIL_COND_V_MSG(!tiles.has(p_atlas_coords), Vector2i(-1, -1), vformat("TileAtlasSource has no tile at %s.", String(p_atlas_coords)));

	return tiles[p_atlas_coords].size_in_atlas;
}

int TileAtlasSource::get_tiles_count() const {
	return tiles_ids.size();
}

Vector2i TileAtlasSource::get_tile_id(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, tiles_ids.size(), TileAtlasSource::INVALID_ATLAS_COORDS);
	return tiles_ids[p_index];
}

Rect2i TileAtlasSource::get_tile_texture_region(Vector2i p_atlas_coords) const {
	ERR_FAIL_COND_V_MSG(!tiles.has(p_atlas_coords), Rect2i(), vformat("TileAtlasSource has no tile at %s.", String(p_atlas_coords)));

	Vector2i size_in_atlas = tiles[p_atlas_coords].size_in_atlas;
	Vector2 region_size = texture_region_size * size_in_atlas + separation * (size_in_atlas - Vector2i(1, 1));

	Vector2 origin = margins + (p_atlas_coords * (texture_region_size + separation));

	return Rect2(origin, region_size);
	;
}

bool TileAtlasSource::can_move_tile_in_atlas(Vector2i p_atlas_coords, Vector2i p_new_atlas_coords, Vector2i p_new_size) const {
	ERR_FAIL_COND_V_MSG(!tiles.has(p_atlas_coords), false, vformat("TileAtlasSource has no tile at %s.", String(p_atlas_coords)));

	Vector2i new_atlas_coords = (p_new_atlas_coords != INVALID_ATLAS_COORDS) ? p_new_atlas_coords : p_atlas_coords;
	if (new_atlas_coords.x < 0 || new_atlas_coords.y < 0) {
		return false;
	}

	Vector2i size = (p_new_size != Vector2i(-1, -1)) ? p_new_size : tiles[p_atlas_coords].size_in_atlas;
	ERR_FAIL_COND_V(size.x <= 0 || size.y <= 0, false);

	Size2i grid_size = get_atlas_grid_size();
	if (new_atlas_coords.x + size.x > grid_size.x || new_atlas_coords.y + size.y > grid_size.y) {
		return false;
	}

	Rect2i new_rect = Rect2i(new_atlas_coords, size);
	// Check if the new tile can fit in the new rect.
	for (int x = new_rect.position.x; x < new_rect.get_end().x; x++) {
		for (int y = new_rect.position.y; y < new_rect.get_end().y; y++) {
			Vector2i coords = get_tile_at_coords(Vector2i(x, y));
			if (coords != p_atlas_coords && coords != TileAtlasSource::INVALID_ATLAS_COORDS) {
				return false;
			}
		}
	}

	return true;
}

void TileAtlasSource::move_tile_in_atlas(Vector2i p_atlas_coords, Vector2i p_new_atlas_coords, Vector2i p_new_size) {
	bool can_move = can_move_tile_in_atlas(p_atlas_coords, p_new_atlas_coords, p_new_size);
	ERR_FAIL_COND_MSG(!can_move, vformat("Cannot move tile at position %s with size %s. Tile already present.", p_new_atlas_coords, p_new_size));

	// Compute the actual new rect from arguments.
	Vector2i new_atlas_coords = (p_new_atlas_coords != INVALID_ATLAS_COORDS) ? p_new_atlas_coords : p_atlas_coords;
	Vector2i size = (p_new_size != Vector2i(-1, -1)) ? p_new_size : tiles[p_atlas_coords].size_in_atlas;

	if (new_atlas_coords == p_atlas_coords && size == tiles[p_atlas_coords].size_in_atlas) {
		return;
	}

	// Remove all covered positions from the mapping cache.
	Size2i old_size = tiles[p_atlas_coords].size_in_atlas;
	for (int x = 0; x < old_size.x; x++) {
		for (int y = 0; y < old_size.y; y++) {
			Vector2i coords = p_atlas_coords + Vector2i(x, y);
			_coords_mapping_cache.erase(coords);
		}
	}

	// Move the tile and update its size.
	if (new_atlas_coords != p_atlas_coords) {
		tiles[new_atlas_coords] = tiles[p_atlas_coords];
		tiles.erase(p_atlas_coords);

		tiles_ids.erase(p_atlas_coords);
		tiles_ids.append(new_atlas_coords);
		tiles_ids.sort();
	}
	tiles[new_atlas_coords].size_in_atlas = size;

	// Add all covered positions to the mapping cache again.
	for (int x = 0; x < size.x; x++) {
		for (int y = 0; y < size.y; y++) {
			Vector2i coords = new_atlas_coords + Vector2i(x, y);
			_coords_mapping_cache[coords] = new_atlas_coords;
		}
	}

	emit_signal("changed");
}

bool TileAtlasSource::has_tiles_outside_texture() {
	Vector2i grid_size = get_atlas_grid_size();
	Vector<Vector2i> to_remove;

	for (Map<Vector2i, TileAtlasSource::TileAlternativesData>::Element *E = tiles.front(); E; E = E->next()) {
		if (E->key().x >= grid_size.x || E->key().y >= grid_size.y) {
			return true;
		}
	}

	return false;
}

void TileAtlasSource::clear_tiles_outside_texture() {
	Vector2i grid_size = get_atlas_grid_size();
	Vector<Vector2i> to_remove;

	for (Map<Vector2i, TileAtlasSource::TileAlternativesData>::Element *E = tiles.front(); E; E = E->next()) {
		if (E->key().x >= grid_size.x || E->key().y >= grid_size.y) {
			to_remove.append(E->key());
		}
	}

	for (int i = 0; i < to_remove.size(); i++) {
		remove_tile(to_remove[i]);
	}
}

void TileAtlasSource::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_texture", "texture"), &TileAtlasSource::set_texture);
	ClassDB::bind_method(D_METHOD("get_texture"), &TileAtlasSource::get_texture);
	ClassDB::bind_method(D_METHOD("set_margins", "margins"), &TileAtlasSource::set_margins);
	ClassDB::bind_method(D_METHOD("get_margins"), &TileAtlasSource::get_margins);
	ClassDB::bind_method(D_METHOD("set_separation", "separation"), &TileAtlasSource::set_separation);
	ClassDB::bind_method(D_METHOD("get_separation"), &TileAtlasSource::get_separation);
	ClassDB::bind_method(D_METHOD("set_texture_region_size", "texture_region_size"), &TileAtlasSource::set_texture_region_size);
	ClassDB::bind_method(D_METHOD("get_texture_region_size"), &TileAtlasSource::get_texture_region_size);
	ClassDB::bind_method(D_METHOD("set_base_texture_offset", "base_texture_offset"), &TileAtlasSource::set_base_texture_offset);
	ClassDB::bind_method(D_METHOD("get_base_texture_offset"), &TileAtlasSource::get_base_texture_offset);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D", PROPERTY_USAGE_NOEDITOR), "set_texture", "get_texture");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "margins", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR), "set_margins", "get_margins");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "separation", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR), "set_separation", "get_separation");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "tile_size", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR), "set_texture_region_size", "get_texture_region_size");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "base_texture_offset", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR), "set_base_texture_offset", "get_base_texture_offset");

	// Base tiles
	ClassDB::bind_method(D_METHOD("create_tile", "atlas_coords", "size"), &TileAtlasSource::create_tile, DEFVAL(Vector2i(1, 1)));
	ClassDB::bind_method(D_METHOD("remove_tile", "atlas_coords"), &TileAtlasSource::remove_tile); // Remove a tile. If p_tile_key.alternative_tile if different from 0, remove the alternative
	ClassDB::bind_method(D_METHOD("has_tile", "atlas_coords"), &TileAtlasSource::has_tile);
	ClassDB::bind_method(D_METHOD("can_move_tile_in_atlas", "atlas_coords", "new_atlas_coords", "new_size"), &TileAtlasSource::can_move_tile_in_atlas, DEFVAL(INVALID_ATLAS_COORDS), DEFVAL(Vector2i(-1, -1)));
	ClassDB::bind_method(D_METHOD("move_tile_in_atlas", "atlas_coords", "new_atlas_coords", "new_size"), &TileAtlasSource::move_tile_in_atlas, DEFVAL(INVALID_ATLAS_COORDS), DEFVAL(Vector2i(-1, -1)));
	ClassDB::bind_method(D_METHOD("get_tile_size_in_atlas", "atlas_coords"), &TileAtlasSource::get_tile_size_in_atlas);

	ClassDB::bind_method(D_METHOD("get_tiles_count"), &TileAtlasSource::get_tiles_count);
	ClassDB::bind_method(D_METHOD("get_tile_id", "index"), &TileAtlasSource::get_tile_id);

	ClassDB::bind_method(D_METHOD("get_tile_at_coords", "atlas_coords"), &TileAtlasSource::get_tile_at_coords);

	// Alternative tiles
	ClassDB::bind_method(D_METHOD("create_alternative_tile", "atlas_coords", "alternative_id_override"), &TileAtlasSource::create_alternative_tile, DEFVAL(-1));
	ClassDB::bind_method(D_METHOD("remove_alternative_tile", "atlas_coords", "alternative_tile"), &TileAtlasSource::remove_alternative_tile);
	ClassDB::bind_method(D_METHOD("set_alternative_tile_id", "atlas_coords", "alternative_tile", "new_id"), &TileAtlasSource::set_alternative_tile_id);
	ClassDB::bind_method(D_METHOD("has_alternative_tile", "atlas_coords", "alternative_tile"), &TileAtlasSource::has_alternative_tile);
	ClassDB::bind_method(D_METHOD("get_next_alternative_tile_id", "atlas_coords"), &TileAtlasSource::get_next_alternative_tile_id);

	ClassDB::bind_method(D_METHOD("get_alternative_tiles_count", "atlas_coords"), &TileAtlasSource::get_alternative_tiles_count);
	ClassDB::bind_method(D_METHOD("get_alternative_tile_id", "atlas_coords", "index"), &TileAtlasSource::get_alternative_tile_id);

	// Helpers.
	ClassDB::bind_method(D_METHOD("get_atlas_grid_size"), &TileAtlasSource::get_atlas_grid_size);
	ClassDB::bind_method(D_METHOD("has_tiles_outside_texture"), &TileAtlasSource::has_tiles_outside_texture);
	ClassDB::bind_method(D_METHOD("clear_tiles_outside_texture"), &TileAtlasSource::clear_tiles_outside_texture);
	ClassDB::bind_method(D_METHOD("get_tile_texture_region", "atlas_coords"), &TileAtlasSource::get_tile_texture_region);

	ADD_SIGNAL(MethodInfo("changed"));
}

TileAtlasSource::~TileAtlasSource() {
	// Free everything needed.
	for (Map<Vector2i, TileAlternativesData>::Element *E_alternatives = tiles.front(); E_alternatives; E_alternatives = E_alternatives->next()) {
		for (Map<int, TileData *>::Element *E_tile_data = E_alternatives->get().alternatives.front(); E_tile_data; E_tile_data = E_tile_data->next()) {
			memdelete(E_tile_data->get());
		}
	}
}

const Vector2i TileAtlasSource::INVALID_ATLAS_COORDS = Vector2i(-1, -1);
const int TileAtlasSource::INVALID_TILE_ALTERNATIVE = -1;

#ifndef DISABLE_DEPRECATED
void TileSet::compatibility_conversion() {
	for (Map<int, CompatibilityTileData *>::Element *E = compatibility_data.front(); E; E = E->next()) {
		CompatibilityTileData *ctd = E->value();

		// Add the texture
		int source_id = add_atlas_source();
		TileAtlasSource *atlas_source = get_atlas_source(source_id);
		atlas_source->set_texture(ctd->texture);

		// Handle each tile as a new source. Not optimal but at least it should stay compatible.
		switch (ctd->tile_mode) {
			case 0: // SINGLE_TILE
				// TODO
				break;
			case 1: // AUTO_TILE
				// TODO
				break;
			case 2: // ATLAS_TILE
				atlas_source->set_margins(ctd->region.get_position());
				atlas_source->set_separation(Vector2i(ctd->autotile_spacing, ctd->autotile_spacing));
				atlas_source->set_texture_region_size(ctd->autotile_tile_size);

				Size2i atlas_size = ctd->region.get_size() / (ctd->autotile_tile_size + atlas_source->get_separation());
				for (int i = 0; i < atlas_size.x; i++) {
					for (int j = 0; j < atlas_size.y; j++) {
						Vector2i coords = Vector2i(i, j);

						for (int flags = 0; flags < 8; flags++) {
							bool flip_h = flags & 1;
							bool flip_v = flags & 2;
							bool transpose = flags & 4;

							int alternative_tile = 0;
							if (!atlas_source->has_tile(coords)) {
								atlas_source->create_tile(coords);
							} else {
								alternative_tile = atlas_source->create_alternative_tile(coords);
							}
							TileData *tile_data = atlas_source->get_tile_data(coords, alternative_tile);

							tile_data->tile_set_flip_h(flip_h);
							tile_data->tile_set_flip_v(flip_v);
							tile_data->tile_set_transpose(transpose);
							tile_data->tile_set_material(ctd->material);
							tile_data->tile_set_modulate(ctd->modulate);
							tile_data->tile_set_z_index(ctd->z_index);
							if (ctd->autotile_z_index_map.has(coords)) {
								tile_data->tile_set_z_index(ctd->autotile_z_index_map[coords]);
							}
							if (ctd->autotile_priority_map.has(coords)) {
								tile_data->tile_set_probability(ctd->autotile_priority_map[coords]);
							}

							// -- TODO: handle --
							// texture offset
							// bitmask_mode
							// bitmask_flags
							// occluder_map
							// navpoly_map
							// "occluder"
							// "occluder_offset"
							// "navigation"
							// "navigation_offset"

							print_line(vformat("Created compatibility tile: source id=%d, coords=(%s), alternative=%d", source_id, String(coords), alternative_tile));
						}
					}
				}
				break;
		}

		// Add the mapping to the map
		compatibility_source_mapping.insert(E->key(), source_id);
	}

	// Reset compatibility data
	compatibility_data = Map<int, CompatibilityTileData *>();
}
#endif // DISABLE_DEPRECATED

bool TileSet::_set(const StringName &p_name, const Variant &p_value) {
	Vector<String> components = String(p_name).split("/", true, 2);

#ifndef DISABLE_DEPRECATED
	// TODO: THIS IS HOW WE CHECK IF WE HAVE A DEPRECATED RESOURCE
	// This should be moved to a dedicated conversion system
	if (components[0].is_valid_integer()) {
		int id = components[0].to_int();

		// Get or create the compatibility object
		CompatibilityTileData *ctd;
		Map<int, CompatibilityTileData *>::Element *E = compatibility_data.find(id);
		if (!E) {
			ctd = memnew(CompatibilityTileData);
			compatibility_data.insert(id, ctd);
		} else {
			ctd = E->get();
		}

		String what = components[1];

		if (what == "name") {
			ctd->name = p_value;
		} else if (what == "texture") {
			ctd->texture = p_value;
		} else if (what == "tex_offset") {
			ctd->tex_offset = p_value;
		} else if (what == "material") {
			ctd->material = p_value;
		} else if (what == "modulate") {
			ctd->modulate = p_value;
		} else if (what == "region") {
			ctd->region = p_value;
		} else if (what == "tile_mode") {
			ctd->tile_mode = p_value;
		} else if (what.left(9) == "autotile") {
			what = what.right(9);
			if (what == "bitmask_mode") {
				ctd->autotile_bitmask_mode = p_value;
			} else if (what == "icon_coordinate") {
				ctd->autotile_icon_coordinate = p_value;
			} else if (what == "tile_size") {
				ctd->autotile_tile_size = p_value;
			} else if (what == "spacing") {
				ctd->autotile_spacing = p_value;
			} else if (what == "bitmask_flags") {
				if (p_value.is_array()) {
					Array p = p_value;
					Vector2i last_coord;
					while (p.size() > 0) {
						if (p[0].get_type() == Variant::VECTOR2) {
							last_coord = p[0];
						} else if (p[0].get_type() == Variant::INT) {
							ctd->autotile_bitmask_flags.insert(last_coord, p[0]);
						}
						p.pop_front();
					}
				}
			} else if (what == "occluder_map") {
				Array p = p_value;
				Vector2 last_coord;
				while (p.size() > 0) {
					if (p[0].get_type() == Variant::VECTOR2) {
						last_coord = p[0];
					} else if (p[0].get_type() == Variant::OBJECT) {
						ctd->autotile_occluder_map.insert(last_coord, p[0]);
					}
					p.pop_front();
				}
			} else if (what == "navpoly_map") {
				Array p = p_value;
				Vector2 last_coord;
				while (p.size() > 0) {
					if (p[0].get_type() == Variant::VECTOR2) {
						last_coord = p[0];
					} else if (p[0].get_type() == Variant::OBJECT) {
						ctd->autotile_navpoly_map.insert(last_coord, p[0]);
					}
					p.pop_front();
				}
			} else if (what == "priority_map") {
				Array p = p_value;
				Vector3 val;
				Vector2 v;
				int priority;
				while (p.size() > 0) {
					val = p[0];
					if (val.z > 1) {
						v.x = val.x;
						v.y = val.y;
						priority = (int)val.z;
						ctd->autotile_priority_map.insert(v, priority);
					}
					p.pop_front();
				}
			} else if (what == "z_index_map") {
				Array p = p_value;
				Vector3 val;
				Vector2 v;
				int z_index;
				while (p.size() > 0) {
					val = p[0];
					if (val.z != 0) {
						v.x = val.x;
						v.y = val.y;
						z_index = (int)val.z;
						ctd->autotile_z_index_map.insert(v, z_index);
					}
					p.pop_front();
				}
			}
			/*
		// INGORED FOR NOW, they seem duplicated data compared to the shapes array
		else if (what == "shape") {
			// TODO
		} else if (what == "shape_offset") {
			// TODO
		} else if (what == "shape_transform") {
			// TODO
		} else if (what == "shape_one_way") {
			// TODO
		} else if (what == "shape_one_way_margin") {
			// TODO
		}
		*/
		} else if (what == "shapes") {
			// TODO
		} else if (what == "occluder") {
			// TODO
		} else if (what == "occluder_offset") {
			// TODO
		} else if (what == "navigation") {
			// TODO
		} else if (what == "navigation_offset") {
			// TODO
		} else if (what == "z_index") {
			ctd->z_index = p_value;

			// TODO: remove the consersion from here, it's not where it should be done
			compatibility_conversion();
		} else {
			return false;
		}
	} else {
#endif // DISABLE_DEPRECATED

		// This is now a new property.
		if (components.size() >= 1) {
			if (components[0] == "atlases") {
				if (components.size() >= 2 && components[1].is_valid_integer()) {
					// Create atlas if it does not exists.
					int source_id = components[1].to_int();
					if (!has_atlas_source(source_id)) {
						add_atlas_source(source_id);
					}

					// Set the TileAtlasSource property.
					if (components.size() >= 3) {
						bool valid;
						get_atlas_source(source_id)->set(components[2], p_value, &valid);
						return valid;
					} else {
						// Only create the TileAtlasSource, nothing else.
						return true;
					}
				}
			} else if (tile_set_plugins.has(components[0])) {
				if (components.size() >= 3) {
					// TileSet-level plugin data.
					if (components[1] == TileSetAtlasPluginRendering::ID) {
						return rendering.set(components[2], p_value);
					}
					// TODO: handle other plugins.
				}
			}
		}

#ifndef DISABLE_DEPRECATED
	}
#endif // DISABLE_DEPRECATED

	return false;
}

bool TileSet::_get(const StringName &p_name, Variant &r_ret) const {
	Vector<String> components = String(p_name).split("/", true, 2);

	if (components.size() >= 2 && components[0] == "atlases" && components[1].is_valid_integer()) {
		// Atlases data.
		int source_id = components[1].to_int();
		if (has_atlas_source(source_id)) {
			if (components.size() >= 3) {
				// Return the TileAtlasSource property.
				bool valid;
				r_ret = get_atlas_source(source_id)->get(components[2], &valid);
				return valid;
			} else {
				// Return null, only used to notify the atlas exists.
				r_ret = source_id;
				return true;
			}
		} else {
			return false;
		}
	} else if (components.size() >= 2 && tile_set_plugins.has(components[0])) {
		// TileSet-level plugin data.
		if (components[1] == TileSetAtlasPluginRendering::ID) {
			return rendering.get(components[2], r_ret);
		}
		// TODO: handle other plugins.
	}

	return false;
}

void TileSet::_get_property_list(List<PropertyInfo> *p_list) const {
	// Atlases data.
	for (Map<int, TileAtlasSource *>::Element *E_source = source_atlas.front(); E_source; E_source = E_source->next()) {
		// Add a dummy property to show the tile exists.
		p_list->push_back(PropertyInfo(Variant::INT, vformat("atlases/%d", E_source->key()), PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR));

		List<PropertyInfo> atlas_property_list;
		E_source->get()->get_property_list(&atlas_property_list);

		// Make sure to display no property in the inspector.
		for (List<PropertyInfo>::Element *E_property = atlas_property_list.front(); E_property; E_property = E_property->next()) {
			E_property->get().usage = E_property->get().usage & ~PROPERTY_USAGE_EDITOR;
		}

		TileSet::_append_property_list_with_prefix(vformat("atlases/%d", E_source->key()), &atlas_property_list, p_list);

		// If we have no properties (everything might be default values), we add a property to notify the atlas exists.
	}

	// Plugin tileset data.
	for (Map<String, TileSetAtlasPlugin *>::Element *E_plugin = tile_set_plugins.front(); E_plugin; E_plugin = E_plugin->next()) {
		// Retrieve the list of tileset properties defined by the atlas plugin.
		List<PropertyInfo> plugin_property_list;
		rendering.get_property_list(&plugin_property_list);
		TileSet::_append_property_list_with_prefix(TileSetAtlasPluginRendering::ID, &plugin_property_list, p_list);

		// TODO: For other plugins:
		// plugin_property_list.clear();
		// PLUGIN.get_property_list(&plugin_property_list);
		// _append_property_list_with_prefix(PLUGIN::ID, &plugin_property_list, p_list);
	}
}

void TileSet::_append_property_list_with_prefix(const StringName &p_name, List<PropertyInfo> *p_to_prepend, List<PropertyInfo> *p_list) {
	for (List<PropertyInfo>::Element *E_property = p_to_prepend->front(); E_property; E_property = E_property->next()) {
		E_property->get().name = vformat("%s/%s", p_name, E_property->get().name);
		p_list->push_back(E_property->get());
	}
}

TileData *TileAtlasSource::_get_atlas_tile_data(Vector2i p_atlas_coords, int p_alternative_tile) {
	ERR_FAIL_COND_V_MSG(!tiles.has(p_atlas_coords), nullptr, vformat("TileAtlasSource has no tile at %s.", String(p_atlas_coords)));
	ERR_FAIL_COND_V_MSG(!tiles[p_atlas_coords].alternatives.has(p_alternative_tile), nullptr, vformat("TileAtlasSource has no alternative with id %d for tile coords %s.", p_alternative_tile, String(p_atlas_coords)));

	return tiles[p_atlas_coords].alternatives[p_alternative_tile];
}

const TileData *TileAtlasSource::_get_atlas_tile_data(Vector2i p_atlas_coords, int p_alternative_tile) const {
	ERR_FAIL_COND_V_MSG(!tiles.has(p_atlas_coords), nullptr, vformat("TileAtlasSource has no tile at %s.", String(p_atlas_coords)));
	ERR_FAIL_COND_V_MSG(!tiles[p_atlas_coords].alternatives.has(p_alternative_tile), nullptr, vformat("TileAtlasSource has no alternative with id %d for tile coords %s.", p_alternative_tile, String(p_atlas_coords)));

	return tiles[p_atlas_coords].alternatives[p_alternative_tile];
}

void TileAtlasSource::_compute_next_alternative_id(const Vector2i p_atlas_coords) {
	ERR_FAIL_COND_MSG(!tiles.has(p_atlas_coords), vformat("TileAtlasSource has no tile at %s.", String(p_atlas_coords)));

	while (tiles[p_atlas_coords].alternatives.has(tiles[p_atlas_coords].next_alternative_id)) {
		tiles[p_atlas_coords].next_alternative_id = (tiles[p_atlas_coords].next_alternative_id % 1073741823) + 1; // 2 ** 30
	};
}

// --- Plugins ---
Vector<TileSetAtlasPlugin *> TileSet::get_tile_set_atlas_plugins() const {
	return tile_set_plugins_vector;
}

// -- Shape and layout --
void TileSet::set_tile_shape(TileSet::TileShape p_shape) {
	tile_shape = p_shape;
	emit_changed();
}
TileSet::TileShape TileSet::get_tile_shape() const {
	return tile_shape;
}

void TileSet::set_tile_layout(TileSet::TileLayout p_layout) {
	tile_layout = p_layout;
	emit_changed();
}
TileSet::TileLayout TileSet::get_tile_layout() const {
	return tile_layout;
}

void TileSet::set_tile_offset_axis(TileSet::TileOffsetAxis p_alignment) {
	tile_offset_axis = p_alignment;
	emit_changed();
}
TileSet::TileOffsetAxis TileSet::get_tile_offset_axis() const {
	return tile_offset_axis;
}

void TileSet::set_tile_size(Size2i p_size) {
	ERR_FAIL_COND(p_size.x < 1 || p_size.y < 1);
	tile_size = p_size;
	emit_changed();
}
Size2i TileSet::get_tile_size() const {
	return tile_size;
}

void TileSet::set_tile_skew(Vector2 p_skew) {
	emit_changed();
	tile_skew = p_skew;
}
Vector2 TileSet::get_tile_skew() const {
	return tile_skew;
}

TileSet::SourceType TileSet::get_source_type(int p_source_id) {
	if (source_atlas.has(p_source_id)) {
		return TileSet::SOURCE_TYPE_ATLAS;
	}
	if (source_scenes.has(p_source_id)) {
		return TileSet::SOURCE_TYPE_ATLAS;
	}
	return TileSet::SOURCE_TYPE_INVALID;
}

int TileSet::get_next_source_id() {
	return next_source_id;
}

void TileSet::_compute_next_source_id() {
	while (source_atlas.has(next_source_id) || source_scenes.has(next_source_id)) {
		next_source_id = (next_source_id + 1) % 1073741824; // 2 ** 30
	};
}

// Atlases
int TileSet::add_atlas_source(int p_atlas_source_id_override) {
	ERR_FAIL_COND_V_MSG(p_atlas_source_id_override >= 0 && (source_atlas.has(p_atlas_source_id_override) || source_scenes.has(p_atlas_source_id_override)), -1, vformat("Cannot create TileSet atlas source. Another atlas source exists with id %d.", p_atlas_source_id_override));

	int new_source_id = p_atlas_source_id_override >= 0 ? p_atlas_source_id_override : next_source_id;
	source_atlas[new_source_id] = memnew(TileAtlasSource);
	source_atlas[new_source_id]->set_texture_region_size(get_tile_size());
	source_atlas_ids.append(new_source_id);
	source_atlas_ids.sort();
	_compute_next_source_id();

	source_atlas[new_source_id]->connect("changed", callable_mp(this, &TileSet::_source_changed));

	emit_changed();

	return new_source_id;
}

void TileSet::remove_atlas_source(int p_source_id) {
	ERR_FAIL_COND_MSG(!source_atlas.has(p_source_id), vformat("Cannot remove TileSet atlas source. No tileset atlas source with id %d.", p_source_id));

	source_atlas[p_source_id]->disconnect("changed", callable_mp(this, &TileSet::_source_changed));

	memdelete(source_atlas[p_source_id]);
	source_atlas.erase(p_source_id);
	source_atlas_ids.erase(p_source_id);
	source_atlas_ids.sort();

	emit_changed();
}

void TileSet::set_atlas_source_id(int p_source_id, int p_new_source_id) {
	ERR_FAIL_COND(p_new_source_id < 0);
	ERR_FAIL_COND_MSG(!source_atlas.has(p_source_id), vformat("Cannot change TileSet atlas source ID. No tileset atlas source with id %d.", p_source_id));
	if (p_source_id == p_new_source_id) {
		return;
	}

	ERR_FAIL_COND_MSG(source_atlas.has(p_new_source_id), vformat("Cannot change TileSet atlas source ID. Another atlas source exists with id %d.", p_new_source_id));

	TileAtlasSource *tas = source_atlas[p_source_id];
	source_atlas[p_new_source_id] = tas;
	source_atlas.erase(p_source_id);

	source_atlas_ids.erase(p_source_id);
	source_atlas_ids.append(p_new_source_id);
	source_atlas_ids.sort();

	emit_changed();
}

bool TileSet::has_atlas_source(int p_source_id) const {
	return source_atlas.has(p_source_id);
}

TileAtlasSource *TileSet::get_atlas_source(int p_source_id) const {
	ERR_FAIL_COND_V_MSG(!source_atlas.has(p_source_id), nullptr, vformat("No TileSet atlas source with id %d.", p_source_id));

	return source_atlas[p_source_id];
}

int TileSet::get_atlas_source_count() const {
	return source_atlas_ids.size();
}

int TileSet::get_atlas_source_id(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, source_atlas_ids.size(), -1);
	return source_atlas_ids[p_index];
}

// Alternatives tiles.
int TileAtlasSource::create_alternative_tile(const Vector2i p_atlas_coords, int p_alternative_id_override) {
	ERR_FAIL_COND_V_MSG(!tiles.has(p_atlas_coords), -1, vformat("TileAtlasSource has no tile at %s.", String(p_atlas_coords)));
	ERR_FAIL_COND_V_MSG(p_alternative_id_override >= 0 && (tiles[p_atlas_coords].alternatives.has(p_alternative_id_override) || tiles[p_atlas_coords].alternatives.has(p_alternative_id_override)), -1, vformat("Cannot create alternative tile. Another alternative exists with id %d.", p_alternative_id_override));

	int new_alternative_id = p_alternative_id_override >= 0 ? p_alternative_id_override : tiles[p_atlas_coords].next_alternative_id;

	tiles[p_atlas_coords].alternatives[new_alternative_id] = memnew(TileData);
	tiles[p_atlas_coords].alternatives_ids.append(new_alternative_id);
	tiles[p_atlas_coords].alternatives_ids.sort();
	_compute_next_alternative_id(p_atlas_coords);

	emit_signal("changed");

	return new_alternative_id;
}

void TileAtlasSource::remove_alternative_tile(const Vector2i p_atlas_coords, int p_alternative_tile) {
	ERR_FAIL_COND_MSG(!tiles.has(p_atlas_coords), vformat("TileAtlasSource has no tile at %s.", String(p_atlas_coords)));
	ERR_FAIL_COND_MSG(!tiles[p_atlas_coords].alternatives.has(p_alternative_tile), vformat("TileAtlasSource has no alternative with id %d for tile coords %s.", p_alternative_tile, String(p_atlas_coords)));
	ERR_FAIL_COND_MSG(p_alternative_tile == 0, "Cannot remove the alternative with id 0, the base tile alternative cannot be removed.");

	memdelete(tiles[p_atlas_coords].alternatives[p_alternative_tile]);
	tiles[p_atlas_coords].alternatives.erase(p_alternative_tile);
	tiles[p_atlas_coords].alternatives_ids.erase(p_alternative_tile);
	tiles[p_atlas_coords].alternatives_ids.sort();

	emit_signal("changed");
}

void TileAtlasSource::set_alternative_tile_id(const Vector2i p_atlas_coords, int p_alternative_tile, int p_new_id) {
	ERR_FAIL_COND_MSG(!tiles.has(p_atlas_coords), vformat("TileAtlasSource has no tile at %s.", String(p_atlas_coords)));
	ERR_FAIL_COND_MSG(!tiles[p_atlas_coords].alternatives.has(p_alternative_tile), vformat("TileAtlasSource has no alternative with id %d for tile coords %s.", p_alternative_tile, String(p_atlas_coords)));
	ERR_FAIL_COND_MSG(p_alternative_tile == 0, "Cannot change the alternative with id 0, the base tile alternative cannot be modified.");

	ERR_FAIL_COND_MSG(tiles[p_atlas_coords].alternatives.has(p_new_id), vformat("TileAtlasSource has already an alternative with id %d at %s.", p_new_id, String(p_atlas_coords)));

	tiles[p_atlas_coords].alternatives[p_new_id] = tiles[p_atlas_coords].alternatives[p_alternative_tile];
	tiles[p_atlas_coords].alternatives_ids.append(p_new_id);

	tiles[p_atlas_coords].alternatives.erase(p_alternative_tile);
	tiles[p_atlas_coords].alternatives_ids.erase(p_alternative_tile);
	tiles[p_atlas_coords].alternatives_ids.sort();

	emit_signal("changed");
}

bool TileAtlasSource::has_alternative_tile(const Vector2i p_atlas_coords, int p_alternative_tile) const {
	return tiles[p_atlas_coords].alternatives.has(p_alternative_tile);
}

int TileAtlasSource::get_next_alternative_tile_id(const Vector2i p_atlas_coords) const {
	ERR_FAIL_COND_V_MSG(!tiles.has(p_atlas_coords), -1, vformat("The TileAtlasSource atlas has no tile at %s.", String(p_atlas_coords)));
	return tiles[p_atlas_coords].next_alternative_id;
}

int TileAtlasSource::get_alternative_tiles_count(const Vector2i p_atlas_coords) const {
	ERR_FAIL_COND_V_MSG(!tiles.has(p_atlas_coords), -1, vformat("The TileAtlasSource atlas has no tile at %s.", String(p_atlas_coords)));
	return tiles[p_atlas_coords].alternatives_ids.size();
}

int TileAtlasSource::get_alternative_tile_id(const Vector2i p_atlas_coords, int p_index) const {
	ERR_FAIL_COND_V_MSG(!tiles.has(p_atlas_coords), -1, vformat("The TileAtlasSource atlas has no tile at %s.", String(p_atlas_coords)));
	ERR_FAIL_INDEX_V(p_index, tiles[p_atlas_coords].alternatives_ids.size(), -1);

	return tiles[p_atlas_coords].alternatives_ids[p_index];
}

TileData *TileAtlasSource::get_tile_data(const Vector2i p_atlas_coords, int p_alternative_tile) const {
	ERR_FAIL_COND_V_MSG(!tiles.has(p_atlas_coords), nullptr, vformat("The TileAtlasSource atlas has no tile at %s.", String(p_atlas_coords)));
	ERR_FAIL_COND_V_MSG(!tiles[p_atlas_coords].alternatives.has(p_alternative_tile), nullptr, vformat("TileAtlasSource has no alternative with id %d for tile coords %s.", p_alternative_tile, String(p_atlas_coords)));

	return tiles[p_atlas_coords].alternatives[p_alternative_tile];
}

// Rendering
void TileSet::set_uv_clipping(bool p_uv_clipping) {
	rendering.uv_clipping = p_uv_clipping;
	emit_changed();
}
bool TileSet::is_uv_clipping() const {
	return rendering.uv_clipping;
};

void TileSet::set_y_sorting(bool p_y_sort) {
	rendering.y_sorting = p_y_sort;
	emit_changed();
}
bool TileSet::is_y_sorting() const {
	return rendering.y_sorting;
};

void TileSet::draw_tile_shape(Control *p_control, Rect2 p_region, Color p_color, bool p_filled, Ref<Texture2D> p_texture) {
	// TODO: optimize this with 2D meshes when they work again.
	if (get_tile_shape() == TileSet::TILE_SHAPE_SQUARE) {
		if (p_filled && p_texture.is_valid()) {
			p_control->draw_texture_rect(p_texture, p_region, false, p_color);
		} else {
			p_control->draw_rect(p_region, p_color, p_filled);
		}
	} else {
		float overlap = 0.0;
		switch (get_tile_shape()) {
			case TileSet::TILE_SHAPE_ISOMETRIC:
				overlap = 0.5;
				break;
			case TileSet::TILE_SHAPE_HEXAGON:
				overlap = 0.25;
				break;
			case TileSet::TILE_SHAPE_HALF_OFFSET_SQUARE:
				overlap = 0.0;
				break;
			default:
				break;
		}
		Vector<Vector2> points;
		Vector<Vector2> uvs;
		if (get_tile_offset_axis() == TileSet::TILE_OFFSET_AXIS_HORIZONTAL) {
			uvs.append(Vector2(0.5, 0.0));
			uvs.append(Vector2(0.0, overlap));
			uvs.append(Vector2(0.0, 1.0 - overlap));
			uvs.append(Vector2(0.5, 1.0));
			uvs.append(Vector2(1.0, 1.0 - overlap));
			uvs.append(Vector2(1.0, overlap));
			uvs.append(Vector2(0.5, 0.0));

			for (int i = 0; i < uvs.size(); i++) {
				points.append(p_region.position + uvs[i] * p_region.size);
			}

		} else { // TileSet::TILE_OFFSET_AXIS_VERTICAL
			uvs.append(Vector2(0.0, 0.5));
			uvs.append(Vector2(overlap, 0.0));
			uvs.append(Vector2(1.0 - overlap, 0.0));
			uvs.append(Vector2(1.0, 0.5));
			uvs.append(Vector2(1.0 - overlap, 1.0));
			uvs.append(Vector2(overlap, 1.0));
			uvs.append(Vector2(0.0, 0.5));

			for (int i = 0; i < uvs.size(); i++) {
				points.append(p_region.position + uvs[i] * p_region.size);
			}
		}
		if (p_filled) {
			// This does hurt performances a lot. We should use a mesh if possible instead.
			p_control->draw_colored_polygon(points, p_color, uvs, p_texture);

			// Should improve performances, but does not work as draw_primitive does not work with textures :/ :
			/*for (int i = 0; i < 6; i += 3) {
				Vector<Vector2> quad;
				quad.append(points[i]);
				quad.append(points[(i + 1) % points.size()]);
				quad.append(points[(i + 2) % points.size()]);
				quad.append(points[(i + 3) % points.size()]);

				Vector<Vector2> uv_quad;
				uv_quad.append(uvs[i]);
				uv_quad.append(uvs[(i + 1) % uvs.size()]);
				uv_quad.append(uvs[(i + 2) % uvs.size()]);
				uv_quad.append(uvs[(i + 3) % uvs.size()]);

				p_control->draw_primitive(quad, Vector<Color>(), uv_quad, p_texture);
			}*/

		} else {
			// This does hurt performances a lot. We should use a mesh if possible instead.
			// tile_shape_grid->draw_polyline(points, p_color);
			for (int i = 0; i < points.size() - 1; i++) {
				p_control->draw_line(points[i], points[i + 1], p_color);
			}
		}
	}
}

Vector2i TileSet::get_tile_effective_texture_offset(int p_atlas_source_id, Vector2i p_atlas_coords, int p_alternative_tile) const {
	ERR_FAIL_COND_V_MSG(!source_atlas.has(p_atlas_source_id), Vector2i(), vformat("The TileSet has no atlas with id %d", p_atlas_source_id));
	ERR_FAIL_COND_V_MSG(!source_atlas[p_atlas_source_id]->has_tile(p_atlas_coords), Vector2i(), vformat("The TileSet atlas source with id %d has no tile at %s.", p_atlas_source_id, String(p_atlas_coords)));
	ERR_FAIL_COND_V_MSG(!source_atlas[p_atlas_source_id]->has_alternative_tile(p_atlas_coords, p_alternative_tile), Vector2i(), vformat("The TileSet atlas source with id %d has no alternative tile with id %d at %s.", p_atlas_source_id, p_alternative_tile, String(p_atlas_coords)));

	Vector2 margin = (source_atlas[p_atlas_source_id]->get_tile_texture_region(p_atlas_coords).size - get_tile_size()) / 2;
	margin = Vector2i(MAX(0, margin.x), MAX(0, margin.y));
	Vector2i effective_texture_offset = source_atlas[p_atlas_source_id]->get_base_texture_offset() + source_atlas[p_atlas_source_id]->get_tile_data(p_atlas_coords, p_alternative_tile)->tile_get_texture_offset();
	if (ABS(effective_texture_offset.x) > margin.x || ABS(effective_texture_offset.y) > margin.y) {
		effective_texture_offset.x = CLAMP(effective_texture_offset.x, -margin.x, margin.x);
		effective_texture_offset.y = CLAMP(effective_texture_offset.y, -margin.y, margin.y);
	}

	return effective_texture_offset;
}

void TileSet::_source_changed() {
	emit_changed();
}

void TileSet::_bind_methods() {
	// -- Shape and layout --
	ClassDB::bind_method(D_METHOD("set_tile_shape", "shape"), &TileSet::set_tile_shape);
	ClassDB::bind_method(D_METHOD("get_tile_shape"), &TileSet::get_tile_shape);
	ClassDB::bind_method(D_METHOD("set_tile_layout", "layout"), &TileSet::set_tile_layout);
	ClassDB::bind_method(D_METHOD("get_tile_layout"), &TileSet::get_tile_layout);
	ClassDB::bind_method(D_METHOD("set_tile_offset_axis", "alignment"), &TileSet::set_tile_offset_axis);
	ClassDB::bind_method(D_METHOD("get_tile_offset_axis"), &TileSet::get_tile_offset_axis);
	ClassDB::bind_method(D_METHOD("set_tile_size", "size"), &TileSet::set_tile_size);
	ClassDB::bind_method(D_METHOD("get_tile_size"), &TileSet::get_tile_size);
	ClassDB::bind_method(D_METHOD("set_tile_skew", "skew"), &TileSet::set_tile_skew);
	ClassDB::bind_method(D_METHOD("get_tile_skew"), &TileSet::get_tile_skew);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "tile_shape", PROPERTY_HINT_ENUM, "Square,Isometric,Half-offset square,Hexagon"), "set_tile_shape", "get_tile_shape");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "tile_layout", PROPERTY_HINT_ENUM, "Stacked,Stacked Offset,Stairs Right,Stairs Down,Diamond Right,Diamond Down"), "set_tile_layout", "get_tile_layout");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "tile_offset_axis", PROPERTY_HINT_ENUM, "Horizontal Offset,Vertical Offset"), "set_tile_offset_axis", "get_tile_offset_axis");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "tile_size"), "set_tile_size", "get_tile_size");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "tile_skew"), "set_tile_skew", "get_tile_skew");

	// -- Sources management --
	ClassDB::bind_method(D_METHOD("get_source_type", "source_id"), &TileSet::get_source_type);
	ClassDB::bind_method(D_METHOD("get_next_source_id"), &TileSet::get_next_source_id);

	// Atlases
	ClassDB::bind_method(D_METHOD("add_atlas_source", "atlas_source_id_override"), &TileSet::add_atlas_source, DEFVAL(-1));
	ClassDB::bind_method(D_METHOD("remove_atlas_source", "source_id"), &TileSet::remove_atlas_source);
	ClassDB::bind_method(D_METHOD("set_atlas_source_id", "source_id"), &TileSet::set_atlas_source_id);
	ClassDB::bind_method(D_METHOD("get_atlas_source_count"), &TileSet::get_atlas_source_count);
	ClassDB::bind_method(D_METHOD("get_atlas_source_id", "index"), &TileSet::get_atlas_source_id);

	// -- Enum binding --
	BIND_ENUM_CONSTANT(SOURCE_TYPE_INVALID);
	BIND_ENUM_CONSTANT(SOURCE_TYPE_ATLAS);
	BIND_ENUM_CONSTANT(SOURCE_TYPE_SCENE);

	BIND_ENUM_CONSTANT(TILE_SHAPE_SQUARE);
	BIND_ENUM_CONSTANT(TILE_SHAPE_ISOMETRIC);
	BIND_ENUM_CONSTANT(TILE_SHAPE_HALF_OFFSET_SQUARE);
	BIND_ENUM_CONSTANT(TILE_SHAPE_HEXAGON);

	BIND_ENUM_CONSTANT(TILE_LAYOUT_STACKED);
	BIND_ENUM_CONSTANT(TILE_LAYOUT_STACKED_OFFSET);
	BIND_ENUM_CONSTANT(TILE_LAYOUT_STAIRS_RIGHT);
	BIND_ENUM_CONSTANT(TILE_LAYOUT_STAIRS_DOWN);
	BIND_ENUM_CONSTANT(TILE_LAYOUT_DIAMOND_RIGHT);
	BIND_ENUM_CONSTANT(TILE_LAYOUT_DIAMOND_DOWN);
}

TileSet::TileSet() {
	// List all plugins.
	tile_set_plugins_vector.append(&tile_set_plugin_rendering);

	for (int i = 0; i < tile_set_plugins_vector.size(); i++) {
		tile_set_plugins[tile_set_plugins_vector[i]->get_id()] = tile_set_plugins_vector[i];
	}
}

TileSet::~TileSet() {
	for (Map<int, CompatibilityTileData *>::Element *E = compatibility_data.front(); E; E = E->next()) {
		memdelete(E->get());
	}
	while (!source_atlas_ids.is_empty()) {
		remove_atlas_source(source_atlas_ids[0]);
	}
}
