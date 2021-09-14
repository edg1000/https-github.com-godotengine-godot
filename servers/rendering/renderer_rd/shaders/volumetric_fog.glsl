#[compute]

#version 450

#VERSION_DEFINES

/* Do not use subgroups here, seems there is not much advantage and causes glitches
#if defined(has_GL_KHR_shader_subgroup_ballot) && defined(has_GL_KHR_shader_subgroup_arithmetic)
#extension GL_KHR_shader_subgroup_ballot: enable
#extension GL_KHR_shader_subgroup_arithmetic: enable

#define USE_SUBGROUPS
#endif
*/

layout(local_size_x = 4, local_size_y = 4, local_size_z = 4) in;

#define SAMPLER_NEAREST_CLAMP 0
#define SAMPLER_LINEAR_CLAMP 1
#define SAMPLER_NEAREST_WITH_MIPMAPS_CLAMP 2
#define SAMPLER_LINEAR_WITH_MIPMAPS_CLAMP 3
#define SAMPLER_NEAREST_WITH_MIPMAPS_ANISOTROPIC_CLAMP 4
#define SAMPLER_LINEAR_WITH_MIPMAPS_ANISOTROPIC_CLAMP 5
#define SAMPLER_NEAREST_REPEAT 6
#define SAMPLER_LINEAR_REPEAT 7
#define SAMPLER_NEAREST_WITH_MIPMAPS_REPEAT 8
#define SAMPLER_LINEAR_WITH_MIPMAPS_REPEAT 9
#define SAMPLER_NEAREST_WITH_MIPMAPS_ANISOTROPIC_REPEAT 10
#define SAMPLER_LINEAR_WITH_MIPMAPS_ANISOTROPIC_REPEAT 11

#include "cluster_data_inc.glsl"
#include "light_data_inc.glsl"

#define M_PI 3.14159265359

layout(set = 0, binding = 1) uniform sampler material_samplers[12];

layout(set = 0, binding = 2, std430) restrict readonly buffer GlobalVariableData {
	vec4 data[];
}
global_variables;

layout(set = 1, binding = 1) uniform texture2D shadow_atlas;
layout(set = 1, binding = 2) uniform texture2D directional_shadow_atlas;

layout(set = 1, binding = 3, std430) restrict readonly buffer OmniLights {
	LightData data[];
}
omni_lights;

layout(set = 1, binding = 4, std430) restrict readonly buffer SpotLights {
	LightData data[];
}
spot_lights;

layout(set = 1, binding = 5, std140) uniform DirectionalLights {
	DirectionalLightData data[MAX_DIRECTIONAL_LIGHT_DATA_STRUCTS];
}
directional_lights;

layout(set = 1, binding = 6, std430) buffer restrict readonly ClusterBuffer {
	uint data[];
}
cluster_buffer;

layout(set = 1, binding = 7) uniform sampler linear_sampler;

layout(rgba16f, set = 1, binding = 8) uniform restrict writeonly image3D density_map;

layout(set = 1, binding = 9) uniform sampler shadow_sampler;

#define MAX_VOXEL_GI_INSTANCES 8

struct VoxelGIData {
	mat4 xform;
	vec3 bounds;
	float dynamic_range;

	float bias;
	float normal_bias;
	bool blend_ambient;
	uint texture_slot;

	float anisotropy_strength;
	float ambient_occlusion;
	float ambient_occlusion_size;
	uint mipmaps;
};

layout(set = 1, binding = 10, std140) uniform VoxelGIs {
	VoxelGIData data[MAX_VOXEL_GI_INSTANCES];
}
voxel_gi_instances;

layout(set = 1, binding = 11) uniform texture3D voxel_gi_textures[MAX_VOXEL_GI_INSTANCES];

layout(set = 1, binding = 12) uniform sampler linear_sampler_with_mipmaps;

layout(set = 1, binding = 13, std140) uniform Params {
	vec2 fog_frustum_size_begin;
	vec2 fog_frustum_size_end;

	float fog_frustum_end;
	float z_near;
	float z_far;
	float time;

	ivec3 fog_volume_size;
	uint directional_light_count;

	vec3 light_color;
	float base_density;

	float detail_spread;
	float gi_inject;
	uint max_voxel_gi_instances;
	uint cluster_type_size;

	vec2 screen_size;
	uint cluster_shift;
	uint cluster_width;

	uint max_cluster_element_count_div_32;
	bool use_temporal_reprojection;
	uint temporal_frame;
	float temporal_blend;

	mat3x4 cam_rotation;
	mat4 to_prev_view;
	mat4 transform;
}
params;

layout(set = 1, binding = 14) uniform texture3D prev_density_texture;

#ifdef ENABLE_SDFGI

// SDFGI Integration on set 1
#define SDFGI_MAX_CASCADES 8

struct SDFVoxelGICascadeData {
	vec3 position;
	float to_probe;
	ivec3 probe_world_offset;
	float to_cell; // 1/bounds * grid_size
};

layout(set = 2, binding = 0, std140) uniform SDFGI {
	vec3 grid_size;
	uint max_cascades;

	bool use_occlusion;
	int probe_axis_size;
	float probe_to_uvw;
	float normal_bias;

	vec3 lightprobe_tex_pixel_size;
	float energy;

	vec3 lightprobe_uv_offset;
	float y_mult;

	vec3 occlusion_clamp;
	uint pad3;

	vec3 occlusion_renormalize;
	uint pad4;

	vec3 cascade_probe_size;
	uint pad5;

	SDFVoxelGICascadeData cascades[SDFGI_MAX_CASCADES];
}
sdfgi;

layout(set = 2, binding = 1) uniform texture2DArray sdfgi_ambient_texture;

layout(set = 2, binding = 2) uniform texture3D sdfgi_occlusion_texture;

#endif //SDFGI

#ifdef MATERIAL_UNIFORMS_USED
layout(set = 3, binding = 0, std140) uniform MaterialUniforms{
#MATERIAL_UNIFORMS
} material;
#endif

#GLOBALS

float get_depth_at_pos(float cell_depth_size, int z) {
	float d = float(z) * cell_depth_size + cell_depth_size * 0.5; //center of voxels
	d = pow(d, params.detail_spread);
	return params.fog_frustum_end * d;
}

vec3 hash3f(uvec3 x) {
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = (x >> 16) ^ x;
	return vec3(x & 0xFFFFF) / vec3(float(0xFFFFF));
}

float get_omni_attenuation(float distance, float inv_range, float decay) {
	float nd = distance * inv_range;
	nd *= nd;
	nd *= nd; // nd^4
	nd = max(1.0 - nd, 0.0);
	nd *= nd; // nd^2
	return nd * pow(max(distance, 0.0001), -decay);
}

void cluster_get_item_range(uint p_offset, out uint item_min, out uint item_max, out uint item_from, out uint item_to) {
	uint item_min_max = cluster_buffer.data[p_offset];
	item_min = item_min_max & 0xFFFF;
	item_max = item_min_max >> 16;
	;

	item_from = item_min >> 5;
	item_to = (item_max == 0) ? 0 : ((item_max - 1) >> 5) + 1; //side effect of how it is stored, as item_max 0 means no elements
}

uint cluster_get_range_clip_mask(uint i, uint z_min, uint z_max) {
	int local_min = clamp(int(z_min) - int(i) * 32, 0, 31);
	int mask_width = min(int(z_max) - int(z_min), 32 - local_min);
	return bitfieldInsert(uint(0), uint(0xFFFFFFFF), local_min, mask_width);
}

#define TEMPORAL_FRAMES 16

const vec3 halton_map[TEMPORAL_FRAMES] = vec3[](
		vec3(0.5, 0.33333333, 0.2),
		vec3(0.25, 0.66666667, 0.4),
		vec3(0.75, 0.11111111, 0.6),
		vec3(0.125, 0.44444444, 0.8),
		vec3(0.625, 0.77777778, 0.04),
		vec3(0.375, 0.22222222, 0.24),
		vec3(0.875, 0.55555556, 0.44),
		vec3(0.0625, 0.88888889, 0.64),
		vec3(0.5625, 0.03703704, 0.84),
		vec3(0.3125, 0.37037037, 0.08),
		vec3(0.8125, 0.7037037, 0.28),
		vec3(0.1875, 0.14814815, 0.48),
		vec3(0.6875, 0.48148148, 0.68),
		vec3(0.4375, 0.81481481, 0.88),
		vec3(0.9375, 0.25925926, 0.12),
		vec3(0.03125, 0.59259259, 0.32));

void main() {
	vec3 fog_cell_size = 1.0 / vec3(params.fog_volume_size);

	ivec3 pos = ivec3(gl_GlobalInvocationID.xyz);
	if (any(greaterThanEqual(pos, params.fog_volume_size))) {
		return; //do not compute
	}

	vec3 posf = vec3(pos);

	//posf += mix(vec3(0.0),vec3(1.0),0.3) * hash3f(uvec3(pos)) * 2.0 - 1.0;

	vec3 fog_unit_pos = posf * fog_cell_size + fog_cell_size * 0.5; //center of voxels

	uvec2 screen_pos = uvec2(fog_unit_pos.xy * params.screen_size);
	uvec2 cluster_pos = screen_pos >> params.cluster_shift;
	uint cluster_offset = (params.cluster_width * cluster_pos.y + cluster_pos.x) * (params.max_cluster_element_count_div_32 + 32);
	//positions in screen are too spread apart, no hopes for optimizing with subgroups

	fog_unit_pos.z = pow(fog_unit_pos.z, params.detail_spread);

	vec3 view_pos;
	view_pos.xy = (fog_unit_pos.xy * 2.0 - 1.0) * mix(params.fog_frustum_size_begin, params.fog_frustum_size_end, vec2(fog_unit_pos.z));
	view_pos.z = -params.fog_frustum_end * fog_unit_pos.z;
	view_pos.y = -view_pos.y;

	vec4 reprojected_density = vec4(0.0);
	float reproject_amount = 0.0;

	if (params.use_temporal_reprojection) {
		vec3 prev_view = (params.to_prev_view * vec4(view_pos, 1.0)).xyz;
		//undo transform into prev view
		prev_view.y = -prev_view.y;
		//z back to unit size
		prev_view.z /= -params.fog_frustum_end;
		//xy back to unit size
		prev_view.xy /= mix(params.fog_frustum_size_begin, params.fog_frustum_size_end, vec2(prev_view.z));
		prev_view.xy = prev_view.xy * 0.5 + 0.5;
		//z back to unspread value
		prev_view.z = pow(prev_view.z, 1.0 / params.detail_spread);

		if (all(greaterThan(prev_view, vec3(0.0))) && all(lessThan(prev_view, vec3(1.0)))) {
			//reprojectinon fits

			reprojected_density = textureLod(sampler3D(prev_density_texture, linear_sampler), prev_view, 0.0);
			reproject_amount = params.temporal_blend;

			// Since we can reproject, now we must jitter the current view pos.
			// This is done here because cells that can't reproject should not jitter.

			fog_unit_pos = posf * fog_cell_size + fog_cell_size * halton_map[params.temporal_frame]; //center of voxels, offset by halton table

			screen_pos = uvec2(fog_unit_pos.xy * params.screen_size);
			cluster_pos = screen_pos >> params.cluster_shift;
			cluster_offset = (params.cluster_width * cluster_pos.y + cluster_pos.x) * (params.max_cluster_element_count_div_32 + 32);
			//positions in screen are too spread apart, no hopes for optimizing with subgroups

			fog_unit_pos.z = pow(fog_unit_pos.z, params.detail_spread);

			view_pos.xy = (fog_unit_pos.xy * 2.0 - 1.0) * mix(params.fog_frustum_size_begin, params.fog_frustum_size_end, vec2(fog_unit_pos.z));
			view_pos.z = -params.fog_frustum_end * fog_unit_pos.z;
			view_pos.y = -view_pos.y;
		}
	}

	uint cluster_z = uint(clamp((abs(view_pos.z) / params.z_far) * 32.0, 0.0, 31.0));

	vec3 total_light = params.light_color;

	float total_density = params.base_density;
	float cell_depth_size = abs(view_pos.z - get_depth_at_pos(fog_cell_size.z, pos.z + 1));
	//compute directional lights

	for (uint i = 0; i < params.directional_light_count; i++) {
		vec3 shadow_attenuation = vec3(1.0);

		if (directional_lights.data[i].shadow_enabled) {
			float depth_z = -view_pos.z;

			vec4 pssm_coord;
			vec3 shadow_color = directional_lights.data[i].shadow_color1.rgb;
			vec3 light_dir = directional_lights.data[i].direction;
			vec4 v = vec4(view_pos, 1.0);
			float z_range;

			if (depth_z < directional_lights.data[i].shadow_split_offsets.x) {
				pssm_coord = (directional_lights.data[i].shadow_matrix1 * v);
				pssm_coord /= pssm_coord.w;
				z_range = directional_lights.data[i].shadow_z_range.x;

			} else if (depth_z < directional_lights.data[i].shadow_split_offsets.y) {
				pssm_coord = (directional_lights.data[i].shadow_matrix2 * v);
				pssm_coord /= pssm_coord.w;
				z_range = directional_lights.data[i].shadow_z_range.y;

			} else if (depth_z < directional_lights.data[i].shadow_split_offsets.z) {
				pssm_coord = (directional_lights.data[i].shadow_matrix3 * v);
				pssm_coord /= pssm_coord.w;
				z_range = directional_lights.data[i].shadow_z_range.z;

			} else {
				pssm_coord = (directional_lights.data[i].shadow_matrix4 * v);
				pssm_coord /= pssm_coord.w;
				z_range = directional_lights.data[i].shadow_z_range.w;
			}

			float depth = texture(sampler2D(directional_shadow_atlas, linear_sampler), pssm_coord.xy).r;
			float shadow = exp(min(0.0, (depth - pssm_coord.z)) * z_range * directional_lights.data[i].shadow_volumetric_fog_fade);

			/*
			//float shadow = textureProj(sampler2DShadow(directional_shadow_atlas,shadow_sampler),pssm_coord);
			float shadow = 0.0;
			for(float xi=-1;xi<=1;xi++) {
				for(float yi=-1;yi<=1;yi++) {
					vec2 ofs = vec2(xi,yi) * 1.5 * params.directional_shadow_pixel_size;
					shadow += textureProj(sampler2DShadow(directional_shadow_atlas,shadow_sampler),pssm_coord + vec4(ofs,0.0,0.0));
				}

			}

			shadow /= 3.0 * 3.0;

*/
			shadow = mix(shadow, 1.0, smoothstep(directional_lights.data[i].fade_from, directional_lights.data[i].fade_to, view_pos.z)); //done with negative values for performance

			shadow_attenuation = mix(shadow_color, vec3(1.0), shadow);
		}

		total_light += shadow_attenuation * directional_lights.data[i].color * directional_lights.data[i].energy / M_PI;
	}

	//compute lights from cluster

	{ //omni lights

		uint cluster_omni_offset = cluster_offset;

		uint item_min;
		uint item_max;
		uint item_from;
		uint item_to;

		cluster_get_item_range(cluster_omni_offset + params.max_cluster_element_count_div_32 + cluster_z, item_min, item_max, item_from, item_to);

#ifdef USE_SUBGROUPS
		item_from = subgroupBroadcastFirst(subgroupMin(item_from));
		item_to = subgroupBroadcastFirst(subgroupMax(item_to));
#endif

		for (uint i = item_from; i < item_to; i++) {
			uint mask = cluster_buffer.data[cluster_omni_offset + i];
			mask &= cluster_get_range_clip_mask(i, item_min, item_max);
#ifdef USE_SUBGROUPS
			uint merged_mask = subgroupBroadcastFirst(subgroupOr(mask));
#else
			uint merged_mask = mask;
#endif

			while (merged_mask != 0) {
				uint bit = findMSB(merged_mask);
				merged_mask &= ~(1 << bit);
#ifdef USE_SUBGROUPS
				if (((1 << bit) & mask) == 0) { //do not process if not originally here
					continue;
				}
#endif
				uint light_index = 32 * i + bit;

				//if (!bool(omni_omni_lights.data[light_index].mask & draw_call.layer_mask)) {
				//	continue; //not masked
				//}

				vec3 light_pos = omni_lights.data[light_index].position;
				float d = distance(omni_lights.data[light_index].position, view_pos);
				float shadow_attenuation = 1.0;

				if (d * omni_lights.data[light_index].inv_radius < 1.0) {
					float attenuation = get_omni_attenuation(d, omni_lights.data[light_index].inv_radius, omni_lights.data[light_index].attenuation);

					vec3 light = omni_lights.data[light_index].color / M_PI;

					if (omni_lights.data[light_index].shadow_enabled) {
						//has shadow
						vec4 v = vec4(view_pos, 1.0);

						vec4 splane = (omni_lights.data[light_index].shadow_matrix * v);
						float shadow_len = length(splane.xyz); //need to remember shadow len from here

						splane.xyz = normalize(splane.xyz);
						vec4 clamp_rect = omni_lights.data[light_index].atlas_rect;

						if (splane.z >= 0.0) {
							splane.z += 1.0;

							clamp_rect.y += clamp_rect.w;

						} else {
							splane.z = 1.0 - splane.z;
						}

						splane.xy /= splane.z;

						splane.xy = splane.xy * 0.5 + 0.5;
						splane.z = shadow_len * omni_lights.data[light_index].inv_radius;
						splane.xy = clamp_rect.xy + splane.xy * clamp_rect.zw;
						splane.w = 1.0; //needed? i think it should be 1 already

						float depth = texture(sampler2D(shadow_atlas, linear_sampler), splane.xy).r;

						shadow_attenuation = exp(min(0.0, (depth - splane.z)) / omni_lights.data[light_index].inv_radius * omni_lights.data[light_index].shadow_volumetric_fog_fade);
					}
					total_light += light * attenuation * shadow_attenuation;
				}
			}
		}
	}

	{ //spot lights

		uint cluster_spot_offset = cluster_offset + params.cluster_type_size;

		uint item_min;
		uint item_max;
		uint item_from;
		uint item_to;

		cluster_get_item_range(cluster_spot_offset + params.max_cluster_element_count_div_32 + cluster_z, item_min, item_max, item_from, item_to);

#ifdef USE_SUBGROUPS
		item_from = subgroupBroadcastFirst(subgroupMin(item_from));
		item_to = subgroupBroadcastFirst(subgroupMax(item_to));
#endif

		for (uint i = item_from; i < item_to; i++) {
			uint mask = cluster_buffer.data[cluster_spot_offset + i];
			mask &= cluster_get_range_clip_mask(i, item_min, item_max);
#ifdef USE_SUBGROUPS
			uint merged_mask = subgroupBroadcastFirst(subgroupOr(mask));
#else
			uint merged_mask = mask;
#endif

			while (merged_mask != 0) {
				uint bit = findMSB(merged_mask);
				merged_mask &= ~(1 << bit);
#ifdef USE_SUBGROUPS
				if (((1 << bit) & mask) == 0) { //do not process if not originally here
					continue;
				}
#endif

				//if (!bool(omni_lights.data[light_index].mask & draw_call.layer_mask)) {
				//	continue; //not masked
				//}

				uint light_index = 32 * i + bit;

				vec3 light_pos = spot_lights.data[light_index].position;
				vec3 light_rel_vec = spot_lights.data[light_index].position - view_pos;
				float d = length(light_rel_vec);
				float shadow_attenuation = 1.0;

				if (d * spot_lights.data[light_index].inv_radius < 1.0) {
					float attenuation = get_omni_attenuation(d, spot_lights.data[light_index].inv_radius, spot_lights.data[light_index].attenuation);

					vec3 spot_dir = spot_lights.data[light_index].direction;
					float scos = max(dot(-normalize(light_rel_vec), spot_dir), spot_lights.data[light_index].cone_angle);
					float spot_rim = max(0.0001, (1.0 - scos) / (1.0 - spot_lights.data[light_index].cone_angle));
					attenuation *= 1.0 - pow(spot_rim, spot_lights.data[light_index].cone_attenuation);

					vec3 light = spot_lights.data[light_index].color / M_PI;

					if (spot_lights.data[light_index].shadow_enabled) {
						//has shadow
						vec4 v = vec4(view_pos, 1.0);

						vec4 splane = (spot_lights.data[light_index].shadow_matrix * v);
						splane /= splane.w;

						float depth = texture(sampler2D(shadow_atlas, linear_sampler), splane.xy).r;

						shadow_attenuation = exp(min(0.0, (depth - splane.z)) / spot_lights.data[light_index].inv_radius * spot_lights.data[light_index].shadow_volumetric_fog_fade);
					}

					total_light += light * attenuation * shadow_attenuation;
				}
			}
		}
	}

	vec3 world_pos = mat3(params.cam_rotation) * view_pos;

	for (uint i = 0; i < params.max_voxel_gi_instances; i++) {
		vec3 position = (voxel_gi_instances.data[i].xform * vec4(world_pos, 1.0)).xyz;

		//this causes corrupted pixels, i have no idea why..
		if (all(bvec2(all(greaterThanEqual(position, vec3(0.0))), all(lessThan(position, voxel_gi_instances.data[i].bounds))))) {
			position /= voxel_gi_instances.data[i].bounds;

			vec4 light = vec4(0.0);
			for (uint j = 0; j < voxel_gi_instances.data[i].mipmaps; j++) {
				vec4 slight = textureLod(sampler3D(voxel_gi_textures[i], linear_sampler_with_mipmaps), position, float(j));
				float a = (1.0 - light.a);
				light += a * slight;
			}

			light.rgb *= voxel_gi_instances.data[i].dynamic_range * params.gi_inject;

			total_light += light.rgb;
		}
	}

	//sdfgi
#ifdef ENABLE_SDFGI

	{
		float blend = -1.0;
		vec3 ambient_total = vec3(0.0);

		for (uint i = 0; i < sdfgi.max_cascades; i++) {
			vec3 cascade_pos = (world_pos - sdfgi.cascades[i].position) * sdfgi.cascades[i].to_probe;

			if (any(lessThan(cascade_pos, vec3(0.0))) || any(greaterThanEqual(cascade_pos, sdfgi.cascade_probe_size))) {
				continue; //skip cascade
			}

			vec3 base_pos = floor(cascade_pos);
			ivec3 probe_base_pos = ivec3(base_pos);

			vec4 ambient_accum = vec4(0.0);

			ivec3 tex_pos = ivec3(probe_base_pos.xy, int(i));
			tex_pos.x += probe_base_pos.z * sdfgi.probe_axis_size;

			for (uint j = 0; j < 8; j++) {
				ivec3 offset = (ivec3(j) >> ivec3(0, 1, 2)) & ivec3(1, 1, 1);
				ivec3 probe_posi = probe_base_pos;
				probe_posi += offset;

				// Compute weight

				vec3 probe_pos = vec3(probe_posi);
				vec3 probe_to_pos = cascade_pos - probe_pos;

				vec3 trilinear = vec3(1.0) - abs(probe_to_pos);
				float weight = trilinear.x * trilinear.y * trilinear.z;

				// Compute lightprobe occlusion

				if (sdfgi.use_occlusion) {
					ivec3 occ_indexv = abs((sdfgi.cascades[i].probe_world_offset + probe_posi) & ivec3(1, 1, 1)) * ivec3(1, 2, 4);
					vec4 occ_mask = mix(vec4(0.0), vec4(1.0), equal(ivec4(occ_indexv.x | occ_indexv.y), ivec4(0, 1, 2, 3)));

					vec3 occ_pos = clamp(cascade_pos, probe_pos - sdfgi.occlusion_clamp, probe_pos + sdfgi.occlusion_clamp) * sdfgi.probe_to_uvw;
					occ_pos.z += float(i);
					if (occ_indexv.z != 0) { //z bit is on, means index is >=4, so make it switch to the other half of textures
						occ_pos.x += 1.0;
					}

					occ_pos *= sdfgi.occlusion_renormalize;
					float occlusion = dot(textureLod(sampler3D(sdfgi_occlusion_texture, linear_sampler), occ_pos, 0.0), occ_mask);

					weight *= max(occlusion, 0.01);
				}

				// Compute ambient texture position

				ivec3 uvw = tex_pos;
				uvw.xy += offset.xy;
				uvw.x += offset.z * sdfgi.probe_axis_size;

				vec3 ambient = texelFetch(sampler2DArray(sdfgi_ambient_texture, linear_sampler), uvw, 0).rgb;

				ambient_accum.rgb += ambient * weight;
				ambient_accum.a += weight;
			}

			if (ambient_accum.a > 0) {
				ambient_accum.rgb /= ambient_accum.a;
			}
			ambient_total = ambient_accum.rgb;
			break;
		}

		total_light += ambient_total * params.gi_inject;
	}

#endif
	vec4 world = params.transform * vec4(view_pos, 1.0);
	world.xyz /= world.w;

	{
#CODE : FOG
	}

	total_density = max(0.0, total_density);
	total_light = max(vec3(0.0), total_light);

	vec4 final_density = vec4(total_light, total_density);

	final_density = mix(final_density, reprojected_density, reproject_amount);

	imageStore(density_map, pos, final_density);
}
