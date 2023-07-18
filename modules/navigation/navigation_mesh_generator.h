/**************************************************************************/
/*  navigation_mesh_generator.h                                           */
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

#ifndef NAVIGATION_MESH_GENERATOR_H
#define NAVIGATION_MESH_GENERATOR_H

#ifndef _3D_DISABLED

#include "scene/3d/navigation_region_3d.h"
#include "scene/resources/navigation_mesh.h"

#include <Recast.h>

class NavigationMeshSourceGeometryData3D;

class NavigationMeshGenerator : public Object {
	GDCLASS(NavigationMeshGenerator, Object);
	Mutex generator_mutex;

	static NavigationMeshGenerator *singleton;

	HashSet<Ref<NavigationMesh>> baking_navmeshes;

protected:
	static void _bind_methods();

	static void _add_vertex(const Vector3 &p_vec3, Vector<float> &p_vertices);
	static void _add_mesh(const Ref<Mesh> &p_mesh, const Transform3D &p_xform, Vector<float> &p_vertices, Vector<int> &p_indices);
	static void _add_mesh_array(const Array &p_array, const Transform3D &p_xform, Vector<float> &p_vertices, Vector<int> &p_indices);
	static void _add_faces(const PackedVector3Array &p_faces, const Transform3D &p_xform, Vector<float> &p_vertices, Vector<int> &p_indices);
	static void _parse_geometry(const Transform3D &p_navmesh_transform, Node *p_node, Vector<float> &p_vertices, Vector<int> &p_indices, NavigationMesh::ParsedGeometryType p_generate_from, uint32_t p_collision_mask, bool p_recurse_children);

public:
	static NavigationMeshGenerator *get_singleton();

	NavigationMeshGenerator();
	~NavigationMeshGenerator();

	void bake(const Ref<NavigationMesh> &p_navigation_mesh, Node *p_root_node);
	void clear(Ref<NavigationMesh> p_navigation_mesh);

	void parse_source_geometry_data(const Ref<NavigationMesh> &p_navigation_mesh, Ref<NavigationMeshSourceGeometryData3D> p_source_geometry_data, Node *p_root_node, const Callable &p_callback = Callable());
	void bake_from_source_geometry_data(Ref<NavigationMesh> p_navigation_mesh, const Ref<NavigationMeshSourceGeometryData3D> &p_source_geometry_data, const Callable &p_callback = Callable());
};

#endif

#endif // NAVIGATION_MESH_GENERATOR_H
