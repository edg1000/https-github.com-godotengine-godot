/**************************************************************************/
/*  openxr_composition_layer_quad.cpp                                     */
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

#include "openxr_composition_layer_quad.h"

#include "../extensions/openxr_composition_layer_extension.h"
#include "../openxr_api.h"
#include "../openxr_interface.h"

#include "scene/3d/mesh_instance_3d.h"
#include "scene/main/viewport.h"
#include "scene/resources/3d/primitive_meshes.h"

OpenXRCompositionLayerQuad::OpenXRCompositionLayerQuad() {
	composition_layer = {
		XR_TYPE_COMPOSITION_LAYER_QUAD, // type
		nullptr, // next
		0, // layerFlags
		XR_NULL_HANDLE, // space
		XR_EYE_VISIBILITY_BOTH, // eyeVisibility
		{}, // subImage
		{ { 0, 0, 0, 0 }, { 0, 0, 0 } }, // pose
		{ (float)quad_size.x, (float)quad_size.y }, // size
	};
	openxr_layer_provider = memnew(OpenXRViewportCompositionLayerProvider((XrCompositionLayerBaseHeader *)&composition_layer));
}

OpenXRCompositionLayerQuad::~OpenXRCompositionLayerQuad() {
}

void OpenXRCompositionLayerQuad::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_quad_size", "size"), &OpenXRCompositionLayerQuad::set_quad_size);
	ClassDB::bind_method(D_METHOD("get_quad_size"), &OpenXRCompositionLayerQuad::get_quad_size);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "quad_size", PROPERTY_HINT_NONE, ""), "set_quad_size", "get_quad_size");
}

void OpenXRCompositionLayerQuad::_on_openxr_session_begun() {
	OpenXRCompositionLayer::_on_openxr_session_begun();
	if (openxr_api) {
		composition_layer.space = openxr_api->get_play_space();
	}
}

Ref<Mesh> OpenXRCompositionLayerQuad::_create_fallback_mesh() {
	Ref<QuadMesh> mesh;
	mesh.instantiate();
	mesh->set_size(quad_size);
	return mesh;
}

void OpenXRCompositionLayerQuad::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_LOCAL_TRANSFORM_CHANGED: {
			Transform3D transform = get_transform();
			Quaternion quat(transform.basis.orthonormalized());
			composition_layer.pose.orientation = { (float)quat.x, (float)quat.y, (float)quat.z, (float)quat.w };
			composition_layer.pose.position = { (float)transform.origin.x, (float)transform.origin.y, (float)transform.origin.z };
		} break;
	}
}

void OpenXRCompositionLayerQuad::set_quad_size(const Size2 &p_size) {
	quad_size = p_size;
	composition_layer.size = { (float)quad_size.x, (float)quad_size.y };
	update_fallback_mesh();
}

Size2 OpenXRCompositionLayerQuad::get_quad_size() const {
	return quad_size;
}
