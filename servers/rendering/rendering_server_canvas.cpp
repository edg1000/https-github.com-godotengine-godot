/*************************************************************************/
/*  rendering_server_canvas.cpp                                          */
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

#include "rendering_server_canvas.h"

#include "core/math/geometry_2d.h"
#include "rendering_server_globals.h"
#include "rendering_server_raster.h"
#include "rendering_server_viewport.h"

static const int z_range = RS::CANVAS_ITEM_Z_MAX - RS::CANVAS_ITEM_Z_MIN + 1;

void RenderingServerCanvas::_render_canvas_item_tree(RID p_to_render_target, Canvas::ChildItem *p_child_items, int p_child_item_count, Item *p_canvas_item, const Transform2D &p_transform, const Rect2 &p_clip_rect, const Color &p_modulate, RasterizerCanvas::Light *p_lights) {
	RENDER_TIMESTAMP("Cull CanvasItem Tree");

	memset(z_list, 0, z_range * sizeof(RasterizerCanvas::Item *));
	memset(z_last_list, 0, z_range * sizeof(RasterizerCanvas::Item *));

	for (int i = 0; i < p_child_item_count; i++) {
		_cull_canvas_item(p_child_items[i].item, p_transform, p_clip_rect, Color(1, 1, 1, 1), 0, z_list, z_last_list, nullptr, nullptr);
	}
	if (p_canvas_item) {
		_cull_canvas_item(p_canvas_item, p_transform, p_clip_rect, Color(1, 1, 1, 1), 0, z_list, z_last_list, nullptr, nullptr);
	}

	RasterizerCanvas::Item *list = nullptr;
	RasterizerCanvas::Item *list_end = nullptr;

	for (int i = 0; i < z_range; i++) {
		if (!z_list[i]) {
			continue;
		}
		if (!list) {
			list = z_list[i];
			list_end = z_last_list[i];
		} else {
			list_end->next = z_list[i];
			list_end = z_last_list[i];
		}
	}

	RENDER_TIMESTAMP("Render Canvas Items");

	RSG::canvas_render->canvas_render_items(p_to_render_target, list, p_modulate, p_lights, p_transform);
}

void _collect_ysort_children(RenderingServerCanvas::Item *p_canvas_item, Transform2D p_transform, RenderingServerCanvas::Item *p_material_owner, RenderingServerCanvas::Item **r_items, int &r_index) {
	int child_item_count = p_canvas_item->child_items.size();
	RenderingServerCanvas::Item **child_items = p_canvas_item->child_items.ptrw();
	for (int i = 0; i < child_item_count; i++) {
		if (child_items[i]->visible) {
			if (r_items) {
				r_items[r_index] = child_items[i];
				child_items[i]->ysort_xform = p_transform;
				child_items[i]->ysort_pos = p_transform.xform(child_items[i]->xform.elements[2]);
				child_items[i]->material_owner = child_items[i]->use_parent_material ? p_material_owner : nullptr;
			}

			r_index++;

			if (child_items[i]->sort_y) {
				_collect_ysort_children(child_items[i], p_transform * child_items[i]->xform, child_items[i]->use_parent_material ? p_material_owner : child_items[i], r_items, r_index);
			}
		}
	}
}

void _mark_ysort_dirty(RenderingServerCanvas::Item *ysort_owner, RID_PtrOwner<RenderingServerCanvas::Item> &canvas_item_owner) {
	do {
		ysort_owner->ysort_children_count = -1;
		ysort_owner = canvas_item_owner.owns(ysort_owner->parent) ? canvas_item_owner.getornull(ysort_owner->parent) : nullptr;
	} while (ysort_owner && ysort_owner->sort_y);
}

void RenderingServerCanvas::_cull_canvas_item(Item *p_canvas_item, const Transform2D &p_transform, const Rect2 &p_clip_rect, const Color &p_modulate, int p_z, RasterizerCanvas::Item **z_list, RasterizerCanvas::Item **z_last_list, Item *p_canvas_clip, Item *p_material_owner) {
	Item *ci = p_canvas_item;

	if (!ci->visible) {
		return;
	}

	if (ci->children_order_dirty) {
		ci->child_items.sort_custom<ItemIndexSort>();
		ci->children_order_dirty = false;
	}

	Rect2 rect = ci->get_rect();
	Transform2D xform = p_transform * ci->xform;
	Rect2 global_rect = xform.xform(rect);
	global_rect.position += p_clip_rect.position;

	if (ci->use_parent_material && p_material_owner) {
		ci->material_owner = p_material_owner;
	} else {
		p_material_owner = ci;
		ci->material_owner = nullptr;
	}

	Color modulate(ci->modulate.r * p_modulate.r, ci->modulate.g * p_modulate.g, ci->modulate.b * p_modulate.b, ci->modulate.a * p_modulate.a);

	if (modulate.a < 0.007) {
		return;
	}

	int child_item_count = ci->child_items.size();
	Item **child_items = ci->child_items.ptrw();

	if (ci->clip) {
		if (p_canvas_clip != nullptr) {
			ci->final_clip_rect = p_canvas_clip->final_clip_rect.clip(global_rect);
		} else {
			ci->final_clip_rect = global_rect;
		}
		ci->final_clip_owner = ci;

	} else {
		ci->final_clip_owner = p_canvas_clip;
	}

	if (ci->sort_y) {
		if (ci->ysort_children_count == -1) {
			ci->ysort_children_count = 0;
			_collect_ysort_children(ci, Transform2D(), p_material_owner, nullptr, ci->ysort_children_count);
		}

		child_item_count = ci->ysort_children_count;
		child_items = (Item **)alloca(child_item_count * sizeof(Item *));

		int i = 0;
		_collect_ysort_children(ci, Transform2D(), p_material_owner, child_items, i);

		SortArray<Item *, ItemPtrSort> sorter;
		sorter.sort(child_items, child_item_count);
	}

	if (ci->z_relative) {
		p_z = CLAMP(p_z + ci->z_index, RS::CANVAS_ITEM_Z_MIN, RS::CANVAS_ITEM_Z_MAX);
	} else {
		p_z = ci->z_index;
	}

	for (int i = 0; i < child_item_count; i++) {
		if (!child_items[i]->behind || (ci->sort_y && child_items[i]->sort_y)) {
			continue;
		}
		if (ci->sort_y) {
			_cull_canvas_item(child_items[i], xform * child_items[i]->ysort_xform, p_clip_rect, modulate, p_z, z_list, z_last_list, (Item *)ci->final_clip_owner, (Item *)child_items[i]->material_owner);
		} else {
			_cull_canvas_item(child_items[i], xform, p_clip_rect, modulate, p_z, z_list, z_last_list, (Item *)ci->final_clip_owner, p_material_owner);
		}
	}

	if (ci->copy_back_buffer) {
		ci->copy_back_buffer->screen_rect = xform.xform(ci->copy_back_buffer->rect).clip(p_clip_rect);
	}

	if (ci->update_when_visible) {
		RenderingServerRaster::redraw_request();
	}

	if ((ci->commands != nullptr && p_clip_rect.intersects(global_rect, true)) || ci->vp_render || ci->copy_back_buffer) {
		//something to draw?
		ci->final_transform = xform;
		ci->final_modulate = Color(modulate.r * ci->self_modulate.r, modulate.g * ci->self_modulate.g, modulate.b * ci->self_modulate.b, modulate.a * ci->self_modulate.a);
		ci->global_rect_cache = global_rect;
		ci->global_rect_cache.position -= p_clip_rect.position;
		ci->light_masked = false;

		int zidx = p_z - RS::CANVAS_ITEM_Z_MIN;

		if (z_last_list[zidx]) {
			z_last_list[zidx]->next = ci;
			z_last_list[zidx] = ci;

		} else {
			z_list[zidx] = ci;
			z_last_list[zidx] = ci;
		}

		ci->z_final = p_z;

		ci->next = nullptr;
	}

	for (int i = 0; i < child_item_count; i++) {
		if (child_items[i]->behind || (ci->sort_y && child_items[i]->sort_y)) {
			continue;
		}
		if (ci->sort_y) {
			_cull_canvas_item(child_items[i], xform * child_items[i]->ysort_xform, p_clip_rect, modulate, p_z, z_list, z_last_list, (Item *)ci->final_clip_owner, (Item *)child_items[i]->material_owner);
		} else {
			_cull_canvas_item(child_items[i], xform, p_clip_rect, modulate, p_z, z_list, z_last_list, (Item *)ci->final_clip_owner, p_material_owner);
		}
	}
}

void RenderingServerCanvas::_light_mask_canvas_items(int p_z, RasterizerCanvas::Item *p_canvas_item, RasterizerCanvas::Light *p_masked_lights) {
	if (!p_masked_lights) {
		return;
	}

	RasterizerCanvas::Item *ci = p_canvas_item;

	while (ci) {
		RasterizerCanvas::Light *light = p_masked_lights;
		while (light) {
			if (ci->light_mask & light->item_mask && p_z >= light->z_min && p_z <= light->z_max && ci->global_rect_cache.intersects_transformed(light->xform_cache, light->rect_cache)) {
				ci->light_masked = true;
			}

			light = light->mask_next_ptr;
		}

		ci = ci->next;
	}
}

void RenderingServerCanvas::render_canvas(RID p_render_target, Canvas *p_canvas, const Transform2D &p_transform, RasterizerCanvas::Light *p_lights, RasterizerCanvas::Light *p_masked_lights, const Rect2 &p_clip_rect) {
	RENDER_TIMESTAMP(">Render Canvas");

	if (p_canvas->children_order_dirty) {
		p_canvas->child_items.sort();
		p_canvas->children_order_dirty = false;
	}

	int l = p_canvas->child_items.size();
	Canvas::ChildItem *ci = p_canvas->child_items.ptrw();

	bool has_mirror = false;
	for (int i = 0; i < l; i++) {
		if (ci[i].mirror.x || ci[i].mirror.y) {
			has_mirror = true;
			break;
		}
	}

	if (!has_mirror) {
		_render_canvas_item_tree(p_render_target, ci, l, nullptr, p_transform, p_clip_rect, p_canvas->modulate, p_lights);

	} else {
		//used for parallaxlayer mirroring
		for (int i = 0; i < l; i++) {
			const Canvas::ChildItem &ci2 = p_canvas->child_items[i];
			_render_canvas_item_tree(p_render_target, nullptr, 0, ci2.item, p_transform, p_clip_rect, p_canvas->modulate, p_lights);

			//mirroring (useful for scrolling backgrounds)
			if (ci2.mirror.x != 0) {
				Transform2D xform2 = p_transform * Transform2D(0, Vector2(ci2.mirror.x, 0));
				_render_canvas_item_tree(p_render_target, nullptr, 0, ci2.item, xform2, p_clip_rect, p_canvas->modulate, p_lights);
			}
			if (ci2.mirror.y != 0) {
				Transform2D xform2 = p_transform * Transform2D(0, Vector2(0, ci2.mirror.y));
				_render_canvas_item_tree(p_render_target, nullptr, 0, ci2.item, xform2, p_clip_rect, p_canvas->modulate, p_lights);
			}
			if (ci2.mirror.y != 0 && ci2.mirror.x != 0) {
				Transform2D xform2 = p_transform * Transform2D(0, ci2.mirror);
				_render_canvas_item_tree(p_render_target, nullptr, 0, ci2.item, xform2, p_clip_rect, p_canvas->modulate, p_lights);
			}
		}
	}

	RENDER_TIMESTAMP("<End Render Canvas");
}

RID RenderingServerCanvas::canvas_create() {
	Canvas *canvas = memnew(Canvas);
	ERR_FAIL_COND_V(!canvas, RID());
	RID rid = canvas_owner.make_rid(canvas);

	return rid;
}

void RenderingServerCanvas::canvas_set_item_mirroring(RID p_canvas, RID p_item, const Point2 &p_mirroring) {
	Canvas *canvas = canvas_owner.getornull(p_canvas);
	ERR_FAIL_COND(!canvas);
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	int idx = canvas->find_item(canvas_item);
	ERR_FAIL_COND(idx == -1);
	canvas->child_items.write[idx].mirror = p_mirroring;
}

void RenderingServerCanvas::canvas_set_modulate(RID p_canvas, const Color &p_color) {
	Canvas *canvas = canvas_owner.getornull(p_canvas);
	ERR_FAIL_COND(!canvas);
	canvas->modulate = p_color;
}

void RenderingServerCanvas::canvas_set_disable_scale(bool p_disable) {
	disable_scale = p_disable;
}

void RenderingServerCanvas::canvas_set_parent(RID p_canvas, RID p_parent, float p_scale) {
	Canvas *canvas = canvas_owner.getornull(p_canvas);
	ERR_FAIL_COND(!canvas);

	canvas->parent = p_parent;
	canvas->parent_scale = p_scale;
}

RID RenderingServerCanvas::canvas_item_create() {
	Item *canvas_item = memnew(Item);
	ERR_FAIL_COND_V(!canvas_item, RID());

	return canvas_item_owner.make_rid(canvas_item);
}

void RenderingServerCanvas::canvas_item_set_parent(RID p_item, RID p_parent) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	if (canvas_item->parent.is_valid()) {
		if (canvas_owner.owns(canvas_item->parent)) {
			Canvas *canvas = canvas_owner.getornull(canvas_item->parent);
			canvas->erase_item(canvas_item);
		} else if (canvas_item_owner.owns(canvas_item->parent)) {
			Item *item_owner = canvas_item_owner.getornull(canvas_item->parent);
			item_owner->child_items.erase(canvas_item);

			if (item_owner->sort_y) {
				_mark_ysort_dirty(item_owner, canvas_item_owner);
			}
		}

		canvas_item->parent = RID();
	}

	if (p_parent.is_valid()) {
		if (canvas_owner.owns(p_parent)) {
			Canvas *canvas = canvas_owner.getornull(p_parent);
			Canvas::ChildItem ci;
			ci.item = canvas_item;
			canvas->child_items.push_back(ci);
			canvas->children_order_dirty = true;
		} else if (canvas_item_owner.owns(p_parent)) {
			Item *item_owner = canvas_item_owner.getornull(p_parent);
			item_owner->child_items.push_back(canvas_item);
			item_owner->children_order_dirty = true;

			if (item_owner->sort_y) {
				_mark_ysort_dirty(item_owner, canvas_item_owner);
			}

		} else {
			ERR_FAIL_MSG("Invalid parent.");
		}
	}

	canvas_item->parent = p_parent;
}

void RenderingServerCanvas::canvas_item_set_visible(RID p_item, bool p_visible) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	canvas_item->visible = p_visible;

	_mark_ysort_dirty(canvas_item, canvas_item_owner);
}

void RenderingServerCanvas::canvas_item_set_light_mask(RID p_item, int p_mask) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	canvas_item->light_mask = p_mask;
}

void RenderingServerCanvas::canvas_item_set_transform(RID p_item, const Transform2D &p_transform) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	canvas_item->xform = p_transform;
}

void RenderingServerCanvas::canvas_item_set_clip(RID p_item, bool p_clip) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	canvas_item->clip = p_clip;
}

void RenderingServerCanvas::canvas_item_set_distance_field_mode(RID p_item, bool p_enable) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	canvas_item->distance_field = p_enable;
}

void RenderingServerCanvas::canvas_item_set_custom_rect(RID p_item, bool p_custom_rect, const Rect2 &p_rect) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	canvas_item->custom_rect = p_custom_rect;
	canvas_item->rect = p_rect;
}

void RenderingServerCanvas::canvas_item_set_modulate(RID p_item, const Color &p_color) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	canvas_item->modulate = p_color;
}

void RenderingServerCanvas::canvas_item_set_self_modulate(RID p_item, const Color &p_color) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	canvas_item->self_modulate = p_color;
}

void RenderingServerCanvas::canvas_item_set_draw_behind_parent(RID p_item, bool p_enable) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	canvas_item->behind = p_enable;
}

void RenderingServerCanvas::canvas_item_set_update_when_visible(RID p_item, bool p_update) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	canvas_item->update_when_visible = p_update;
}

void RenderingServerCanvas::canvas_item_set_default_texture_filter(RID p_item, RS::CanvasItemTextureFilter p_filter) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);
	canvas_item->texture_filter = p_filter;
}

void RenderingServerCanvas::canvas_item_set_default_texture_repeat(RID p_item, RS::CanvasItemTextureRepeat p_repeat) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);
	canvas_item->texture_repeat = p_repeat;
}

void RenderingServerCanvas::canvas_item_add_line(RID p_item, const Point2 &p_from, const Point2 &p_to, const Color &p_color, float p_width) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	Item::CommandPrimitive *line = canvas_item->alloc_command<Item::CommandPrimitive>();
	ERR_FAIL_COND(!line);
	if (p_width > 1.001) {
		Vector2 t = (p_from - p_to).tangent().normalized();
		line->points[0] = p_from + t * p_width;
		line->points[1] = p_from - t * p_width;
		line->points[2] = p_to - t * p_width;
		line->points[3] = p_to + t * p_width;
		line->point_count = 4;
	} else {
		line->point_count = 2;
		line->points[0] = p_from;
		line->points[1] = p_to;
	}
	for (uint32_t i = 0; i < line->point_count; i++) {
		line->colors[i] = p_color;
	}
	line->specular_shininess = Color(1, 1, 1, 1);
}

void RenderingServerCanvas::canvas_item_add_polyline(RID p_item, const Vector<Point2> &p_points, const Vector<Color> &p_colors, float p_width) {
	ERR_FAIL_COND(p_points.size() < 2);
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	Item::CommandPolygon *pline = canvas_item->alloc_command<Item::CommandPolygon>();
	ERR_FAIL_COND(!pline);

	pline->texture_binding.create(canvas_item->texture_filter, canvas_item->texture_repeat, RID(), RID(), RID(), RS::CANVAS_ITEM_TEXTURE_FILTER_NEAREST, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED, RID());

	if (true || p_width <= 1) {
#define TODO make thick lines possible
		Vector<int> indices;
		int pc = p_points.size();
		indices.resize((pc - 1) * 2);
		{
			int *iptr = indices.ptrw();
			for (int i = 0; i < (pc - 1); i++) {
				iptr[i * 2 + 0] = i;
				iptr[i * 2 + 1] = i + 1;
			}
		}

		pline->primitive = RS::PRIMITIVE_LINES;
		pline->specular_shininess = Color(1, 1, 1, 1);
		pline->polygon.create(indices, p_points, p_colors);
	} else {
#if 0
		//make a trianglestrip for drawing the line...
		Vector2 prev_t;
		pline->triangles.resize(p_points.size() * 2);
		if (p_antialiased) {
			pline->lines.resize(p_points.size() * 2);
		}

		if (p_colors.size() == 0) {
			pline->triangle_colors.push_back(Color(1, 1, 1, 1));
			if (p_antialiased) {
				pline->line_colors.push_back(Color(1, 1, 1, 1));
			}
		} else if (p_colors.size() == 1) {
			pline->triangle_colors = p_colors;
			pline->line_colors = p_colors;
		} else {
			if (p_colors.size() != p_points.size()) {
				pline->triangle_colors.push_back(p_colors[0]);
				pline->line_colors.push_back(p_colors[0]);
			} else {
				pline->triangle_colors.resize(pline->triangles.size());
				pline->line_colors.resize(pline->lines.size());
			}
		}

		for (int i = 0; i < p_points.size(); i++) {

			Vector2 t;
			if (i == p_points.size() - 1) {
				t = prev_t;
			} else {
				t = (p_points[i + 1] - p_points[i]).normalized().tangent();
				if (i == 0) {
					prev_t = t;
				}
			}

			Vector2 tangent = ((t + prev_t).normalized()) * p_width * 0.5;

			if (p_antialiased) {
				pline->lines.write[i] = p_points[i] + tangent;
				pline->lines.write[p_points.size() * 2 - i - 1] = p_points[i] - tangent;
				if (pline->line_colors.size() > 1) {
					pline->line_colors.write[i] = p_colors[i];
					pline->line_colors.write[p_points.size() * 2 - i - 1] = p_colors[i];
				}
			}

			pline->triangles.write[i * 2 + 0] = p_points[i] + tangent;
			pline->triangles.write[i * 2 + 1] = p_points[i] - tangent;

			if (pline->triangle_colors.size() > 1) {

				pline->triangle_colors.write[i * 2 + 0] = p_colors[i];
				pline->triangle_colors.write[i * 2 + 1] = p_colors[i];
			}

			prev_t = t;
		}
#endif
	}
}

void RenderingServerCanvas::canvas_item_add_multiline(RID p_item, const Vector<Point2> &p_points, const Vector<Color> &p_colors, float p_width) {
	ERR_FAIL_COND(p_points.size() < 2);
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	Item::CommandPolygon *pline = canvas_item->alloc_command<Item::CommandPolygon>();
	ERR_FAIL_COND(!pline);

	pline->texture_binding.create(canvas_item->texture_filter, canvas_item->texture_repeat, RID(), RID(), RID(), RS::CANVAS_ITEM_TEXTURE_FILTER_NEAREST, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED, RID());

	if (true || p_width <= 1) {
#define TODO make thick lines possible

		pline->primitive = RS::PRIMITIVE_LINES;
		pline->specular_shininess = Color(1, 1, 1, 1);
		pline->polygon.create(Vector<int>(), p_points, p_colors);
	} else {
	}
}

void RenderingServerCanvas::canvas_item_add_rect(RID p_item, const Rect2 &p_rect, const Color &p_color) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	Item::CommandRect *rect = canvas_item->alloc_command<Item::CommandRect>();
	ERR_FAIL_COND(!rect);
	rect->modulate = p_color;
	rect->rect = p_rect;
}

void RenderingServerCanvas::canvas_item_add_circle(RID p_item, const Point2 &p_pos, float p_radius, const Color &p_color) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	Item::CommandPolygon *circle = canvas_item->alloc_command<Item::CommandPolygon>();
	ERR_FAIL_COND(!circle);

	circle->texture_binding.create(canvas_item->texture_filter, canvas_item->texture_repeat, RID(), RID(), RID(), RS::CANVAS_ITEM_TEXTURE_FILTER_NEAREST, RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED, RID());

	circle->primitive = RS::PRIMITIVE_TRIANGLES;
	circle->specular_shininess = Color(1, 1, 1, 1);

	Vector<int> indices;
	Vector<Vector2> points;

	static const int circle_points = 64;

	points.resize(circle_points);
	for (int i = 0; i < circle_points; i++) {
		float angle = (i / float(circle_points)) * 2 * Math_PI;
		points.write[i].x = Math::cos(angle) * p_radius;
		points.write[i].y = Math::sin(angle) * p_radius;
		points.write[i] += p_pos;
	}
	indices.resize((circle_points - 2) * 3);

	for (int i = 0; i < circle_points - 2; i++) {
		indices.write[i * 3 + 0] = 0;
		indices.write[i * 3 + 1] = i + 1;
		indices.write[i * 3 + 2] = i + 2;
	}

	Vector<Color> color;
	color.push_back(p_color);
	circle->polygon.create(indices, points, color);
}

void RenderingServerCanvas::canvas_item_add_texture_rect(RID p_item, const Rect2 &p_rect, RID p_texture, bool p_tile, const Color &p_modulate, bool p_transpose, RID p_normal_map, RID p_specular_map, const Color &p_specular_color_shininess, RenderingServer::CanvasItemTextureFilter p_filter, RenderingServer::CanvasItemTextureRepeat p_repeat) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	Item::CommandRect *rect = canvas_item->alloc_command<Item::CommandRect>();
	ERR_FAIL_COND(!rect);
	rect->modulate = p_modulate;
	rect->rect = p_rect;
	rect->flags = 0;
	if (p_tile) {
		rect->flags |= RasterizerCanvas::CANVAS_RECT_TILE;
		rect->flags |= RasterizerCanvas::CANVAS_RECT_REGION;
		rect->source = Rect2(0, 0, fabsf(p_rect.size.width), fabsf(p_rect.size.height));
	}

	if (p_rect.size.x < 0) {
		rect->flags |= RasterizerCanvas::CANVAS_RECT_FLIP_H;
		rect->rect.size.x = -rect->rect.size.x;
	}
	if (p_rect.size.y < 0) {
		rect->flags |= RasterizerCanvas::CANVAS_RECT_FLIP_V;
		rect->rect.size.y = -rect->rect.size.y;
	}
	if (p_transpose) {
		rect->flags |= RasterizerCanvas::CANVAS_RECT_TRANSPOSE;
		SWAP(rect->rect.size.x, rect->rect.size.y);
	}
	rect->texture_binding.create(canvas_item->texture_filter, canvas_item->texture_repeat, p_texture, p_normal_map, p_specular_map, p_filter, p_repeat, RID());
	rect->specular_shininess = p_specular_color_shininess;
}

void RenderingServerCanvas::canvas_item_add_texture_rect_region(RID p_item, const Rect2 &p_rect, RID p_texture, const Rect2 &p_src_rect, const Color &p_modulate, bool p_transpose, RID p_normal_map, RID p_specular_map, const Color &p_specular_color_shininess, bool p_clip_uv, RenderingServer::CanvasItemTextureFilter p_filter, RenderingServer::CanvasItemTextureRepeat p_repeat) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	Item::CommandRect *rect = canvas_item->alloc_command<Item::CommandRect>();
	ERR_FAIL_COND(!rect);
	rect->modulate = p_modulate;
	rect->rect = p_rect;
	rect->texture_binding.create(canvas_item->texture_filter, canvas_item->texture_repeat, p_texture, p_normal_map, p_specular_map, p_filter, p_repeat, RID());
	rect->specular_shininess = p_specular_color_shininess;
	rect->source = p_src_rect;
	rect->flags = RasterizerCanvas::CANVAS_RECT_REGION;

	if (p_rect.size.x < 0) {
		rect->flags |= RasterizerCanvas::CANVAS_RECT_FLIP_H;
		rect->rect.size.x = -rect->rect.size.x;
	}
	if (p_src_rect.size.x < 0) {
		rect->flags ^= RasterizerCanvas::CANVAS_RECT_FLIP_H;
		rect->source.size.x = -rect->source.size.x;
	}
	if (p_rect.size.y < 0) {
		rect->flags |= RasterizerCanvas::CANVAS_RECT_FLIP_V;
		rect->rect.size.y = -rect->rect.size.y;
	}
	if (p_src_rect.size.y < 0) {
		rect->flags ^= RasterizerCanvas::CANVAS_RECT_FLIP_V;
		rect->source.size.y = -rect->source.size.y;
	}

	if (p_transpose) {
		rect->flags |= RasterizerCanvas::CANVAS_RECT_TRANSPOSE;
		SWAP(rect->rect.size.x, rect->rect.size.y);
	}

	if (p_clip_uv) {
		rect->flags |= RasterizerCanvas::CANVAS_RECT_CLIP_UV;
	}
}

void RenderingServerCanvas::canvas_item_add_nine_patch(RID p_item, const Rect2 &p_rect, const Rect2 &p_source, RID p_texture, const Vector2 &p_topleft, const Vector2 &p_bottomright, RS::NinePatchAxisMode p_x_axis_mode, RS::NinePatchAxisMode p_y_axis_mode, bool p_draw_center, const Color &p_modulate, RID p_normal_map, RID p_specular_map, const Color &p_specular_color_shininess, RenderingServer::CanvasItemTextureFilter p_filter, RenderingServer::CanvasItemTextureRepeat p_repeat) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	Item::CommandNinePatch *style = canvas_item->alloc_command<Item::CommandNinePatch>();
	ERR_FAIL_COND(!style);
	style->texture_binding.create(canvas_item->texture_filter, canvas_item->texture_repeat, p_texture, p_normal_map, p_specular_map, p_filter, p_repeat, RID());
	style->specular_shininess = p_specular_color_shininess;
	style->rect = p_rect;
	style->source = p_source;
	style->draw_center = p_draw_center;
	style->color = p_modulate;
	style->margin[MARGIN_LEFT] = p_topleft.x;
	style->margin[MARGIN_TOP] = p_topleft.y;
	style->margin[MARGIN_RIGHT] = p_bottomright.x;
	style->margin[MARGIN_BOTTOM] = p_bottomright.y;
	style->axis_x = p_x_axis_mode;
	style->axis_y = p_y_axis_mode;
}

void RenderingServerCanvas::canvas_item_add_primitive(RID p_item, const Vector<Point2> &p_points, const Vector<Color> &p_colors, const Vector<Point2> &p_uvs, RID p_texture, float p_width, RID p_normal_map, RID p_specular_map, const Color &p_specular_color_shininess, RenderingServer::CanvasItemTextureFilter p_filter, RenderingServer::CanvasItemTextureRepeat p_repeat) {
	uint32_t pc = p_points.size();
	ERR_FAIL_COND(pc == 0 || pc > 4);

	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	Item::CommandPrimitive *prim = canvas_item->alloc_command<Item::CommandPrimitive>();
	ERR_FAIL_COND(!prim);

	for (int i = 0; i < p_points.size(); i++) {
		prim->points[i] = p_points[i];
		if (i < p_uvs.size()) {
			prim->uvs[i] = p_uvs[i];
		}
		if (i < p_colors.size()) {
			prim->colors[i] = p_colors[i];
		} else if (p_colors.size()) {
			prim->colors[i] = p_colors[0];
		} else {
			prim->colors[i] = Color(1, 1, 1, 1);
		}
	}

	prim->point_count = p_points.size();

	prim->texture_binding.create(canvas_item->texture_filter, canvas_item->texture_repeat, p_texture, p_normal_map, p_specular_map, p_filter, p_repeat, RID());
	prim->specular_shininess = p_specular_color_shininess;
}

void RenderingServerCanvas::canvas_item_add_polygon(RID p_item, const Vector<Point2> &p_points, const Vector<Color> &p_colors, const Vector<Point2> &p_uvs, RID p_texture, RID p_normal_map, RID p_specular_map, const Color &p_specular_color_shininess, RenderingServer::CanvasItemTextureFilter p_filter, RenderingServer::CanvasItemTextureRepeat p_repeat) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);
#ifdef DEBUG_ENABLED
	int pointcount = p_points.size();
	ERR_FAIL_COND(pointcount < 3);
	int color_size = p_colors.size();
	int uv_size = p_uvs.size();
	ERR_FAIL_COND(color_size != 0 && color_size != 1 && color_size != pointcount);
	ERR_FAIL_COND(uv_size != 0 && (uv_size != pointcount));
#endif
	Vector<int> indices = Geometry2D::triangulate_polygon(p_points);
	ERR_FAIL_COND_MSG(indices.empty(), "Invalid polygon data, triangulation failed.");

	Item::CommandPolygon *polygon = canvas_item->alloc_command<Item::CommandPolygon>();
	ERR_FAIL_COND(!polygon);
	polygon->primitive = RS::PRIMITIVE_TRIANGLES;
	polygon->texture_binding.create(canvas_item->texture_filter, canvas_item->texture_repeat, p_texture, p_normal_map, p_specular_map, p_filter, p_repeat, RID());
	polygon->specular_shininess = p_specular_color_shininess;
	polygon->polygon.create(indices, p_points, p_colors, p_uvs);
}

void RenderingServerCanvas::canvas_item_add_triangle_array(RID p_item, const Vector<int> &p_indices, const Vector<Point2> &p_points, const Vector<Color> &p_colors, const Vector<Point2> &p_uvs, const Vector<int> &p_bones, const Vector<float> &p_weights, RID p_texture, int p_count, RID p_normal_map, RID p_specular_map, const Color &p_specular_color_shininess, RenderingServer::CanvasItemTextureFilter p_filter, RenderingServer::CanvasItemTextureRepeat p_repeat) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	int vertex_count = p_points.size();
	ERR_FAIL_COND(vertex_count == 0);
	ERR_FAIL_COND(!p_colors.empty() && p_colors.size() != vertex_count && p_colors.size() != 1);
	ERR_FAIL_COND(!p_uvs.empty() && p_uvs.size() != vertex_count);
	ERR_FAIL_COND(!p_bones.empty() && p_bones.size() != vertex_count * 4);
	ERR_FAIL_COND(!p_weights.empty() && p_weights.size() != vertex_count * 4);

	Vector<int> indices = p_indices;

	Item::CommandPolygon *polygon = canvas_item->alloc_command<Item::CommandPolygon>();
	ERR_FAIL_COND(!polygon);
	polygon->texture_binding.create(canvas_item->texture_filter, canvas_item->texture_repeat, p_texture, p_normal_map, p_specular_map, p_filter, p_repeat, RID());
	polygon->specular_shininess = p_specular_color_shininess;
	polygon->polygon.create(indices, p_points, p_colors, p_uvs, p_bones, p_weights);

	polygon->primitive = RS::PRIMITIVE_TRIANGLES;
}

void RenderingServerCanvas::canvas_item_add_set_transform(RID p_item, const Transform2D &p_transform) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	Item::CommandTransform *tr = canvas_item->alloc_command<Item::CommandTransform>();
	ERR_FAIL_COND(!tr);
	tr->xform = p_transform;
}

void RenderingServerCanvas::canvas_item_add_mesh(RID p_item, const RID &p_mesh, const Transform2D &p_transform, const Color &p_modulate, RID p_texture, RID p_normal_map, RID p_specular_map, const Color &p_specular_color_shininess, RenderingServer::CanvasItemTextureFilter p_filter, RenderingServer::CanvasItemTextureRepeat p_repeat) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	Item::CommandMesh *m = canvas_item->alloc_command<Item::CommandMesh>();
	ERR_FAIL_COND(!m);
	m->mesh = p_mesh;
	m->texture_binding.create(canvas_item->texture_filter, canvas_item->texture_repeat, p_texture, p_normal_map, p_specular_map, p_filter, p_repeat, RID());
	m->specular_shininess = p_specular_color_shininess;
	m->transform = p_transform;
	m->modulate = p_modulate;
}

void RenderingServerCanvas::canvas_item_add_particles(RID p_item, RID p_particles, RID p_texture, RID p_normal_map, RID p_specular_map, const Color &p_specular_color_shininess, RenderingServer::CanvasItemTextureFilter p_filter, RenderingServer::CanvasItemTextureRepeat p_repeat) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	Item::CommandParticles *part = canvas_item->alloc_command<Item::CommandParticles>();
	ERR_FAIL_COND(!part);
	part->particles = p_particles;
	part->texture_binding.create(canvas_item->texture_filter, canvas_item->texture_repeat, p_texture, p_normal_map, p_specular_map, p_filter, p_repeat, RID());
	part->specular_shininess = p_specular_color_shininess;

	//take the chance and request processing for them, at least once until they become visible again
	RSG::storage->particles_request_process(p_particles);
}

void RenderingServerCanvas::canvas_item_add_multimesh(RID p_item, RID p_mesh, RID p_texture, RID p_normal_map, RID p_specular_map, const Color &p_specular_color_shininess, RenderingServer::CanvasItemTextureFilter p_filter, RenderingServer::CanvasItemTextureRepeat p_repeat) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	Item::CommandMultiMesh *mm = canvas_item->alloc_command<Item::CommandMultiMesh>();
	ERR_FAIL_COND(!mm);
	mm->multimesh = p_mesh;
	mm->texture_binding.create(canvas_item->texture_filter, canvas_item->texture_repeat, p_texture, p_normal_map, p_specular_map, p_filter, p_repeat, mm->multimesh);
	mm->specular_shininess = p_specular_color_shininess;
}

void RenderingServerCanvas::canvas_item_add_clip_ignore(RID p_item, bool p_ignore) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	Item::CommandClipIgnore *ci = canvas_item->alloc_command<Item::CommandClipIgnore>();
	ERR_FAIL_COND(!ci);
	ci->ignore = p_ignore;
}

void RenderingServerCanvas::canvas_item_set_sort_children_by_y(RID p_item, bool p_enable) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	canvas_item->sort_y = p_enable;

	_mark_ysort_dirty(canvas_item, canvas_item_owner);
}

void RenderingServerCanvas::canvas_item_set_z_index(RID p_item, int p_z) {
	ERR_FAIL_COND(p_z < RS::CANVAS_ITEM_Z_MIN || p_z > RS::CANVAS_ITEM_Z_MAX);

	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	canvas_item->z_index = p_z;
}

void RenderingServerCanvas::canvas_item_set_z_as_relative_to_parent(RID p_item, bool p_enable) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	canvas_item->z_relative = p_enable;
}

void RenderingServerCanvas::canvas_item_attach_skeleton(RID p_item, RID p_skeleton) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	canvas_item->skeleton = p_skeleton;
}

void RenderingServerCanvas::canvas_item_set_copy_to_backbuffer(RID p_item, bool p_enable, const Rect2 &p_rect) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);
	if (bool(canvas_item->copy_back_buffer != nullptr) != p_enable) {
		if (p_enable) {
			canvas_item->copy_back_buffer = memnew(RasterizerCanvas::Item::CopyBackBuffer);
		} else {
			memdelete(canvas_item->copy_back_buffer);
			canvas_item->copy_back_buffer = nullptr;
		}
	}

	if (p_enable) {
		canvas_item->copy_back_buffer->rect = p_rect;
		canvas_item->copy_back_buffer->full = p_rect == Rect2();
	}
}

void RenderingServerCanvas::canvas_item_clear(RID p_item) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	canvas_item->clear();
}

void RenderingServerCanvas::canvas_item_set_draw_index(RID p_item, int p_index) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	canvas_item->index = p_index;

	if (canvas_item_owner.owns(canvas_item->parent)) {
		Item *canvas_item_parent = canvas_item_owner.getornull(canvas_item->parent);
		canvas_item_parent->children_order_dirty = true;
		return;
	}

	Canvas *canvas = canvas_owner.getornull(canvas_item->parent);
	if (canvas) {
		canvas->children_order_dirty = true;
		return;
	}
}

void RenderingServerCanvas::canvas_item_set_material(RID p_item, RID p_material) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	canvas_item->material = p_material;
}

void RenderingServerCanvas::canvas_item_set_use_parent_material(RID p_item, bool p_enable) {
	Item *canvas_item = canvas_item_owner.getornull(p_item);
	ERR_FAIL_COND(!canvas_item);

	canvas_item->use_parent_material = p_enable;
}

RID RenderingServerCanvas::canvas_light_create() {
	RasterizerCanvas::Light *clight = memnew(RasterizerCanvas::Light);
	clight->light_internal = RSG::canvas_render->light_create();
	return canvas_light_owner.make_rid(clight);
}

void RenderingServerCanvas::canvas_light_attach_to_canvas(RID p_light, RID p_canvas) {
	RasterizerCanvas::Light *clight = canvas_light_owner.getornull(p_light);
	ERR_FAIL_COND(!clight);

	if (clight->canvas.is_valid()) {
		Canvas *canvas = canvas_owner.getornull(clight->canvas);
		canvas->lights.erase(clight);
	}

	if (!canvas_owner.owns(p_canvas)) {
		p_canvas = RID();
	}

	clight->canvas = p_canvas;

	if (clight->canvas.is_valid()) {
		Canvas *canvas = canvas_owner.getornull(clight->canvas);
		canvas->lights.insert(clight);
	}
}

void RenderingServerCanvas::canvas_light_set_enabled(RID p_light, bool p_enabled) {
	RasterizerCanvas::Light *clight = canvas_light_owner.getornull(p_light);
	ERR_FAIL_COND(!clight);

	clight->enabled = p_enabled;
}

void RenderingServerCanvas::canvas_light_set_scale(RID p_light, float p_scale) {
	RasterizerCanvas::Light *clight = canvas_light_owner.getornull(p_light);
	ERR_FAIL_COND(!clight);

	clight->scale = p_scale;
}

void RenderingServerCanvas::canvas_light_set_transform(RID p_light, const Transform2D &p_transform) {
	RasterizerCanvas::Light *clight = canvas_light_owner.getornull(p_light);
	ERR_FAIL_COND(!clight);

	clight->xform = p_transform;
}

void RenderingServerCanvas::canvas_light_set_texture(RID p_light, RID p_texture) {
	RasterizerCanvas::Light *clight = canvas_light_owner.getornull(p_light);
	ERR_FAIL_COND(!clight);

	clight->texture = p_texture;
	clight->version++;
	RSG::canvas_render->light_set_texture(clight->light_internal, p_texture);
}

void RenderingServerCanvas::canvas_light_set_texture_offset(RID p_light, const Vector2 &p_offset) {
	RasterizerCanvas::Light *clight = canvas_light_owner.getornull(p_light);
	ERR_FAIL_COND(!clight);

	clight->texture_offset = p_offset;
}

void RenderingServerCanvas::canvas_light_set_color(RID p_light, const Color &p_color) {
	RasterizerCanvas::Light *clight = canvas_light_owner.getornull(p_light);
	ERR_FAIL_COND(!clight);

	clight->color = p_color;
}

void RenderingServerCanvas::canvas_light_set_height(RID p_light, float p_height) {
	RasterizerCanvas::Light *clight = canvas_light_owner.getornull(p_light);
	ERR_FAIL_COND(!clight);

	clight->height = p_height;
}

void RenderingServerCanvas::canvas_light_set_energy(RID p_light, float p_energy) {
	RasterizerCanvas::Light *clight = canvas_light_owner.getornull(p_light);
	ERR_FAIL_COND(!clight);

	clight->energy = p_energy;
}

void RenderingServerCanvas::canvas_light_set_z_range(RID p_light, int p_min_z, int p_max_z) {
	RasterizerCanvas::Light *clight = canvas_light_owner.getornull(p_light);
	ERR_FAIL_COND(!clight);

	clight->z_min = p_min_z;
	clight->z_max = p_max_z;
}

void RenderingServerCanvas::canvas_light_set_layer_range(RID p_light, int p_min_layer, int p_max_layer) {
	RasterizerCanvas::Light *clight = canvas_light_owner.getornull(p_light);
	ERR_FAIL_COND(!clight);

	clight->layer_max = p_max_layer;
	clight->layer_min = p_min_layer;
}

void RenderingServerCanvas::canvas_light_set_item_cull_mask(RID p_light, int p_mask) {
	RasterizerCanvas::Light *clight = canvas_light_owner.getornull(p_light);
	ERR_FAIL_COND(!clight);

	clight->item_mask = p_mask;
}

void RenderingServerCanvas::canvas_light_set_item_shadow_cull_mask(RID p_light, int p_mask) {
	RasterizerCanvas::Light *clight = canvas_light_owner.getornull(p_light);
	ERR_FAIL_COND(!clight);

	clight->item_shadow_mask = p_mask;
}

void RenderingServerCanvas::canvas_light_set_mode(RID p_light, RS::CanvasLightMode p_mode) {
	RasterizerCanvas::Light *clight = canvas_light_owner.getornull(p_light);
	ERR_FAIL_COND(!clight);

	clight->mode = p_mode;
}

void RenderingServerCanvas::canvas_light_set_shadow_enabled(RID p_light, bool p_enabled) {
	RasterizerCanvas::Light *clight = canvas_light_owner.getornull(p_light);
	ERR_FAIL_COND(!clight);

	if (clight->use_shadow == p_enabled) {
		return;
	}
	clight->use_shadow = p_enabled;
	clight->version++;
	RSG::canvas_render->light_set_use_shadow(clight->light_internal, clight->use_shadow, clight->shadow_buffer_size);
}

void RenderingServerCanvas::canvas_light_set_shadow_buffer_size(RID p_light, int p_size) {
	ERR_FAIL_COND(p_size < 32 || p_size > 16384);

	RasterizerCanvas::Light *clight = canvas_light_owner.getornull(p_light);
	ERR_FAIL_COND(!clight);

	int new_size = next_power_of_2(p_size);
	if (new_size == clight->shadow_buffer_size) {
		return;
	}

	clight->shadow_buffer_size = next_power_of_2(p_size);
	clight->version++;

	RSG::canvas_render->light_set_use_shadow(clight->light_internal, clight->use_shadow, clight->shadow_buffer_size);
}

void RenderingServerCanvas::canvas_light_set_shadow_filter(RID p_light, RS::CanvasLightShadowFilter p_filter) {
	RasterizerCanvas::Light *clight = canvas_light_owner.getornull(p_light);
	ERR_FAIL_COND(!clight);

	clight->shadow_filter = p_filter;
}

void RenderingServerCanvas::canvas_light_set_shadow_color(RID p_light, const Color &p_color) {
	RasterizerCanvas::Light *clight = canvas_light_owner.getornull(p_light);
	ERR_FAIL_COND(!clight);

	clight->shadow_color = p_color;
}

void RenderingServerCanvas::canvas_light_set_shadow_smooth(RID p_light, float p_smooth) {
	RasterizerCanvas::Light *clight = canvas_light_owner.getornull(p_light);
	ERR_FAIL_COND(!clight);
	clight->shadow_smooth = p_smooth;
}

RID RenderingServerCanvas::canvas_light_occluder_create() {
	RasterizerCanvas::LightOccluderInstance *occluder = memnew(RasterizerCanvas::LightOccluderInstance);

	return canvas_light_occluder_owner.make_rid(occluder);
}

void RenderingServerCanvas::canvas_light_occluder_attach_to_canvas(RID p_occluder, RID p_canvas) {
	RasterizerCanvas::LightOccluderInstance *occluder = canvas_light_occluder_owner.getornull(p_occluder);
	ERR_FAIL_COND(!occluder);

	if (occluder->canvas.is_valid()) {
		Canvas *canvas = canvas_owner.getornull(occluder->canvas);
		canvas->occluders.erase(occluder);
	}

	if (!canvas_owner.owns(p_canvas)) {
		p_canvas = RID();
	}

	occluder->canvas = p_canvas;

	if (occluder->canvas.is_valid()) {
		Canvas *canvas = canvas_owner.getornull(occluder->canvas);
		canvas->occluders.insert(occluder);
	}
}

void RenderingServerCanvas::canvas_light_occluder_set_enabled(RID p_occluder, bool p_enabled) {
	RasterizerCanvas::LightOccluderInstance *occluder = canvas_light_occluder_owner.getornull(p_occluder);
	ERR_FAIL_COND(!occluder);

	occluder->enabled = p_enabled;
}

void RenderingServerCanvas::canvas_light_occluder_set_polygon(RID p_occluder, RID p_polygon) {
	RasterizerCanvas::LightOccluderInstance *occluder = canvas_light_occluder_owner.getornull(p_occluder);
	ERR_FAIL_COND(!occluder);

	if (occluder->polygon.is_valid()) {
		LightOccluderPolygon *occluder_poly = canvas_light_occluder_polygon_owner.getornull(p_polygon);
		if (occluder_poly) {
			occluder_poly->owners.erase(occluder);
		}
	}

	occluder->polygon = p_polygon;
	occluder->occluder = RID();

	if (occluder->polygon.is_valid()) {
		LightOccluderPolygon *occluder_poly = canvas_light_occluder_polygon_owner.getornull(p_polygon);
		if (!occluder_poly) {
			occluder->polygon = RID();
			ERR_FAIL_COND(!occluder_poly);
		} else {
			occluder_poly->owners.insert(occluder);
			occluder->occluder = occluder_poly->occluder;
			occluder->aabb_cache = occluder_poly->aabb;
			occluder->cull_cache = occluder_poly->cull_mode;
		}
	}
}

void RenderingServerCanvas::canvas_light_occluder_set_transform(RID p_occluder, const Transform2D &p_xform) {
	RasterizerCanvas::LightOccluderInstance *occluder = canvas_light_occluder_owner.getornull(p_occluder);
	ERR_FAIL_COND(!occluder);

	occluder->xform = p_xform;
}

void RenderingServerCanvas::canvas_light_occluder_set_light_mask(RID p_occluder, int p_mask) {
	RasterizerCanvas::LightOccluderInstance *occluder = canvas_light_occluder_owner.getornull(p_occluder);
	ERR_FAIL_COND(!occluder);

	occluder->light_mask = p_mask;
}

RID RenderingServerCanvas::canvas_occluder_polygon_create() {
	LightOccluderPolygon *occluder_poly = memnew(LightOccluderPolygon);
	occluder_poly->occluder = RSG::canvas_render->occluder_polygon_create();
	return canvas_light_occluder_polygon_owner.make_rid(occluder_poly);
}

void RenderingServerCanvas::canvas_occluder_polygon_set_shape(RID p_occluder_polygon, const Vector<Vector2> &p_shape, bool p_closed) {
	if (p_shape.size() < 3) {
		canvas_occluder_polygon_set_shape_as_lines(p_occluder_polygon, p_shape);
		return;
	}

	Vector<Vector2> lines;
	int lc = p_shape.size() * 2;

	lines.resize(lc - (p_closed ? 0 : 2));
	{
		Vector2 *w = lines.ptrw();
		const Vector2 *r = p_shape.ptr();

		int max = lc / 2;
		if (!p_closed) {
			max--;
		}
		for (int i = 0; i < max; i++) {
			Vector2 a = r[i];
			Vector2 b = r[(i + 1) % (lc / 2)];
			w[i * 2 + 0] = a;
			w[i * 2 + 1] = b;
		}
	}

	canvas_occluder_polygon_set_shape_as_lines(p_occluder_polygon, lines);
}

void RenderingServerCanvas::canvas_occluder_polygon_set_shape_as_lines(RID p_occluder_polygon, const Vector<Vector2> &p_shape) {
	LightOccluderPolygon *occluder_poly = canvas_light_occluder_polygon_owner.getornull(p_occluder_polygon);
	ERR_FAIL_COND(!occluder_poly);
	ERR_FAIL_COND(p_shape.size() & 1);

	int lc = p_shape.size();
	occluder_poly->aabb = Rect2();
	{
		const Vector2 *r = p_shape.ptr();
		for (int i = 0; i < lc; i++) {
			if (i == 0) {
				occluder_poly->aabb.position = r[i];
			} else {
				occluder_poly->aabb.expand_to(r[i]);
			}
		}
	}

	RSG::canvas_render->occluder_polygon_set_shape_as_lines(occluder_poly->occluder, p_shape);
	for (Set<RasterizerCanvas::LightOccluderInstance *>::Element *E = occluder_poly->owners.front(); E; E = E->next()) {
		E->get()->aabb_cache = occluder_poly->aabb;
	}
}

void RenderingServerCanvas::canvas_occluder_polygon_set_cull_mode(RID p_occluder_polygon, RS::CanvasOccluderPolygonCullMode p_mode) {
	LightOccluderPolygon *occluder_poly = canvas_light_occluder_polygon_owner.getornull(p_occluder_polygon);
	ERR_FAIL_COND(!occluder_poly);
	occluder_poly->cull_mode = p_mode;
	RSG::canvas_render->occluder_polygon_set_cull_mode(occluder_poly->occluder, p_mode);
	for (Set<RasterizerCanvas::LightOccluderInstance *>::Element *E = occluder_poly->owners.front(); E; E = E->next()) {
		E->get()->cull_cache = p_mode;
	}
}

bool RenderingServerCanvas::free(RID p_rid) {
	if (canvas_owner.owns(p_rid)) {
		Canvas *canvas = canvas_owner.getornull(p_rid);
		ERR_FAIL_COND_V(!canvas, false);

		while (canvas->viewports.size()) {
			RenderingServerViewport::Viewport *vp = RSG::viewport->viewport_owner.getornull(canvas->viewports.front()->get());
			ERR_FAIL_COND_V(!vp, true);

			Map<RID, RenderingServerViewport::Viewport::CanvasData>::Element *E = vp->canvas_map.find(p_rid);
			ERR_FAIL_COND_V(!E, true);
			vp->canvas_map.erase(p_rid);

			canvas->viewports.erase(canvas->viewports.front());
		}

		for (int i = 0; i < canvas->child_items.size(); i++) {
			canvas->child_items[i].item->parent = RID();
		}

		for (Set<RasterizerCanvas::Light *>::Element *E = canvas->lights.front(); E; E = E->next()) {
			E->get()->canvas = RID();
		}

		for (Set<RasterizerCanvas::LightOccluderInstance *>::Element *E = canvas->occluders.front(); E; E = E->next()) {
			E->get()->canvas = RID();
		}

		canvas_owner.free(p_rid);

		memdelete(canvas);

	} else if (canvas_item_owner.owns(p_rid)) {
		Item *canvas_item = canvas_item_owner.getornull(p_rid);
		ERR_FAIL_COND_V(!canvas_item, true);

		if (canvas_item->parent.is_valid()) {
			if (canvas_owner.owns(canvas_item->parent)) {
				Canvas *canvas = canvas_owner.getornull(canvas_item->parent);
				canvas->erase_item(canvas_item);
			} else if (canvas_item_owner.owns(canvas_item->parent)) {
				Item *item_owner = canvas_item_owner.getornull(canvas_item->parent);
				item_owner->child_items.erase(canvas_item);

				if (item_owner->sort_y) {
					_mark_ysort_dirty(item_owner, canvas_item_owner);
				}
			}
		}

		for (int i = 0; i < canvas_item->child_items.size(); i++) {
			canvas_item->child_items[i]->parent = RID();
		}

		/*
		if (canvas_item->material) {
			canvas_item->material->owners.erase(canvas_item);
		}
		*/

		canvas_item_owner.free(p_rid);

		memdelete(canvas_item);

	} else if (canvas_light_owner.owns(p_rid)) {
		RasterizerCanvas::Light *canvas_light = canvas_light_owner.getornull(p_rid);
		ERR_FAIL_COND_V(!canvas_light, true);

		if (canvas_light->canvas.is_valid()) {
			Canvas *canvas = canvas_owner.getornull(canvas_light->canvas);
			if (canvas) {
				canvas->lights.erase(canvas_light);
			}
		}

		RSG::canvas_render->free(canvas_light->light_internal);

		canvas_light_owner.free(p_rid);
		memdelete(canvas_light);

	} else if (canvas_light_occluder_owner.owns(p_rid)) {
		RasterizerCanvas::LightOccluderInstance *occluder = canvas_light_occluder_owner.getornull(p_rid);
		ERR_FAIL_COND_V(!occluder, true);

		if (occluder->polygon.is_valid()) {
			LightOccluderPolygon *occluder_poly = canvas_light_occluder_polygon_owner.getornull(occluder->polygon);
			if (occluder_poly) {
				occluder_poly->owners.erase(occluder);
			}
		}

		if (occluder->canvas.is_valid() && canvas_owner.owns(occluder->canvas)) {
			Canvas *canvas = canvas_owner.getornull(occluder->canvas);
			canvas->occluders.erase(occluder);
		}

		canvas_light_occluder_owner.free(p_rid);
		memdelete(occluder);

	} else if (canvas_light_occluder_polygon_owner.owns(p_rid)) {
		LightOccluderPolygon *occluder_poly = canvas_light_occluder_polygon_owner.getornull(p_rid);
		ERR_FAIL_COND_V(!occluder_poly, true);
		RSG::canvas_render->free(occluder_poly->occluder);

		while (occluder_poly->owners.size()) {
			occluder_poly->owners.front()->get()->polygon = RID();
			occluder_poly->owners.erase(occluder_poly->owners.front());
		}

		canvas_light_occluder_polygon_owner.free(p_rid);
		memdelete(occluder_poly);
	} else {
		return false;
	}

	return true;
}

RenderingServerCanvas::RenderingServerCanvas() {
	z_list = (RasterizerCanvas::Item **)memalloc(z_range * sizeof(RasterizerCanvas::Item *));
	z_last_list = (RasterizerCanvas::Item **)memalloc(z_range * sizeof(RasterizerCanvas::Item *));

	disable_scale = false;
}

RenderingServerCanvas::~RenderingServerCanvas() {
	memfree(z_list);
	memfree(z_last_list);
}
