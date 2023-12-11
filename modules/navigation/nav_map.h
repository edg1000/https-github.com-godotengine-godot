/**************************************************************************/
/*  nav_map.h                                                             */
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

#ifndef NAV_MAP_H
#define NAV_MAP_H

#include "nav_rid.h"
#include "nav_utils.h"

#include "core/math/math_defs.h"
#include "core/object/worker_thread_pool.h"

#include <KdTree2d.h>
#include <KdTree3d.h>
#include <RVOSimulator2d.h>
#include <RVOSimulator3d.h>

class NavLink;
class NavRegion;
class NavAgent;
class NavObstacle;

class NavMap : public NavRid {
	/// Map Up
	Vector3 up = Vector3(0, 1, 0);

	/// To find the polygons edges the vertices are displaced in a grid where
	/// each cell has the following cell_size and cell_height.
	real_t cell_size = 0.25; // Must match ProjectSettings default 3D cell_size and NavigationMesh cell_size.
	real_t cell_height = 0.25; // Must match ProjectSettings default 3D cell_height and NavigationMesh cell_height.

	bool use_edge_connections = true;
	/// This value is used to detect the near edges to connect.
	real_t edge_connection_margin = 0.25;

	/// This value is used to limit how far links search to find polygons to connect to.
	real_t link_connection_radius = 1.0;

	bool polygons_dirty = true;
	bool connections_dirty = true;

	/// Map regions
	LocalVector<NavRegion *> regions;

	/// Map links
	LocalVector<NavLink *> links;
	LocalVector<gd::Polygon> link_polygons;

	/// Map polygons
	LocalVector<gd::Polygon> polygons;

	/// RVO avoidance worlds
	RVO2D::RVOSimulator2D rvo_simulation_2d;
	RVO3D::RVOSimulator3D rvo_simulation_3d;

	/// avoidance controlled agents
	LocalVector<NavAgent *> active_2d_avoidance_agents;
	LocalVector<NavAgent *> active_3d_avoidance_agents;

	/// dirty flag when one of the agent's arrays are modified
	bool agents_dirty = true;

	/// All the Agents (even the controlled one)
	LocalVector<NavAgent *> agents;

	/// All the avoidance obstacles (both static and dynamic)
	LocalVector<NavObstacle *> obstacles;

	/// Are rvo obstacles modified?
	bool obstacles_dirty = true;

	/// Physics delta time
	real_t deltatime = 0.0;

	/// Change the id each time the map is updated.
	uint32_t map_update_id = 0;

	bool use_threads = true;
	bool avoidance_use_multiple_threads = true;
	bool avoidance_use_high_priority_threads = true;

	// Performance Monitor
	int pm_region_count = 0;
	int pm_agent_count = 0;
	int pm_link_count = 0;
	int pm_polygon_count = 0;
	int pm_edge_count = 0;
	int pm_edge_merge_count = 0;
	int pm_edge_connection_count = 0;
	int pm_edge_free_count = 0;

public:
	NavMap();
	~NavMap();

	void set_up(Vector3 p_up);
	Vector3 get_up() const {
		return up;
	}

	void set_cell_size(real_t p_cell_size);
	real_t get_cell_size() const {
		return cell_size;
	}

	void set_cell_height(real_t p_cell_height);
	real_t get_cell_height() const { return cell_height; }

	void set_use_edge_connections(bool p_enabled);
	bool get_use_edge_connections() const {
		return use_edge_connections;
	}

	void set_edge_connection_margin(real_t p_edge_connection_margin);
	real_t get_edge_connection_margin() const {
		return edge_connection_margin;
	}

	void set_link_connection_radius(real_t p_link_connection_radius);
	real_t get_link_connection_radius() const {
		return link_connection_radius;
	}

	gd::PointKey get_point_key(const Vector3 &p_pos) const;

	Vector<Vector3> get_path(Vector3 p_origin, Vector3 p_destination, bool p_optimize, uint32_t p_navigation_layers, Vector<int32_t> *r_path_types, TypedArray<RID> *r_path_rids, Vector<int64_t> *r_path_owners) const;
	Vector3 get_closest_point_to_segment(const Vector3 &p_from, const Vector3 &p_to, const bool p_use_collision) const;
	Vector3 get_closest_point(const Vector3 &p_point) const;
	Vector3 get_closest_point_normal(const Vector3 &p_point) const;
	gd::ClosestPointQueryResult get_closest_point_info(const Vector3 &p_point) const;
	RID get_closest_point_owner(const Vector3 &p_point) const;

	void add_region(NavRegion *p_region);
	void remove_region(NavRegion *p_region);
	const LocalVector<NavRegion *> &get_regions() const {
		return regions;
	}

	void add_link(NavLink *p_link);
	void remove_link(NavLink *p_link);
	const LocalVector<NavLink *> &get_links() const {
		return links;
	}

	bool has_agent(NavAgent *agent) const;
	void add_agent(NavAgent *agent);
	void remove_agent(NavAgent *agent);
	const LocalVector<NavAgent *> &get_agents() const {
		return agents;
	}

	void set_agent_as_controlled(NavAgent *agent);
	void remove_agent_as_controlled(NavAgent *agent);

	bool has_obstacle(NavObstacle *obstacle) const;
	void add_obstacle(NavObstacle *obstacle);
	void remove_obstacle(NavObstacle *obstacle);
	const LocalVector<NavObstacle *> &get_obstacles() const {
		return obstacles;
	}

	uint32_t get_map_update_id() const {
		return map_update_id;
	}

	void sync();
	void step(real_t p_deltatime);
	void dispatch_callbacks();

	// Performance Monitor
	int get_pm_region_count() const { return pm_region_count; }
	int get_pm_agent_count() const { return pm_agent_count; }
	int get_pm_link_count() const { return pm_link_count; }
	int get_pm_polygon_count() const { return pm_polygon_count; }
	int get_pm_edge_count() const { return pm_edge_count; }
	int get_pm_edge_merge_count() const { return pm_edge_merge_count; }
	int get_pm_edge_connection_count() const { return pm_edge_connection_count; }
	int get_pm_edge_free_count() const { return pm_edge_free_count; }

private:
	void compute_single_step(uint32_t index, NavAgent **agent);

	void compute_single_avoidance_step_2d(uint32_t index, NavAgent **agent);
	void compute_single_avoidance_step_3d(uint32_t index, NavAgent **agent);

	void clip_path(const LocalVector<gd::Node> &p_navigation_polys, Vector<Vector3> &path, const gd::Node *p_from_node, const Vector3 &p_to_point, const gd::Node *p_to_node, Vector<int32_t> *r_path_types, TypedArray<RID> *r_path_rids, Vector<int64_t> *r_path_owners) const;
	void _update_connections();
	void _update_rvo_simulation();
	void _update_rvo_obstacles_tree_2d();
	void _update_rvo_agents_tree_2d();
	void _update_rvo_agents_tree_3d();
};

#endif // NAV_MAP_H
