/**************************************************************************/
/*  navigation_geometry_parser_3d.cpp                                     */
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

#include "navigation_geometry_parser_3d.h"

#ifndef _3D_DISABLED
bool NavigationGeometryParser3D::parses_node(Node *p_node) {
	bool parses_this_node;
	if (Object::cast_to<Node3D>(p_node) == nullptr) {
		parses_this_node = false;
		return parses_this_node;
	}
	if (GDVIRTUAL_CALL(_parses_node, p_node, parses_this_node)) {
		return parses_this_node;
	}

	return false;
}

void NavigationGeometryParser3D::parse_node_geometry(Ref<NavigationMesh> p_navigation_mesh, Node *p_node, Ref<NavigationMeshSourceGeometryData3D> p_source_geometry) {
	parse_geometry(p_node, p_navigation_mesh, p_source_geometry);
}

void NavigationGeometryParser3D::parse_geometry(Node *p_node, Ref<NavigationMesh> p_navigation_mesh, Ref<NavigationMeshSourceGeometryData3D> p_source_geometry) {
	if (GDVIRTUAL_CALL(_parse_geometry, p_node, p_navigation_mesh, p_source_geometry)) {
		return;
	}
}

void NavigationGeometryParser3D::_bind_methods() {
	GDVIRTUAL_BIND(_parses_node, "node");
	GDVIRTUAL_BIND(_parse_geometry, "node", "navigation_mesh", "source_geometry")
}
#endif // _3D_DISABLED
