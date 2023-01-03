/**************************************************************************/
/*  navigation_region_2d.h                                                */
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

#ifndef NAVIGATION_REGION_2D_H
#define NAVIGATION_REGION_2D_H

#include "scene/resources/navigation_polygon.h"

class NavigationRegion2D : public Node2D {
	GDCLASS(NavigationRegion2D, Node2D);

	bool enabled = true;
	RID region;
	RID map_override;
	uint32_t navigation_layers = 1;
	real_t enter_cost = 0.0;
	real_t travel_cost = 1.0;
	Ref<NavigationPolygon> navigation_polygon;
	bool baking_started = false;

	void _navigation_polygon_changed();

#ifdef DEBUG_ENABLED
	void _update_debug_mesh();
	void _update_debug_edge_connections_mesh();
	void _navigation_map_changed(RID p_map);
#endif // DEBUG_ENABLED

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	RID get_region_rid() const;

	void set_enabled(bool p_enabled);
	bool is_enabled() const;

	void set_navigation_polygon(const Ref<NavigationPolygon> &p_navigation_polygon);
	Ref<NavigationPolygon> get_navigation_polygon() const;

	void set_navigation_map(RID p_navigation_map);
	RID get_navigation_map() const;

	void set_navigation_layers(uint32_t p_navigation_layers);
	uint32_t get_navigation_layers() const;

	void set_navigation_layer_value(int p_layer_number, bool p_value);
	bool get_navigation_layer_value(int p_layer_number) const;

	void set_enter_cost(real_t p_enter_cost);
	real_t get_enter_cost() const;

	void set_travel_cost(real_t p_travel_cost);
	real_t get_travel_cost() const;

	PackedStringArray get_configuration_warnings() const override;

	void bake_navigation_polygon(bool p_on_thread);
	void _bake_finished();

	NavigationRegion2D();
	~NavigationRegion2D();

#ifdef TOOLS_ENABLED
	virtual Rect2 _edit_get_rect() const override;
	virtual bool _edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const override;
#endif
};

#endif // NAVIGATION_REGION_2D_H
