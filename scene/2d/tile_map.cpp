/*************************************************************************/
/*  tile_map.cpp                                                         */
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

#include "tile_map.h"

#include "collision_object_2d.h"
#include "core/io/marshalls.h"
#include "core/os/os.h"
#include "scene/2d/area_2d.h"
#include "servers/navigation_server_2d.h"
#include "servers/physics_server_2d.h"

int TileMap::_get_quadrant_size() const {
	// YSort: The wuadrant is reduced to 1 to have one CanvasItem per quadrant
	if (use_y_sort) {
		return 1;
	} else {
		return quadrant_size;
	}
}

void TileMap::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			pending_update = true;
			_recreate_quadrants();
			update_dirty_quadrants();
			RID space = get_world_2d()->get_space();
			_update_quadrant_transform();
			_update_quadrant_space(space);
			update_configuration_warnings();

		} break;

		case NOTIFICATION_EXIT_TREE: {
			_update_quadrant_space(RID());
			for (Map<PosKey, Quadrant>::Element *E = quadrant_map.front(); E; E = E->next()) {
				Quadrant &q = E->get();
				for (Map<PosKey, Quadrant::NavPoly>::Element *F = q.navpoly_ids.front(); F; F = F->next()) {
					NavigationServer2D::get_singleton()->region_set_map(F->get().region, RID());
				}
				q.navpoly_ids.clear();

				for (Map<PosKey, Quadrant::Occluder>::Element *F = q.occluder_instances.front(); F; F = F->next()) {
					RS::get_singleton()->free(F->get().id);
				}
				q.occluder_instances.clear();
			}

		} break;

		case NOTIFICATION_TRANSFORM_CHANGED: {
			//move stuff
			_update_quadrant_transform();

		} break;
	}
}

void TileMap::_update_quadrant_space(const RID &p_space) {
	// Collisions: update the 2D space for collisions
	for (Map<PosKey, Quadrant>::Element *E = quadrant_map.front(); E; E = E->next()) {
		Quadrant &q = E->get();
		PhysicsServer2D::get_singleton()->body_set_space(q.body, p_space);
	}
}

void TileMap::_update_quadrant_transform() {
	if (!is_inside_tree()) {
		return;
	}

	Transform2D global_transform = get_global_transform();

	for (Map<PosKey, Quadrant>::Element *E = quadrant_map.front(); E; E = E->next()) {
		Quadrant &q = E->get();
		Transform2D xform;
		xform.set_origin(q.pos);

		xform = global_transform * xform;
		PhysicsServer2D::get_singleton()->body_set_state(q.body, PhysicsServer2D::BODY_STATE_TRANSFORM, xform);

		if (bake_navigation) {
			for (Map<PosKey, Quadrant::NavPoly>::Element *F = q.navpoly_ids.front(); F; F = F->next()) {
				NavigationServer2D::get_singleton()->region_set_transform(F->get().region, F->get().xform);
			}
		}

		for (Map<PosKey, Quadrant::Occluder>::Element *F = q.occluder_instances.front(); F; F = F->next()) {
			RS::get_singleton()->canvas_light_occluder_set_transform(F->get().id, global_transform * F->get().xform);
		}
	}
}

Ref<TileSet> TileMap::get_tileset() const {
	return tile_set;
}

void TileMap::set_tileset(const Ref<TileSet> &p_tileset) {
	// Set the tileset, registering to its changes.
	if (tile_set.is_valid()) {
		tile_set->disconnect("changed", callable_mp(this, &TileMap::_recreate_quadrants));
	}

	_clear_quadrants();
	tile_set = p_tileset;

	if (tile_set.is_valid()) {
		tile_set->connect("changed", callable_mp(this, &TileMap::_recreate_quadrants));
	} else {
		clear();
	}

	_recreate_quadrants();
	emit_signal("settings_changed");
}

Size2 TileMap::get_cell_size() const {
	return cell_size;
}

void TileMap::set_cell_size(Size2 p_size) {
	ERR_FAIL_COND(p_size.x < 1 || p_size.y < 1);

	_clear_quadrants();
	cell_size = p_size;
	_recreate_quadrants();
	emit_signal("settings_changed");
}

int TileMap::get_quadrant_size() const {
	return quadrant_size;
}

void TileMap::set_quadrant_size(int p_size) {
	ERR_FAIL_COND_MSG(p_size < 1, "Quadrant size cannot be smaller than 1.");

	_clear_quadrants();
	quadrant_size = p_size;
	_recreate_quadrants();
	emit_signal("settings_changed");
}

void TileMap::_fix_cell_transform(Transform2D &xform, const Cell &p_cell, const Vector2 &p_offset, const Size2 &p_sc) {
	Size2 s = p_sc;
	Vector2 offset = p_offset;

	// Flip/transpose: update the tile transform.
	if (tile_set->tile_get_transpose(p_cell.source_id, p_cell.get_tileset_coords(), p_cell.alternative_tile)) {
		SWAP(xform.elements[0].x, xform.elements[0].y);
		SWAP(xform.elements[1].x, xform.elements[1].y);
		SWAP(offset.x, offset.y);
		SWAP(s.x, s.y);
	}

	if (tile_set->tile_get_flip_h(p_cell.source_id, p_cell.get_tileset_coords(), p_cell.alternative_tile)) {
		xform.elements[0].x = -xform.elements[0].x;
		xform.elements[1].x = -xform.elements[1].x;
		offset.x = s.x - offset.x;
	}

	if (tile_set->tile_get_flip_v(p_cell.source_id, p_cell.get_tileset_coords(), p_cell.alternative_tile)) {
		xform.elements[0].y = -xform.elements[0].y;
		xform.elements[1].y = -xform.elements[1].y;
		offset.y = s.y - offset.y;
	}

	xform.elements[2] += offset;
}

void TileMap::_add_shape(int &shape_idx, const Quadrant &p_q, const Ref<Shape2D> &p_shape, const TileSet::ShapeData &p_shape_data, const Transform2D &p_xform, const Vector2 &p_metadata) {
	// Collisions: add a collision shape
	PhysicsServer2D *ps = PhysicsServer2D::get_singleton();

	ps->body_add_shape(p_q.body, p_shape->get_rid(), p_xform);
	ps->body_set_shape_metadata(p_q.body, shape_idx, p_metadata);
	ps->body_set_shape_as_one_way_collision(p_q.body, shape_idx, p_shape_data.one_way_collision, p_shape_data.one_way_collision_margin);

	shape_idx++;
}

void TileMap::update_dirty_quadrants() {
	if (!pending_update) {
		return;
	}
	if (!is_inside_tree() || !tile_set.is_valid()) {
		pending_update = false;
		return;
	}

	// Draw offset.
	RenderingServer *vs = RenderingServer::get_singleton();
	Vector2 tofs = get_cell_draw_offset();

	// Collision: get the physics server.
	PhysicsServer2D *ps = PhysicsServer2D::get_singleton();

	SceneTree *st = SceneTree::get_singleton();

	// Collisions: color for debug.
	Color debug_collision_color;
	bool debug_shapes = st && st->is_debugging_collisions_hint();
	if (debug_shapes) {
		debug_collision_color = st->get_debug_collisions_color();
	}

	// Navigation: color for debug.
	Color debug_navigation_color;
	bool debug_navigation = st && st->is_debugging_navigation_hint();
	if (debug_navigation) {
		debug_navigation_color = st->get_debug_navigation_color();
	}

	// Update the dirty quadrants, removing them from dirty_quadrant_list.
	while (dirty_quadrant_list.first()) {
		Quadrant &q = *dirty_quadrant_list.first()->self();

		// Free all canvas items in the quadrant.
		for (List<RID>::Element *E = q.canvas_items.front(); E; E = E->next()) {
			vs->free(E->get());
		}
		q.canvas_items.clear();

		// Collisions: Clear shapes in the quadrant.
		ps->body_clear_shapes(q.body);

		for (Map<PosKey, Quadrant::NavPoly>::Element *E = q.navpoly_ids.front(); E; E = E->next()) {
			NavigationServer2D::get_singleton()->region_set_map(E->get().region, RID());
		}
		q.navpoly_ids.clear();

		// Occlusion: Clear occlusion shapes in the quadrant.
		for (Map<PosKey, Quadrant::Occluder>::Element *E = q.occluder_instances.front(); E; E = E->next()) {
			RS::get_singleton()->free(E->get().id);
		}
		q.occluder_instances.clear();

		Ref<ShaderMaterial> prev_material;
		int prev_z_index = 0;
		RID prev_canvas_item;
		RID prev_debug_canvas_item;
		int shape_idx = 0;

		// Iterate over the cells of the quadrant.
		for (int i = 0; i < q.cells.size(); i++) {
			Map<PosKey, Cell>::Element *E = tile_map.find(q.cells[i]);

			Cell &c = E->get();

			// Check if the tilset has a tile with the given ID, otherwise, ignore it.
			if (!tile_set->has_tile(c.source_id, c.get_tileset_coords(), c.alternative_tile)) {
				continue;
			}

			// Get the texture.
			Ref<Texture2D> tex = tile_set->tile_get_texture(c.source_id);
			if (!tex.is_valid()) {
				continue;
			}

			// Get the texture offset.
			Vector2 tile_ofs = tile_set->tile_get_texture_offset(c.source_id);

			// Compute the offset ?
			Vector2 wofs = _map_to_world(E->key().x, E->key().y);
			Vector2 offset = wofs - q.pos + tofs;

			// Get the material.
			Ref<ShaderMaterial> mat = tile_set->tile_get_material(c.source_id);

			// Get the Z-index.
			int z_index = tile_set->tile_get_z_index(c.source_id);
			if (tile_set->tile_get_tile_mode(c.source_id) == TileSet::AUTO_TILE || tile_set->tile_get_tile_mode(c.source_id) == TileSet::ATLAS_TILE) {
				z_index += tile_set->autotile_get_z_index(c.source_id, c.get_tileset_coords());
			}

			// Create two canvas items, for rendering and debug.
			RID canvas_item;
			RID debug_canvas_item;

			// Check if the material or the z_index changed.
			if (prev_canvas_item == RID() || prev_material != mat || prev_z_index != z_index) {
				canvas_item = vs->canvas_item_create();
				if (mat.is_valid()) {
					vs->canvas_item_set_material(canvas_item, mat->get_rid());
				}
				vs->canvas_item_set_parent(canvas_item, get_canvas_item());
				_update_item_material_state(canvas_item);
				Transform2D xform;
				xform.set_origin(q.pos);
				vs->canvas_item_set_transform(canvas_item, xform);
				vs->canvas_item_set_light_mask(canvas_item, get_light_mask());
				vs->canvas_item_set_z_index(canvas_item, z_index);

				vs->canvas_item_set_default_texture_filter(canvas_item, RS::CanvasItemTextureFilter(CanvasItem::get_texture_filter()));
				vs->canvas_item_set_default_texture_repeat(canvas_item, RS::CanvasItemTextureRepeat(CanvasItem::get_texture_repeat()));

				q.canvas_items.push_back(canvas_item);

				// Debug canvas item, drawn on top of the normal one.
				if (debug_shapes) {
					debug_canvas_item = vs->canvas_item_create();
					vs->canvas_item_set_parent(debug_canvas_item, canvas_item);
					vs->canvas_item_set_z_as_relative_to_parent(debug_canvas_item, false);
					vs->canvas_item_set_z_index(debug_canvas_item, RS::CANVAS_ITEM_Z_MAX - 1);
					q.canvas_items.push_back(debug_canvas_item);
					prev_debug_canvas_item = debug_canvas_item;
				}

				prev_canvas_item = canvas_item;
				prev_material = mat;
				prev_z_index = z_index;

			} else {
				// Keep the same canvas_item to draw on.
				canvas_item = prev_canvas_item;
				if (debug_shapes) {
					debug_canvas_item = prev_debug_canvas_item;
				}
			}

			// Get the tile region in the tileset, if it is defined.
			Rect2 r = tile_set->tile_get_region(c.source_id);
			if (tile_set->tile_get_tile_mode(c.source_id) == TileSet::AUTO_TILE || tile_set->tile_get_tile_mode(c.source_id) == TileSet::ATLAS_TILE) {
				int spacing = tile_set->autotile_get_spacing(c.source_id);
				r.size = tile_set->autotile_get_size(c.source_id);
				r.position += (r.size + Vector2(spacing, spacing)) * c.get_tileset_coords();
			}

			// Get the texture size.
			Size2 s;
			if (r == Rect2()) {
				s = tex->get_size(); // No region, use the full texture.
			} else {
				s = r.size; // Region, use the region size.
			}

			bool transpose = tile_set->tile_get_transpose(c.source_id, c.get_tileset_coords(), c.alternative_tile);

			// Compute the rectangle in the texture.
			Rect2 rect;
			rect.position = offset.floor();
			rect.size = s;
			rect.size.x += fp_adjust;
			rect.size.y += fp_adjust;
			if (transpose) {
				SWAP(tile_ofs.x, tile_ofs.y);
			}

			if (tile_set->tile_get_flip_h(c.source_id, c.get_tileset_coords(), c.alternative_tile)) {
				rect.size.x = -rect.size.x;
				tile_ofs.x = -tile_ofs.x;
			}

			if (tile_set->tile_get_flip_v(c.source_id, c.get_tileset_coords(), c.alternative_tile)) {
				rect.size.y = -rect.size.y;
				tile_ofs.y = -tile_ofs.y;
			}

			rect.position += tile_ofs;

			// Get the tile modulation.
			Color modulate = tile_set->tile_get_modulate(c.source_id);
			Color self_modulate = get_self_modulate();
			modulate = Color(modulate.r * self_modulate.r, modulate.g * self_modulate.g, modulate.b * self_modulate.b, modulate.a * self_modulate.a);

			// Draw the tile.
			if (r == Rect2()) {
				tex->draw_rect(canvas_item, rect, false, modulate, transpose);
			} else {
				tex->draw_rect_region(canvas_item, rect, r, modulate, transpose, clip_uv);
			}

			// Collisions: get the tilset collision shapes, and add the shape.
			Vector<TileSet::ShapeData> shapes = tile_set->tile_get_shapes(c.source_id);
			for (int j = 0; j < shapes.size(); j++) {
				Ref<Shape2D> shape = shapes[j].shape;
				if (shape.is_valid()) {
					if (tile_set->tile_get_tile_mode(c.source_id) == TileSet::SINGLE_TILE || (shapes[j].autotile_coord.x == c.coord_x && shapes[j].autotile_coord.y == c.coord_y)) {
						Transform2D xform;
						xform.set_origin(offset.floor());

						Vector2 shape_ofs = shapes[j].shape_transform.get_origin();

						_fix_cell_transform(xform, c, shape_ofs, s);

						xform *= shapes[j].shape_transform.untranslated();

						// Draw the debug shape.
						if (debug_canvas_item.is_valid()) {
							vs->canvas_item_add_set_transform(debug_canvas_item, xform);
							shape->draw(debug_canvas_item, debug_collision_color);
						}

						// Add the shape from the one in the tileset.
						if (shape->has_meta("decomposed")) {
							Array _shapes = shape->get_meta("decomposed");
							for (int k = 0; k < _shapes.size(); k++) {
								Ref<ConvexPolygonShape2D> convex = _shapes[k];
								if (convex.is_valid()) {
									_add_shape(shape_idx, q, convex, shapes[j], xform, Vector2(E->key().x, E->key().y));
#ifdef DEBUG_ENABLED
								} else {
									print_error("The TileSet assigned to the TileMap " + get_name() + " has an invalid convex shape.");
#endif
								}
							}
						} else {
							_add_shape(shape_idx, q, shape, shapes[j], xform, Vector2(E->key().x, E->key().y));
						}
					}
				}
			}

			// Change the debug_canvas_item transform ?
			if (debug_canvas_item.is_valid()) {
				vs->canvas_item_add_set_transform(debug_canvas_item, Transform2D());
			}

			// Navigation: handle navigation shapes.
			if (bake_navigation) {
				// Get the navigation polygon.
				Ref<NavigationPolygon> navpoly;
				Vector2 npoly_ofs;
				if (tile_set->tile_get_tile_mode(c.source_id) == TileSet::AUTO_TILE || tile_set->tile_get_tile_mode(c.source_id) == TileSet::ATLAS_TILE) {
					navpoly = tile_set->autotile_get_navigation_polygon(c.source_id, c.get_tileset_coords());
					npoly_ofs = Vector2();
				} else {
					navpoly = tile_set->tile_get_navigation_polygon(c.source_id);
					npoly_ofs = tile_set->tile_get_navigation_polygon_offset(c.source_id);
				}

				if (navpoly.is_valid()) {
					Transform2D xform;
					xform.set_origin(offset.floor() + q.pos);
					_fix_cell_transform(xform, c, npoly_ofs, s);

					RID region = NavigationServer2D::get_singleton()->region_create();
					NavigationServer2D::get_singleton()->region_set_map(region, get_world_2d()->get_navigation_map());
					NavigationServer2D::get_singleton()->region_set_transform(region, xform);
					NavigationServer2D::get_singleton()->region_set_navpoly(region, navpoly);

					Quadrant::NavPoly np;
					np.region = region;
					np.xform = xform;
					q.navpoly_ids[E->key()] = np;

					// Diplay debug info.
					if (debug_navigation) {
						RID debug_navigation_item = vs->canvas_item_create();
						vs->canvas_item_set_parent(debug_navigation_item, canvas_item);
						vs->canvas_item_set_z_as_relative_to_parent(debug_navigation_item, false);
						vs->canvas_item_set_z_index(debug_navigation_item, RS::CANVAS_ITEM_Z_MAX - 2); // Display one below collision debug

						if (debug_navigation_item.is_valid()) {
							Vector<Vector2> navigation_polygon_vertices = navpoly->get_vertices();
							int vsize = navigation_polygon_vertices.size();

							if (vsize > 2) {
								Vector<Color> colors;
								Vector<Vector2> vertices;
								vertices.resize(vsize);
								colors.resize(vsize);
								{
									const Vector2 *vr = navigation_polygon_vertices.ptr();
									for (int j = 0; j < vsize; j++) {
										vertices.write[j] = vr[j];
										colors.write[j] = debug_navigation_color;
									}
								}

								Vector<int> indices;

								for (int j = 0; j < navpoly->get_polygon_count(); j++) {
									Vector<int> polygon = navpoly->get_polygon(j);

									for (int k = 2; k < polygon.size(); k++) {
										int kofs[3] = { 0, k - 1, k };
										for (int l = 0; l < 3; l++) {
											int idx = polygon[kofs[l]];
											ERR_FAIL_INDEX(idx, vsize);
											indices.push_back(idx);
										}
									}
								}
								Transform2D navxform;
								navxform.set_origin(offset.floor());
								_fix_cell_transform(navxform, c, npoly_ofs, s);

								vs->canvas_item_set_transform(debug_navigation_item, navxform);
								vs->canvas_item_add_triangle_array(debug_navigation_item, indices, vertices, colors);
							}
						}
					}
				}
			}

			// Occlusion: handle occluder shape.
			Ref<OccluderPolygon2D> occluder;
			if (tile_set->tile_get_tile_mode(c.source_id) == TileSet::AUTO_TILE || tile_set->tile_get_tile_mode(c.source_id) == TileSet::ATLAS_TILE) {
				occluder = tile_set->autotile_get_light_occluder(c.source_id, c.get_tileset_coords());
			} else {
				occluder = tile_set->tile_get_light_occluder(c.source_id);
			}
			if (occluder.is_valid()) {
				Vector2 occluder_ofs = tile_set->tile_get_occluder_offset(c.source_id);
				Transform2D xform;
				xform.set_origin(offset.floor() + q.pos);
				_fix_cell_transform(xform, c, occluder_ofs, s);

				RID orid = RS::get_singleton()->canvas_light_occluder_create();
				RS::get_singleton()->canvas_light_occluder_set_transform(orid, get_global_transform() * xform);
				RS::get_singleton()->canvas_light_occluder_set_polygon(orid, occluder->get_rid());
				RS::get_singleton()->canvas_light_occluder_attach_to_canvas(orid, get_canvas());
				RS::get_singleton()->canvas_light_occluder_set_light_mask(orid, occluder_light_mask);
				Quadrant::Occluder oc;
				oc.xform = xform;
				oc.id = orid;
				q.occluder_instances[E->key()] = oc;
			}
		}

		dirty_quadrant_list.remove(dirty_quadrant_list.first());
		quadrant_order_dirty = true;
	}

	pending_update = false;

	// Reset the drawing indices
	if (quadrant_order_dirty) {
		int index = -(int64_t)0x80000000; //always must be drawn below children.
		for (Map<PosKey, Quadrant>::Element *E = quadrant_map.front(); E; E = E->next()) {
			Quadrant &q = E->get();
			for (List<RID>::Element *F = q.canvas_items.front(); F; F = F->next()) {
				RS::get_singleton()->canvas_item_set_draw_index(F->get(), index++);
			}
		}

		quadrant_order_dirty = false;
	}

	_recompute_rect_cache();
}

void TileMap::_recompute_rect_cache() {
	// Compute the displayed area of the tilemap.
#ifdef DEBUG_ENABLED

	if (!rect_cache_dirty) {
		return;
	}

	Rect2 r_total;
	for (Map<PosKey, Quadrant>::Element *E = quadrant_map.front(); E; E = E->next()) {
		Rect2 r;
		r.position = _map_to_world(E->key().x * _get_quadrant_size(), E->key().y * _get_quadrant_size());
		r.expand_to(_map_to_world(E->key().x * _get_quadrant_size() + _get_quadrant_size(), E->key().y * _get_quadrant_size()));
		r.expand_to(_map_to_world(E->key().x * _get_quadrant_size() + _get_quadrant_size(), E->key().y * _get_quadrant_size() + _get_quadrant_size()));
		r.expand_to(_map_to_world(E->key().x * _get_quadrant_size(), E->key().y * _get_quadrant_size() + _get_quadrant_size()));
		if (E == quadrant_map.front()) {
			r_total = r;
		} else {
			r_total = r_total.merge(r);
		}
	}

	rect_cache = r_total;

	item_rect_changed();

	rect_cache_dirty = false;
#endif
}

Map<TileMap::PosKey, TileMap::Quadrant>::Element *TileMap::_create_quadrant(const PosKey &p_qk) {
	// Recreate a quadrant.

	Quadrant q;
	q.pos = _map_to_world(p_qk.x * _get_quadrant_size(), p_qk.y * _get_quadrant_size());
	q.pos += get_cell_draw_offset();
	if (tile_origin == TILE_ORIGIN_CENTER) {
		q.pos += cell_size / 2;
	} else if (tile_origin == TILE_ORIGIN_BOTTOM_LEFT) {
		q.pos.y += cell_size.y;
	}

	Transform2D xform;
	xform.set_origin(q.pos);

	// Collisions: create the body and set its parameters.
	q.body = PhysicsServer2D::get_singleton()->body_create();
	PhysicsServer2D::get_singleton()->body_set_mode(q.body, use_kinematic ? PhysicsServer2D::BODY_MODE_KINEMATIC : PhysicsServer2D::BODY_MODE_STATIC);

	PhysicsServer2D::get_singleton()->body_attach_object_instance_id(q.body, get_instance_id());
	PhysicsServer2D::get_singleton()->body_set_collision_layer(q.body, collision_layer);
	PhysicsServer2D::get_singleton()->body_set_collision_mask(q.body, collision_mask);
	PhysicsServer2D::get_singleton()->body_set_param(q.body, PhysicsServer2D::BODY_PARAM_FRICTION, friction);
	PhysicsServer2D::get_singleton()->body_set_param(q.body, PhysicsServer2D::BODY_PARAM_BOUNCE, bounce);

	if (is_inside_tree()) {
		xform = get_global_transform() * xform;
		RID space = get_world_2d()->get_space();
		PhysicsServer2D::get_singleton()->body_set_space(q.body, space);
	}

	PhysicsServer2D::get_singleton()->body_set_state(q.body, PhysicsServer2D::BODY_STATE_TRANSFORM, xform);

	rect_cache_dirty = true;
	quadrant_order_dirty = true;
	return quadrant_map.insert(p_qk, q);
}

void TileMap::_erase_quadrant(Map<PosKey, Quadrant>::Element *Q) {
	// Remove a quadrant.
	Quadrant &q = Q->get();

	// Collisions: Free the body.
	PhysicsServer2D::get_singleton()->free(q.body);

	// Free the canvas item..
	for (List<RID>::Element *E = q.canvas_items.front(); E; E = E->next()) {
		RenderingServer::get_singleton()->free(E->get());
	}
	q.canvas_items.clear();

	// ???
	if (q.dirty_list.in_list()) {
		dirty_quadrant_list.remove(&q.dirty_list);
	}

	for (Map<PosKey, Quadrant::NavPoly>::Element *E = q.navpoly_ids.front(); E; E = E->next()) {
		NavigationServer2D::get_singleton()->region_set_map(E->get().region, RID());
	}
	q.navpoly_ids.clear();

	for (Map<PosKey, Quadrant::Occluder>::Element *E = q.occluder_instances.front(); E; E = E->next()) {
		RS::get_singleton()->free(E->get().id);
	}
	q.occluder_instances.clear();

	quadrant_map.erase(Q);
	rect_cache_dirty = true;
}

void TileMap::_make_quadrant_dirty(Map<PosKey, Quadrant>::Element *Q, bool update) {
	// Make the given quadrant dirty, then trigger an update later.
	Quadrant &q = Q->get();
	if (!q.dirty_list.in_list()) {
		dirty_quadrant_list.add(&q.dirty_list);
	}

	if (pending_update) {
		return;
	}
	pending_update = true;
	if (!is_inside_tree()) {
		return;
	}

	if (update) {
		call_deferred("update_dirty_quadrants");
	}
}

void TileMap::set_cell(const Vector2i &p_coords, int p_source_id, const Vector2i p_tileset_coords, int p_alternative_tile) {
	// Set the current cell tile (using integer position)
	PosKey pk(p_coords);
	Map<PosKey, Cell>::Element *E = tile_map.find(pk);

	if (!E && p_source_id == INVALID_CELL) {
		return; // Nothing to do, the tile is already empty.
	}

	// Get the quadrant
	PosKey qk = pk.to_quadrant(_get_quadrant_size());
	Map<PosKey, Quadrant>::Element *Q = quadrant_map.find(qk);

	if (p_source_id == INVALID_CELL) {
		// Erase existing cell in the tile map.
		tile_map.erase(pk);

		// Erase existing cell in the quadrant.
		ERR_FAIL_COND(!Q);
		Quadrant &q = Q->get();
		q.cells.erase(pk);

		// Remove or make the quadrant dirty.
		if (q.cells.size() == 0) {
			_erase_quadrant(Q);
		} else {
			_make_quadrant_dirty(Q);
		}

		used_size_cache_dirty = true;
	} else {
		if (!E) {
			// Insert a new cell in the tile map.
			E = tile_map.insert(pk, Cell());

			// Create a new quadrant if needed, then insert the cell if needed.
			if (!Q) {
				Q = _create_quadrant(qk);
			}
			Quadrant &q = Q->get();
			q.cells.insert(pk);
		} else {
			ERR_FAIL_COND(!Q); // Quadrant should exist...

			if (E->get().source_id == p_source_id && E->get().get_tileset_coords() == p_tileset_coords && E->get().alternative_tile == p_alternative_tile) {
				return; //nothing changed
			}
		}

		Cell &c = E->get();

		c.source_id = p_source_id;
		c.set_tileset_coords(p_tileset_coords);
		c.alternative_tile = p_alternative_tile;

		_make_quadrant_dirty(Q);
		used_size_cache_dirty = true;
	}
}

void TileMap::_set_celld(const Vector2i &p_coords, const Dictionary &p_data) {
	// Set the current cell tile data.
	set_cell(p_coords, p_data["source_id"], p_data["tileset_coords"], p_data["p_alternative_tile"]);
}

int TileMap::get_cell_source_id(const Vector2i &p_coords) const {
	// Get a cell source id from position
	PosKey pk(p_coords);

	const Map<PosKey, Cell>::Element *E = tile_map.find(pk);

	if (!E) {
		return INVALID_CELL;
	}

	return E->get().source_id;
}

Vector2i TileMap::get_cell_tileset_coords(const Vector2i &p_coords) const {
	// Get a cell tileset coords from position
	PosKey pk(p_coords);

	const Map<PosKey, Cell>::Element *E = tile_map.find(pk);

	if (!E) {
		return Vector2i();
	}

	return E->get().get_tileset_coords();
}

int TileMap::get_cell_alternative_tile(const Vector2i &p_coords) const {
	// Get a cell id from positions
	PosKey pk(p_coords);

	const Map<PosKey, Cell>::Element *E = tile_map.find(pk);

	if (!E) {
		return 0;
	}

	return E->get().alternative_tile;
}

void TileMap::make_bitmask_area_dirty(const Vector2 &p_pos) {
	// Autotiles: trigger bitmask update making them dirty
	for (int x = p_pos.x - 1; x <= p_pos.x + 1; x++) {
		for (int y = p_pos.y - 1; y <= p_pos.y + 1; y++) {
			PosKey p(x, y);
			if (dirty_bitmask.find(p) == nullptr) {
				dirty_bitmask.push_back(p);
			}
		}
	}
}

void TileMap::update_bitmask_area(const Vector2 &p_pos) {
	// Autotiles: update the cells because of a bitmask change
	for (int x = p_pos.x - 1; x <= p_pos.x + 1; x++) {
		for (int y = p_pos.y - 1; y <= p_pos.y + 1; y++) {
			update_cell_bitmask(x, y);
		}
	}
}

void TileMap::update_bitmask_region(const Vector2 &p_start, const Vector2 &p_end) {
	// Autotiles: update the cells because of a bitmask change in the given region
	if ((p_end.x < p_start.x || p_end.y < p_start.y) || (p_end.x == p_start.x && p_end.y == p_start.y)) {
		// Update everything
		Array a = get_used_cells();
		for (int i = 0; i < a.size(); i++) {
			Vector2 vector = (Vector2)a[i];
			update_cell_bitmask(vector.x, vector.y);
		}
		return;
	}
	// Update cells in the region
	for (int x = p_start.x - 1; x <= p_end.x + 1; x++) {
		for (int y = p_start.y - 1; y <= p_end.y + 1; y++) {
			update_cell_bitmask(x, y);
		}
	}
}

void TileMap::update_cell_bitmask(int p_x, int p_y) {
	// Autotiles: Run the autotiling on a given cell
	ERR_FAIL_COND_MSG(tile_set.is_null(), "Cannot update cell bitmask if Tileset is not open.");
	PosKey p(p_x, p_y);
	Map<PosKey, Cell>::Element *E = tile_map.find(p);
	if (E != nullptr) {
		int id = get_cell_source_id(Vector2i(p_x, p_y));
		if (tile_set->tile_get_tile_mode(id) == TileSet::AUTO_TILE) {
			uint16_t mask = 0;
			int top_left = get_cell_source_id(Vector2i(p_x - 1, p_y - 1));
			int top = get_cell_source_id(Vector2i(p_x, p_y - 1));
			int top_right = get_cell_source_id(Vector2i(p_x + 1, p_y - 1));
			int right = get_cell_source_id(Vector2i(p_x + 1, p_y));
			int bottom_right = get_cell_source_id(Vector2i(p_x + 1, p_y + 1));
			int bottom = get_cell_source_id(Vector2i(p_x, p_y + 1));
			int bottom_left = get_cell_source_id(Vector2i(p_x - 1, p_y + 1));
			int left = get_cell_source_id(Vector2i(p_x - 1, p_y));

			if (tile_set->autotile_get_bitmask_mode(id) == TileSet::BITMASK_2X2) {
				if (tile_set->is_tile_bound(id, top_left) && tile_set->is_tile_bound(id, top) && tile_set->is_tile_bound(id, left)) {
					mask |= TileSet::BIND_TOPLEFT;
				}
				if (tile_set->is_tile_bound(id, top_right) && tile_set->is_tile_bound(id, top) && tile_set->is_tile_bound(id, right)) {
					mask |= TileSet::BIND_TOPRIGHT;
				}
				if (tile_set->is_tile_bound(id, bottom_left) && tile_set->is_tile_bound(id, bottom) && tile_set->is_tile_bound(id, left)) {
					mask |= TileSet::BIND_BOTTOMLEFT;
				}
				if (tile_set->is_tile_bound(id, bottom_right) && tile_set->is_tile_bound(id, bottom) && tile_set->is_tile_bound(id, right)) {
					mask |= TileSet::BIND_BOTTOMRIGHT;
				}
			} else {
				if (tile_set->autotile_get_bitmask_mode(id) == TileSet::BITMASK_3X3_MINIMAL) {
					if (tile_set->is_tile_bound(id, top_left) && tile_set->is_tile_bound(id, top) && tile_set->is_tile_bound(id, left)) {
						mask |= TileSet::BIND_TOPLEFT;
					}
					if (tile_set->is_tile_bound(id, top_right) && tile_set->is_tile_bound(id, top) && tile_set->is_tile_bound(id, right)) {
						mask |= TileSet::BIND_TOPRIGHT;
					}
					if (tile_set->is_tile_bound(id, bottom_left) && tile_set->is_tile_bound(id, bottom) && tile_set->is_tile_bound(id, left)) {
						mask |= TileSet::BIND_BOTTOMLEFT;
					}
					if (tile_set->is_tile_bound(id, bottom_right) && tile_set->is_tile_bound(id, bottom) && tile_set->is_tile_bound(id, right)) {
						mask |= TileSet::BIND_BOTTOMRIGHT;
					}
				} else {
					if (tile_set->is_tile_bound(id, top_left)) {
						mask |= TileSet::BIND_TOPLEFT;
					}
					if (tile_set->is_tile_bound(id, top_right)) {
						mask |= TileSet::BIND_TOPRIGHT;
					}
					if (tile_set->is_tile_bound(id, bottom_left)) {
						mask |= TileSet::BIND_BOTTOMLEFT;
					}
					if (tile_set->is_tile_bound(id, bottom_right)) {
						mask |= TileSet::BIND_BOTTOMRIGHT;
					}
				}
				if (tile_set->is_tile_bound(id, top)) {
					mask |= TileSet::BIND_TOP;
				}
				if (tile_set->is_tile_bound(id, left)) {
					mask |= TileSet::BIND_LEFT;
				}
				mask |= TileSet::BIND_CENTER;
				if (tile_set->is_tile_bound(id, right)) {
					mask |= TileSet::BIND_RIGHT;
				}
				if (tile_set->is_tile_bound(id, bottom)) {
					mask |= TileSet::BIND_BOTTOM;
				}
			}
			Vector2 coord = tile_set->autotile_get_subtile_for_bitmask(id, mask, this, Vector2(p_x, p_y));
			E->get().coord_x = (int)coord.x;
			E->get().coord_y = (int)coord.y;

			PosKey qk = p.to_quadrant(_get_quadrant_size());
			Map<PosKey, Quadrant>::Element *Q = quadrant_map.find(qk);
			_make_quadrant_dirty(Q);

		} else if (tile_set->tile_get_tile_mode(id) == TileSet::SINGLE_TILE) {
			E->get().coord_x = 0;
			E->get().coord_y = 0;
		} else if (tile_set->tile_get_tile_mode(id) == TileSet::ATLAS_TILE) {
			if (tile_set->autotile_get_bitmask(id, Vector2(p_x, p_y)) == TileSet::BIND_CENTER) {
				Vector2 coord = tile_set->atlastile_get_subtile_by_priority(id, this, Vector2(p_x, p_y));

				E->get().coord_x = (int)coord.x;
				E->get().coord_y = (int)coord.y;
			}
		}
	}
}

void TileMap::update_dirty_bitmask() {
	// Autotiles: Update the dirty bitmasks.
	while (dirty_bitmask.size() > 0) {
		update_cell_bitmask(dirty_bitmask[0].x, dirty_bitmask[0].y);
		dirty_bitmask.pop_front();
	}
}

void TileMap::fix_invalid_tiles() {
	// Autotiles: Fix invalid tiles in the tilemap if it has any.
	ERR_FAIL_COND_MSG(tile_set.is_null(), "Cannot fix invalid tiles if Tileset is not open.");
	for (Map<PosKey, Cell>::Element *E = tile_map.front(); E; E = E->next()) {
		Vector2i v = Vector2i(E->key().x, E->key().y);
		int source_id = get_cell_source_id(v);
		Vector2i tileset_coords = get_cell_tileset_coords(v);
		int alternative_tile = get_cell_alternative_tile(v);
		if (!tile_set->has_tile(source_id, tileset_coords, alternative_tile)) {
			set_cell(v, INVALID_CELL, Vector2i(), 0);
		}
	}
}

void TileMap::_recreate_quadrants() {
	// Clear then recreate all quadrants
	_clear_quadrants();

	for (Map<PosKey, Cell>::Element *E = tile_map.front(); E; E = E->next()) {
		PosKey qk = PosKey(E->key().x, E->key().y).to_quadrant(_get_quadrant_size());

		Map<PosKey, Quadrant>::Element *Q = quadrant_map.find(qk);
		if (!Q) {
			Q = _create_quadrant(qk);
			dirty_quadrant_list.add(&Q->get().dirty_list);
		}

		Q->get().cells.insert(E->key());
		_make_quadrant_dirty(Q, false);
	}
	update_dirty_quadrants();
}

void TileMap::_clear_quadrants() {
	// Clear quadrants.
	while (quadrant_map.size()) {
		_erase_quadrant(quadrant_map.front());
	}
}

void TileMap::set_material(const Ref<Material> &p_material) {
	// Set material for the whole tilemap.
	CanvasItem::set_material(p_material);
	_update_all_items_material_state();
}

void TileMap::set_use_parent_material(bool p_use_parent_material) {
	// Set use_parent_material for the whole tilemap.
	CanvasItem::set_use_parent_material(p_use_parent_material);
	_update_all_items_material_state();
}

void TileMap::_update_all_items_material_state() {
	// Update use_parent_material for the whole tilemap.
	for (Map<PosKey, Quadrant>::Element *E = quadrant_map.front(); E; E = E->next()) {
		Quadrant &q = E->get();
		for (List<RID>::Element *F = q.canvas_items.front(); F; F = F->next()) {
			_update_item_material_state(F->get());
		}
	}
}

void TileMap::_update_item_material_state(const RID &p_canvas_item) {
	// Update use_parent_material for a given canvas_item.
	RS::get_singleton()->canvas_item_set_use_parent_material(p_canvas_item, get_use_parent_material() || get_material().is_valid());
}

void TileMap::clear() {
	// Remove all tiles.
	_clear_quadrants();
	tile_map.clear();
	used_size_cache_dirty = true;
}

void TileMap::_set_tile_data(const Vector<int> &p_data) {
	// Set data for a given tile from raw data.
	ERR_FAIL_COND(format > FORMAT_3);

	int c = p_data.size();
	const int *r = p_data.ptr();

	int offset = (format >= FORMAT_2) ? 3 : 2;

	clear();
	for (int i = 0; i < c; i += offset) {
		const uint8_t *ptr = (const uint8_t *)&r[i];
		uint8_t local[12];
		for (int j = 0; j < ((format >= FORMAT_2) ? 12 : 8); j++) {
			local[j] = ptr[j];
		}

#ifdef BIG_ENDIAN_ENABLED

		SWAP(local[0], local[3]);
		SWAP(local[1], local[2]);
		SWAP(local[4], local[7]);
		SWAP(local[5], local[6]);
		//TODO: ask someone to check this...
		if (FORMAT >= FORMAT_2) {
			SWAP(local[8], local[11]);
			SWAP(local[9], local[10]);
		}
#endif
		uint16_t x = decode_uint16(&local[0]);
		uint16_t y = decode_uint16(&local[2]);

		if (format == FORMAT_3) {
			uint16_t source_id = decode_uint16(&local[4]);
			uint16_t tileset_coords_x = decode_uint16(&local[6]);
			uint16_t tileset_coords_y = decode_uint32(&local[8]);
			uint16_t alternative_tile = decode_uint16(&local[10]);
			set_cell(Vector2i(x, y), source_id, Vector2i(tileset_coords_x, tileset_coords_y), alternative_tile);
		} else {
			uint32_t v = decode_uint32(&local[4]);
			v &= (1 << 29) - 1;

			// We generate an alternative tile number out of the the flags
			// An option should create the alternative in the tilset for compatibility
			bool flip_h = v & (1 << 29);
			bool flip_v = v & (1 << 30);
			bool transpose = v & (1 << 31);
			int16_t coord_x = 0;
			int16_t coord_y = 0;
			if (format == FORMAT_2) {
				coord_x = decode_uint16(&local[8]);
				coord_y = decode_uint16(&local[10]);
			}

			int compatibility_alternative_tile = ((int)flip_h) + ((int)flip_v << 1) + ((int)transpose << 2);

			set_cell(Vector2i(x, y), v, Vector2i(coord_x, coord_y), compatibility_alternative_tile);
		}
	}
}

Vector<int> TileMap::_get_tile_data() const {
	// Export tile data to raw format
	Vector<int> data;
	data.resize(tile_map.size() * 3);
	int *w = data.ptrw();

	// Save in highest format

	int idx = 0;
	for (const Map<PosKey, Cell>::Element *E = tile_map.front(); E; E = E->next()) {
		uint8_t *ptr = (uint8_t *)&w[idx];
		encode_uint16(E->key().x, &ptr[0]);
		encode_uint16(E->key().y, &ptr[2]);
		encode_uint16(E->get().source_id, &ptr[4]);
		encode_uint16(E->get().coord_x, &ptr[6]);
		encode_uint16(E->get().coord_y, &ptr[8]);
		encode_uint16(E->get().alternative_tile, &ptr[10]);
		idx += 3;
	}

	return data;
}

#ifdef TOOLS_ENABLED
Rect2 TileMap::_edit_get_rect() const {
	// Return the visible rect of the tilemap
	if (pending_update) {
		const_cast<TileMap *>(this)->update_dirty_quadrants();
	} else {
		const_cast<TileMap *>(this)->_recompute_rect_cache();
	}
	return rect_cache;
}
#endif

uint32_t TileMap::get_collision_layer() const {
	// Collisions: Get collision layer.
	return collision_layer;
}

void TileMap::set_collision_layer(uint32_t p_layer) {
	// Collisions: Set collision layer.
	collision_layer = p_layer;
	for (Map<PosKey, Quadrant>::Element *E = quadrant_map.front(); E; E = E->next()) {
		Quadrant &q = E->get();
		PhysicsServer2D::get_singleton()->body_set_collision_layer(q.body, collision_layer);
	}
}

uint32_t TileMap::get_collision_mask() const {
	// Collisions: Get collision mask.
	return collision_mask;
}

void TileMap::set_collision_mask(uint32_t p_mask) {
	// Collisions: Set collision mask.
	collision_mask = p_mask;

	for (Map<PosKey, Quadrant>::Element *E = quadrant_map.front(); E; E = E->next()) {
		Quadrant &q = E->get();
		PhysicsServer2D::get_singleton()->body_set_collision_mask(q.body, collision_mask);
	}
}
bool TileMap::get_collision_layer_bit(int p_bit) const {
	// Collisions: Get collision layer bit.
	ERR_FAIL_INDEX_V_MSG(p_bit, 32, false, "Collision layer bit must be between 0 and 31 inclusive.");
	return get_collision_layer() & (1 << p_bit);
}

void TileMap::set_collision_layer_bit(int p_bit, bool p_value) {
	ERR_FAIL_INDEX_MSG(p_bit, 32, "Collision layer bit must be between 0 and 31 inclusive.");
	// Collisions: Set collision layer bit.
	uint32_t layer = get_collision_layer();
	if (p_value) {
		layer |= 1 << p_bit;
	} else {
		layer &= ~(1 << p_bit);
	}
	set_collision_layer(layer);
}

bool TileMap::get_collision_mask_bit(int p_bit) const {
	// Collisions: Get collision mask bit.
	ERR_FAIL_INDEX_V_MSG(p_bit, 32, false, "Collision mask bit must be between 0 and 31 inclusive.");
	return get_collision_mask() & (1 << p_bit);
}

void TileMap::set_collision_mask_bit(int p_bit, bool p_value) {
	ERR_FAIL_INDEX_MSG(p_bit, 32, "Collision mask bit must be between 0 and 31 inclusive.");
	// Collisions: Set collision mask bit.
	uint32_t mask = get_collision_mask();
	if (p_value) {
		mask |= 1 << p_bit;
	} else {
		mask &= ~(1 << p_bit);
	}
	set_collision_mask(mask);
}

bool TileMap::get_collision_use_kinematic() const {
	// Collisions: get kinematic.
	return use_kinematic;
}

void TileMap::set_collision_use_kinematic(bool p_use_kinematic) {
	// Collisions: set kinematic.
	_clear_quadrants();
	use_kinematic = p_use_kinematic;
	_recreate_quadrants();
}

void TileMap::set_collision_friction(float p_friction) {
	// Collisions: set friction.
	friction = p_friction;
	for (Map<PosKey, Quadrant>::Element *E = quadrant_map.front(); E; E = E->next()) {
		Quadrant &q = E->get();
		PhysicsServer2D::get_singleton()->body_set_param(q.body, PhysicsServer2D::BODY_PARAM_FRICTION, p_friction);
	}
}

float TileMap::get_collision_friction() const {
	// Collisions: get friction.
	return friction;
}

void TileMap::set_collision_bounce(float p_bounce) {
	// Collisions: set bounce.
	bounce = p_bounce;
	for (Map<PosKey, Quadrant>::Element *E = quadrant_map.front(); E; E = E->next()) {
		Quadrant &q = E->get();
		PhysicsServer2D::get_singleton()->body_set_param(q.body, PhysicsServer2D::BODY_PARAM_BOUNCE, p_bounce);
	}
}

float TileMap::get_collision_bounce() const {
	return bounce;
}

void TileMap::set_bake_navigation(bool p_bake_navigation) {
	bake_navigation = p_bake_navigation;
	for (Map<PosKey, Quadrant>::Element *F = quadrant_map.front(); F; F = F->next()) {
		_make_quadrant_dirty(F);
	}
}

bool TileMap::is_baking_navigation() {
	return bake_navigation;
}

TileMap::Mode TileMap::get_mode() const {
	// Mode: get tile mode.
	return mode;
}

void TileMap::set_mode(Mode p_mode) {
	// Mode: set tile mode.
	_clear_quadrants();
	mode = p_mode;
	_recreate_quadrants();
	emit_signal("settings_changed");
}

TileMap::HalfOffset TileMap::get_half_offset() const {
	// Half offset: get half offset
	return half_offset;
}

void TileMap::set_half_offset(HalfOffset p_half_offset) {
	// Half offset: set half offset
	_clear_quadrants();
	half_offset = p_half_offset;
	_recreate_quadrants();
	emit_signal("settings_changed");
}

TileMap::TileOrigin TileMap::get_tile_origin() const {
	// Origin: get tile origin.
	return tile_origin;
}

void TileMap::set_tile_origin(TileOrigin p_tile_origin) {
	// Origin: set tile origin.
	_clear_quadrants();
	tile_origin = p_tile_origin;
	_recreate_quadrants();
	emit_signal("settings_changed");
}

Vector2 TileMap::get_cell_draw_offset() const {
	// Mode: Return the draw offset depending on the mode
	switch (mode) {
		case MODE_SQUARE: {
			return Vector2();
		} break;
		case MODE_ISOMETRIC: {
			return Vector2(-cell_size.x * 0.5, 0);

		} break;
		case MODE_CUSTOM: {
			Vector2 min;
			min.x = MIN(custom_transform[0].x, min.x);
			min.y = MIN(custom_transform[0].y, min.y);
			min.x = MIN(custom_transform[1].x, min.x);
			min.y = MIN(custom_transform[1].y, min.y);
			return min;
		} break;
	}

	return Vector2();
}

Transform2D TileMap::get_cell_transform() const {
	// Mode: Return transform depending on the mode
	switch (mode) {
		case MODE_SQUARE: {
			Transform2D m;
			m[0] *= cell_size.x;
			m[1] *= cell_size.y;
			return m;
		} break;
		case MODE_ISOMETRIC: {
			//isometric only makes sense when y is positive in both x and y vectors, otherwise
			//the drawing of tiles will overlap
			Transform2D m;
			m[0] = Vector2(cell_size.x * 0.5, cell_size.y * 0.5);
			m[1] = Vector2(-cell_size.x * 0.5, cell_size.y * 0.5);
			return m;

		} break;
		case MODE_CUSTOM: {
			return custom_transform;
		} break;
	}

	return Transform2D();
}

Transform2D TileMap::get_custom_transform() const {
	// Mode: get custom transform.
	return custom_transform;
}

void TileMap::set_custom_transform(const Transform2D &p_xform) {
	// Mode: set custom transform.
	_clear_quadrants();
	custom_transform = p_xform;
	_recreate_quadrants();
	emit_signal("settings_changed");
}

Vector2 TileMap::_map_to_world(int p_x, int p_y, bool p_ignore_ofs) const {
	// Mode: map to world.
	Vector2 ret = get_cell_transform().xform(Vector2(p_x, p_y));
	if (!p_ignore_ofs) {
		switch (half_offset) {
			case HALF_OFFSET_X:
			case HALF_OFFSET_NEGATIVE_X: {
				if (ABS(p_y) & 1) {
					ret += get_cell_transform()[0] * (half_offset == HALF_OFFSET_X ? 0.5 : -0.5);
				}
			} break;
			case HALF_OFFSET_Y:
			case HALF_OFFSET_NEGATIVE_Y: {
				if (ABS(p_x) & 1) {
					ret += get_cell_transform()[1] * (half_offset == HALF_OFFSET_Y ? 0.5 : -0.5);
				}
			} break;
			case HALF_OFFSET_DISABLED: {
				// Nothing to do.
			}
		}
	}
	return ret;
}

bool TileMap::_set(const StringName &p_name, const Variant &p_value) {
	if (p_name == "format") {
		if (p_value.get_type() == Variant::INT) {
			format = (DataFormat)(p_value.operator int64_t()); // Set format used for loading
			return true;
		}
	} else if (p_name == "tile_data") {
		if (p_value.is_array()) {
			_set_tile_data(p_value);
			return true;
		}
		return false;
	}
	return false;
}

bool TileMap::_get(const StringName &p_name, Variant &r_ret) const {
	if (p_name == "format") {
		r_ret = FORMAT_3; // When saving, always save highest format
		return true;
	} else if (p_name == "tile_data") {
		r_ret = _get_tile_data();
		return true;
	}
	return false;
}

void TileMap::_get_property_list(List<PropertyInfo> *p_list) const {
	PropertyInfo p(Variant::INT, "format", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR | PROPERTY_USAGE_INTERNAL);
	p_list->push_back(p);

	p = PropertyInfo(Variant::OBJECT, "tile_data", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR | PROPERTY_USAGE_INTERNAL);
	p_list->push_back(p);
}

Vector2 TileMap::map_to_world(const Vector2 &p_pos, bool p_ignore_ofs) const {
	return _map_to_world(p_pos.x, p_pos.y, p_ignore_ofs);
}

Vector2 TileMap::world_to_map(const Vector2 &p_pos) const {
	Vector2 ret = get_cell_transform().affine_inverse().xform(p_pos);

	// Account for precision errors on the border (GH-23250).
	// 0.00005 is 5*CMP_EPSILON, results would start being unpredictable if
	// cell size is > 15,000, but we can hardly have more precision anyway with
	// floating point.
	ret += Vector2(0.00005, 0.00005);

	// Half offset: map according to half offset
	switch (half_offset) {
		case HALF_OFFSET_X: {
			if (int(floor(ret.y)) & 1) {
				ret.x -= 0.5;
			}
		} break;
		case HALF_OFFSET_NEGATIVE_X: {
			if (int(floor(ret.y)) & 1) {
				ret.x += 0.5;
			}
		} break;
		case HALF_OFFSET_Y: {
			if (int(floor(ret.x)) & 1) {
				ret.y -= 0.5;
			}
		} break;
		case HALF_OFFSET_NEGATIVE_Y: {
			if (int(floor(ret.x)) & 1) {
				ret.y += 0.5;
			}
		} break;
		case HALF_OFFSET_DISABLED: {
			// Nothing to do.
		}
	}

	return ret.floor();
}

bool TileMap::is_y_sort_enabled() const {
	// YSort: is Ysort
	return use_y_sort;
}

void TileMap::set_y_sort_enabled(bool p_enable) {
	// YSort: set Ysort
	_clear_quadrants();
	use_y_sort = p_enable;
	RS::get_singleton()->canvas_item_set_sort_children_by_y(get_canvas_item(), use_y_sort);
	_recreate_quadrants();
	emit_signal("settings_changed");
}

TypedArray<Vector2i> TileMap::get_used_cells() const {
	// Returns the cells used in the tilemap.
	TypedArray<Vector2i> a;
	a.resize(tile_map.size());
	int i = 0;
	for (Map<PosKey, Cell>::Element *E = tile_map.front(); E; E = E->next()) {
		Vector2i p(E->key().x, E->key().y);
		a[i++] = p;
	}

	return a;
}

TypedArray<Vector2i> TileMap::get_used_cells_by_index(int p_source_id, const Vector2i p_tileset_coords, int p_alternative_tile) const {
	// Return all cells matching a given ID.
	TypedArray<Vector2i> a;
	for (Map<PosKey, Cell>::Element *E = tile_map.front(); E; E = E->next()) {
		if (E->value().source_id == p_source_id && E->value().get_tileset_coords() == p_tileset_coords && E->value().alternative_tile == p_alternative_tile) {
			Vector2i p(E->key().x, E->key().y);
			a.push_back(p);
		}
	}

	return a;
}

Rect2 TileMap::get_used_rect() { // Not const because of cache
	// Return the rect of the currently used area
	if (used_size_cache_dirty) {
		if (tile_map.size() > 0) {
			used_size_cache = Rect2(tile_map.front()->key().x, tile_map.front()->key().y, 0, 0);

			for (Map<PosKey, Cell>::Element *E = tile_map.front(); E; E = E->next()) {
				used_size_cache.expand_to(Vector2(E->key().x, E->key().y));
			}

			used_size_cache.size += Vector2(1, 1);
		} else {
			used_size_cache = Rect2();
		}

		used_size_cache_dirty = false;
	}

	return used_size_cache;
}

int TileMap::get_occluder_light_mask() const {
	// Occlusion: set light mask.
	return occluder_light_mask;
}

void TileMap::set_occluder_light_mask(int p_mask) {
	// Occlusion: set occluder light mask.
	occluder_light_mask = p_mask;
	for (Map<PosKey, Quadrant>::Element *E = quadrant_map.front(); E; E = E->next()) {
		for (Map<PosKey, Quadrant::Occluder>::Element *F = E->get().occluder_instances.front(); F; F = F->next()) {
			RenderingServer::get_singleton()->canvas_light_occluder_set_light_mask(F->get().id, occluder_light_mask);
		}
	}
}

void TileMap::set_light_mask(int p_light_mask) {
	// Occlusion: set light mask.
	CanvasItem::set_light_mask(p_light_mask);
	for (Map<PosKey, Quadrant>::Element *E = quadrant_map.front(); E; E = E->next()) {
		for (List<RID>::Element *F = E->get().canvas_items.front(); F; F = F->next()) {
			RenderingServer::get_singleton()->canvas_item_set_light_mask(F->get(), get_light_mask());
		}
	}
}

bool TileMap::get_clip_uv() const {
	// Clip uv: get clip uv.
	return clip_uv;
}

void TileMap::set_clip_uv(bool p_enable) {
	// Clip uv: set clip uv.
	if (clip_uv == p_enable) {
		return;
	}

	_clear_quadrants();
	clip_uv = p_enable;
	_recreate_quadrants();
}

void TileMap::set_texture_filter(TextureFilter p_texture_filter) {
	// Set a default texture filter for the whole tilemap
	CanvasItem::set_texture_filter(p_texture_filter);
	for (Map<PosKey, Quadrant>::Element *F = quadrant_map.front(); F; F = F->next()) {
		Quadrant &q = F->get();
		for (List<RID>::Element *E = q.canvas_items.front(); E; E = E->next()) {
			RenderingServer::get_singleton()->canvas_item_set_default_texture_filter(E->get(), RS::CanvasItemTextureFilter(p_texture_filter));
			_make_quadrant_dirty(F);
		}
	}
}

void TileMap::set_texture_repeat(CanvasItem::TextureRepeat p_texture_repeat) {
	// Set a default texture repeat for the whole tilemap
	CanvasItem::set_texture_repeat(p_texture_repeat);
	for (Map<PosKey, Quadrant>::Element *F = quadrant_map.front(); F; F = F->next()) {
		Quadrant &q = F->get();
		for (List<RID>::Element *E = q.canvas_items.front(); E; E = E->next()) {
			RenderingServer::get_singleton()->canvas_item_set_default_texture_repeat(E->get(), RS::CanvasItemTextureRepeat(p_texture_repeat));
			_make_quadrant_dirty(F);
		}
	}
}

void TileMap::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_tileset", "tileset"), &TileMap::set_tileset);
	ClassDB::bind_method(D_METHOD("get_tileset"), &TileMap::get_tileset);

	ClassDB::bind_method(D_METHOD("set_mode", "mode"), &TileMap::set_mode);
	ClassDB::bind_method(D_METHOD("get_mode"), &TileMap::get_mode);

	ClassDB::bind_method(D_METHOD("set_half_offset", "half_offset"), &TileMap::set_half_offset);
	ClassDB::bind_method(D_METHOD("get_half_offset"), &TileMap::get_half_offset);

	ClassDB::bind_method(D_METHOD("set_custom_transform", "custom_transform"), &TileMap::set_custom_transform);
	ClassDB::bind_method(D_METHOD("get_custom_transform"), &TileMap::get_custom_transform);

	ClassDB::bind_method(D_METHOD("set_cell_size", "size"), &TileMap::set_cell_size);
	ClassDB::bind_method(D_METHOD("get_cell_size"), &TileMap::get_cell_size);

	ClassDB::bind_method(D_METHOD("_set_old_cell_size", "size"), &TileMap::_set_old_cell_size);
	ClassDB::bind_method(D_METHOD("_get_old_cell_size"), &TileMap::_get_old_cell_size);

	ClassDB::bind_method(D_METHOD("set_quadrant_size", "size"), &TileMap::set_quadrant_size);
	ClassDB::bind_method(D_METHOD("get_quadrant_size"), &TileMap::get_quadrant_size);

	ClassDB::bind_method(D_METHOD("set_tile_origin", "origin"), &TileMap::set_tile_origin);
	ClassDB::bind_method(D_METHOD("get_tile_origin"), &TileMap::get_tile_origin);

	ClassDB::bind_method(D_METHOD("set_clip_uv", "enable"), &TileMap::set_clip_uv);
	ClassDB::bind_method(D_METHOD("get_clip_uv"), &TileMap::get_clip_uv);

	ClassDB::bind_method(D_METHOD("set_y_sort_enabled", "enable"), &TileMap::set_y_sort_enabled);
	ClassDB::bind_method(D_METHOD("is_y_sort_enabled"), &TileMap::is_y_sort_enabled);

	ClassDB::bind_method(D_METHOD("set_collision_use_kinematic", "use_kinematic"), &TileMap::set_collision_use_kinematic);
	ClassDB::bind_method(D_METHOD("get_collision_use_kinematic"), &TileMap::get_collision_use_kinematic);

	ClassDB::bind_method(D_METHOD("set_collision_layer", "layer"), &TileMap::set_collision_layer);
	ClassDB::bind_method(D_METHOD("get_collision_layer"), &TileMap::get_collision_layer);

	ClassDB::bind_method(D_METHOD("set_collision_mask", "mask"), &TileMap::set_collision_mask);
	ClassDB::bind_method(D_METHOD("get_collision_mask"), &TileMap::get_collision_mask);

	ClassDB::bind_method(D_METHOD("set_collision_layer_bit", "bit", "value"), &TileMap::set_collision_layer_bit);
	ClassDB::bind_method(D_METHOD("get_collision_layer_bit", "bit"), &TileMap::get_collision_layer_bit);

	ClassDB::bind_method(D_METHOD("set_collision_mask_bit", "bit", "value"), &TileMap::set_collision_mask_bit);
	ClassDB::bind_method(D_METHOD("get_collision_mask_bit", "bit"), &TileMap::get_collision_mask_bit);

	ClassDB::bind_method(D_METHOD("set_collision_friction", "value"), &TileMap::set_collision_friction);
	ClassDB::bind_method(D_METHOD("get_collision_friction"), &TileMap::get_collision_friction);

	ClassDB::bind_method(D_METHOD("set_collision_bounce", "value"), &TileMap::set_collision_bounce);
	ClassDB::bind_method(D_METHOD("get_collision_bounce"), &TileMap::get_collision_bounce);

	ClassDB::bind_method(D_METHOD("set_bake_navigation", "bake_navigation"), &TileMap::set_bake_navigation);
	ClassDB::bind_method(D_METHOD("is_baking_navigation"), &TileMap::is_baking_navigation);

	ClassDB::bind_method(D_METHOD("set_occluder_light_mask", "mask"), &TileMap::set_occluder_light_mask);
	ClassDB::bind_method(D_METHOD("get_occluder_light_mask"), &TileMap::get_occluder_light_mask);

	ClassDB::bind_method(D_METHOD("set_cell", "coords", "source_id", "tileset_coords", "alternative_tile"), &TileMap::set_cell, DEFVAL(0), DEFVAL(Vector2i()), DEFVAL(INVALID_CELL));
	ClassDB::bind_method(D_METHOD("_set_celld", "coords", "data"), &TileMap::_set_celld);
	ClassDB::bind_method(D_METHOD("get_cell_source_id", "coords"), &TileMap::get_cell_source_id);
	ClassDB::bind_method(D_METHOD("get_cell_tileset_coords", "coords"), &TileMap::get_cell_tileset_coords);
	ClassDB::bind_method(D_METHOD("get_cell_alternative_tile", "coords"), &TileMap::get_cell_alternative_tile);

	ClassDB::bind_method(D_METHOD("fix_invalid_tiles"), &TileMap::fix_invalid_tiles);
	ClassDB::bind_method(D_METHOD("clear"), &TileMap::clear);

	ClassDB::bind_method(D_METHOD("get_used_cells"), &TileMap::get_used_cells);
	ClassDB::bind_method(D_METHOD("get_used_cells_by_index", "index"), &TileMap::get_used_cells_by_index);
	ClassDB::bind_method(D_METHOD("get_used_rect"), &TileMap::get_used_rect);

	ClassDB::bind_method(D_METHOD("map_to_world", "map_position", "ignore_half_ofs"), &TileMap::map_to_world, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("world_to_map", "world_position"), &TileMap::world_to_map);

	ClassDB::bind_method(D_METHOD("_clear_quadrants"), &TileMap::_clear_quadrants);
	ClassDB::bind_method(D_METHOD("update_dirty_quadrants"), &TileMap::update_dirty_quadrants);

	ClassDB::bind_method(D_METHOD("update_bitmask_area", "position"), &TileMap::update_bitmask_area);
	ClassDB::bind_method(D_METHOD("update_bitmask_region", "start", "end"), &TileMap::update_bitmask_region, DEFVAL(Vector2()), DEFVAL(Vector2()));

	ClassDB::bind_method(D_METHOD("_set_tile_data"), &TileMap::_set_tile_data);
	ClassDB::bind_method(D_METHOD("_get_tile_data"), &TileMap::_get_tile_data);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "mode", PROPERTY_HINT_ENUM, "Square,Isometric,Custom"), "set_mode", "get_mode");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "tile_set", PROPERTY_HINT_RESOURCE_TYPE, "TileSet"), "set_tileset", "get_tileset");

	ADD_GROUP("Cell", "cell_");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "cell_size", PROPERTY_HINT_RANGE, "1,8192,1"), "set_cell_size", "get_cell_size");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "cell_quadrant_size", PROPERTY_HINT_RANGE, "1,128,1"), "set_quadrant_size", "get_quadrant_size");
	ADD_PROPERTY(PropertyInfo(Variant::TRANSFORM2D, "cell_custom_transform"), "set_custom_transform", "get_custom_transform");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "cell_half_offset", PROPERTY_HINT_ENUM, "Offset X,Offset Y,Disabled,Offset Negative X,Offset Negative Y"), "set_half_offset", "get_half_offset");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "cell_tile_origin", PROPERTY_HINT_ENUM, "Top Left,Center,Bottom Left"), "set_tile_origin", "get_tile_origin");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "cell_y_sort"), "set_y_sort_enabled", "is_y_sort_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "cell_clip_uv"), "set_clip_uv", "get_clip_uv");

	ADD_GROUP("Collision", "collision_");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "collision_use_kinematic", PROPERTY_HINT_NONE, ""), "set_collision_use_kinematic", "get_collision_use_kinematic");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "collision_friction", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_collision_friction", "get_collision_friction");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "collision_bounce", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_collision_bounce", "get_collision_bounce");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_layer", PROPERTY_HINT_LAYERS_2D_PHYSICS), "set_collision_layer", "get_collision_layer");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_mask", PROPERTY_HINT_LAYERS_2D_PHYSICS), "set_collision_mask", "get_collision_mask");

	ADD_GROUP("Occluder", "occluder_");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "occluder_light_mask", PROPERTY_HINT_LAYERS_2D_RENDER), "set_occluder_light_mask", "get_occluder_light_mask");

	ADD_GROUP("Navigation", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "bake_navigation"), "set_bake_navigation", "is_baking_navigation");

	ADD_PROPERTY_DEFAULT("format", FORMAT_1);

	ADD_SIGNAL(MethodInfo("settings_changed"));

	BIND_CONSTANT(INVALID_CELL);

	BIND_ENUM_CONSTANT(MODE_SQUARE);
	BIND_ENUM_CONSTANT(MODE_ISOMETRIC);
	BIND_ENUM_CONSTANT(MODE_CUSTOM);

	BIND_ENUM_CONSTANT(HALF_OFFSET_X);
	BIND_ENUM_CONSTANT(HALF_OFFSET_Y);
	BIND_ENUM_CONSTANT(HALF_OFFSET_DISABLED);
	BIND_ENUM_CONSTANT(HALF_OFFSET_NEGATIVE_X);
	BIND_ENUM_CONSTANT(HALF_OFFSET_NEGATIVE_Y);

	BIND_ENUM_CONSTANT(TILE_ORIGIN_TOP_LEFT);
	BIND_ENUM_CONSTANT(TILE_ORIGIN_CENTER);
	BIND_ENUM_CONSTANT(TILE_ORIGIN_BOTTOM_LEFT);
}

TileMap::TileMap() {
	rect_cache_dirty = true;
	used_size_cache_dirty = true;
	pending_update = false;
	quadrant_order_dirty = false;
	quadrant_size = 16;
	cell_size = Size2(64, 64);
	custom_transform = Transform2D(64, 0, 0, 64, 0, 0);
	collision_layer = 1;
	collision_mask = 1;
	friction = 1;
	bounce = 0;
	mode = MODE_SQUARE;
	half_offset = HALF_OFFSET_DISABLED;
	use_kinematic = false;
	navigation = nullptr;
	use_y_sort = false;
	occluder_light_mask = 1;
	clip_uv = false;
	format = FORMAT_1; // Assume lowest possible format if none is present

	fp_adjust = 0.00001;
	tile_origin = TILE_ORIGIN_TOP_LEFT;
	set_notify_transform(true);
	set_notify_local_transform(false);
}

TileMap::~TileMap() {
	clear();
}
