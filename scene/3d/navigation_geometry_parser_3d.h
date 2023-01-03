/**************************************************************************/
/*  navigation_geometry_parser_3d.h                                       */
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

#ifndef NAVIGATION_GEOMETRY_PARSER_3D_H
#define NAVIGATION_GEOMETRY_PARSER_3D_H

#include "core/templates/vector.h"
#include "scene/3d/node_3d.h"
#include "scene/resources/navigation_mesh.h"
#include "scene/resources/navigation_mesh_source_geometry_data_3d.h"

class NavigationGeometryParser3D : public RefCounted {
	GDCLASS(NavigationGeometryParser3D, RefCounted);

protected:
	static void _bind_methods();

	GDVIRTUAL1RC(bool, _parses_node, Node *);

	GDVIRTUAL3C(_parse_geometry, Node *, Ref<NavigationMesh>, Ref<NavigationMeshSourceGeometryData3D>);

public:
	virtual bool parses_node(Node *p_node);
	virtual void parse_geometry(Node *p_node, Ref<NavigationMesh> p_navigation_mesh, Ref<NavigationMeshSourceGeometryData3D> p_source_geometry);

	void parse_node_geometry(Ref<NavigationMesh> p_navigationmesh, Node *p_node, Ref<NavigationMeshSourceGeometryData3D> p_source_geometry);
};

#endif // NAVIGATION_GEOMETRY_PARSER_3D_H
