/*************************************************************************/
/*  openxr_util.cpp                                                      */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
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

#include "openxr_util.h"

#define ENUM_TO_STRING_CASE(e) \
	case e: {                  \
		return String(#e);     \
	} break;

// TODO see if we can generate this code further using the xml file with meta data supplied by OpenXR

String OpenXRUtil::get_view_configuration_name(XrViewConfigurationType p_view_configuration) {
	switch (p_view_configuration) {
		ENUM_TO_STRING_CASE(XR_VIEW_CONFIGURATION_TYPE_PRIMARY_MONO)
		ENUM_TO_STRING_CASE(XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO)
		ENUM_TO_STRING_CASE(XR_VIEW_CONFIGURATION_TYPE_PRIMARY_QUAD_VARJO)
		ENUM_TO_STRING_CASE(XR_VIEW_CONFIGURATION_TYPE_SECONDARY_MONO_FIRST_PERSON_OBSERVER_MSFT)
		ENUM_TO_STRING_CASE(XR_VIEW_CONFIGURATION_TYPE_MAX_ENUM)
		default: {
			return String("View Configuration ") + String::num_int64(int64_t(p_view_configuration));
		} break;
	}
}

String OpenXRUtil::get_reference_space_name(XrReferenceSpaceType p_reference_space) {
	switch (p_reference_space) {
		ENUM_TO_STRING_CASE(XR_REFERENCE_SPACE_TYPE_VIEW)
		ENUM_TO_STRING_CASE(XR_REFERENCE_SPACE_TYPE_LOCAL)
		ENUM_TO_STRING_CASE(XR_REFERENCE_SPACE_TYPE_STAGE)
		ENUM_TO_STRING_CASE(XR_REFERENCE_SPACE_TYPE_UNBOUNDED_MSFT)
		ENUM_TO_STRING_CASE(XR_REFERENCE_SPACE_TYPE_COMBINED_EYE_VARJO)
		ENUM_TO_STRING_CASE(XR_REFERENCE_SPACE_TYPE_MAX_ENUM)
		default: {
			return String("Reference space ") + String::num_int64(int64_t(p_reference_space));
		} break;
	}
}

String OpenXRUtil::get_structure_type_name(XrStructureType p_structure_type) {
	switch (p_structure_type) {
		ENUM_TO_STRING_CASE(XR_TYPE_UNKNOWN)
		ENUM_TO_STRING_CASE(XR_TYPE_API_LAYER_PROPERTIES)
		ENUM_TO_STRING_CASE(XR_TYPE_EXTENSION_PROPERTIES)
		ENUM_TO_STRING_CASE(XR_TYPE_INSTANCE_CREATE_INFO)
		ENUM_TO_STRING_CASE(XR_TYPE_SYSTEM_GET_INFO)
		ENUM_TO_STRING_CASE(XR_TYPE_SYSTEM_PROPERTIES)
		ENUM_TO_STRING_CASE(XR_TYPE_VIEW_LOCATE_INFO)
		ENUM_TO_STRING_CASE(XR_TYPE_VIEW)
		ENUM_TO_STRING_CASE(XR_TYPE_SESSION_CREATE_INFO)
		ENUM_TO_STRING_CASE(XR_TYPE_SWAPCHAIN_CREATE_INFO)
		ENUM_TO_STRING_CASE(XR_TYPE_SESSION_BEGIN_INFO)
		ENUM_TO_STRING_CASE(XR_TYPE_VIEW_STATE)
		ENUM_TO_STRING_CASE(XR_TYPE_FRAME_END_INFO)
		ENUM_TO_STRING_CASE(XR_TYPE_HAPTIC_VIBRATION)
		ENUM_TO_STRING_CASE(XR_TYPE_EVENT_DATA_BUFFER)
		ENUM_TO_STRING_CASE(XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING)
		ENUM_TO_STRING_CASE(XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED)
		ENUM_TO_STRING_CASE(XR_TYPE_ACTION_STATE_BOOLEAN)
		ENUM_TO_STRING_CASE(XR_TYPE_ACTION_STATE_FLOAT)
		ENUM_TO_STRING_CASE(XR_TYPE_ACTION_STATE_VECTOR2F)
		ENUM_TO_STRING_CASE(XR_TYPE_ACTION_STATE_POSE)
		ENUM_TO_STRING_CASE(XR_TYPE_ACTION_SET_CREATE_INFO)
		ENUM_TO_STRING_CASE(XR_TYPE_ACTION_CREATE_INFO)
		ENUM_TO_STRING_CASE(XR_TYPE_INSTANCE_PROPERTIES)
		ENUM_TO_STRING_CASE(XR_TYPE_FRAME_WAIT_INFO)
		ENUM_TO_STRING_CASE(XR_TYPE_COMPOSITION_LAYER_PROJECTION)
		ENUM_TO_STRING_CASE(XR_TYPE_COMPOSITION_LAYER_QUAD)
		ENUM_TO_STRING_CASE(XR_TYPE_REFERENCE_SPACE_CREATE_INFO)
		ENUM_TO_STRING_CASE(XR_TYPE_ACTION_SPACE_CREATE_INFO)
		ENUM_TO_STRING_CASE(XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING)
		ENUM_TO_STRING_CASE(XR_TYPE_VIEW_CONFIGURATION_VIEW)
		ENUM_TO_STRING_CASE(XR_TYPE_SPACE_LOCATION)
		ENUM_TO_STRING_CASE(XR_TYPE_SPACE_VELOCITY)
		ENUM_TO_STRING_CASE(XR_TYPE_FRAME_STATE)
		ENUM_TO_STRING_CASE(XR_TYPE_VIEW_CONFIGURATION_PROPERTIES)
		ENUM_TO_STRING_CASE(XR_TYPE_FRAME_BEGIN_INFO)
		ENUM_TO_STRING_CASE(XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW)
		ENUM_TO_STRING_CASE(XR_TYPE_EVENT_DATA_EVENTS_LOST)
		ENUM_TO_STRING_CASE(XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING)
		ENUM_TO_STRING_CASE(XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED)
		ENUM_TO_STRING_CASE(XR_TYPE_INTERACTION_PROFILE_STATE)
		ENUM_TO_STRING_CASE(XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO)
		ENUM_TO_STRING_CASE(XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO)
		ENUM_TO_STRING_CASE(XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO)
		ENUM_TO_STRING_CASE(XR_TYPE_ACTION_STATE_GET_INFO)
		ENUM_TO_STRING_CASE(XR_TYPE_HAPTIC_ACTION_INFO)
		ENUM_TO_STRING_CASE(XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO)
		ENUM_TO_STRING_CASE(XR_TYPE_ACTIONS_SYNC_INFO)
		ENUM_TO_STRING_CASE(XR_TYPE_BOUND_SOURCES_FOR_ACTION_ENUMERATE_INFO)
		ENUM_TO_STRING_CASE(XR_TYPE_INPUT_SOURCE_LOCALIZED_NAME_GET_INFO)
		ENUM_TO_STRING_CASE(XR_TYPE_COMPOSITION_LAYER_CUBE_KHR)
		ENUM_TO_STRING_CASE(XR_TYPE_INSTANCE_CREATE_INFO_ANDROID_KHR)
		ENUM_TO_STRING_CASE(XR_TYPE_COMPOSITION_LAYER_DEPTH_INFO_KHR)
		ENUM_TO_STRING_CASE(XR_TYPE_VULKAN_SWAPCHAIN_FORMAT_LIST_CREATE_INFO_KHR)
		ENUM_TO_STRING_CASE(XR_TYPE_EVENT_DATA_PERF_SETTINGS_EXT)
		ENUM_TO_STRING_CASE(XR_TYPE_COMPOSITION_LAYER_CYLINDER_KHR)
		ENUM_TO_STRING_CASE(XR_TYPE_COMPOSITION_LAYER_EQUIRECT_KHR)
		ENUM_TO_STRING_CASE(XR_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT)
		ENUM_TO_STRING_CASE(XR_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT)
		ENUM_TO_STRING_CASE(XR_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT)
		ENUM_TO_STRING_CASE(XR_TYPE_DEBUG_UTILS_LABEL_EXT)
		ENUM_TO_STRING_CASE(XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR)
		ENUM_TO_STRING_CASE(XR_TYPE_GRAPHICS_BINDING_OPENGL_XLIB_KHR)
		ENUM_TO_STRING_CASE(XR_TYPE_GRAPHICS_BINDING_OPENGL_XCB_KHR)
		ENUM_TO_STRING_CASE(XR_TYPE_GRAPHICS_BINDING_OPENGL_WAYLAND_KHR)
		ENUM_TO_STRING_CASE(XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR)
		ENUM_TO_STRING_CASE(XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR)
		ENUM_TO_STRING_CASE(XR_TYPE_GRAPHICS_BINDING_OPENGL_ES_ANDROID_KHR)
		ENUM_TO_STRING_CASE(XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_ES_KHR)
		ENUM_TO_STRING_CASE(XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_ES_KHR)
		ENUM_TO_STRING_CASE(XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR)
		ENUM_TO_STRING_CASE(XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR)
		ENUM_TO_STRING_CASE(XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR)
		ENUM_TO_STRING_CASE(XR_TYPE_GRAPHICS_BINDING_D3D11_KHR)
		ENUM_TO_STRING_CASE(XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR)
		ENUM_TO_STRING_CASE(XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR)
		ENUM_TO_STRING_CASE(XR_TYPE_GRAPHICS_BINDING_D3D12_KHR)
		ENUM_TO_STRING_CASE(XR_TYPE_SWAPCHAIN_IMAGE_D3D12_KHR)
		ENUM_TO_STRING_CASE(XR_TYPE_GRAPHICS_REQUIREMENTS_D3D12_KHR)
		ENUM_TO_STRING_CASE(XR_TYPE_SYSTEM_EYE_GAZE_INTERACTION_PROPERTIES_EXT)
		ENUM_TO_STRING_CASE(XR_TYPE_EYE_GAZE_SAMPLE_TIME_EXT)
		ENUM_TO_STRING_CASE(XR_TYPE_VISIBILITY_MASK_KHR)
		ENUM_TO_STRING_CASE(XR_TYPE_EVENT_DATA_VISIBILITY_MASK_CHANGED_KHR)
		ENUM_TO_STRING_CASE(XR_TYPE_SESSION_CREATE_INFO_OVERLAY_EXTX)
		ENUM_TO_STRING_CASE(XR_TYPE_EVENT_DATA_MAIN_SESSION_VISIBILITY_CHANGED_EXTX)
		ENUM_TO_STRING_CASE(XR_TYPE_COMPOSITION_LAYER_COLOR_SCALE_BIAS_KHR)
		ENUM_TO_STRING_CASE(XR_TYPE_SPATIAL_ANCHOR_CREATE_INFO_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_SPATIAL_ANCHOR_SPACE_CREATE_INFO_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_COMPOSITION_LAYER_IMAGE_LAYOUT_FB)
		ENUM_TO_STRING_CASE(XR_TYPE_COMPOSITION_LAYER_ALPHA_BLEND_FB)
		ENUM_TO_STRING_CASE(XR_TYPE_VIEW_CONFIGURATION_DEPTH_RANGE_EXT)
		ENUM_TO_STRING_CASE(XR_TYPE_GRAPHICS_BINDING_EGL_MNDX)
		ENUM_TO_STRING_CASE(XR_TYPE_SPATIAL_GRAPH_NODE_SPACE_CREATE_INFO_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_SYSTEM_HAND_TRACKING_PROPERTIES_EXT)
		ENUM_TO_STRING_CASE(XR_TYPE_HAND_TRACKER_CREATE_INFO_EXT)
		ENUM_TO_STRING_CASE(XR_TYPE_HAND_JOINTS_LOCATE_INFO_EXT)
		ENUM_TO_STRING_CASE(XR_TYPE_HAND_JOINT_LOCATIONS_EXT)
		ENUM_TO_STRING_CASE(XR_TYPE_HAND_JOINT_VELOCITIES_EXT)
		ENUM_TO_STRING_CASE(XR_TYPE_SYSTEM_HAND_TRACKING_MESH_PROPERTIES_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_HAND_MESH_SPACE_CREATE_INFO_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_HAND_MESH_UPDATE_INFO_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_HAND_MESH_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_HAND_POSE_TYPE_INFO_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_SECONDARY_VIEW_CONFIGURATION_SESSION_BEGIN_INFO_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_SECONDARY_VIEW_CONFIGURATION_STATE_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_SECONDARY_VIEW_CONFIGURATION_FRAME_STATE_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_SECONDARY_VIEW_CONFIGURATION_FRAME_END_INFO_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_SECONDARY_VIEW_CONFIGURATION_LAYER_INFO_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_SECONDARY_VIEW_CONFIGURATION_SWAPCHAIN_CREATE_INFO_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_CONTROLLER_MODEL_KEY_STATE_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_CONTROLLER_MODEL_NODE_PROPERTIES_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_CONTROLLER_MODEL_PROPERTIES_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_CONTROLLER_MODEL_NODE_STATE_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_CONTROLLER_MODEL_STATE_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_VIEW_CONFIGURATION_VIEW_FOV_EPIC)
		ENUM_TO_STRING_CASE(XR_TYPE_HOLOGRAPHIC_WINDOW_ATTACHMENT_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_COMPOSITION_LAYER_REPROJECTION_INFO_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_COMPOSITION_LAYER_REPROJECTION_PLANE_OVERRIDE_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_ANDROID_SURFACE_SWAPCHAIN_CREATE_INFO_FB)
		ENUM_TO_STRING_CASE(XR_TYPE_COMPOSITION_LAYER_SECURE_CONTENT_FB)
		ENUM_TO_STRING_CASE(XR_TYPE_INTERACTION_PROFILE_ANALOG_THRESHOLD_VALVE)
		ENUM_TO_STRING_CASE(XR_TYPE_HAND_JOINTS_MOTION_RANGE_INFO_EXT)
		ENUM_TO_STRING_CASE(XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR)
		ENUM_TO_STRING_CASE(XR_TYPE_VULKAN_INSTANCE_CREATE_INFO_KHR)
		ENUM_TO_STRING_CASE(XR_TYPE_VULKAN_DEVICE_CREATE_INFO_KHR)
		ENUM_TO_STRING_CASE(XR_TYPE_VULKAN_GRAPHICS_DEVICE_GET_INFO_KHR)
		ENUM_TO_STRING_CASE(XR_TYPE_COMPOSITION_LAYER_EQUIRECT2_KHR)
		ENUM_TO_STRING_CASE(XR_TYPE_SCENE_OBSERVER_CREATE_INFO_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_SCENE_CREATE_INFO_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_NEW_SCENE_COMPUTE_INFO_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_VISUAL_MESH_COMPUTE_LOD_INFO_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_SCENE_COMPONENTS_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_SCENE_COMPONENTS_GET_INFO_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_SCENE_COMPONENT_LOCATIONS_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_SCENE_COMPONENTS_LOCATE_INFO_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_SCENE_OBJECTS_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_SCENE_COMPONENT_PARENT_FILTER_INFO_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_SCENE_OBJECT_TYPES_FILTER_INFO_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_SCENE_PLANES_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_SCENE_PLANE_ALIGNMENT_FILTER_INFO_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_SCENE_MESHES_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_SCENE_MESH_BUFFERS_GET_INFO_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_SCENE_MESH_BUFFERS_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_SCENE_MESH_VERTEX_BUFFER_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_SCENE_MESH_INDICES_UINT32_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_SCENE_MESH_INDICES_UINT16_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_SERIALIZED_SCENE_FRAGMENT_DATA_GET_INFO_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_SCENE_DESERIALIZE_INFO_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_EVENT_DATA_DISPLAY_REFRESH_RATE_CHANGED_FB)
		ENUM_TO_STRING_CASE(XR_TYPE_VIVE_TRACKER_PATHS_HTCX)
		ENUM_TO_STRING_CASE(XR_TYPE_EVENT_DATA_VIVE_TRACKER_CONNECTED_HTCX)
		ENUM_TO_STRING_CASE(XR_TYPE_SYSTEM_COLOR_SPACE_PROPERTIES_FB)
		ENUM_TO_STRING_CASE(XR_TYPE_HAND_TRACKING_MESH_FB)
		ENUM_TO_STRING_CASE(XR_TYPE_HAND_TRACKING_SCALE_FB)
		ENUM_TO_STRING_CASE(XR_TYPE_HAND_TRACKING_AIM_STATE_FB)
		ENUM_TO_STRING_CASE(XR_TYPE_HAND_TRACKING_CAPSULES_STATE_FB)
		ENUM_TO_STRING_CASE(XR_TYPE_FOVEATION_PROFILE_CREATE_INFO_FB)
		ENUM_TO_STRING_CASE(XR_TYPE_SWAPCHAIN_CREATE_INFO_FOVEATION_FB)
		ENUM_TO_STRING_CASE(XR_TYPE_SWAPCHAIN_STATE_FOVEATION_FB)
		ENUM_TO_STRING_CASE(XR_TYPE_FOVEATION_LEVEL_PROFILE_CREATE_INFO_FB)
		ENUM_TO_STRING_CASE(XR_TYPE_TRIANGLE_MESH_CREATE_INFO_FB)
		ENUM_TO_STRING_CASE(XR_TYPE_SYSTEM_PASSTHROUGH_PROPERTIES_FB)
		ENUM_TO_STRING_CASE(XR_TYPE_PASSTHROUGH_CREATE_INFO_FB)
		ENUM_TO_STRING_CASE(XR_TYPE_PASSTHROUGH_LAYER_CREATE_INFO_FB)
		ENUM_TO_STRING_CASE(XR_TYPE_COMPOSITION_LAYER_PASSTHROUGH_FB)
		ENUM_TO_STRING_CASE(XR_TYPE_GEOMETRY_INSTANCE_CREATE_INFO_FB)
		ENUM_TO_STRING_CASE(XR_TYPE_GEOMETRY_INSTANCE_TRANSFORM_FB)
		ENUM_TO_STRING_CASE(XR_TYPE_PASSTHROUGH_STYLE_FB)
		ENUM_TO_STRING_CASE(XR_TYPE_PASSTHROUGH_COLOR_MAP_MONO_TO_RGBA_FB)
		ENUM_TO_STRING_CASE(XR_TYPE_PASSTHROUGH_COLOR_MAP_MONO_TO_MONO_FB)
		ENUM_TO_STRING_CASE(XR_TYPE_EVENT_DATA_PASSTHROUGH_STATE_CHANGED_FB)
		ENUM_TO_STRING_CASE(XR_TYPE_BINDING_MODIFICATIONS_KHR)
		ENUM_TO_STRING_CASE(XR_TYPE_VIEW_LOCATE_FOVEATED_RENDERING_VARJO)
		ENUM_TO_STRING_CASE(XR_TYPE_FOVEATED_VIEW_CONFIGURATION_VIEW_VARJO)
		ENUM_TO_STRING_CASE(XR_TYPE_SYSTEM_FOVEATED_RENDERING_PROPERTIES_VARJO)
		ENUM_TO_STRING_CASE(XR_TYPE_COMPOSITION_LAYER_DEPTH_TEST_VARJO)
		ENUM_TO_STRING_CASE(XR_TYPE_SYSTEM_MARKER_TRACKING_PROPERTIES_VARJO)
		ENUM_TO_STRING_CASE(XR_TYPE_EVENT_DATA_MARKER_TRACKING_UPDATE_VARJO)
		ENUM_TO_STRING_CASE(XR_TYPE_MARKER_SPACE_CREATE_INFO_VARJO)
		ENUM_TO_STRING_CASE(XR_TYPE_SPATIAL_ANCHOR_PERSISTENCE_INFO_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_SPATIAL_ANCHOR_FROM_PERSISTED_ANCHOR_CREATE_INFO_MSFT)
		ENUM_TO_STRING_CASE(XR_TYPE_SWAPCHAIN_IMAGE_FOVEATION_VULKAN_FB)
		ENUM_TO_STRING_CASE(XR_TYPE_SWAPCHAIN_STATE_ANDROID_SURFACE_DIMENSIONS_FB)
		ENUM_TO_STRING_CASE(XR_TYPE_SWAPCHAIN_STATE_SAMPLER_OPENGL_ES_FB)
		ENUM_TO_STRING_CASE(XR_TYPE_SWAPCHAIN_STATE_SAMPLER_VULKAN_FB)
		ENUM_TO_STRING_CASE(XR_TYPE_COMPOSITION_LAYER_SPACE_WARP_INFO_FB)
		ENUM_TO_STRING_CASE(XR_TYPE_SYSTEM_SPACE_WARP_PROPERTIES_FB)
		ENUM_TO_STRING_CASE(XR_STRUCTURE_TYPE_MAX_ENUM)
		default: {
			return String("Structure type ") + String::num_int64(int64_t(p_structure_type));
		} break;
	}
}

String OpenXRUtil::get_session_state_name(XrSessionState p_session_state) {
	switch (p_session_state) {
		ENUM_TO_STRING_CASE(XR_SESSION_STATE_UNKNOWN)
		ENUM_TO_STRING_CASE(XR_SESSION_STATE_IDLE)
		ENUM_TO_STRING_CASE(XR_SESSION_STATE_READY)
		ENUM_TO_STRING_CASE(XR_SESSION_STATE_SYNCHRONIZED)
		ENUM_TO_STRING_CASE(XR_SESSION_STATE_VISIBLE)
		ENUM_TO_STRING_CASE(XR_SESSION_STATE_FOCUSED)
		ENUM_TO_STRING_CASE(XR_SESSION_STATE_STOPPING)
		ENUM_TO_STRING_CASE(XR_SESSION_STATE_LOSS_PENDING)
		ENUM_TO_STRING_CASE(XR_SESSION_STATE_EXITING)
		ENUM_TO_STRING_CASE(XR_SESSION_STATE_MAX_ENUM)
		default: {
			return String("Session state ") + String::num_int64(int64_t(p_session_state));
		} break;
	}
}

String OpenXRUtil::get_action_type_name(XrActionType p_action_type) {
	switch (p_action_type) {
		ENUM_TO_STRING_CASE(XR_ACTION_TYPE_BOOLEAN_INPUT)
		ENUM_TO_STRING_CASE(XR_ACTION_TYPE_FLOAT_INPUT)
		ENUM_TO_STRING_CASE(XR_ACTION_TYPE_VECTOR2F_INPUT)
		ENUM_TO_STRING_CASE(XR_ACTION_TYPE_POSE_INPUT)
		ENUM_TO_STRING_CASE(XR_ACTION_TYPE_VIBRATION_OUTPUT)
		ENUM_TO_STRING_CASE(XR_ACTION_TYPE_MAX_ENUM)
		default: {
			return String("Action type ") + String::num_int64(int64_t(p_action_type));
		} break;
	}
}

String OpenXRUtil::make_xr_version_string(XrVersion p_version) {
	String version;

	version += String::num_int64(XR_VERSION_MAJOR(p_version));
	version += String(".");
	version += String::num_int64(XR_VERSION_MINOR(p_version));
	version += String(".");
	version += String::num_int64(XR_VERSION_PATCH(p_version));

	return version;
}
