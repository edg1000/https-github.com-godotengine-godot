/**************************************************************************/
/*  graph_edit.cpp                                                        */
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

#include "graph_edit.h"

#include "core/input/input.h"
#include "core/math/math_funcs.h"
#include "core/os/keyboard.h"
#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/graph_edit_arranger.h"
#include "scene/gui/view_panner.h"

constexpr int MINIMAP_OFFSET = 12;
constexpr int MINIMAP_PADDING = 5;
constexpr int MIN_DRAG_DISTANCE_FOR_VALID_CONNECTION = 20;
constexpr int MAX_CONNECTION_LINE_CURVE_TESSELATION_STAGES = 5;

bool GraphEditFilter::has_point(const Point2 &p_point) const {
	return ge->_filter_input(p_point);
}

GraphEditFilter::GraphEditFilter(GraphEdit *p_edit) {
	ge = p_edit;
}

GraphEditMinimap::GraphEditMinimap(GraphEdit *p_edit) {
	ge = p_edit;

	graph_proportions = Vector2(1, 1);
	graph_padding = Vector2(0, 0);
	camera_position = Vector2(100, 50);
	camera_size = Vector2(200, 200);
	minimap_padding = Vector2(MINIMAP_PADDING, MINIMAP_PADDING);
	minimap_offset = minimap_padding + _convert_from_graph_position(graph_padding);

	is_pressing = false;
	is_resizing = false;
}

Control::CursorShape GraphEditMinimap::get_cursor_shape(const Point2 &p_pos) const {
	Ref<Texture2D> resizer = get_theme_icon(SNAME("resizer"));
	if (is_resizing || (p_pos.x < resizer->get_width() && p_pos.y < resizer->get_height())) {
		return CURSOR_FDIAGSIZE;
	}

	return Control::get_cursor_shape(p_pos);
}

void GraphEditMinimap::update_minimap() {
	Vector2 graph_offset = _get_graph_offset();
	Vector2 graph_size = _get_graph_size();

	camera_position = ge->get_scroll_ofs() - graph_offset;
	camera_size = ge->get_size();

	Vector2 render_size = _get_render_size();
	float target_ratio = render_size.x / render_size.y;
	float graph_ratio = graph_size.x / graph_size.y;

	graph_proportions = graph_size;
	graph_padding = Vector2(0, 0);
	if (graph_ratio > target_ratio) {
		graph_proportions.x = graph_size.x;
		graph_proportions.y = graph_size.x / target_ratio;
		graph_padding.y = Math::abs(graph_size.y - graph_proportions.y) / 2;
	} else {
		graph_proportions.x = graph_size.y * target_ratio;
		graph_proportions.y = graph_size.y;
		graph_padding.x = Math::abs(graph_size.x - graph_proportions.x) / 2;
	}

	// This centers minimap inside the minimap rectangle.
	minimap_offset = minimap_padding + _convert_from_graph_position(graph_padding);
}

Rect2 GraphEditMinimap::get_camera_rect() {
	Vector2 camera_center = _convert_from_graph_position(camera_position + camera_size / 2) + minimap_offset;
	Vector2 camera_viewport = _convert_from_graph_position(camera_size);
	Vector2 camera_pos = (camera_center - camera_viewport / 2);
	return Rect2(camera_pos, camera_viewport);
}

Vector2 GraphEditMinimap::_get_render_size() {
	if (!is_inside_tree()) {
		return Vector2(0, 0);
	}

	return get_size() - 2 * minimap_padding;
}

Vector2 GraphEditMinimap::_get_graph_offset() {
	return Vector2(ge->h_scroll->get_min(), ge->v_scroll->get_min());
}

Vector2 GraphEditMinimap::_get_graph_size() {
	Vector2 graph_size = Vector2(ge->h_scroll->get_max(), ge->v_scroll->get_max()) - Vector2(ge->h_scroll->get_min(), ge->v_scroll->get_min());

	if (graph_size.x == 0) {
		graph_size.x = 1;
	}
	if (graph_size.y == 0) {
		graph_size.y = 1;
	}

	return graph_size;
}

Vector2 GraphEditMinimap::_convert_from_graph_position(const Vector2 &p_position) {
	Vector2 map_position = Vector2(0, 0);
	Vector2 render_size = _get_render_size();

	map_position.x = p_position.x * render_size.x / graph_proportions.x;
	map_position.y = p_position.y * render_size.y / graph_proportions.y;

	return map_position;
}

Vector2 GraphEditMinimap::_convert_to_graph_position(const Vector2 &p_position) {
	Vector2 graph_position = Vector2(0, 0);
	Vector2 render_size = _get_render_size();

	graph_position.x = p_position.x * graph_proportions.x / render_size.x;
	graph_position.y = p_position.y * graph_proportions.y / render_size.y;

	return graph_position;
}

void GraphEditMinimap::gui_input(const Ref<InputEvent> &p_ev) {
	ERR_FAIL_COND(p_ev.is_null());

	if (!ge->is_minimap_enabled()) {
		return;
	}

	Ref<InputEventMouseButton> mb = p_ev;
	Ref<InputEventMouseMotion> mm = p_ev;

	if (mb.is_valid() && mb->get_button_index() == MouseButton::LEFT) {
		if (mb->is_pressed()) {
			is_pressing = true;

			Ref<Texture2D> resizer = get_theme_icon(SNAME("resizer"));
			Rect2 resizer_hitbox = Rect2(Point2(), resizer->get_size());
			if (resizer_hitbox.has_point(mb->get_position())) {
				is_resizing = true;
			} else {
				Vector2 click_position = _convert_to_graph_position(mb->get_position() - minimap_padding) - graph_padding;
				_adjust_graph_scroll(click_position);
			}
		} else {
			is_pressing = false;
			is_resizing = false;
		}
		accept_event();
	} else if (mm.is_valid() && is_pressing) {
		if (is_resizing) {
			// Prevent setting minimap wider than GraphEdit.
			Vector2 new_minimap_size;
			new_minimap_size.x = MIN(get_size().x - mm->get_relative().x, ge->get_size().x - 2.0 * minimap_padding.x);
			new_minimap_size.y = MIN(get_size().y - mm->get_relative().y, ge->get_size().y - 2.0 * minimap_padding.y);
			ge->set_minimap_size(new_minimap_size);

			queue_redraw();
		} else {
			Vector2 click_position = _convert_to_graph_position(mm->get_position() - minimap_padding) - graph_padding;
			_adjust_graph_scroll(click_position);
		}
		accept_event();
	}
}

void GraphEditMinimap::_adjust_graph_scroll(const Vector2 &p_offset) {
	Vector2 graph_offset = _get_graph_offset();
	ge->set_scroll_ofs(p_offset + graph_offset - camera_size / 2);
}

Control::CursorShape GraphEdit::get_cursor_shape(const Point2 &p_pos) const {
	if (moving_selection) {
		return CURSOR_MOVE;
	}

	return Control::get_cursor_shape(p_pos);
}

PackedStringArray GraphEdit::get_configuration_warnings() const {
	PackedStringArray warnings = Control::get_configuration_warnings();

	warnings.push_back(RTR("Please be aware that GraphEdit and GraphNode will undergo extensive refactoring in a future 4.x version involving compatibility-breaking API changes."));

	return warnings;
}

Error GraphEdit::connect_node(const StringName &p_from, int p_from_port, const StringName &p_to, int p_to_port) {
	if (is_node_connected(p_from, p_from_port, p_to, p_to_port)) {
		return OK;
	}
	Connection c;
	c.from = p_from;
	c.from_port = p_from_port;
	c.to = p_to;
	c.to_port = p_to_port;
	c.activity = 0;
	connections.push_back(c);
	top_layer->queue_redraw();
	minimap->queue_redraw();
	queue_redraw();
	connections_layer->queue_redraw();

	return OK;
}

bool GraphEdit::is_node_connected(const StringName &p_from, int p_from_port, const StringName &p_to, int p_to_port) {
	for (const Connection &E : connections) {
		if (E.from == p_from && E.from_port == p_from_port && E.to == p_to && E.to_port == p_to_port) {
			return true;
		}
	}

	return false;
}

void GraphEdit::disconnect_node(const StringName &p_from, int p_from_port, const StringName &p_to, int p_to_port) {
	for (const List<Connection>::Element *E = connections.front(); E; E = E->next()) {
		if (E->get().from == p_from && E->get().from_port == p_from_port && E->get().to == p_to && E->get().to_port == p_to_port) {
			connections.erase(E);
			top_layer->queue_redraw();
			minimap->queue_redraw();
			queue_redraw();
			connections_layer->queue_redraw();
			return;
		}
	}
}

void GraphEdit::get_connection_list(List<Connection> *r_connections) const {
	*r_connections = connections;
}

void GraphEdit::set_scroll_ofs(const Vector2 &p_ofs) {
	setting_scroll_ofs = true;
	h_scroll->set_value(p_ofs.x);
	v_scroll->set_value(p_ofs.y);
	_update_scroll();
	setting_scroll_ofs = false;
}

Vector2 GraphEdit::get_scroll_ofs() const {
	return Vector2(h_scroll->get_value(), v_scroll->get_value());
}

void GraphEdit::_scroll_moved(double) {
	if (!awaiting_scroll_offset_update) {
		call_deferred(SNAME("_update_scroll_offset"));
		awaiting_scroll_offset_update = true;
	}
	top_layer->queue_redraw();
	minimap->queue_redraw();
	queue_redraw();
}

void GraphEdit::_update_scroll_offset() {
	set_block_minimum_size_adjust(true);

	for (int i = 0; i < get_child_count(); i++) {
		GraphControl *graph_node = Object::cast_to<GraphControl>(get_child(i));
		if (!graph_node) {
			continue;
		}

		Point2 pos = graph_node->get_position_offset() * zoom;
		pos -= Point2(h_scroll->get_value(), v_scroll->get_value());
		graph_node->set_position(pos);
		if (graph_node->get_scale() != Vector2(zoom, zoom)) {
			graph_node->set_scale(Vector2(zoom, zoom));
		}
	}

	connections_layer->set_position(-Point2(h_scroll->get_value(), v_scroll->get_value()));
	set_block_minimum_size_adjust(false);
	awaiting_scroll_offset_update = false;

	// In godot, signals on change value are avoided as a convention.
	if (!setting_scroll_ofs) {
		emit_signal(SNAME("scroll_offset_changed"), get_scroll_ofs());
	}
}

void GraphEdit::_update_scroll() {
	if (updating) {
		return;
	}

	updating = true;

	set_block_minimum_size_adjust(true);

	Rect2 screen;
	for (int i = 0; i < get_child_count(); i++) {
		GraphControl *graph_node = Object::cast_to<GraphControl>(get_child(i));
		if (!graph_node) {
			continue;
		}

		Rect2 r;
		r.position = graph_node->get_position_offset() * zoom;
		r.size = graph_node->get_size() * zoom;
		screen = screen.merge(r);
	}

	screen.position -= get_size();
	screen.size += get_size() * 2.0;

	h_scroll->set_min(screen.position.x);
	h_scroll->set_max(screen.position.x + screen.size.x);
	h_scroll->set_page(get_size().x);
	if (h_scroll->get_max() - h_scroll->get_min() <= h_scroll->get_page()) {
		h_scroll->hide();
	} else {
		h_scroll->show();
	}

	v_scroll->set_min(screen.position.y);
	v_scroll->set_max(screen.position.y + screen.size.y);
	v_scroll->set_page(get_size().y);

	if (v_scroll->get_max() - v_scroll->get_min() <= v_scroll->get_page()) {
		v_scroll->hide();
	} else {
		v_scroll->show();
	}

	Size2 hmin = h_scroll->get_combined_minimum_size();
	Size2 vmin = v_scroll->get_combined_minimum_size();

	// Avoid scrollbar overlapping.
	h_scroll->set_anchor_and_offset(SIDE_RIGHT, ANCHOR_END, v_scroll->is_visible() ? -vmin.width : 0);
	v_scroll->set_anchor_and_offset(SIDE_BOTTOM, ANCHOR_END, h_scroll->is_visible() ? -hmin.height : 0);

	set_block_minimum_size_adjust(false);

	if (!awaiting_scroll_offset_update) {
		call_deferred(SNAME("_update_scroll_offset"));
		awaiting_scroll_offset_update = true;
	}

	updating = false;
}

void GraphEdit::_graph_node_moved_to_front(Node *p_gn) {
	GraphControl *graph_node = Object::cast_to<GraphControl>(p_gn);
	ERR_FAIL_NULL(graph_node);
	GraphFrame *gn_c = Object::cast_to<GraphFrame>(p_gn);

	if (gn_c) {
		// Check whether the selected comment node encloses another comment node.
		int enclosed_frame_node_min_idx = INT_MAX;
		bool has_enclosed_frames = false;
		for (int i = 0; i < frame_enclosed_nodes[graph_node->get_name()].size(); i++) {
			GraphFrame *enclosed_child_frame_node = Object::cast_to<GraphFrame>(frame_enclosed_nodes[graph_node->get_name()][i]);

			if (enclosed_child_frame_node && (enclosed_child_frame_node != gn_c)) {
				has_enclosed_frames = true;
				int idx = enclosed_child_frame_node->get_index();
				enclosed_frame_node_min_idx = MIN(enclosed_frame_node_min_idx, idx);
			}
		}
		if (has_enclosed_frames) {
			int new_index = MAX(MIN(background_nodes_separator_idx, enclosed_frame_node_min_idx - 1), 0);
			call_deferred("move_child", gn_c, new_index);
		} else {
			call_deferred("move_child", gn_c, background_nodes_separator_idx - 1);
		}
	} else {
		graph_node->move_to_front();
	}
}

void GraphEdit::_graph_node_selected(Node *p_gn) {
	GraphControl *gn = Object::cast_to<GraphControl>(p_gn);
	ERR_FAIL_NULL(gn);

	emit_signal(SNAME("node_selected"), gn);
}

void GraphEdit::_graph_node_deselected(Node *p_gn) {
	GraphControl *gn = Object::cast_to<GraphControl>(p_gn);
	ERR_FAIL_NULL(gn);

	emit_signal(SNAME("node_deselected"), gn);
}

void GraphEdit::_graph_node_resized(Vector2 p_new_minsize, Node *p_gn) {
	print_line("Resized.");
	GraphFrame *graph_frame = Object::cast_to<GraphFrame>(p_gn);
	if (graph_frame) {
		_update_frame_enclosed_nodes_list(graph_frame, frame_enclosed_nodes);
		_graph_node_moved_to_front(p_gn);
	}
}

void GraphEdit::_graph_node_moved(Node *p_gn) {
	GraphControl *graph_node = Object::cast_to<GraphControl>(p_gn);
	ERR_FAIL_NULL(graph_node);
	top_layer->queue_redraw();
	minimap->queue_redraw();
	queue_redraw();
	connections_layer->queue_redraw();
}

void GraphEdit::_graph_node_slot_updated(int p_index, Node *p_gn) {
	GraphNode *graph_node = Object::cast_to<GraphNode>(p_gn);
	ERR_FAIL_NULL(graph_node);
	top_layer->queue_redraw();
	minimap->queue_redraw();
	queue_redraw();
	connections_layer->queue_redraw();
}

void GraphEdit::_reorder_frame_nodes(GraphFrame *p_node) {
	_update_frame_enclosed_nodes_list(p_node, frame_enclosed_nodes);
	_graph_node_moved_to_front(p_node);
}

void GraphEdit::add_child_notify(Node *p_child) {
	Control::add_child_notify(p_child);

	top_layer->call_deferred(SNAME("move_to_front")); // Top layer always on top!

	GraphControl *graph_control = Object::cast_to<GraphControl>(p_child);
	if (graph_control) {
		graph_control->set_scale(Vector2(zoom, zoom));
		graph_control->connect("position_offset_changed", callable_mp(this, &GraphEdit::_graph_node_moved).bind(graph_control));
		graph_control->connect("node_selected", callable_mp(this, &GraphEdit::_graph_node_selected).bind(graph_control));
		graph_control->connect("node_deselected", callable_mp(this, &GraphEdit::_graph_node_deselected).bind(graph_control));

		GraphNode *graph_node = Object::cast_to<GraphNode>(graph_control);
		if (graph_node) {
			graph_control->connect("slot_updated", callable_mp(this, &GraphEdit::_graph_node_slot_updated).bind(graph_control));
		}

		GraphFrame *graph_frame = Object::cast_to<GraphFrame>(graph_control);
		if (graph_frame) {
			background_nodes_separator_idx++;
			graph_frame->connect("minimum_size_changed", callable_mp(this, &GraphEdit::_reorder_frame_nodes).bind(graph_frame));

			// Move connections layer to ensure that connection lines are drawn in front of comment nodes, but behind normal nodes.
			call_deferred("move_child", connections_layer, background_nodes_separator_idx);

			_reorder_frame_nodes(graph_frame);
		}

		graph_control->connect("raise_request", callable_mp(this, &GraphEdit::_graph_node_moved_to_front).bind(graph_control));
		graph_control->connect("resize_request", callable_mp(this, &GraphEdit::_graph_node_resized).bind(graph_control));
		graph_control->connect("item_rect_changed", callable_mp((CanvasItem *)connections_layer, &CanvasItem::queue_redraw));
		graph_control->connect("item_rect_changed", callable_mp((CanvasItem *)minimap, &GraphEditMinimap::queue_redraw));
		_graph_node_moved(graph_control);
		graph_control->set_mouse_filter(MOUSE_FILTER_PASS);
	}
}

void GraphEdit::remove_child_notify(Node *p_child) {
	Control::remove_child_notify(p_child);

	if (p_child == top_layer) {
		top_layer = nullptr;
		minimap = nullptr;
	} else if (p_child == connections_layer) {
		connections_layer = nullptr;
	}

	if (top_layer != nullptr && is_inside_tree()) {
		top_layer->call_deferred(SNAME("move_to_front")); // Top layer always on top!
	}

	GraphControl *graph_control = Object::cast_to<GraphControl>(p_child);
	if (graph_control) {
		graph_control->disconnect("position_offset_changed", callable_mp(this, &GraphEdit::_graph_node_moved));
		graph_control->disconnect("node_selected", callable_mp(this, &GraphEdit::_graph_node_selected));
		graph_control->disconnect("node_deselected", callable_mp(this, &GraphEdit::_graph_node_deselected));

		GraphNode *graph_node = Object::cast_to<GraphNode>(graph_control);
		if (graph_node) {
			graph_control->disconnect("slot_updated", callable_mp(this, &GraphEdit::_graph_node_slot_updated));
		}

		GraphFrame *graph_comment = Object::cast_to<GraphFrame>(graph_control);
		if (graph_comment) {
			background_nodes_separator_idx--;
		}

		graph_control->disconnect("raise_request", callable_mp(this, &GraphEdit::_graph_node_moved_to_front));

		// In case of the whole GraphEdit being destroyed these references can already be freed.
		if (connections_layer != nullptr && connections_layer->is_inside_tree()) {
			graph_control->disconnect("item_rect_changed", callable_mp((CanvasItem *)connections_layer, &CanvasItem::queue_redraw));
		}
		if (minimap != nullptr && minimap->is_inside_tree()) {
			graph_control->disconnect("item_rect_changed", callable_mp((CanvasItem *)minimap, &GraphEditMinimap::queue_redraw));
		}
	}
}

void GraphEdit::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE:
		case NOTIFICATION_THEME_CHANGED: {
			port_hotzone_inner_extent = get_theme_constant("port_hotzone_inner_extent");
			port_hotzone_outer_extent = get_theme_constant("port_hotzone_outer_extent");

			zoom_minus_button->set_icon(get_theme_icon(SNAME("minus")));
			zoom_reset_button->set_icon(get_theme_icon(SNAME("reset")));
			zoom_plus_button->set_icon(get_theme_icon(SNAME("more")));
			snap_button->set_icon(get_theme_icon(SNAME("snap")));
			show_grid_button->set_icon(get_theme_icon(SNAME("toggle_grid")));
			minimap_button->set_icon(get_theme_icon(SNAME("minimap")));
			layout_button->set_icon(get_theme_icon(SNAME("layout")));

			zoom_label->set_custom_minimum_size(Size2(48, 0) * get_theme_default_base_scale());
		} break;

		case NOTIFICATION_READY: {
			Size2 hmin = h_scroll->get_combined_minimum_size();
			Size2 vmin = v_scroll->get_combined_minimum_size();

			h_scroll->set_anchor_and_offset(SIDE_LEFT, ANCHOR_BEGIN, 0);
			h_scroll->set_anchor_and_offset(SIDE_RIGHT, ANCHOR_END, 0);
			h_scroll->set_anchor_and_offset(SIDE_TOP, ANCHOR_END, -hmin.height);
			h_scroll->set_anchor_and_offset(SIDE_BOTTOM, ANCHOR_END, 0);

			v_scroll->set_anchor_and_offset(SIDE_LEFT, ANCHOR_END, -vmin.width);
			v_scroll->set_anchor_and_offset(SIDE_RIGHT, ANCHOR_END, 0);
			v_scroll->set_anchor_and_offset(SIDE_TOP, ANCHOR_BEGIN, 0);
			v_scroll->set_anchor_and_offset(SIDE_BOTTOM, ANCHOR_END, 0);
		} break;

		case NOTIFICATION_DRAW: {
			draw_style_box(get_theme_stylebox(SNAME("bg")), Rect2(Point2(), get_size()));

			if (show_grid) {
				// Draw background pattern.
				Vector2 offset = get_scroll_ofs() / zoom;
				Size2 size = get_size() / zoom;

				Point2i from = (offset / float(grid_snap_distance)).floor();
				Point2i len = (size / float(grid_snap_distance)).floor() + Vector2(1, 1);

				Color grid_minor = get_theme_color(SNAME("grid_minor"));
				Color grid_major = get_theme_color(SNAME("grid_major"));

				for (int i = from.x; i < from.x + len.x; i++) {
					Color color;

					if (ABS(i) % 10 == 0) {
						color = grid_major;
					} else {
						color = grid_minor;
					}

					float base_ofs = i * grid_snap_distance * zoom - offset.x * zoom;
					draw_line(Vector2(base_ofs, 0), Vector2(base_ofs, get_size().height), color);
				}

				for (int i = from.y; i < from.y + len.y; i++) {
					Color color;

					if (ABS(i) % 10 == 0) {
						color = grid_major;
					} else {
						color = grid_minor;
					}

					float base_ofs = i * grid_snap_distance * zoom - offset.y * zoom;
					draw_line(Vector2(0, base_ofs), Vector2(get_size().width, base_ofs), color);
				}
			}
		} break;

		case NOTIFICATION_RESIZED: {
			_update_scroll();
			top_layer->queue_redraw();
			minimap->queue_redraw();
		} break;
	}
}

void GraphEdit::_update_frame_enclosed_nodes_list(GraphFrame *p_node, HashMap<StringName, Vector<GraphControl *>> &p_frame_enclosed_nodes) {
	Rect2 frame_node_rect = p_node->get_rect();

	Vector<GraphControl *> enclosed_nodes;
	for (int i = 0; i < get_child_count(); i++) {
		GraphControl *graph_node = Object::cast_to<GraphControl>(get_child(i));
		if (!graph_node || graph_node->is_selected()) {
			continue;
		}

		Rect2 node_rect = graph_node->get_rect();
		print_line("node_rect: " + node_rect);

		bool included = frame_node_rect.encloses(node_rect);
		if (included) {
			enclosed_nodes.push_back(graph_node);
		}
	}

	p_frame_enclosed_nodes.insert(p_node->get_name(), enclosed_nodes);
}

void GraphEdit::_set_drag_frame_enclosed_nodes(GraphFrame *p_node, HashMap<StringName, Vector<GraphControl *>> &p_frame_enclosed_nodes, bool p_drag) {
	for (int i = 0; i < p_frame_enclosed_nodes[p_node->get_name()].size(); i++) {
		p_frame_enclosed_nodes[p_node->get_name()][i]->set_drag(p_drag);
	}
}

void GraphEdit::_set_position_of_frame_enclosed_nodes(GraphFrame *p_node, HashMap<StringName, Vector<GraphControl *>> &p_frame_enclosed_nodes, Vector2 p_drag_accum) {
	for (int i = 0; i < p_frame_enclosed_nodes[p_node->get_name()].size(); i++) {
		Vector2 pos = (p_frame_enclosed_nodes[p_node->get_name()][i]->get_drag_from() * zoom + drag_accum) / zoom;
		if (use_grid_snap ^ Input::get_singleton()->is_key_pressed(Key::CTRL)) {
			pos = pos.snapped(Vector2(grid_snap_distance, grid_snap_distance));
		}
		p_frame_enclosed_nodes[p_node->get_name()][i]->set_position_offset(pos);
	}
}

bool GraphEdit::_filter_input(const Point2 &p_point) {
	Ref<Texture2D> port_icon = get_theme_icon(SNAME("port"), SNAME("GraphNode"));

	for (int i = get_child_count() - 1; i >= 0; i--) {
		GraphNode *graph_node = Object::cast_to<GraphNode>(get_child(i));
		if (!graph_node) {
			continue;
		}

		for (int j = 0; j < graph_node->get_port_input_count(); j++) {
			Vector2i port_size = Vector2i(port_icon->get_width(), port_icon->get_height());

			// Determine slot height.
			int slot_index = graph_node->get_port_input_slot(j);
			Control *child = Object::cast_to<Control>(graph_node->get_child(slot_index));

			port_size.height = MAX(port_size.height, child ? child->get_size().y : 0);

			if (is_in_input_hotzone(graph_node, j, p_point / zoom, port_size)) {
				return true;
			}
		}

		for (int j = 0; j < graph_node->get_port_output_count(); j++) {
			Vector2i port_size = Vector2i(port_icon->get_width(), port_icon->get_height());

			// Determine slot height.
			int slot_index = graph_node->get_port_output_slot(j);
			Control *child = Object::cast_to<Control>(graph_node->get_child(slot_index));
			port_size.height = MAX(port_size.height, child ? child->get_size().y : 0);

			if (is_in_output_hotzone(graph_node, j, p_point / zoom, port_size)) {
				return true;
			}
		}
	}

	return false;
}

void GraphEdit::_top_layer_input(const Ref<InputEvent> &p_ev) {
	Ref<InputEventMouseButton> mb = p_ev;
	if (mb.is_valid() && mb->get_button_index() == MouseButton::LEFT && mb->is_pressed()) {
		Ref<Texture2D> port_icon = get_theme_icon(SNAME("port"), SNAME("GraphNode"));

		connecting_valid = false;
		click_pos = mb->get_position() / zoom;
		for (int i = get_child_count() - 1; i >= 0; i--) {
			GraphNode *graph_node = Object::cast_to<GraphNode>(get_child(i));
			if (!graph_node) {
				continue;
			}

			for (int j = 0; j < graph_node->get_port_output_count(); j++) {
				Vector2 pos = graph_node->get_port_output_position(j) * zoom + graph_node->get_position();
				Vector2i port_size = Vector2i(port_icon->get_width(), port_icon->get_height());

				// Determine slot height.
				int slot_index = graph_node->get_port_output_slot(j);
				Control *child = Object::cast_to<Control>(graph_node->get_child(slot_index));
				port_size.height = MAX(port_size.height, child ? child->get_size().y : 0);

				if (child) {
					print_line("child: " + child->get_name() + "minsize: " + itos(child->get_minimum_size().y) + "size: " + itos(child->get_size().y));
				}

				if (is_in_output_hotzone(graph_node, j, click_pos, port_size)) {
					if (valid_left_disconnect_types.has(graph_node->get_port_output_type(j))) {
						//check disconnect
						for (const Connection &E : connections) {
							if (E.from == graph_node->get_name() && E.from_port == j) {
								Node *to = get_node(NodePath(E.to));
								if (Object::cast_to<GraphNode>(to)) {
									connecting_from = E.to;
									connecting_index = E.to_port;
									connecting_out = false;
									connecting_type = Object::cast_to<GraphNode>(to)->get_port_input_type(E.to_port);
									connecting_color = Object::cast_to<GraphNode>(to)->get_port_input_color(E.to_port);
									connecting_target = false;
									connecting_to = pos;

									if (connecting_type >= 0) {
										just_disconnected = true;

										emit_signal(SNAME("disconnection_request"), E.from, E.from_port, E.to, E.to_port);
										to = get_node(NodePath(connecting_from)); // Maybe it was erased.
										if (Object::cast_to<GraphNode>(to)) {
											connecting = true;
											emit_signal(SNAME("connection_drag_started"), connecting_from, connecting_index, false);
										}
									}
									return;
								}
							}
						}
					}

					connecting_from = graph_node->get_name();
					connecting_index = j;
					connecting_out = true;
					connecting_type = graph_node->get_port_output_type(j);
					connecting_color = graph_node->get_port_output_color(j);
					connecting_target = false;
					connecting_to = pos;
					if (connecting_type >= 0) {
						connecting = true;
						just_disconnected = false;
						emit_signal(SNAME("connection_drag_started"), connecting_from, connecting_index, true);
					}
					return;
				}
			}

			for (int j = 0; j < graph_node->get_port_input_count(); j++) {
				Vector2 pos = graph_node->get_port_input_position(j) * zoom + graph_node->get_position();

				Vector2i port_size = Vector2i(port_icon->get_width(), port_icon->get_height());

				// Determine slot height.
				int slot_index = graph_node->get_port_input_slot(j);
				Control *child = Object::cast_to<Control>(graph_node->get_child(slot_index));
				port_size.height = MAX(port_size.height, child ? child->get_size().y : 0);

				if (is_in_input_hotzone(graph_node, j, click_pos, port_size)) {
					if (right_disconnects || valid_right_disconnect_types.has(graph_node->get_port_input_type(j))) {
						// Check disconnect.
						for (const Connection &E : connections) {
							if (E.to == graph_node->get_name() && E.to_port == j) {
								Node *fr = get_node(NodePath(E.from));
								if (Object::cast_to<GraphNode>(fr)) {
									connecting_from = E.from;
									connecting_index = E.from_port;
									connecting_out = true;
									connecting_type = Object::cast_to<GraphNode>(fr)->get_port_output_type(E.from_port);
									connecting_color = Object::cast_to<GraphNode>(fr)->get_port_output_color(E.from_port);
									connecting_target = false;
									connecting_to = pos;
									just_disconnected = true;

									if (connecting_type >= 0) {
										emit_signal(SNAME("disconnection_request"), E.from, E.from_port, E.to, E.to_port);
										fr = get_node(NodePath(connecting_from));
										if (Object::cast_to<GraphNode>(fr)) {
											connecting = true;
											emit_signal(SNAME("connection_drag_started"), connecting_from, connecting_index, true);
										}
									}
									return;
								}
							}
						}
					}

					connecting_from = graph_node->get_name();
					connecting_index = j;
					connecting_out = false;
					connecting_type = graph_node->get_port_input_type(j);
					connecting_color = graph_node->get_port_input_color(j);
					connecting_target = false;
					connecting_to = pos;
					if (connecting_type >= 0) {
						connecting = true;
						just_disconnected = false;
						emit_signal(SNAME("connection_drag_started"), connecting_from, connecting_index, false);
					}
					return;
				}
			}
		}
	}

	Ref<InputEventMouseMotion> mm = p_ev;
	if (mm.is_valid() && connecting) {
		connecting_to = mm->get_position();
		connecting_target = false;
		top_layer->queue_redraw();
		minimap->queue_redraw();

		connecting_valid = just_disconnected || click_pos.distance_to(connecting_to / zoom) > MIN_DRAG_DISTANCE_FOR_VALID_CONNECTION;

		if (connecting_valid) {
			Vector2 mpos = mm->get_position() / zoom;
			for (int i = get_child_count() - 1; i >= 0; i--) {
				Ref<Texture2D> port_icon = get_theme_icon(SNAME("port"), SNAME("GraphNode"));
				GraphNode *graph_node = Object::cast_to<GraphNode>(get_child(i));
				if (!graph_node) {
					continue;
				}

				if (!connecting_out) {
					for (int j = 0; j < graph_node->get_port_output_count(); j++) {
						Vector2 pos = graph_node->get_port_output_position(j) * zoom + graph_node->get_position();
						Vector2i port_size = Vector2i(port_icon->get_width(), port_icon->get_height());

						// Determine slot height.
						int slot_index = graph_node->get_port_output_slot(j);
						Control *child = Object::cast_to<Control>(graph_node->get_child(slot_index));
						port_size.height = MAX(port_size.height, child ? child->get_size().y : 0);

						int type = graph_node->get_port_output_type(j);
						if ((type == connecting_type ||
									valid_connection_types.has(ConnectionType(connecting_type, type))) &&
								is_in_output_hotzone(graph_node, j, mpos, port_size)) {
							if (!is_node_hover_valid(graph_node->get_name(), j, connecting_from, connecting_index)) {
								continue;
							}
							connecting_target = true;
							connecting_to = pos;
							connecting_target_to = graph_node->get_name();
							connecting_target_index = j;
							return;
						}
					}
				} else {
					for (int j = 0; j < graph_node->get_port_input_count(); j++) {
						Vector2 pos = graph_node->get_port_input_position(j) * zoom + graph_node->get_position();
						Vector2i port_size = Vector2i(port_icon->get_width(), port_icon->get_height());

						// Determine slot height.
						int slot_index = graph_node->get_port_input_slot(j);
						Control *child = Object::cast_to<Control>(graph_node->get_child(slot_index));
						port_size.height = MAX(port_size.height, child ? child->get_size().y : 0);

						int type = graph_node->get_port_input_type(j);
						if ((type == connecting_type || valid_connection_types.has(ConnectionType(connecting_type, type))) &&
								is_in_input_hotzone(graph_node, j, mpos, port_size)) {
							if (!is_node_hover_valid(connecting_from, connecting_index, graph_node->get_name(), j)) {
								continue;
							}
							connecting_target = true;
							connecting_to = pos;
							connecting_target_to = graph_node->get_name();
							connecting_target_index = j;
							return;
						}
					}
				}
			}
		}
	}

	if (mb.is_valid() &&
			mb->get_button_index() == MouseButton::LEFT &&
			!mb->is_pressed()) {
		if (connecting_valid) {
			if (connecting && connecting_target) {
				if (connecting_out) {
					emit_signal(SNAME("connection_request"), connecting_from, connecting_index, connecting_target_to, connecting_target_index);
				} else {
					emit_signal(SNAME("connection_request"), connecting_target_to, connecting_target_index, connecting_from, connecting_index);
				}
			} else if (!just_disconnected) {
				if (connecting_out) {
					emit_signal(SNAME("connection_to_empty"), connecting_from, connecting_index, mb->get_position());
				} else {
					emit_signal(SNAME("connection_from_empty"), connecting_from, connecting_index, mb->get_position());
				}
			}
		}

		if (connecting) {
			force_connection_drag_end();
		}
	}
}

bool GraphEdit::_check_clickable_control(Control *p_control, const Vector2 &mpos, const Vector2 &p_offset) {
	if (p_control->is_set_as_top_level() || !p_control->is_visible() || !p_control->is_inside_tree()) {
		return false;
	}

	Rect2 control_rect = p_control->get_rect();
	control_rect.position *= zoom;
	control_rect.size *= zoom;
	control_rect.position += p_offset;

	if (!control_rect.has_point(mpos) || p_control->get_mouse_filter() == MOUSE_FILTER_IGNORE) {
		// Test children.
		for (int i = 0; i < p_control->get_child_count(); i++) {
			Control *child_rect = Object::cast_to<Control>(p_control->get_child(i));
			if (!child_rect) {
				continue;
			}
			if (_check_clickable_control(child_rect, mpos, control_rect.position)) {
				return true;
			}
		}

		return false;
	} else {
		return true;
	}
}

bool GraphEdit::is_in_input_hotzone(GraphNode *p_graph_node, int p_slot_index, const Vector2 &p_mouse_pos, const Vector2i &p_port_size) {
	bool success;
	if (GDVIRTUAL_CALL(_is_in_input_hotzone, p_graph_node, p_slot_index, p_mouse_pos, success)) {
		return success;
	} else {
		Vector2 pos = p_graph_node->get_port_input_position(p_slot_index) * zoom + p_graph_node->get_position();
		return is_in_port_hotzone(pos / zoom, p_mouse_pos, p_port_size, true);
	}
}

bool GraphEdit::is_in_output_hotzone(GraphNode *p_graph_node, int p_slot_index, const Vector2 &p_mouse_pos, const Vector2i &p_port_size) {
	if (p_graph_node->is_resizable()) {
		Ref<Texture2D> resizer = p_graph_node->get_theme_icon(SNAME("resizer"));
		Rect2 resizer_rect = Rect2(p_graph_node->get_position() / zoom + p_graph_node->get_size() - resizer->get_size(), resizer->get_size());
		if (resizer_rect.has_point(p_mouse_pos)) {
			return false;
		}
	}

	bool success;
	if (GDVIRTUAL_CALL(_is_in_output_hotzone, p_graph_node, p_slot_index, p_mouse_pos, success)) {
		return success;
	} else {
		Vector2 pos = p_graph_node->get_port_output_position(p_slot_index) * zoom + p_graph_node->get_position();
		return is_in_port_hotzone(pos / zoom, p_mouse_pos, p_port_size, false);
	}
}

bool GraphEdit::is_in_port_hotzone(const Vector2 &p_pos, const Vector2 &p_mouse_pos, const Vector2i &p_port_size, bool p_left) {
	Rect2 hotzone = Rect2(
			p_pos.x - (p_left ? port_hotzone_outer_extent : port_hotzone_inner_extent),
			p_pos.y - p_port_size.height / 2.0,
			port_hotzone_inner_extent + port_hotzone_outer_extent,
			p_port_size.height);

	if (!hotzone.has_point(p_mouse_pos)) {
		return false;
	}

	for (int i = 0; i < get_child_count(); i++) {
		GraphNode *child = Object::cast_to<GraphNode>(get_child(i));
		if (!child) {
			continue;
		}

		Rect2 child_rect = child->get_rect();
		if (child_rect.has_point(p_mouse_pos * zoom)) {
			for (int j = 0; j < child->get_child_count(); j++) {
				Control *subchild = Object::cast_to<Control>(child->get_child(j));
				if (!subchild) {
					continue;
				}

				if (_check_clickable_control(subchild, p_mouse_pos * zoom, child_rect.position)) {
					return false;
				}
			}
		}
	}

	return true;
}

PackedVector2Array GraphEdit::get_connection_line(const Vector2 &p_from, const Vector2 &p_to) {
	Vector<Vector2> ret;
	if (GDVIRTUAL_CALL(_get_connection_line, p_from, p_to, ret)) {
		return ret;
	}

	float x_diff = (p_to.x - p_from.x);
	float cp_offset = x_diff * lines_curvature;
	if (x_diff < 0) {
		cp_offset *= -1;
	}

	Curve2D curve;
	curve.add_point(p_from);
	curve.set_point_out(0, Vector2(cp_offset, 0));
	curve.add_point(p_to);
	curve.set_point_in(1, Vector2(-cp_offset, 0));

	if (lines_curvature > 0) {
		return curve.tessellate(MAX_CONNECTION_LINE_CURVE_TESSELATION_STAGES, 2.0);
	} else {
		return curve.tessellate(1);
	}
}

void GraphEdit::_draw_connection_line(CanvasItem *p_where, const Vector2 &p_from, const Vector2 &p_to, const Color &p_color, const Color &p_to_color, float p_width, float p_zoom) {
	Vector<Vector2> points = get_connection_line(p_from / p_zoom, p_to / p_zoom);
	Vector<Vector2> scaled_points;
	Vector<Color> colors;
	float length = (p_from / p_zoom).distance_to(p_to / p_zoom);
	for (int i = 0; i < points.size(); i++) {
		float d = (p_from / p_zoom).distance_to(points[i]) / length;
		colors.push_back(p_color.lerp(p_to_color, d));
		scaled_points.push_back(points[i] * p_zoom);
	}

	// Thickness below 0.5 doesn't look good on the graph or its minimap.
	p_where->draw_polyline_colors(scaled_points, colors, MAX(0.5, Math::floor(p_width * get_theme_default_base_scale())), lines_antialiased);
}

void GraphEdit::_connections_layer_draw() {
	Color activity_color = get_theme_color(SNAME("activity"));

	// Draw connections.
	List<List<Connection>::Element *> to_erase;
	for (List<Connection>::Element *E = connections.front(); E; E = E->next()) {
		const Connection &c = E->get();

		Node *from = get_node(NodePath(c.from));
		GraphNode *gnode_from = Object::cast_to<GraphNode>(from);

		if (!gnode_from) {
			to_erase.push_back(E);
			continue;
		}

		Node *to = get_node(NodePath(c.to));
		GraphNode *gnode_to = Object::cast_to<GraphNode>(to);

		if (!gnode_to) {
			to_erase.push_back(E);
			continue;
		}

		Vector2 frompos = gnode_from->get_port_output_position(c.from_port) * zoom + gnode_from->get_position_offset() * zoom;
		Color color = gnode_from->get_port_output_color(c.from_port);
		Vector2 topos = gnode_to->get_port_input_position(c.to_port) * zoom + gnode_to->get_position_offset() * zoom;
		Color tocolor = gnode_to->get_port_input_color(c.to_port);

		if (c.activity > 0) {
			color = color.lerp(activity_color, c.activity);
			tocolor = tocolor.lerp(activity_color, c.activity);
		}
		_draw_connection_line(connections_layer, frompos, topos, color, tocolor, lines_thickness, zoom);
	}

	for (List<Connection>::Element *&E : to_erase) {
		connections.erase(E);
	}
}

void GraphEdit::_top_layer_draw() {
	_update_scroll();

	if (connecting) {
		Node *fromn = get_node_or_null(NodePath(connecting_from));
		ERR_FAIL_NULL(fromn);
		GraphNode *from = Object::cast_to<GraphNode>(fromn);
		ERR_FAIL_NULL(from);
		Vector2 pos;
		if (connecting_out) {
			pos = from->get_port_output_position(connecting_index) * zoom;
		} else {
			pos = from->get_port_input_position(connecting_index) * zoom;
		}
		pos += from->get_position();

		Vector2 topos;
		topos = connecting_to;

		Color col = connecting_color;

		if (connecting_target) {
			col.r += 0.4;
			col.g += 0.4;
			col.b += 0.4;
		}

		if (!connecting_out) {
			SWAP(pos, topos);
		}
		_draw_connection_line(top_layer, pos, topos, col, col, lines_thickness, zoom);
	}

	if (box_selecting) {
		top_layer->draw_rect(box_selecting_rect, get_theme_color(SNAME("selection_fill")));
		top_layer->draw_rect(box_selecting_rect, get_theme_color(SNAME("selection_stroke")), false);
	}
}

void GraphEdit::_minimap_draw() {
	if (!is_minimap_enabled()) {
		return;
	}

	minimap->update_minimap();

	// Draw the minimap background.
	Rect2 minimap_rect = Rect2(Point2(), minimap->get_size());
	minimap->draw_style_box(minimap->get_theme_stylebox(SNAME("bg")), minimap_rect);

	Vector2 graph_offset = minimap->_get_graph_offset();
	Vector2 minimap_offset = minimap->minimap_offset;

	// Draw frame nodes.
	for (int i = get_child_count() - 1; i >= 0; i--) {
		GraphFrame *graph_node = Object::cast_to<GraphFrame>(get_child(i));
		if (!graph_node) {
			continue;
		}

		Vector2 node_position = minimap->_convert_from_graph_position(graph_node->get_position_offset() * zoom - graph_offset) + minimap_offset;
		Vector2 node_size = minimap->_convert_from_graph_position(graph_node->get_size() * zoom);
		Rect2 node_rect = Rect2(node_position, node_size);

		Ref<StyleBoxFlat> sb_minimap = minimap->get_theme_stylebox(SNAME("node"))->duplicate();

		// Override default values with colors provided by the GraphNode's stylebox, if possible.
		Ref<StyleBoxFlat> sb_frame = graph_node->get_theme_stylebox(graph_node->is_selected() ? SNAME("frame_selected") : SNAME("frame"));
		if (sb_frame.is_valid()) {
			Color node_color = sb_frame->get_bg_color();
			if (graph_node->is_tint_color_enabled()) {
				node_color = graph_node->get_tint_color();
			}
			sb_minimap->set_bg_color(node_color);
		}

		minimap->draw_style_box(sb_minimap, node_rect);
	}

	// Draw regular graph nodes.
	for (int i = get_child_count() - 1; i >= 0; i--) {
		GraphNode *graph_node = Object::cast_to<GraphNode>(get_child(i));
		if (!graph_node) {
			continue;
		}

		Vector2 node_position = minimap->_convert_from_graph_position(graph_node->get_position_offset() * zoom - graph_offset) + minimap_offset;
		Vector2 node_size = minimap->_convert_from_graph_position(graph_node->get_size() * zoom);
		Rect2 node_rect = Rect2(node_position, node_size);

		Ref<StyleBoxFlat> sb_minimap = minimap->get_theme_stylebox(SNAME("node"))->duplicate();

		// Override default values with colors provided by the GraphNode's stylebox, if possible.
		Ref<StyleBoxFlat> sbf = graph_node->get_theme_stylebox(graph_node->is_selected() ? "frame_selected" : "frame");
		if (sbf.is_valid()) {
			Color node_color = sbf->get_border_color();
			sb_minimap->set_bg_color(node_color);
		}

		minimap->draw_style_box(sb_minimap, node_rect);
	}

	// Draw node connections.
	Color activity_color = get_theme_color(SNAME("activity"));
	for (const Connection &E : connections) {
		Node *from = get_node(NodePath(E.from));
		GraphNode *gfrom = Object::cast_to<GraphNode>(from);
		if (!gfrom) {
			continue;
		}

		Node *to = get_node(NodePath(E.to));
		GraphNode *gto = Object::cast_to<GraphNode>(to);
		if (!gto) {
			continue;
		}

		Vector2 from_port_position = gfrom->get_position_offset() * zoom + gfrom->get_port_output_position(E.from_port) * zoom;
		Vector2 from_position = minimap->_convert_from_graph_position(from_port_position - graph_offset) + minimap_offset;
		Color from_color = gfrom->get_port_output_color(E.from_port);
		Vector2 to_port_position = gto->get_position_offset() * zoom + gto->get_port_input_position(E.to_port) * zoom;
		Vector2 to_position = minimap->_convert_from_graph_position(to_port_position - graph_offset) + minimap_offset;
		Color to_color = gto->get_port_input_color(E.to_port);

		if (E.activity > 0) {
			from_color = from_color.lerp(activity_color, E.activity);
			to_color = to_color.lerp(activity_color, E.activity);
		}
		_draw_connection_line(minimap, from_position, to_position, from_color, to_color, 0.5, minimap->_convert_from_graph_position(Vector2(zoom, zoom)).length());
	}

	// Draw the "camera" viewport.
	Rect2 camera_rect = minimap->get_camera_rect();
	minimap->draw_style_box(minimap->get_theme_stylebox(SNAME("camera")), camera_rect);

	// Draw the resizer control.
	Ref<Texture2D> resizer = minimap->get_theme_icon(SNAME("resizer"));
	Color resizer_color = minimap->get_theme_color(SNAME("resizer_color"));
	minimap->draw_texture(resizer, Point2(), resizer_color);
}

void GraphEdit::set_selected(Node *p_child) {
	for (int i = get_child_count() - 1; i >= 0; i--) {
		GraphNode *graph_node = Object::cast_to<GraphNode>(get_child(i));
		if (!graph_node) {
			continue;
		}

		graph_node->set_selected(graph_node == p_child);
	}
}

void GraphEdit::gui_input(const Ref<InputEvent> &p_ev) {
	ERR_FAIL_COND(p_ev.is_null());
	if (panner->gui_input(p_ev, warped_panning ? get_global_rect() : Rect2())) {
		return;
	}

	Ref<InputEventMouseMotion> mm = p_ev;

	if (mm.is_valid() && dragging) {
		if (!moving_selection) {
			emit_signal(SNAME("begin_node_move"));
			moving_selection = true;
		}

		just_selected = true;
		drag_accum += mm->get_relative();
		for (int i = get_child_count() - 1; i >= 0; i--) {
			GraphControl *graph_node = Object::cast_to<GraphControl>(get_child(i));
			if (graph_node && graph_node->is_selected() && graph_node->is_draggable()) {
				Vector2 pos = (graph_node->get_drag_from() * zoom + drag_accum) / zoom;

				// Snapping can be toggled temporarily by holding down Ctrl.
				// This is done here as to not toggle the grid when holding down Ctrl.
				if (use_grid_snap ^ Input::get_singleton()->is_key_pressed(Key::CTRL)) {
					pos = pos.snapped(Vector2(grid_snap_distance, grid_snap_distance));
				}

				graph_node->set_position_offset(pos);

				GraphFrame *graph_frame = Object::cast_to<GraphFrame>(get_child(i));
				if (graph_frame) {
					_set_position_of_frame_enclosed_nodes(graph_frame, frame_enclosed_nodes, drag_accum);
				}
			}
		}
	}

	if (mm.is_valid() && box_selecting) {
		box_selecting_to = mm->get_position();

		box_selecting_rect = Rect2(box_selecting_from.min(box_selecting_to), (box_selecting_from - box_selecting_to).abs());

		for (int i = get_child_count() - 1; i >= 0; i--) {
			GraphControl *graph_node = Object::cast_to<GraphControl>(get_child(i));
			if (!graph_node) {
				continue;
			}

			Rect2 r = graph_node->get_rect();
			bool in_box = r.intersects(box_selecting_rect);

			if (in_box) {
				graph_node->set_selected(box_selection_mode_additive);
			} else {
				graph_node->set_selected(prev_selected.find(graph_node) != nullptr);
			}
		}

		top_layer->queue_redraw();
		minimap->queue_redraw();
	}

	Ref<InputEventMouseButton> mb = p_ev;
	if (mb.is_valid()) {
		if (mb->get_button_index() == MouseButton::RIGHT && mb->is_pressed()) {
			if (box_selecting) {
				box_selecting = false;
				for (int i = get_child_count() - 1; i >= 0; i--) {
					GraphControl *graph_node = Object::cast_to<GraphControl>(get_child(i));
					if (!graph_node) {
						continue;
					}

					graph_node->set_selected(prev_selected.find(graph_node) != nullptr);
				}
				top_layer->queue_redraw();
				minimap->queue_redraw();
			} else {
				if (connecting) {
					force_connection_drag_end();
				} else {
					emit_signal(SNAME("popup_request"), mb->get_position());
				}
			}
		}

		if (mb->get_button_index() == MouseButton::LEFT && !mb->is_pressed() && dragging) {
			if (!just_selected && drag_accum == Vector2() && Input::get_singleton()->is_key_pressed(Key::CTRL)) {
				// Deselect current node.
				for (int i = get_child_count() - 1; i >= 0; i--) {
					GraphControl *graph_node = Object::cast_to<GraphControl>(get_child(i));

					if (graph_node) {
						Rect2 r = graph_node->get_rect();
						if (r.has_point(mb->get_position())) {
							graph_node->set_selected(false);
						}
					}
				}
			}

			if (drag_accum != Vector2()) {
				for (int i = get_child_count() - 1; i >= 0; i--) {
					GraphControl *graph_node = Object::cast_to<GraphControl>(get_child(i));
					if (graph_node && graph_node->is_selected()) {
						graph_node->set_drag(false);
						GraphFrame *comment_node = Object::cast_to<GraphFrame>(get_child(i));
						if (comment_node) {
							_set_drag_frame_enclosed_nodes(comment_node, frame_enclosed_nodes, false);
						}
					}
				}
			}

			if (moving_selection) {
				emit_signal(SNAME("end_node_move"));
				moving_selection = false;
			}

			dragging = false;

			top_layer->queue_redraw();
			minimap->queue_redraw();
			queue_redraw();
			connections_layer->queue_redraw();
		}

		// Node selection logic.
		if (mb->get_button_index() == MouseButton::LEFT && mb->is_pressed()) {
			GraphControl *graph_node = nullptr;

			// Find node which was clicked on.
			for (int i = get_child_count() - 1; i >= 0; i--) {
				GraphControl *gn_selected = Object::cast_to<GraphControl>(get_child(i));

				if (!gn_selected) {
					continue;
				}

				if (gn_selected->is_resizing()) {
					continue;
				}

				if (gn_selected->has_point((mb->get_position() - gn_selected->get_position()) / zoom)) {
					graph_node = gn_selected;
					break;
				}
			}

			if (graph_node) {
				if (_filter_input(mb->get_position())) {
					return;
				}

				// Left-clicked on a node, select it.
				dragging = true;
				drag_accum = Vector2();
				just_selected = !graph_node->is_selected();
				if (!graph_node->is_selected() && !Input::get_singleton()->is_key_pressed(Key::CTRL)) {
					for (int i = 0; i < get_child_count(); i++) {
						GraphControl *o_gn = Object::cast_to<GraphControl>(get_child(i));
						if (!o_gn) {
							continue;
						}

						o_gn->set_selected(o_gn == graph_node);
					}
				}

				graph_node->set_selected(true);
				for (int i = 0; i < get_child_count(); i++) {
					GraphControl *o_gn = Object::cast_to<GraphControl>(get_child(i));
					if (!o_gn) {
						continue;
					}
					if (o_gn->is_selected()) {
						o_gn->set_drag(true);
						GraphFrame *comment_node = Object::cast_to<GraphFrame>(get_child(i));
						if (comment_node) {
							_update_frame_enclosed_nodes_list(comment_node, frame_enclosed_nodes);
							_graph_node_moved_to_front(comment_node);
							_set_drag_frame_enclosed_nodes(comment_node, frame_enclosed_nodes, true);
						}
					}
				}

			} else {
				if (_filter_input(mb->get_position())) {
					return;
				}
				if (panner->is_panning()) {
					return;
				}

				// Left-clicked on empty space, start box select.
				box_selecting = true;
				box_selecting_from = mb->get_position();
				if (mb->is_ctrl_pressed()) {
					box_selection_mode_additive = true;
					prev_selected.clear();
					for (int i = get_child_count() - 1; i >= 0; i--) {
						GraphControl *gn2 = Object::cast_to<GraphControl>(get_child(i));
						if (!gn2 || !gn2->is_selected()) {
							continue;
						}

						prev_selected.push_back(gn2);
					}
				} else if (mb->is_shift_pressed()) {
					box_selection_mode_additive = false;
					prev_selected.clear();
					for (int i = get_child_count() - 1; i >= 0; i--) {
						GraphControl *gn2 = Object::cast_to<GraphControl>(get_child(i));
						if (!gn2 || !gn2->is_selected()) {
							continue;
						}

						prev_selected.push_back(gn2);
					}
				} else {
					box_selection_mode_additive = true;
					prev_selected.clear();
					for (int i = get_child_count() - 1; i >= 0; i--) {
						GraphControl *gn2 = Object::cast_to<GraphControl>(get_child(i));
						if (!gn2) {
							continue;
						}

						gn2->set_selected(false);
					}
				}
			}
		}

		if (mb->get_button_index() == MouseButton::LEFT && !mb->is_pressed() && box_selecting) {
			// Box selection ended. Nodes were selected during mouse movement.
			box_selecting = false;
			box_selecting_rect = Rect2();
			prev_selected.clear();
			top_layer->queue_redraw();
			minimap->queue_redraw();
		}
	}

	if (p_ev->is_pressed()) {
		if (p_ev->is_action("ui_graph_duplicate", true)) {
			emit_signal(SNAME("duplicate_nodes_request"));
			accept_event();
		} else if (p_ev->is_action("ui_copy", true)) {
			emit_signal(SNAME("copy_nodes_request"));
			accept_event();
		} else if (p_ev->is_action("ui_paste", true)) {
			emit_signal(SNAME("paste_nodes_request"));
			accept_event();
		} else if (p_ev->is_action("ui_graph_delete", true)) {
			TypedArray<StringName> nodes;

			for (int i = 0; i < get_child_count(); i++) {
				GraphNode *gn = Object::cast_to<GraphNode>(get_child(i));
				if (!gn) {
					continue;
				}
				if (gn->is_selected()) {
					nodes.push_back(gn->get_name());
				}
			}

			emit_signal(SNAME("delete_nodes_request"), nodes);
			accept_event();
		}
	}
}

void GraphEdit::_pan_callback(Vector2 p_scroll_vec, Ref<InputEvent> p_event) {
	h_scroll->set_value(h_scroll->get_value() - p_scroll_vec.x);
	v_scroll->set_value(v_scroll->get_value() - p_scroll_vec.y);
}

void GraphEdit::_zoom_callback(float p_zoom_factor, Vector2 p_origin, Ref<InputEvent> p_event) {
	set_zoom_custom(zoom * p_zoom_factor, p_origin);
}

void GraphEdit::set_connection_activity(const StringName &p_from, int p_from_port, const StringName &p_to, int p_to_port, float p_activity) {
	for (Connection &E : connections) {
		if (E.from == p_from && E.from_port == p_from_port && E.to == p_to && E.to_port == p_to_port) {
			if (Math::is_equal_approx(E.activity, p_activity)) {
				// Update only if changed.
				top_layer->queue_redraw();
				minimap->queue_redraw();
				connections_layer->queue_redraw();
			}
			E.activity = p_activity;
			return;
		}
	}
}

void GraphEdit::clear_connections() {
	connections.clear();
	minimap->queue_redraw();
	queue_redraw();
	connections_layer->queue_redraw();
}

void GraphEdit::force_connection_drag_end() {
	ERR_FAIL_COND_MSG(!connecting, "Drag end requested without active drag!");
	connecting = false;
	connecting_valid = false;
	top_layer->queue_redraw();
	minimap->queue_redraw();
	queue_redraw();
	connections_layer->queue_redraw();
	emit_signal(SNAME("connection_drag_ended"));
}

bool GraphEdit::is_node_hover_valid(const StringName &p_from, const int p_from_port, const StringName &p_to, const int p_to_port) {
	bool valid = true;
	GDVIRTUAL_CALL(_is_node_hover_valid, p_from, p_from_port, p_to, p_to_port, valid);
	return valid;
}

void GraphEdit::set_panning_scheme(PanningScheme p_scheme) {
	panning_scheme = p_scheme;
	panner->set_control_scheme((ViewPanner::ControlScheme)p_scheme);
}

GraphEdit::PanningScheme GraphEdit::get_panning_scheme() const {
	return panning_scheme;
}

void GraphEdit::set_zoom(float p_zoom) {
	set_zoom_custom(p_zoom, get_size() / 2);
}

void GraphEdit::set_zoom_custom(float p_zoom, const Vector2 &p_center) {
	p_zoom = CLAMP(p_zoom, zoom_min, zoom_max);
	if (zoom == p_zoom) {
		return;
	}

	Vector2 sbofs = (Vector2(h_scroll->get_value(), v_scroll->get_value()) + p_center) / zoom;

	zoom = p_zoom;
	top_layer->queue_redraw();

	zoom_minus_button->set_disabled(zoom == zoom_min);
	zoom_plus_button->set_disabled(zoom == zoom_max);

	_update_scroll();
	minimap->queue_redraw();
	connections_layer->queue_redraw();

	if (is_visible_in_tree()) {
		Vector2 ofs = sbofs * zoom - p_center;
		h_scroll->set_value(ofs.x);
		v_scroll->set_value(ofs.y);
	}

	_update_zoom_label();
	queue_redraw();
}

float GraphEdit::get_zoom() const {
	return zoom;
}

void GraphEdit::set_zoom_step(float p_zoom_step) {
	p_zoom_step = abs(p_zoom_step);
	ERR_FAIL_COND(!isfinite(p_zoom_step));
	if (zoom_step == p_zoom_step) {
		return;
	}

	zoom_step = p_zoom_step;
	panner->set_scroll_zoom_factor(zoom_step);
}

float GraphEdit::get_zoom_step() const {
	return zoom_step;
}

void GraphEdit::set_zoom_min(float p_zoom_min) {
	ERR_FAIL_COND_MSG(p_zoom_min > zoom_max, "Cannot set min zoom level greater than max zoom level.");

	if (zoom_min == p_zoom_min) {
		return;
	}

	zoom_min = p_zoom_min;
	set_zoom(zoom);
}

float GraphEdit::get_zoom_min() const {
	return zoom_min;
}

void GraphEdit::set_zoom_max(float p_zoom_max) {
	ERR_FAIL_COND_MSG(p_zoom_max < zoom_min, "Cannot set max zoom level lesser than min zoom level.");

	if (zoom_max == p_zoom_max) {
		return;
	}

	zoom_max = p_zoom_max;
	set_zoom(zoom);
}

float GraphEdit::get_zoom_max() const {
	return zoom_max;
}

void GraphEdit::set_show_zoom_label(bool p_enable) {
	if (zoom_label->is_visible() == p_enable) {
		return;
	}

	zoom_label->set_visible(p_enable);
}

bool GraphEdit::is_showing_zoom_label() const {
	return zoom_label->is_visible();
}

void GraphEdit::set_right_disconnects(bool p_enable) {
	right_disconnects = p_enable;
}

bool GraphEdit::is_right_disconnects_enabled() const {
	return right_disconnects;
}

void GraphEdit::add_valid_right_disconnect_type(int p_type) {
	valid_right_disconnect_types.insert(p_type);
}

void GraphEdit::remove_valid_right_disconnect_type(int p_type) {
	valid_right_disconnect_types.erase(p_type);
}

void GraphEdit::add_valid_left_disconnect_type(int p_type) {
	valid_left_disconnect_types.insert(p_type);
}

void GraphEdit::remove_valid_left_disconnect_type(int p_type) {
	valid_left_disconnect_types.erase(p_type);
}

TypedArray<Dictionary> GraphEdit::_get_connection_list() const {
	List<Connection> conns;
	get_connection_list(&conns);
	TypedArray<Dictionary> arr;
	for (const Connection &E : conns) {
		Dictionary d;
		d["from"] = E.from;
		d["from_port"] = E.from_port;
		d["to"] = E.to;
		d["to_port"] = E.to_port;
		arr.push_back(d);
	}
	return arr;
}

void GraphEdit::_zoom_minus() {
	set_zoom(zoom / zoom_step);
}

void GraphEdit::_zoom_reset() {
	set_zoom(1);
}

void GraphEdit::_zoom_plus() {
	set_zoom(zoom * zoom_step);
}

void GraphEdit::_update_zoom_label() {
	int zoom_percent = static_cast<int>(Math::round(zoom * 100));
	String zoom_text = itos(zoom_percent) + "%";
	zoom_label->set_text(zoom_text);
}

void GraphEdit::add_valid_connection_type(int p_type, int p_with_type) {
	ConnectionType ct(p_type, p_with_type);
	valid_connection_types.insert(ct);
}

void GraphEdit::remove_valid_connection_type(int p_type, int p_with_type) {
	ConnectionType ct(p_type, p_with_type);
	valid_connection_types.erase(ct);
}

bool GraphEdit::is_valid_connection_type(int p_type, int p_with_type) const {
	ConnectionType ct(p_type, p_with_type);
	return valid_connection_types.has(ct);
}

void GraphEdit::set_use_grid_snap(bool p_enable) {
	use_grid_snap = p_enable;
	snap_button->set_pressed(p_enable);
	queue_redraw();
}

bool GraphEdit::is_using_grid_snap() const {
	return use_grid_snap;
}

void GraphEdit::set_grid_snap_distance(int p_snap_distance) {
	ERR_FAIL_COND(p_snap_distance < 2);
	grid_snap_distance = p_snap_distance;
	snap_distance_spinbox->set_value(p_snap_distance);
	queue_redraw();
}

int GraphEdit::get_grid_snap_distance() const {
	return grid_snap_distance;
}

void GraphEdit::set_show_grid(bool p_show) {
	show_grid = p_show;
	show_grid_button->set_pressed(p_show);
	queue_redraw();
}

bool GraphEdit::is_showing_grid() const {
	return show_grid;
}

void GraphEdit::_snap_toggled() {
	use_grid_snap = snap_button->is_pressed();
}

void GraphEdit::_snap_value_changed(double) {
	grid_snap_distance = snap_distance_spinbox->get_value();
	queue_redraw();
}

void GraphEdit::_show_grid_toggled() {
	show_grid = show_grid_button->is_pressed();
	queue_redraw();
}

void GraphEdit::set_minimap_size(Vector2 p_size) {
	minimap->set_size(p_size);
	Vector2 minimap_size = minimap->get_size(); // The size might've been adjusted by the minimum size.

	minimap->set_anchors_preset(Control::PRESET_BOTTOM_RIGHT);
	minimap->set_offset(Side::SIDE_LEFT, -minimap_size.x - MINIMAP_OFFSET);
	minimap->set_offset(Side::SIDE_TOP, -minimap_size.y - MINIMAP_OFFSET);
	minimap->set_offset(Side::SIDE_RIGHT, -MINIMAP_OFFSET);
	minimap->set_offset(Side::SIDE_BOTTOM, -MINIMAP_OFFSET);
	minimap->queue_redraw();
}

Vector2 GraphEdit::get_minimap_size() const {
	return minimap->get_size();
}

void GraphEdit::set_minimap_opacity(float p_opacity) {
	if (minimap->get_modulate().a == p_opacity) {
		return;
	}
	minimap->set_modulate(Color(1, 1, 1, p_opacity));
	minimap->queue_redraw();
}

float GraphEdit::get_minimap_opacity() const {
	Color minimap_modulate = minimap->get_modulate();
	return minimap_modulate.a;
}

void GraphEdit::set_minimap_enabled(bool p_enable) {
	if (minimap_button->is_pressed() == p_enable) {
		return;
	}
	minimap_button->set_pressed(p_enable);
	_minimap_toggled();
	minimap->queue_redraw();
}

bool GraphEdit::is_minimap_enabled() const {
	return minimap_button->is_pressed();
}

void GraphEdit::set_arrange_nodes_button_hidden(bool p_enable) {
	arrange_nodes_button_hidden = p_enable;
	if (arrange_nodes_button_hidden) {
		layout_button->hide();
	} else {
		layout_button->show();
	}
}

bool GraphEdit::is_arrange_nodes_button_hidden() const {
	return arrange_nodes_button_hidden;
}

void GraphEdit::_minimap_toggled() {
	if (is_minimap_enabled()) {
		minimap->set_visible(true);
		minimap->queue_redraw();
	} else {
		minimap->set_visible(false);
	}
}

void GraphEdit::set_connection_lines_curvature(float p_curvature) {
	lines_curvature = p_curvature;
	queue_redraw();
}

float GraphEdit::get_connection_lines_curvature() const {
	return lines_curvature;
}

void GraphEdit::set_connection_lines_thickness(float p_thickness) {
	if (lines_thickness == p_thickness) {
		return;
	}
	lines_thickness = p_thickness;
	queue_redraw();
}

float GraphEdit::get_connection_lines_thickness() const {
	return lines_thickness;
}

void GraphEdit::set_connection_lines_antialiased(bool p_antialiased) {
	if (lines_antialiased == p_antialiased) {
		return;
	}
	lines_antialiased = p_antialiased;
	queue_redraw();
}

bool GraphEdit::is_connection_lines_antialiased() const {
	return lines_antialiased;
}

HBoxContainer *GraphEdit::get_menu_hbox() {
	return zoom_hb;
}

Ref<ViewPanner> GraphEdit::get_panner() {
	return panner;
}

void GraphEdit::set_warped_panning(bool p_warped) {
	warped_panning = p_warped;
}

void GraphEdit::arrange_nodes() {
	arranger->arrange_nodes();
}

void GraphEdit::_bind_methods() {
	ClassDB::bind_method(D_METHOD("connect_node", "from_node", "from_port", "to_node", "to_port"), &GraphEdit::connect_node);
	ClassDB::bind_method(D_METHOD("is_node_connected", "from_node", "from_port", "to_node", "to_port"), &GraphEdit::is_node_connected);
	ClassDB::bind_method(D_METHOD("disconnect_node", "from_node", "from_port", "to_node", "to_port"), &GraphEdit::disconnect_node);
	ClassDB::bind_method(D_METHOD("set_connection_activity", "from_node", "from_port", "to_node", "to_port", "amount"), &GraphEdit::set_connection_activity);
	ClassDB::bind_method(D_METHOD("get_connection_list"), &GraphEdit::_get_connection_list);
	ClassDB::bind_method(D_METHOD("clear_connections"), &GraphEdit::clear_connections);
	ClassDB::bind_method(D_METHOD("force_connection_drag_end"), &GraphEdit::force_connection_drag_end);
	ClassDB::bind_method(D_METHOD("get_scroll_ofs"), &GraphEdit::get_scroll_ofs);
	ClassDB::bind_method(D_METHOD("set_scroll_ofs", "offset"), &GraphEdit::set_scroll_ofs);

	ClassDB::bind_method(D_METHOD("add_valid_right_disconnect_type", "type"), &GraphEdit::add_valid_right_disconnect_type);
	ClassDB::bind_method(D_METHOD("remove_valid_right_disconnect_type", "type"), &GraphEdit::remove_valid_right_disconnect_type);
	ClassDB::bind_method(D_METHOD("add_valid_left_disconnect_type", "type"), &GraphEdit::add_valid_left_disconnect_type);
	ClassDB::bind_method(D_METHOD("remove_valid_left_disconnect_type", "type"), &GraphEdit::remove_valid_left_disconnect_type);
	ClassDB::bind_method(D_METHOD("add_valid_connection_type", "from_type", "to_type"), &GraphEdit::add_valid_connection_type);
	ClassDB::bind_method(D_METHOD("remove_valid_connection_type", "from_type", "to_type"), &GraphEdit::remove_valid_connection_type);
	ClassDB::bind_method(D_METHOD("is_valid_connection_type", "from_type", "to_type"), &GraphEdit::is_valid_connection_type);
	ClassDB::bind_method(D_METHOD("get_connection_line", "from_node", "to_node"), &GraphEdit::get_connection_line);

	ClassDB::bind_method(D_METHOD("set_panning_scheme", "scheme"), &GraphEdit::set_panning_scheme);
	ClassDB::bind_method(D_METHOD("get_panning_scheme"), &GraphEdit::get_panning_scheme);

	ClassDB::bind_method(D_METHOD("set_zoom", "zoom"), &GraphEdit::set_zoom);
	ClassDB::bind_method(D_METHOD("get_zoom"), &GraphEdit::get_zoom);

	ClassDB::bind_method(D_METHOD("set_zoom_min", "zoom_min"), &GraphEdit::set_zoom_min);
	ClassDB::bind_method(D_METHOD("get_zoom_min"), &GraphEdit::get_zoom_min);

	ClassDB::bind_method(D_METHOD("set_zoom_max", "zoom_max"), &GraphEdit::set_zoom_max);
	ClassDB::bind_method(D_METHOD("get_zoom_max"), &GraphEdit::get_zoom_max);

	ClassDB::bind_method(D_METHOD("set_zoom_step", "zoom_step"), &GraphEdit::set_zoom_step);
	ClassDB::bind_method(D_METHOD("get_zoom_step"), &GraphEdit::get_zoom_step);

	ClassDB::bind_method(D_METHOD("set_show_zoom_label", "enable"), &GraphEdit::set_show_zoom_label);
	ClassDB::bind_method(D_METHOD("is_showing_zoom_label"), &GraphEdit::is_showing_zoom_label);

	ClassDB::bind_method(D_METHOD("set_show_grid", "enable"), &GraphEdit::set_show_grid);
	ClassDB::bind_method(D_METHOD("is_showing_grid"), &GraphEdit::is_showing_grid);

	ClassDB::bind_method(D_METHOD("set_use_grid_snap", "enable"), &GraphEdit::set_use_grid_snap);
	ClassDB::bind_method(D_METHOD("is_using_grid_snap"), &GraphEdit::is_using_grid_snap);

	ClassDB::bind_method(D_METHOD("set_grid_snap_distance", "pixels"), &GraphEdit::set_grid_snap_distance);
	ClassDB::bind_method(D_METHOD("get_grid_snap_distance"), &GraphEdit::get_grid_snap_distance);

	ClassDB::bind_method(D_METHOD("set_connection_lines_curvature", "curvature"), &GraphEdit::set_connection_lines_curvature);
	ClassDB::bind_method(D_METHOD("get_connection_lines_curvature"), &GraphEdit::get_connection_lines_curvature);

	ClassDB::bind_method(D_METHOD("set_connection_lines_thickness", "pixels"), &GraphEdit::set_connection_lines_thickness);
	ClassDB::bind_method(D_METHOD("get_connection_lines_thickness"), &GraphEdit::get_connection_lines_thickness);

	ClassDB::bind_method(D_METHOD("set_connection_lines_antialiased", "pixels"), &GraphEdit::set_connection_lines_antialiased);
	ClassDB::bind_method(D_METHOD("is_connection_lines_antialiased"), &GraphEdit::is_connection_lines_antialiased);

	ClassDB::bind_method(D_METHOD("set_minimap_size", "size"), &GraphEdit::set_minimap_size);
	ClassDB::bind_method(D_METHOD("get_minimap_size"), &GraphEdit::get_minimap_size);
	ClassDB::bind_method(D_METHOD("set_minimap_opacity", "opacity"), &GraphEdit::set_minimap_opacity);
	ClassDB::bind_method(D_METHOD("get_minimap_opacity"), &GraphEdit::get_minimap_opacity);

	ClassDB::bind_method(D_METHOD("set_minimap_enabled", "enable"), &GraphEdit::set_minimap_enabled);
	ClassDB::bind_method(D_METHOD("is_minimap_enabled"), &GraphEdit::is_minimap_enabled);

	ClassDB::bind_method(D_METHOD("set_arrange_nodes_button_hidden", "enable"), &GraphEdit::set_arrange_nodes_button_hidden);
	ClassDB::bind_method(D_METHOD("is_arrange_nodes_button_hidden"), &GraphEdit::is_arrange_nodes_button_hidden);

	ClassDB::bind_method(D_METHOD("set_right_disconnects", "enable"), &GraphEdit::set_right_disconnects);
	ClassDB::bind_method(D_METHOD("is_right_disconnects_enabled"), &GraphEdit::is_right_disconnects_enabled);

	ClassDB::bind_method(D_METHOD("_update_scroll_offset"), &GraphEdit::_update_scroll_offset);
	GDVIRTUAL_BIND(_is_in_input_hotzone, "in_node", "in_port", "mouse_position");
	GDVIRTUAL_BIND(_is_in_output_hotzone, "in_node", "in_port", "mouse_position");

	ClassDB::bind_method(D_METHOD("get_menu_hbox"), &GraphEdit::get_menu_hbox);

	ClassDB::bind_method(D_METHOD("arrange_nodes"), &GraphEdit::arrange_nodes);

	ClassDB::bind_method(D_METHOD("set_selected", "node"), &GraphEdit::set_selected);

	GDVIRTUAL_BIND(_get_connection_line, "from_position", "to_position")
	GDVIRTUAL_BIND(_is_node_hover_valid, "from_node", "from_port", "to_node", "to_port");

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "right_disconnects"), "set_right_disconnects", "is_right_disconnects_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "scroll_offset", PROPERTY_HINT_NONE, "suffix:px"), "set_scroll_ofs", "get_scroll_ofs");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_grid"), "set_show_grid", "is_showing_grid");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_snap"), "set_use_grid_snap", "is_using_grid_snap");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "snap_distance", PROPERTY_HINT_NONE, "suffix:px"), "set_grid_snap_distance", "get_grid_snap_distance");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "panning_scheme", PROPERTY_HINT_ENUM, "Scroll Zooms,Scroll Pans"), "set_panning_scheme", "get_panning_scheme");

	ADD_GROUP("Connection Lines", "connection_lines");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "connection_lines_curvature"), "set_connection_lines_curvature", "get_connection_lines_curvature");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "connection_lines_thickness", PROPERTY_HINT_NONE, "suffix:px"), "set_connection_lines_thickness", "get_connection_lines_thickness");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "connection_lines_antialiased"), "set_connection_lines_antialiased", "is_connection_lines_antialiased");

	ADD_GROUP("Zoom", "");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "zoom"), "set_zoom", "get_zoom");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "zoom_min"), "set_zoom_min", "get_zoom_min");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "zoom_max"), "set_zoom_max", "get_zoom_max");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "zoom_step"), "set_zoom_step", "get_zoom_step");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_zoom_label"), "set_show_zoom_label", "is_showing_zoom_label");

	ADD_GROUP("Minimap", "minimap_");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "minimap_enabled"), "set_minimap_enabled", "is_minimap_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "minimap_size", PROPERTY_HINT_NONE, "suffix:px"), "set_minimap_size", "get_minimap_size");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "minimap_opacity"), "set_minimap_opacity", "get_minimap_opacity");

	ADD_GROUP("UI", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "arrange_nodes_button_hidden"), "set_arrange_nodes_button_hidden", "is_arrange_nodes_button_hidden");

	ADD_SIGNAL(MethodInfo("connection_request", PropertyInfo(Variant::STRING_NAME, "from_node"), PropertyInfo(Variant::INT, "from_port"), PropertyInfo(Variant::STRING_NAME, "to_node"), PropertyInfo(Variant::INT, "to_port")));
	ADD_SIGNAL(MethodInfo("disconnection_request", PropertyInfo(Variant::STRING_NAME, "from_node"), PropertyInfo(Variant::INT, "from_port"), PropertyInfo(Variant::STRING_NAME, "to_node"), PropertyInfo(Variant::INT, "to_port")));
	ADD_SIGNAL(MethodInfo("popup_request", PropertyInfo(Variant::VECTOR2, "position")));
	ADD_SIGNAL(MethodInfo("duplicate_nodes_request"));
	ADD_SIGNAL(MethodInfo("copy_nodes_request"));
	ADD_SIGNAL(MethodInfo("paste_nodes_request"));
	ADD_SIGNAL(MethodInfo("node_selected", PropertyInfo(Variant::OBJECT, "node", PROPERTY_HINT_RESOURCE_TYPE, "Node")));
	ADD_SIGNAL(MethodInfo("node_deselected", PropertyInfo(Variant::OBJECT, "node", PROPERTY_HINT_RESOURCE_TYPE, "Node")));
	ADD_SIGNAL(MethodInfo("connection_to_empty", PropertyInfo(Variant::STRING_NAME, "from_node"), PropertyInfo(Variant::INT, "from_port"), PropertyInfo(Variant::VECTOR2, "release_position")));
	ADD_SIGNAL(MethodInfo("connection_from_empty", PropertyInfo(Variant::STRING_NAME, "to_node"), PropertyInfo(Variant::INT, "to_port"), PropertyInfo(Variant::VECTOR2, "release_position")));
	ADD_SIGNAL(MethodInfo("delete_nodes_request", PropertyInfo(Variant::ARRAY, "nodes", PROPERTY_HINT_ARRAY_TYPE, "StringName")));
	ADD_SIGNAL(MethodInfo("begin_node_move"));
	ADD_SIGNAL(MethodInfo("end_node_move"));
	ADD_SIGNAL(MethodInfo("scroll_offset_changed", PropertyInfo(Variant::VECTOR2, "offset")));
	ADD_SIGNAL(MethodInfo("connection_drag_started", PropertyInfo(Variant::STRING_NAME, "from_node"), PropertyInfo(Variant::INT, "from_port"), PropertyInfo(Variant::BOOL, "is_output")));
	ADD_SIGNAL(MethodInfo("connection_drag_ended"));

	BIND_ENUM_CONSTANT(SCROLL_ZOOMS);
	BIND_ENUM_CONSTANT(SCROLL_PANS);
}

GraphEdit::GraphEdit() {
	set_focus_mode(FOCUS_ALL);

	// Allow dezooming 8 times from the default zoom level.
	// At low zoom levels, text is unreadable due to its small size and poor filtering,
	// but this is still useful for previewing and navigation.
	zoom_min = (1 / Math::pow(zoom_step, 8));
	// Allow zooming 4 times from the default zoom level.
	zoom_max = (1 * Math::pow(zoom_step, 4));

	panner.instantiate();
	panner->set_callbacks(callable_mp(this, &GraphEdit::_pan_callback), callable_mp(this, &GraphEdit::_zoom_callback));

	top_layer = memnew(GraphEditFilter(this));
	add_child(top_layer, false, INTERNAL_MODE_BACK);
	top_layer->set_mouse_filter(MOUSE_FILTER_PASS);
	top_layer->set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);
	top_layer->connect("draw", callable_mp(this, &GraphEdit::_top_layer_draw));
	top_layer->connect("gui_input", callable_mp(this, &GraphEdit::_top_layer_input));
	top_layer->connect("focus_exited", callable_mp(panner.ptr(), &ViewPanner::release_pan_key));

	connections_layer = memnew(Control);
	add_child(connections_layer, false);
	connections_layer->connect("draw", callable_mp(this, &GraphEdit::_connections_layer_draw));
	connections_layer->set_name("CLAYER");
	connections_layer->set_disable_visibility_clip(true); // so it can draw freely and be offset
	connections_layer->set_mouse_filter(MOUSE_FILTER_IGNORE);

	h_scroll = memnew(HScrollBar);
	h_scroll->set_name("_h_scroll");
	top_layer->add_child(h_scroll);

	v_scroll = memnew(VScrollBar);
	v_scroll->set_name("_v_scroll");
	top_layer->add_child(v_scroll);

	// Set large minmax so it can scroll even if not resized yet.
	h_scroll->set_min(-10000);
	h_scroll->set_max(10000);

	v_scroll->set_min(-10000);
	v_scroll->set_max(10000);

	h_scroll->connect("value_changed", callable_mp(this, &GraphEdit::_scroll_moved));
	v_scroll->connect("value_changed", callable_mp(this, &GraphEdit::_scroll_moved));

	zoom_hb = memnew(HBoxContainer);
	top_layer->add_child(zoom_hb);
	zoom_hb->set_position(Vector2(10, 10));

	zoom_label = memnew(Label);
	zoom_hb->add_child(zoom_label);
	zoom_label->set_visible(false);
	zoom_label->set_v_size_flags(Control::SIZE_SHRINK_CENTER);
	zoom_label->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
	zoom_label->set_custom_minimum_size(Size2(48, 0));
	_update_zoom_label();

	zoom_minus_button = memnew(Button);
	zoom_minus_button->set_flat(true);
	zoom_hb->add_child(zoom_minus_button);
	zoom_minus_button->set_tooltip_text(RTR("Zoom Out"));
	zoom_minus_button->connect("pressed", callable_mp(this, &GraphEdit::_zoom_minus));
	zoom_minus_button->set_focus_mode(FOCUS_NONE);

	zoom_reset_button = memnew(Button);
	zoom_reset_button->set_flat(true);
	zoom_hb->add_child(zoom_reset_button);
	zoom_reset_button->set_tooltip_text(RTR("Zoom Reset"));
	zoom_reset_button->connect("pressed", callable_mp(this, &GraphEdit::_zoom_reset));
	zoom_reset_button->set_focus_mode(FOCUS_NONE);

	zoom_plus_button = memnew(Button);
	zoom_plus_button->set_flat(true);
	zoom_hb->add_child(zoom_plus_button);
	zoom_plus_button->set_tooltip_text(RTR("Zoom In"));
	zoom_plus_button->connect("pressed", callable_mp(this, &GraphEdit::_zoom_plus));
	zoom_plus_button->set_focus_mode(FOCUS_NONE);

	show_grid_button = memnew(Button);
	show_grid_button->set_flat(true);
	show_grid_button->set_toggle_mode(true);
	show_grid_button->set_tooltip_text(RTR("Show grid."));
	show_grid_button->connect("pressed", callable_mp(this, &GraphEdit::_show_grid_toggled));
	show_grid_button->set_pressed(true);
	show_grid_button->set_focus_mode(FOCUS_NONE);
	zoom_hb->add_child(show_grid_button);

	snap_button = memnew(Button);
	snap_button->set_flat(true);
	snap_button->set_toggle_mode(true);
	snap_button->set_tooltip_text(RTR("Enable snap."));
	snap_button->connect("pressed", callable_mp(this, &GraphEdit::_snap_toggled));
	snap_button->set_pressed(true);
	snap_button->set_focus_mode(FOCUS_NONE);
	zoom_hb->add_child(snap_button);

	snap_distance_spinbox = memnew(SpinBox);
	snap_distance_spinbox->set_min(5);
	snap_distance_spinbox->set_max(100);
	snap_distance_spinbox->set_step(1);
	snap_distance_spinbox->set_value(20);
	snap_distance_spinbox->connect("value_changed", callable_mp(this, &GraphEdit::_snap_value_changed));
	zoom_hb->add_child(snap_distance_spinbox);

	minimap_button = memnew(Button);
	minimap_button->set_flat(true);
	minimap_button->set_toggle_mode(true);
	minimap_button->set_tooltip_text(RTR("Enable grid minimap."));
	minimap_button->connect("pressed", callable_mp(this, &GraphEdit::_minimap_toggled));
	minimap_button->set_pressed(true);
	minimap_button->set_focus_mode(FOCUS_NONE);
	zoom_hb->add_child(minimap_button);

	layout_button = memnew(Button);
	layout_button->set_flat(true);
	zoom_hb->add_child(layout_button);
	layout_button->set_tooltip_text(RTR("Arrange nodes."));
	layout_button->connect("pressed", callable_mp(this, &GraphEdit::arrange_nodes));
	layout_button->set_focus_mode(FOCUS_NONE);

	Vector2 minimap_size = Vector2(240, 160);
	float minimap_opacity = 0.65;

	minimap = memnew(GraphEditMinimap(this));
	top_layer->add_child(minimap);
	minimap->set_name("_minimap");
	minimap->set_modulate(Color(1, 1, 1, minimap_opacity));
	minimap->set_mouse_filter(MOUSE_FILTER_PASS);
	minimap->set_custom_minimum_size(Vector2(50, 50));
	minimap->set_size(minimap_size);
	minimap->set_anchors_preset(Control::PRESET_BOTTOM_RIGHT);
	minimap->set_offset(Side::SIDE_LEFT, -minimap_size.x - MINIMAP_OFFSET);
	minimap->set_offset(Side::SIDE_TOP, -minimap_size.y - MINIMAP_OFFSET);
	minimap->set_offset(Side::SIDE_RIGHT, -MINIMAP_OFFSET);
	minimap->set_offset(Side::SIDE_BOTTOM, -MINIMAP_OFFSET);
	minimap->connect("draw", callable_mp(this, &GraphEdit::_minimap_draw));

	set_clip_contents(true);

	arranger = Ref<GraphEditArranger>(memnew(GraphEditArranger(this)));
}
