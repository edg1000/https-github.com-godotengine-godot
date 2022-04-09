/*************************************************************************/
/*  cluster_builder_rd.h                                                 */
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

#ifndef CLUSTER_BUILDER_RD_H
#define CLUSTER_BUILDER_RD_H

#include "servers/rendering/renderer_rd/renderer_storage_rd.h"
#include "servers/rendering/renderer_rd/shaders/cluster_debug.glsl.gen.h"
#include "servers/rendering/renderer_rd/shaders/cluster_render.glsl.gen.h"
#include "servers/rendering/renderer_rd/shaders/cluster_store.glsl.gen.h"

class ClusterBuilderSharedDataRD {
	friend class ClusterBuilderRD;

	RID sphere_vertex_buffer;
	RID sphere_vertex_array;
	RID sphere_index_buffer;
	RID sphere_index_array;
	float sphere_overfit = 0.0; //because an icosphere is not a perfect sphere, we need to enlarge it to cover the sphere area

	RID cone_vertex_buffer;
	RID cone_vertex_array;
	RID cone_index_buffer;
	RID cone_index_array;
	float cone_overfit = 0.0; //because an cone mesh is not a perfect sphere, we need to enlarge it to cover the actual cone area

	RID box_vertex_buffer;
	RID box_vertex_array;
	RID box_index_buffer;
	RID box_index_array;

	enum Divisor {
		DIVISOR_1,
		DIVISOR_2,
		DIVISOR_4,
	};

	struct ClusterRender {
		struct PushConstant {
			uint32_t base_index;
			uint32_t pad0;
			uint32_t pad1;
			uint32_t pad2;
		};

		ClusterRenderShaderRD cluster_render_shader;
		RID shader_version;
		RID shader;
		enum PipelineVersion {
			PIPELINE_NORMAL,
			PIPELINE_MSAA,
			PIPELINE_MAX
		};

		RID shader_pipelines[PIPELINE_MAX];
	} cluster_render;

	struct ClusterStore {
		struct PushConstant {
			uint32_t cluster_render_data_size; // how much data for a single cluster takes
			uint32_t max_render_element_count_div_32; //divided by 32
			uint32_t cluster_screen_size[2];
			uint32_t render_element_count_div_32; //divided by 32
			uint32_t max_cluster_element_count_div_32; //divided by 32
			uint32_t pad1;
			uint32_t pad2;
		};

		ClusterStoreShaderRD cluster_store_shader;
		RID shader_version;
		RID shader;
		RID shader_pipeline;
	} cluster_store;

	struct ClusterDebug {
		struct PushConstant {
			uint32_t screen_size[2];
			uint32_t cluster_screen_size[2];

			uint32_t cluster_shift;
			uint32_t cluster_type;
			float z_near;
			float z_far;

			uint32_t orthogonal;
			uint32_t max_cluster_element_count_div_32;
			uint32_t pad1;
			uint32_t pad2;
		};

		ClusterDebugShaderRD cluster_debug_shader;
		RID shader_version;
		RID shader;
		RID shader_pipeline;
	} cluster_debug;

public:
	ClusterBuilderSharedDataRD();
	~ClusterBuilderSharedDataRD();
};

class ClusterBuilderRD {
public:
	enum LightType {
		LIGHT_TYPE_OMNI,
		LIGHT_TYPE_SPOT
	};

	enum BoxType {
		BOX_TYPE_REFLECTION_PROBE,
		BOX_TYPE_DECAL,
	};

	enum ElementType {
		ELEMENT_TYPE_OMNI_LIGHT,
		ELEMENT_TYPE_SPOT_LIGHT,
		ELEMENT_TYPE_DECAL,
		ELEMENT_TYPE_REFLECTION_PROBE,
		ELEMENT_TYPE_MAX,

	};

private:
	ClusterBuilderSharedDataRD *shared = nullptr;

	struct RenderElementData {
		uint32_t type; //0-4
		uint32_t touches_near;
		uint32_t touches_far;
		uint32_t original_index;
		float transform_inv[12]; //transposed transform for less space
		float scale[3];
		uint32_t pad;
	};

	uint32_t cluster_count_by_type[ELEMENT_TYPE_MAX] = {};
	uint32_t max_elements_by_type = 0;

	RenderElementData *render_elements = nullptr;
	uint32_t render_element_count = 0;
	uint32_t render_element_max = 0;

	Transform3D view_xform;
	CameraMatrix adjusted_projection;
	CameraMatrix projection;
	float z_far = 0;
	float z_near = 0;
	bool orthogonal = false;

	enum Divisor {
		DIVISOR_1,
		DIVISOR_2,
		DIVISOR_4,
	};

	uint32_t cluster_size = 32;
	bool use_msaa = true;
	Divisor divisor = DIVISOR_4;

	Size2i screen_size;
	Size2i cluster_screen_size;

	RID framebuffer;
	RID cluster_render_buffer; //used for creating
	RID cluster_buffer; //used for rendering
	RID element_buffer; //used for storing, to hint element touches far plane or near plane
	uint32_t cluster_render_buffer_size = 0;
	uint32_t cluster_buffer_size = 0;

	RID cluster_render_uniform_set;
	RID cluster_store_uniform_set;

	//persistent data

	void _clear();

	struct StateUniform {
		float projection[16];
		float inv_z_far;
		uint32_t screen_to_clusters_shift; // shift to obtain coordinates in block indices
		uint32_t cluster_screen_width; //
		uint32_t cluster_data_size; // how much data for a single cluster takes
		uint32_t cluster_depth_offset;
		uint32_t pad0;
		uint32_t pad1;
		uint32_t pad2;
	};

	RID state_uniform;

	RID debug_uniform_set;

public:
	void setup(Size2i p_screen_size, uint32_t p_max_elements, RID p_depth_buffer, RID p_depth_buffer_sampler, RID p_color_buffer);

	void begin(const Transform3D &p_view_transform, const CameraMatrix &p_cam_projection, bool p_flip_y);

	_FORCE_INLINE_ void add_light(LightType p_type, const Transform3D &p_transform, float p_radius, float p_spot_aperture) {
		if (p_type == LIGHT_TYPE_OMNI && cluster_count_by_type[ELEMENT_TYPE_OMNI_LIGHT] == max_elements_by_type) {
			return; //max number elements reached
		}
		if (p_type == LIGHT_TYPE_SPOT && cluster_count_by_type[ELEMENT_TYPE_SPOT_LIGHT] == max_elements_by_type) {
			return; //max number elements reached
		}

		RenderElementData &e = render_elements[render_element_count];

		Transform3D xform = view_xform * p_transform;

		float radius = xform.basis.get_uniform_scale();
		if (radius < 0.98 || radius > 1.02) {
			xform.basis.orthonormalize();
		}

		radius *= p_radius;

		if (p_type == LIGHT_TYPE_OMNI) {
			radius *= shared->sphere_overfit; // overfit icosphere

			//omni
			float depth = -xform.origin.z;
			if (orthogonal) {
				e.touches_near = (depth - radius) < z_near;
			} else {
				//contains camera inside light
				float radius2 = radius * shared->sphere_overfit; // overfit again for outer size (camera may be outside actual sphere but behind an icosphere vertex)
				e.touches_near = xform.origin.length_squared() < radius2 * radius2;
			}

			e.touches_far = (depth + radius) > z_far;
			e.scale[0] = radius;
			e.scale[1] = radius;
			e.scale[2] = radius;
			e.type = ELEMENT_TYPE_OMNI_LIGHT;
			e.original_index = cluster_count_by_type[ELEMENT_TYPE_OMNI_LIGHT];

			RendererStorageRD::store_transform_transposed_3x4(xform, e.transform_inv);

			cluster_count_by_type[ELEMENT_TYPE_OMNI_LIGHT]++;

		} else {
			//spot
			radius *= shared->cone_overfit; // overfit icosphere

			real_t len = Math::tan(Math::deg2rad(p_spot_aperture)) * radius;
			//approximate, probably better to use a cone support function
			float max_d = -1e20;
			float min_d = 1e20;
#define CONE_MINMAX(m_x, m_y)                                             \
	{                                                                     \
		float d = -xform.xform(Vector3(len * m_x, len * m_y, -radius)).z; \
		min_d = MIN(d, min_d);                                            \
		max_d = MAX(d, max_d);                                            \
	}

			CONE_MINMAX(1, 1);
			CONE_MINMAX(-1, 1);
			CONE_MINMAX(-1, -1);
			CONE_MINMAX(1, -1);

			if (orthogonal) {
				e.touches_near = min_d < z_near;
			} else {
				//contains camera inside light
				Plane base_plane(-xform.basis.get_axis(Vector3::AXIS_Z), xform.origin);
				float dist = base_plane.distance_to(Vector3());
				if (dist >= 0 && dist < radius) {
					//inside, check angle
					float angle = Math::rad2deg(Math::acos((-xform.origin.normalized()).dot(-xform.basis.get_axis(Vector3::AXIS_Z))));
					e.touches_near = angle < p_spot_aperture * 1.05; //overfit aperture a little due to cone overfit
				} else {
					e.touches_near = false;
				}
			}

			e.touches_far = max_d > z_far;

			e.scale[0] = len * shared->cone_overfit;
			e.scale[1] = len * shared->cone_overfit;
			e.scale[2] = radius;

			e.type = ELEMENT_TYPE_SPOT_LIGHT;
			e.original_index = cluster_count_by_type[ELEMENT_TYPE_SPOT_LIGHT]; //use omni since they share index

			RendererStorageRD::store_transform_transposed_3x4(xform, e.transform_inv);

			cluster_count_by_type[ELEMENT_TYPE_SPOT_LIGHT]++;
		}

		render_element_count++;
	}

	_FORCE_INLINE_ void add_box(BoxType p_box_type, const Transform3D &p_transform, const Vector3 &p_half_extents) {
		if (p_box_type == BOX_TYPE_DECAL && cluster_count_by_type[ELEMENT_TYPE_DECAL] == max_elements_by_type) {
			return; //max number elements reached
		}
		if (p_box_type == BOX_TYPE_REFLECTION_PROBE && cluster_count_by_type[ELEMENT_TYPE_REFLECTION_PROBE] == max_elements_by_type) {
			return; //max number elements reached
		}

		RenderElementData &e = render_elements[render_element_count];
		Transform3D xform = view_xform * p_transform;

		//extract scale and scale the matrix by it, makes things simpler
		Vector3 scale = p_half_extents;
		for (uint32_t i = 0; i < 3; i++) {
			float s = xform.basis.elements[i].length();
			scale[i] *= s;
			xform.basis.elements[i] /= s;
		};

		float box_depth = Math::abs(xform.basis.xform_inv(Vector3(0, 0, -1)).dot(scale));
		float depth = -xform.origin.z;

		if (orthogonal) {
			e.touches_near = depth - box_depth < z_near;
		} else {
			//contains camera inside box
			Vector3 inside = xform.xform_inv(Vector3(0, 0, 0)).abs();
			e.touches_near = inside.x < scale.x && inside.y < scale.y && inside.z < scale.z;
		}

		e.touches_far = depth + box_depth > z_far;

		e.scale[0] = scale.x;
		e.scale[1] = scale.y;
		e.scale[2] = scale.z;

		e.type = (p_box_type == BOX_TYPE_DECAL) ? ELEMENT_TYPE_DECAL : ELEMENT_TYPE_REFLECTION_PROBE;
		e.original_index = cluster_count_by_type[e.type];

		RendererStorageRD::store_transform_transposed_3x4(xform, e.transform_inv);

		cluster_count_by_type[e.type]++;
		render_element_count++;
	}

	void bake_cluster();
	void debug(ElementType p_element);

	RID get_cluster_buffer() const;
	uint32_t get_cluster_size() const;
	uint32_t get_max_cluster_elements() const;

	void set_shared(ClusterBuilderSharedDataRD *p_shared);

	ClusterBuilderRD();
	~ClusterBuilderRD();
};

#endif // CLUSTER_BUILDER_H
