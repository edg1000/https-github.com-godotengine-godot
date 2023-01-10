/**************************************************************************/
/*  occluder_shape.h                                                      */
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

#ifndef OCCLUDER_SHAPE_H
#define OCCLUDER_SHAPE_H

#include "core/math/plane.h"
#include "core/resource.h"
#include "core/vector.h"

class OccluderShape : public Resource {
	GDCLASS(OccluderShape, Resource);
	OBJ_SAVE_TYPE(OccluderShape);
	RES_BASE_EXTENSION("occ");
	RID _shape;

protected:
	static void _bind_methods();

	RID get_shape() const { return _shape; }
	OccluderShape();

public:
	virtual RID get_rid() const { return _shape; }
	~OccluderShape();

	virtual Transform center_node(const Transform &p_global_xform, const Transform &p_parent_xform, real_t p_snap) = 0;

#ifdef TOOLS_ENABLED
	// for editor gizmo
	virtual AABB get_fallback_gizmo_aabb() const;
	virtual bool requires_uniform_scale() const { return false; }
#endif
};

class OccluderShapeSphere : public OccluderShape {
	GDCLASS(OccluderShapeSphere, OccluderShape);

	// We bandit a plane to store position / radius
	Vector<Plane> _spheres;
	const real_t _min_radius = 0.1;

#ifdef TOOLS_ENABLED
	AABB _aabb_local;
	void _update_aabb();
#endif

protected:
	static void _bind_methods();

public:
	void set_spheres(const Vector<Plane> &p_spheres);
	Vector<Plane> get_spheres() const { return _spheres; }

	void set_sphere_position(int p_idx, const Vector3 &p_position);
	void set_sphere_radius(int p_idx, real_t p_radius);

	void update_shape_to_visual_server();
	virtual Transform center_node(const Transform &p_global_xform, const Transform &p_parent_xform, real_t p_snap);

#ifdef TOOLS_ENABLED
	virtual AABB get_fallback_gizmo_aabb() const;
	virtual bool requires_uniform_scale() const { return false; }
#endif

	OccluderShapeSphere();
};

#endif // OCCLUDER_SHAPE_H
