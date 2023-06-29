/**************************************************************************/
/*  particle_process_material.cpp                                         */
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

#include "particle_process_material.h"

#include "core/version.h"

Mutex ParticleProcessMaterial::material_mutex;
SelfList<ParticleProcessMaterial>::List *ParticleProcessMaterial::dirty_materials = nullptr;
HashMap<ParticleProcessMaterial::MaterialKey, ParticleProcessMaterial::ShaderData, ParticleProcessMaterial::MaterialKey> ParticleProcessMaterial::shader_map;
ParticleProcessMaterial::ShaderNames *ParticleProcessMaterial::shader_names = nullptr;

void ParticleProcessMaterial::init_shaders() {
	dirty_materials = memnew(SelfList<ParticleProcessMaterial>::List);

	shader_names = memnew(ShaderNames);

	shader_names->direction = "direction";
	shader_names->spread = "spread";
	shader_names->flatness = "flatness";
	shader_names->direction_origin = "direction_origin";
	shader_names->direction_origin_influence = "direction_origin_influence";
	shader_names->initial_linear_velocity_min = "initial_linear_velocity_min";
	shader_names->initial_angle_min = "initial_angle_min";
	shader_names->angular_velocity_min = "angular_velocity_min";
	shader_names->orbit_velocity_min = "orbit_velocity_min";
	shader_names->linear_accel_min = "linear_accel_min";
	shader_names->radial_accel_min = "radial_accel_min";
	shader_names->tangent_accel_min = "tangent_accel_min";
	shader_names->damping_min = "damping_min";
	shader_names->scale_min = "scale_min";
	shader_names->hue_variation_min = "hue_variation_min";
	shader_names->anim_speed_min = "anim_speed_min";
	shader_names->anim_offset_min = "anim_offset_min";

	shader_names->initial_linear_velocity_max = "initial_linear_velocity_max";
	shader_names->initial_angle_max = "initial_angle_max";
	shader_names->angular_velocity_max = "angular_velocity_max";
	shader_names->orbit_velocity_max = "orbit_velocity_max";
	shader_names->linear_accel_max = "linear_accel_max";
	shader_names->radial_accel_max = "radial_accel_max";
	shader_names->tangent_accel_max = "tangent_accel_max";
	shader_names->damping_max = "damping_max";
	shader_names->scale_max = "scale_max";
	shader_names->hue_variation_max = "hue_variation_max";
	shader_names->anim_speed_max = "anim_speed_max";
	shader_names->anim_offset_max = "anim_offset_max";

	shader_names->angle_texture = "angle_texture";
	shader_names->linear_velocity_texture = "linear_velocity_texture";
	shader_names->angular_velocity_texture = "angular_velocity_texture";
	shader_names->orbit_velocity_texture = "orbit_velocity_texture";
	shader_names->linear_accel_texture = "linear_accel_texture";
	shader_names->radial_accel_texture = "radial_accel_texture";
	shader_names->tangent_accel_texture = "tangent_accel_texture";
	shader_names->damping_texture = "damping_texture";
	shader_names->scale_texture = "scale_texture";
	shader_names->hue_variation_texture = "hue_variation_texture";
	shader_names->anim_speed_texture = "anim_speed_texture";
	shader_names->anim_offset_texture = "anim_offset_texture";

	shader_names->color = "color_value";
	shader_names->color_ramp = "color_ramp";
	shader_names->split_color_curves = "split_color_curves";
	shader_names->alpha_ramp = "alpha_ramp";
	shader_names->emission_ramp = "emission_ramp";
	shader_names->color_initial_ramp = "color_initial_ramp";

	shader_names->emission_sphere_radius = "emission_sphere_radius";
	shader_names->emission_box_extents = "emission_box_extents";
	shader_names->emission_texture_point_count = "emission_texture_point_count";
	shader_names->emission_texture_points = "emission_texture_points";
	shader_names->emission_texture_normal = "emission_texture_normal";
	shader_names->emission_texture_color = "emission_texture_color";
	shader_names->emission_ring_axis = "emission_ring_axis";
	shader_names->emission_ring_height = "emission_ring_height";
	shader_names->emission_ring_radius = "emission_ring_radius";
	shader_names->emission_ring_inner_radius = "emission_ring_inner_radius";

	shader_names->turbulence_enabled = "turbulence_enabled";
	shader_names->turbulence_noise_strength = "turbulence_noise_strength";
	shader_names->turbulence_noise_scale = "turbulence_noise_scale";
	shader_names->turbulence_noise_speed = "turbulence_noise_speed";
	shader_names->turbulence_noise_speed_random = "turbulence_noise_speed_random";
	shader_names->turbulence_influence_over_life = "turbulence_influence_over_life";
	shader_names->turbulence_influence_min = "turbulence_influence_min";
	shader_names->turbulence_influence_max = "turbulence_influence_max";
	shader_names->turbulence_initial_displacement_min = "turbulence_initial_displacement_min";
	shader_names->turbulence_initial_displacement_max = "turbulence_initial_displacement_max";

	shader_names->gravity = "gravity";

	shader_names->lifetime_randomness = "lifetime_randomness";

	shader_names->sub_emitter_frequency = "sub_emitter_frequency";
	shader_names->sub_emitter_amount_at_end = "sub_emitter_amount_at_end";
	shader_names->sub_emitter_amount_at_collision = "sub_emitter_amount_at_collision";
	shader_names->sub_emitter_keep_velocity = "sub_emitter_keep_velocity";

	shader_names->collision_friction = "collision_friction";
	shader_names->collision_bounce = "collision_bounce";
}

void ParticleProcessMaterial::finish_shaders() {
	memdelete(dirty_materials);
	dirty_materials = nullptr;

	memdelete(shader_names);
}

void ParticleProcessMaterial::_update_shader() {
	dirty_materials->remove(&element);

	MaterialKey mk = _compute_key();
	if (mk.key == current_key.key) {
		return; //no update required in the end
	}

	if (shader_map.has(current_key)) {
		shader_map[current_key].users--;
		if (shader_map[current_key].users == 0) {
			//deallocate shader, as it's no longer in use
			RS::get_singleton()->free(shader_map[current_key].shader);
			shader_map.erase(current_key);
		}
	}

	current_key = mk;

	if (shader_map.has(mk)) {
		RS::get_singleton()->material_set_shader(_get_material(), shader_map[mk].shader);
		shader_map[mk].users++;
		return;
	}
	//must create a shader!

	// Add a comment to describe the shader origin (useful when converting to ShaderMaterial).
	String code = "// NOTE: Shader automatically converted from " VERSION_NAME " " VERSION_FULL_CONFIG "'s ParticleProcessMaterial.\n\n";

	code += "shader_type particles;\n";

	if (collision_scale) {
		code += "render_mode collision_use_scale;\n";
	}

	code += "uniform vec3 direction;\n";
	code += "uniform float spread;\n";
	code += "uniform float flatness;\n";
	code += "uniform vec3 direction_origin = vec3(0.0);\n";
	code += "uniform float direction_origin_influence = 0.0;\n";
	code += "uniform float initial_linear_velocity_min;\n";
	code += "uniform float initial_angle_min;\n";
	code += "uniform float angular_velocity_min;\n";
	code += "uniform float orbit_velocity_min;\n";
	code += "uniform float linear_accel_min;\n";
	code += "uniform float radial_accel_min;\n";
	code += "uniform float tangent_accel_min;\n";
	code += "uniform float damping_min;\n";
	code += "uniform float scale_min;\n";
	code += "uniform float hue_variation_min;\n";
	code += "uniform float anim_speed_min;\n";
	code += "uniform float anim_offset_min;\n";

	code += "uniform float initial_linear_velocity_max;\n";
	code += "uniform float initial_angle_max;\n";
	code += "uniform float angular_velocity_max;\n";
	code += "uniform float orbit_velocity_max;\n";
	code += "uniform float linear_accel_max;\n";
	code += "uniform float radial_accel_max;\n";
	code += "uniform float tangent_accel_max;\n";
	code += "uniform float damping_max;\n";
	code += "uniform float scale_max;\n";
	code += "uniform float hue_variation_max;\n";
	code += "uniform float anim_speed_max;\n";
	code += "uniform float anim_offset_max;\n";
	code += "uniform float lifetime_randomness;\n";

	switch (emission_shape) {
		case EMISSION_SHAPE_POINT: {
			//do none
		} break;
		case EMISSION_SHAPE_SPHERE: {
			code += "uniform float emission_sphere_radius;\n";
		} break;
		case EMISSION_SHAPE_SPHERE_SURFACE: {
			code += "uniform float emission_sphere_radius;\n";
		} break;
		case EMISSION_SHAPE_BOX: {
			code += "uniform vec3 emission_box_extents;\n";
		} break;
		case EMISSION_SHAPE_DIRECTED_POINTS: {
			code += "uniform sampler2D emission_texture_normal : hint_default_black;\n";
			[[fallthrough]];
		}
		case EMISSION_SHAPE_POINTS: {
			code += "uniform sampler2D emission_texture_points : hint_default_black;\n";
			code += "uniform int emission_texture_point_count;\n";
			if (emission_color_texture.is_valid()) {
				code += "uniform sampler2D emission_texture_color : hint_default_white;\n";
			}
		} break;
		case EMISSION_SHAPE_RING: {
			code += "uniform vec3 " + shader_names->emission_ring_axis + ";\n";
			code += "uniform float " + shader_names->emission_ring_height + ";\n";
			code += "uniform float " + shader_names->emission_ring_radius + ";\n";
			code += "uniform float " + shader_names->emission_ring_inner_radius + ";\n";
		} break;
		case EMISSION_SHAPE_MAX: { // Max value for validity check.
			break;
		}
	}

	if (sub_emitter_mode != SUB_EMITTER_DISABLED && !RenderingServer::get_singleton()->is_low_end()) {
		if (sub_emitter_mode == SUB_EMITTER_CONSTANT) {
			code += "uniform float sub_emitter_frequency;\n";
		}
		if (sub_emitter_mode == SUB_EMITTER_AT_END) {
			code += "uniform int sub_emitter_amount_at_end;\n";
		}
		if (sub_emitter_mode == SUB_EMITTER_AT_COLLISION) {
			code += "uniform int sub_emitter_amount_at_collision;\n";
		}
		code += "uniform bool sub_emitter_keep_velocity;\n";
	}

	code += "uniform vec4 color_value : source_color;\n";

	code += "uniform vec3 gravity;\n";

	if (color_ramp.is_valid()) {
		code += "uniform sampler2D color_ramp : repeat_disable;\n";
	}

	if (color_initial_ramp.is_valid()) {
		code += "uniform sampler2D color_initial_ramp : repeat_disable;\n";
	}

	if (split_color_curves) {
		code += "uniform sampler2D alpha_ramp: hint_default_white;";
		code += "uniform sampler2D emission_ramp: hint_default_black;";
	}

	if (tex_parameters[PARAM_INITIAL_LINEAR_VELOCITY].is_valid()) {
		code += "uniform sampler2D linear_velocity_texture : repeat_disable;\n";
	}
	if (tex_parameters[PARAM_ORBIT_VELOCITY].is_valid()) {
		code += "uniform sampler2D orbit_velocity_texture : repeat_disable;\n";
	}
	if (tex_parameters[PARAM_ANGULAR_VELOCITY].is_valid()) {
		code += "uniform sampler2D angular_velocity_texture : repeat_disable;\n";
	}
	if (tex_parameters[PARAM_LINEAR_ACCEL].is_valid()) {
		code += "uniform sampler2D linear_accel_texture : repeat_disable;\n";
	}
	if (tex_parameters[PARAM_RADIAL_ACCEL].is_valid()) {
		code += "uniform sampler2D radial_accel_texture : repeat_disable;\n";
	}
	if (tex_parameters[PARAM_TANGENTIAL_ACCEL].is_valid()) {
		code += "uniform sampler2D tangent_accel_texture : repeat_disable;\n";
	}
	if (tex_parameters[PARAM_DAMPING].is_valid()) {
		code += "uniform sampler2D damping_texture : repeat_disable;\n";
	}
	if (tex_parameters[PARAM_ANGLE].is_valid()) {
		code += "uniform sampler2D angle_texture : repeat_disable;\n";
	}
	if (tex_parameters[PARAM_SCALE].is_valid()) {
		code += "uniform sampler2D scale_texture : repeat_disable;\n";
	}
	if (tex_parameters[PARAM_HUE_VARIATION].is_valid()) {
		code += "uniform sampler2D hue_variation_texture : repeat_disable;\n";
	}
	if (tex_parameters[PARAM_ANIM_SPEED].is_valid()) {
		code += "uniform sampler2D anim_speed_texture : repeat_disable;\n";
	}
	if (tex_parameters[PARAM_ANIM_OFFSET].is_valid()) {
		code += "uniform sampler2D anim_offset_texture : repeat_disable;\n";
	}

	if (collision_mode == COLLISION_RIGID) {
		code += "uniform float collision_friction;\n";
		code += "uniform float collision_bounce;\n";
	}

	if (turbulence_enabled) {
		code += "uniform float turbulence_noise_strength;\n";
		code += "uniform float turbulence_noise_scale;\n";
		code += "uniform float turbulence_influence_min;\n";
		code += "uniform float turbulence_influence_max;\n";
		code += "uniform float turbulence_initial_displacement_min;\n";
		code += "uniform float turbulence_initial_displacement_max;\n";
		code += "uniform float turbulence_noise_speed_random;\n";
		code += "uniform vec3 turbulence_noise_speed = vec3(1.0, 1.0, 1.0);\n";
		if (tex_parameters[PARAM_TURB_INFLUENCE_OVER_LIFE].is_valid()) {
			code += "uniform sampler2D turbulence_influence_over_life;\n";
		}
		if (turbulence_color_ramp.is_valid()) {
			code += "uniform sampler2D turbulence_color_ramp;\n";
		}
		code += "\n";

		//functions for 3D noise / turbulence
		code += "\n\n";
		code += "vec4 grad(vec4 p) {\n";
		code += "	p = fract(vec4(\n";
		code += "		dot(p, vec4(0.143081, 0.001724, 0.280166, 0.262771)),\n";
		code += "		dot(p, vec4(0.645401, -0.047791, -0.146698, 0.595016)),\n";
		code += "		dot(p, vec4(-0.499665, -0.095734, 0.425674, -0.207367)),\n";
		code += "		dot(p, vec4(-0.013596, -0.848588, 0.423736, 0.17044))));\n";
		code += "	return fract((p.xyzw * p.yzwx) * 2365.952041) * 2.0 - 1.0;\n";
		code += "}\n";
		code += "float noise(vec4 coord) {\n";
		code += "	// Domain rotation to improve the look of XYZ slices + animation patterns.\n";
		code += "	coord = vec4(\n";
		code += "		coord.xyz + dot(coord, vec4(vec3(-0.1666667), -0.5)),\n";
		code += "		dot(coord, vec4(0.5)));\n\n";
		code += "	vec4 base = floor(coord), delta = coord - base;\n\n";
		code += "	vec4 grad_0000 = grad(base + vec4(0.0, 0.0, 0.0, 0.0)), grad_1000 = grad(base + vec4(1.0, 0.0, 0.0, 0.0));\n";
		code += "	vec4 grad_0100 = grad(base + vec4(0.0, 1.0, 0.0, 0.0)), grad_1100 = grad(base + vec4(1.0, 1.0, 0.0, 0.0));\n";
		code += "	vec4 grad_0010 = grad(base + vec4(0.0, 0.0, 1.0, 0.0)), grad_1010 = grad(base + vec4(1.0, 0.0, 1.0, 0.0));\n";
		code += "	vec4 grad_0110 = grad(base + vec4(0.0, 1.0, 1.0, 0.0)), grad_1110 = grad(base + vec4(1.0, 1.0, 1.0, 0.0));\n";
		code += "	vec4 grad_0001 = grad(base + vec4(0.0, 0.0, 0.0, 1.0)), grad_1001 = grad(base + vec4(1.0, 0.0, 0.0, 1.0));\n";
		code += "	vec4 grad_0101 = grad(base + vec4(0.0, 1.0, 0.0, 1.0)), grad_1101 = grad(base + vec4(1.0, 1.0, 0.0, 1.0));\n";
		code += "	vec4 grad_0011 = grad(base + vec4(0.0, 0.0, 1.0, 1.0)), grad_1011 = grad(base + vec4(1.0, 0.0, 1.0, 1.0));\n";
		code += "	vec4 grad_0111 = grad(base + vec4(0.0, 1.0, 1.0, 1.0)), grad_1111 = grad(base + vec4(1.0, 1.0, 1.0, 1.0));\n\n";
		code += "	vec4 result_0123 = vec4(\n";
		code += "		dot(delta - vec4(0.0, 0.0, 0.0, 0.0), grad_0000), dot(delta - vec4(1.0, 0.0, 0.0, 0.0), grad_1000),\n";
		code += "		dot(delta - vec4(0.0, 1.0, 0.0, 0.0), grad_0100), dot(delta - vec4(1.0, 1.0, 0.0, 0.0), grad_1100));\n";
		code += "	vec4 result_4567 = vec4(\n";
		code += "		dot(delta - vec4(0.0, 0.0, 1.0, 0.0), grad_0010), dot(delta - vec4(1.0, 0.0, 1.0, 0.0), grad_1010),\n";
		code += "		dot(delta - vec4(0.0, 1.0, 1.0, 0.0), grad_0110), dot(delta - vec4(1.0, 1.0, 1.0, 0.0), grad_1110));\n";
		code += "	vec4 result_89AB = vec4(\n";
		code += "		dot(delta - vec4(0.0, 0.0, 0.0, 1.0), grad_0001), dot(delta - vec4(1.0, 0.0, 0.0, 1.0), grad_1001),\n";
		code += "		dot(delta - vec4(0.0, 1.0, 0.0, 1.0), grad_0101), dot(delta - vec4(1.0, 1.0, 0.0, 1.0), grad_1101));\n";
		code += "	vec4 result_CDEF = vec4(\n";
		code += "		dot(delta - vec4(0.0, 0.0, 1.0, 1.0), grad_0011), dot(delta - vec4(1.0, 0.0, 1.0, 1.0), grad_1011),\n";
		code += "		dot(delta - vec4(0.0, 1.0, 1.0, 1.0), grad_0111), dot(delta - vec4(1.0, 1.0, 1.0, 1.0), grad_1111));\n\n";
		code += "	vec4 fade = delta * delta * delta * (10.0 + delta * (-15.0 + delta * 6.0));\n";
		code += "	vec4 result_W0 = mix(result_0123, result_89AB, fade.w), result_W1 = mix(result_4567, result_CDEF, fade.w);\n";
		code += "	vec4 result_WZ = mix(result_W0, result_W1, fade.z);\n";
		code += "	vec2 result_WZY = mix(result_WZ.xy, result_WZ.zw, fade.y);\n";
		code += "	return mix(result_WZY.x, result_WZY.y, fade.x);\n";
		code += "}\n\n";
		code += "// Curl 3D and three-noise function with friendly permission by Isaac Cohen.\n";
		code += "// Modified to accept 4D noise.\n";
		code += "vec3 noise_3x(vec4 p) {\n";
		code += "	float s = noise(p);\n";
		code += "	float s1 = noise(p + vec4(vec3(0.0), 1.7320508 * 2048.333333));\n";
		code += "	float s2 = noise(p - vec4(vec3(0.0), 1.7320508 * 2048.333333));\n";
		code += "	vec3 c = vec3(s, s1, s2);\n";
		code += "	return c;\n";
		code += "}\n";
		code += "vec3 curl_3d(vec4 p, float c) {\n";
		code += "	float epsilon = 0.001 + c;\n";
		code += "	vec4 dx = vec4(epsilon, 0.0, 0.0, 0.0);\n";
		code += "	vec4 dy = vec4(0.0, epsilon, 0.0, 0.0);\n";
		code += "	vec4 dz = vec4(0.0, 0.0, epsilon, 0.0);\n";
		code += "	vec3 x0 = noise_3x(p - dx).xyz;\n";
		code += "	vec3 x1 = noise_3x(p + dx).xyz;\n";
		code += "	vec3 y0 = noise_3x(p - dy).xyz;\n";
		code += "	vec3 y1 = noise_3x(p + dy).xyz;\n";
		code += "	vec3 z0 = noise_3x(p - dz).xyz;\n";
		code += "	vec3 z1 = noise_3x(p + dz).xyz;\n";
		code += "	float x = (y1.z - y0.z) - (z1.y - z0.y);\n";
		code += "	float y = (z1.x - z0.x) - (x1.z - x0.z);\n";
		code += "	float z = (x1.y - x0.y) - (y1.x - y0.x);\n";
		code += "	return normalize(vec3(x, y, z));\n";
		code += "}\n";
		code += "vec3 get_noise_direction(vec3 pos) {\n";
		code += "	float adj_contrast = max((turbulence_noise_strength - 1.0), 0.0) * 70.0;\n";
		code += "	vec4 noise_time = TIME * vec4(turbulence_noise_speed, turbulence_noise_speed_random);\n";
		code += "	vec4 noise_pos = vec4(pos * turbulence_noise_scale, 0.0);\n";
		code += "	vec3 noise_direction = curl_3d(noise_pos + noise_time, adj_contrast);\n";
		code += "	noise_direction = mix(0.9 * noise_direction, noise_direction, turbulence_noise_strength - 9.0);\n";
		code += "	return noise_direction;\n";
		code += "}\n";
	}

	//need a random function
	code += "\n\n";
	code += "float rand_from_seed(inout uint seed) {\n";
	code += "	int k;\n";
	code += "	int s = int(seed);\n";
	code += "	if (s == 0)\n";
	code += "	s = 305420679;\n";
	code += "	k = s / 127773;\n";
	code += "	s = 16807 * (s - k * 127773) - 2836 * k;\n";
	code += "	if (s < 0)\n";
	code += "		s += 2147483647;\n";
	code += "	seed = uint(s);\n";
	code += "	return float(seed % uint(65536)) / 65535.0;\n";
	code += "}\n";
	code += "\n";

	code += "float rand_from_seed_m1_p1(inout uint seed) {\n";
	code += "	return rand_from_seed(seed) * 2.0 - 1.0;\n";
	code += "}\n";
	code += "\n";

	//improve seed quality
	code += "uint hash(uint x) {\n";
	code += "	x = ((x >> uint(16)) ^ x) * uint(73244475);\n";
	code += "	x = ((x >> uint(16)) ^ x) * uint(73244475);\n";
	code += "	x = (x >> uint(16)) ^ x;\n";
	code += "	return x;\n";
	code += "}\n";
	code += "\n";

	code += "void start() {\n";
	code += "	uint base_number = NUMBER;\n";
	code += "	uint alt_seed = hash(base_number + uint(1) + RANDOM_SEED);\n";
	code += "	float angle_rand = rand_from_seed(alt_seed);\n";
	code += "	float scale_rand = rand_from_seed(alt_seed);\n";
	code += "	float hue_rot_rand = rand_from_seed(alt_seed);\n";
	code += "	float anim_offset_rand = rand_from_seed(alt_seed);\n";
	if (color_initial_ramp.is_valid()) {
		code += "	float color_initial_rand = rand_from_seed(alt_seed);\n";
	}
	code += "	float pi = 3.14159;\n";
	code += "	float degree_to_rad = pi / 180.0;\n";
	code += "\n";

	if (emission_shape == EMISSION_SHAPE_POINTS || emission_shape == EMISSION_SHAPE_DIRECTED_POINTS) {
		code += "	int point = min(emission_texture_point_count - 1, int(rand_from_seed(alt_seed) * float(emission_texture_point_count)));\n";
		code += "	ivec2 emission_tex_size = textureSize(emission_texture_points, 0);\n";
		code += "	ivec2 emission_tex_ofs = ivec2(point % emission_tex_size.x, point / emission_tex_size.x);\n";
	}
	if (tex_parameters[PARAM_ANGLE].is_valid()) {
		code += "	float tex_angle = textureLod(angle_texture, vec2(0.0, 0.0), 0.0).r;\n";
	} else {
		code += "	float tex_angle = 1.0;\n";
	}

	if (tex_parameters[PARAM_ANIM_OFFSET].is_valid()) {
		code += "	float tex_anim_offset = textureLod(anim_offset_texture, vec2(0.0, 0.0), 0.0).r;\n";
	} else {
		code += "	float tex_anim_offset = 1.0;\n";
	}

	code += "	float spread_rad = spread * degree_to_rad;\n";

	code += "	if (RESTART_VELOCITY) {\n";

	if (tex_parameters[PARAM_INITIAL_LINEAR_VELOCITY].is_valid()) {
		code += "		float tex_linear_velocity = textureLod(linear_velocity_texture, vec2(0.0, 0.0), 0.0).r;\n";
	} else {
		code += "		float tex_linear_velocity = 1.0;\n";
	}

	code += "	float base_angle = (tex_angle) * mix(initial_angle_min, initial_angle_max, angle_rand);\n";
	code += "	CUSTOM.x = base_angle * degree_to_rad;\n"; // angle
	code += "	CUSTOM.y = 0.0;\n"; // phase
	code += "	CUSTOM.w = (1.0 - lifetime_randomness * rand_from_seed(alt_seed));\n";
	code += "	CUSTOM.z = (tex_anim_offset) * mix(anim_offset_min, anim_offset_max, anim_offset_rand);\n\n"; // animation offset (0-1)

	code += "	if (RESTART_ROT_SCALE) {\n";
	code += "		TRANSFORM[0].xyz = vec3(1.0, 0.0, 0.0);\n";
	code += "		TRANSFORM[1].xyz = vec3(0.0, 1.0, 0.0);\n";
	code += "		TRANSFORM[2].xyz = vec3(0.0, 0.0, 1.0);\n";
	code += "	}\n\n";

	code += "	if (RESTART_POSITION) {\n";

	switch (emission_shape) {
		case EMISSION_SHAPE_POINT: {
			//do none, identity (will later be multiplied by emission transform)
			code += "		TRANSFORM[3].xyz = vec3(0.0, 0.0, 0.0);\n";
		} break;
		case EMISSION_SHAPE_SPHERE: {
			code += "		float s = rand_from_seed(alt_seed) * 2.0 - 1.0;\n";
			code += "		float t = rand_from_seed(alt_seed) * 2.0 * pi;\n";
			code += "		float p = rand_from_seed(alt_seed);\n";
			code += "		float radius = emission_sphere_radius * sqrt(1.0 - s * s);\n";
			code += "		TRANSFORM[3].xyz = mix(vec3(0.0, 0.0, 0.0), vec3(radius * cos(t), radius * sin(t), emission_sphere_radius * s), p);\n";
		} break;
		case EMISSION_SHAPE_SPHERE_SURFACE: {
			code += "		float s = rand_from_seed(alt_seed) * 2.0 - 1.0;\n";
			code += "		float t = rand_from_seed(alt_seed) * 2.0 * pi;\n";
			code += "		float radius = emission_sphere_radius * sqrt(1.0 - s * s);\n";
			code += "		TRANSFORM[3].xyz = vec3(radius * cos(t), radius * sin(t), emission_sphere_radius * s);\n";
		} break;
		case EMISSION_SHAPE_BOX: {
			code += "		TRANSFORM[3].xyz = vec3(rand_from_seed(alt_seed) * 2.0 - 1.0, rand_from_seed(alt_seed) * 2.0 - 1.0, rand_from_seed(alt_seed) * 2.0 - 1.0) * emission_box_extents;\n";
		} break;
		case EMISSION_SHAPE_POINTS:
		case EMISSION_SHAPE_DIRECTED_POINTS: {
			code += "		TRANSFORM[3].xyz = texelFetch(emission_texture_points, emission_tex_ofs, 0).xyz;\n";

			if (emission_shape == EMISSION_SHAPE_DIRECTED_POINTS) {
				if (particle_flags[PARTICLE_FLAG_DISABLE_Z]) {
					code += "		{\n";
					code += "			mat2 rotm;";
					code += "			rotm[0] = texelFetch(emission_texture_normal, emission_tex_ofs, 0).xy;\n";
					code += "			rotm[1] = rotm[0].yx * vec2(1.0, -1.0);\n";
					code += "			if (RESTART_VELOCITY) VELOCITY.xy = rotm * VELOCITY.xy;\n";
					code += "		}\n";
				} else {
					code += "		{\n";
					code += "			vec3 normal = texelFetch(emission_texture_normal, emission_tex_ofs, 0).xyz;\n";
					code += "			vec3 v0 = abs(normal.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(0.0, 1.0, 0.0);\n";
					code += "			vec3 tangent = normalize(cross(v0, normal));\n";
					code += "			vec3 bitangent = normalize(cross(tangent, normal));\n";
					code += "			if (RESTART_VELOCITY) VELOCITY = mat3(tangent, bitangent, normal) * VELOCITY;\n";
					code += "		}\n";
				}
			}
		} break;
		case EMISSION_SHAPE_RING: {
			code += "		float ring_spawn_angle = rand_from_seed(alt_seed) * 2.0 * pi;\n";
			code += "		float ring_random_radius = rand_from_seed(alt_seed) * (emission_ring_radius - emission_ring_inner_radius) + emission_ring_inner_radius;\n";
			code += "		vec3 axis = normalize(emission_ring_axis);\n";
			code += "		vec3 ortho_axis = vec3(0.0);\n";
			code += "		if (axis == vec3(1.0, 0.0, 0.0)) {\n";
			code += "			ortho_axis = cross(axis, vec3(0.0, 1.0, 0.0));\n";
			code += "		} else {\n";
			code += " 			ortho_axis = cross(axis, vec3(1.0, 0.0, 0.0));\n";
			code += "		}\n";
			code += "		ortho_axis = normalize(ortho_axis);\n";
			code += "		float s = sin(ring_spawn_angle);\n";
			code += "		float c = cos(ring_spawn_angle);\n";
			code += "		float oc = 1.0 - c;\n";
			code += "		ortho_axis = mat3(\n";
			code += "			vec3(c + axis.x * axis.x * oc, axis.x * axis.y * oc - axis.z * s, axis.x * axis.z *oc + axis.y * s),\n";
			code += "			vec3(axis.x * axis.y * oc + s * axis.z, c + axis.y * axis.y * oc, axis.y * axis.z * oc - axis.x * s),\n";
			code += "			vec3(axis.z * axis.x * oc - axis.y * s, axis.z * axis.y * oc + axis.x * s, c + axis.z * axis.z * oc)\n";
			code += "			) * ortho_axis;\n";
			code += "		ortho_axis = normalize(ortho_axis);\n";
			code += "		TRANSFORM[3].xyz = ortho_axis * ring_random_radius + (rand_from_seed(alt_seed) * emission_ring_height - emission_ring_height / 2.0) * axis;\n";
		} break;
		case EMISSION_SHAPE_MAX: { // Max value for validity check.
			break;
		}
	}

	code += "	if (RESTART_VELOCITY) VELOCITY = (EMISSION_TRANSFORM * vec4(VELOCITY, 0.0)).xyz;\n";
	// Apply noise/turbulence: initial displacement.
	if (turbulence_enabled) {
		code += "	vec3 noise_direction = get_noise_direction(TRANSFORM[3].xyz);\n";
		code += "	float turb_init_displacement = mix(turbulence_initial_displacement_min, turbulence_initial_displacement_max, rand_from_seed(alt_seed));";
		code += "	TRANSFORM[3].xyz += noise_direction * turb_init_displacement;\n";
	}

	code += "   float initial_velocity_magnitude = mix(initial_linear_velocity_min, initial_linear_velocity_max,rand_from_seed(alt_seed));\n";

	if (particle_flags[PARTICLE_FLAG_DISABLE_Z]) {
		code += "		{\n";
		code += "			float angle1_rad = rand_from_seed_m1_p1(alt_seed) * spread_rad;\n";
		code += "			angle1_rad += direction.x != 0.0 ? atan(direction.y, direction.x) : sign(direction.y) * (pi / 2.0);\n";
		code += "			vec3 rot = vec3(cos(angle1_rad), sin(angle1_rad), 0.0);\n";
		code += "			VELOCITY = rot;\n";
		code += "		}\n";

	} else {
		//initiate velocity spread in 3D
		code += "		{\n";
		code += "			float angle1_rad = rand_from_seed_m1_p1(alt_seed) * spread_rad;\n";
		code += "			float angle2_rad = rand_from_seed_m1_p1(alt_seed) * spread_rad * (1.0 - flatness);\n";
		code += "			vec3 direction_xz = vec3(sin(angle1_rad), 0.0, cos(angle1_rad));\n";
		code += "			vec3 direction_yz = vec3(0.0, sin(angle2_rad), cos(angle2_rad));\n";
		code += "			direction_yz.z = direction_yz.z / max(0.0001,sqrt(abs(direction_yz.z))); // better uniform distribution\n";
		code += "			vec3 spread_direction = vec3(direction_xz.x * direction_yz.z, direction_yz.y, direction_xz.z * direction_yz.z);\n";
		code += "			vec3 direction_nrm = length(direction) > 0.0 ? normalize(direction) : vec3(0.0, 0.0, 1.0);\n";
		code += "			// rotate spread to direction\n";
		code += "			vec3 binormal = cross(vec3(0.0, 1.0, 0.0), direction_nrm);\n";
		code += "			if (length(binormal) < 0.0001) {\n";
		code += "				// direction is parallel to Y. Choose Z as the binormal.\n";
		code += "				binormal = vec3(0.0, 0.0, 1.0);\n";
		code += "			}\n";
		code += "			binormal = normalize(binormal);\n";
		code += "			vec3 normal = cross(binormal, direction_nrm);\n";
		code += "			spread_direction = binormal * spread_direction.x + normal * spread_direction.y + direction_nrm * spread_direction.z;\n";
		code += "			VELOCITY = spread_direction;\n";
		code += "		}\n";
	}
	code += "			";
	code += "   vec3 velocity_from_point_dir = TRANSFORM[3].xyz - direction_origin;\n";
	code += "   if (length(velocity_from_point_dir) < 0.01) {velocity_from_point_dir = VELOCITY;\n} else {velocity_from_point_dir = normalize(velocity_from_point_dir);}\n";
	code += "   VELOCITY = normalize(mix(VELOCITY, velocity_from_point_dir, direction_origin_influence)) * initial_velocity_magnitude;\n";
	code += "	}\n";
	code += "	TRANSFORM = EMISSION_TRANSFORM * TRANSFORM;\n";
	if (particle_flags[PARTICLE_FLAG_DISABLE_Z]) {
		code += "	VELOCITY.z = 0.0;\n";
		code += "	TRANSFORM[3].z = 0.0;\n";
	}
	code += "	}\n";
	code += "}\n\n";

	code += "void process() {\n";
	code += "	uint base_number = NUMBER;\n";
	code += "	uint alt_seed = hash(base_number + uint(1) + RANDOM_SEED);\n";
	code += "	float angle_rand = rand_from_seed(alt_seed);\n";
	code += "	float scale_rand = rand_from_seed(alt_seed);\n";
	code += "	float hue_rot_rand = rand_from_seed(alt_seed);\n";
	code += "	float anim_offset_rand = rand_from_seed(alt_seed);\n";
	code += "   float initial_velocity_magnitude = mix(initial_linear_velocity_min, initial_linear_velocity_max,rand_from_seed(alt_seed));\n";
	if (color_initial_ramp.is_valid()) {
		code += "	float color_initial_rand = rand_from_seed(alt_seed);\n";
	}
	code += "	float pi = 3.14159;\n";
	code += "	float degree_to_rad = pi / 180.0;\n";
	code += "\n";

	if (emission_shape == EMISSION_SHAPE_POINTS || emission_shape == EMISSION_SHAPE_DIRECTED_POINTS) {
		code += "	int point = min(emission_texture_point_count - 1, int(rand_from_seed(alt_seed) * float(emission_texture_point_count)));\n";
		code += "	ivec2 emission_tex_size = textureSize(emission_texture_points, 0);\n";
		code += "	ivec2 emission_tex_ofs = ivec2(point % emission_tex_size.x, point / emission_tex_size.x);\n";
	}

	code += "	CUSTOM.y += DELTA / LIFETIME;\n";
	code += "	float tv = CUSTOM.y / CUSTOM.w;\n";
	if (tex_parameters[PARAM_INITIAL_LINEAR_VELOCITY].is_valid()) {
		code += "	float tex_linear_velocity = textureLod(linear_velocity_texture, vec2(tv, 0.0), 0.0).r;\n";
	} else {
		code += "	float tex_linear_velocity = 1.0;\n";
	}

	if (particle_flags[PARTICLE_FLAG_DISABLE_Z]) {
		if (tex_parameters[PARAM_ORBIT_VELOCITY].is_valid()) {
			code += "	float tex_orbit_velocity = textureLod(orbit_velocity_texture, vec2(tv, 0.0), 0.0).r;\n";
		} else {
			code += "	float tex_orbit_velocity = 1.0;\n";
		}
	}

	if (tex_parameters[PARAM_ANGULAR_VELOCITY].is_valid()) {
		code += "	float tex_angular_velocity = textureLod(angular_velocity_texture, vec2(tv, 0.0), 0.0).r;\n";
	} else {
		code += "	float tex_angular_velocity = 1.0;\n";
	}

	if (tex_parameters[PARAM_LINEAR_ACCEL].is_valid()) {
		code += "	float tex_linear_accel = textureLod(linear_accel_texture, vec2(tv, 0.0), 0.0).r;\n";
	} else {
		code += "	float tex_linear_accel = 1.0;\n";
	}

	if (tex_parameters[PARAM_RADIAL_ACCEL].is_valid()) {
		code += "	float tex_radial_accel = textureLod(radial_accel_texture, vec2(tv, 0.0), 0.0).r;\n";
	} else {
		code += "	float tex_radial_accel = 1.0;\n";
	}

	if (tex_parameters[PARAM_TANGENTIAL_ACCEL].is_valid()) {
		code += "	float tex_tangent_accel = textureLod(tangent_accel_texture, vec2(tv, 0.0), 0.0).r;\n";
	} else {
		code += "	float tex_tangent_accel = 1.0;\n";
	}

	if (tex_parameters[PARAM_DAMPING].is_valid()) {
		code += "	float tex_damping = textureLod(damping_texture, vec2(tv, 0.0), 0.0).r;\n";
	} else {
		code += "	float tex_damping = 1.0;\n";
	}

	if (tex_parameters[PARAM_ANGLE].is_valid()) {
		code += "	float tex_angle = textureLod(angle_texture, vec2(tv, 0.0), 0.0).r;\n";
	} else {
		code += "	float tex_angle = 1.0;\n";
	}

	if (tex_parameters[PARAM_ANIM_SPEED].is_valid()) {
		code += "	float tex_anim_speed = textureLod(anim_speed_texture, vec2(tv, 0.0), 0.0).r;\n";
	} else {
		code += "	float tex_anim_speed = 1.0;\n";
	}

	if (tex_parameters[PARAM_ANIM_OFFSET].is_valid()) {
		code += "	float tex_anim_offset = textureLod(anim_offset_texture, vec2(tv, 0.0), 0.0).r;\n";
	} else {
		code += "	float tex_anim_offset = 1.0;\n";
	}

	code += "	vec3 force = gravity;\n";
	code += "	vec3 pos = TRANSFORM[3].xyz;\n";
	if (particle_flags[PARTICLE_FLAG_DISABLE_Z]) {
		code += "	pos.z = 0.0;\n";
	}
	code += "	// apply linear acceleration\n";
	code += "	force += length(VELOCITY) > 0.0 ? normalize(VELOCITY) * tex_linear_accel * mix(linear_accel_min, linear_accel_max, rand_from_seed(alt_seed)) : vec3(0.0);\n";
	code += "	// apply radial acceleration\n";
	code += "	vec3 org = EMISSION_TRANSFORM[3].xyz;\n";
	code += "	vec3 diff = pos - org;\n";
	code += "	force += length(diff) > 0.0 ? normalize(diff) * tex_radial_accel * mix(radial_accel_min, radial_accel_max, rand_from_seed(alt_seed)) : vec3(0.0);\n";
	code += "	// apply tangential acceleration;\n";
	code += "	float tangent_accel_val = tex_tangent_accel * mix(tangent_accel_min, tangent_accel_max, rand_from_seed(alt_seed));\n";
	if (particle_flags[PARTICLE_FLAG_DISABLE_Z]) {
		code += "	force += length(diff.yx) > 0.0 ? vec3(normalize(diff.yx * vec2(-1.0, 1.0)), 0.0) * tangent_accel_val : vec3(0.0);\n";

	} else {
		code += "	vec3 crossDiff = cross(normalize(diff), normalize(gravity));\n";
		code += "	force += length(crossDiff) > 0.0 ? normalize(crossDiff) * tangent_accel_val : vec3(0.0);\n";
	}
	if (attractor_interaction_enabled) {
		code += "	force += ATTRACTOR_FORCE;\n\n";
	}

	code += "	// apply attractor forces\n";
	code += "	VELOCITY += force * DELTA;\n";

	// Apply noise/turbulence.
	if (turbulence_enabled) {
		code += "	// apply turbulence\n";
		if (tex_parameters[PARAM_TURB_INFLUENCE_OVER_LIFE].is_valid()) {
			code += "	float turbulence_influence = textureLod(turbulence_influence_over_life, vec2(tv, 0.0), 0.0).r;\n";
		} else {
			code += "	const float turbulence_influence = 1.0;\n";
		}
		code += "	\n";
		code += "	vec3 noise_direction = get_noise_direction(TRANSFORM[3].xyz);\n";
		// If collision happened, turbulence is no longer applied.
		// We don't need this check when the collision mode is "hide on contact",
		// as the particle will be hidden anyway.
		String extra_tab = "";
		if (collision_mode != COLLISION_RIGID) {
			code += "	if (!COLLIDED) {\n";
			extra_tab = "	";
		}
		code += extra_tab + "	\n";
		code += extra_tab + "	float vel_mag = length(VELOCITY);\n";
		code += extra_tab + "	float vel_infl = clamp(mix(turbulence_influence_min, turbulence_influence_max, rand_from_seed(alt_seed)) * turbulence_influence, 0.0, 1.0);\n";
		code += extra_tab + "	VELOCITY = mix(VELOCITY, normalize(noise_direction) * vel_mag * (1.0 + (1.0 - vel_infl) * 0.2), vel_infl);\n";
		if (collision_mode != COLLISION_RIGID) {
			code += "	}";
		}
	}
	code += "	\n";
	code += "	// orbit velocity\n";
	if (particle_flags[PARTICLE_FLAG_DISABLE_Z]) {
		code += "	float orbit_amount = tex_orbit_velocity * mix(orbit_velocity_min, orbit_velocity_max, rand_from_seed(alt_seed));\n";
		code += "	if (orbit_amount != 0.0) {\n";
		code += "	     float ang = orbit_amount * DELTA * pi * 2.0;\n";
		code += "	     mat2 rot = mat2(vec2(cos(ang), -sin(ang)), vec2(sin(ang), cos(ang)));\n";
		code += "	     TRANSFORM[3].xy -= diff.xy;\n";
		code += "	     TRANSFORM[3].xy += rot * diff.xy;\n";
		code += "	}\n";
	}

	code += "	float dmp = mix(damping_min, damping_max, rand_from_seed(alt_seed));\n";
	code += "	if (dmp * tex_damping > 0.0) {\n";
	code += "		float v = length(VELOCITY);\n";
	code += "		float damp = tex_damping * dmp;\n";
	code += "		v -= damp * DELTA;\n";
	code += "		if (v < 0.0) {\n";
	code += "			VELOCITY = vec3(0.0);\n";
	code += "		} else {\n";
	code += "			VELOCITY = normalize(VELOCITY) * v;\n";
	code += "		}\n";
	code += "	}\n";
	code += "	float base_angle = (tex_angle) * mix(initial_angle_min, initial_angle_max, angle_rand);\n";
	code += "	base_angle += CUSTOM.y * LIFETIME * (tex_angular_velocity) * mix(angular_velocity_min,angular_velocity_max, rand_from_seed(alt_seed));\n";
	code += "	CUSTOM.x = base_angle * degree_to_rad;\n"; // angle
	code += "	CUSTOM.z = (tex_anim_offset) * mix(anim_offset_min, anim_offset_max, rand_from_seed(alt_seed)) + tv * tex_anim_speed * mix(anim_speed_min, anim_speed_max, rand_from_seed(alt_seed));\n"; // angle

	// apply color
	// apply hue rotation
	if (tex_parameters[PARAM_SCALE].is_valid()) {
		code += "	vec3 tex_scale = textureLod(scale_texture, vec2(tv, 0.0), 0.0).rgb;\n";
	} else {
		code += "	vec3 tex_scale = vec3(1.0);\n";
	}

	if (tex_parameters[PARAM_HUE_VARIATION].is_valid()) {
		code += "	float tex_hue_variation = textureLod(hue_variation_texture, vec2(tv, 0.0), 0.0).r;\n";
	} else {
		code += "	float tex_hue_variation = 1.0;\n";
	}

	code += "	float hue_rot_angle = (tex_hue_variation) * pi * 2.0 * mix(hue_variation_min, hue_variation_max, rand_from_seed(alt_seed));\n";
	code += "	float hue_rot_c = cos(hue_rot_angle);\n";
	code += "	float hue_rot_s = sin(hue_rot_angle);\n";
	code += "	mat4 hue_rot_mat = mat4(vec4(0.299, 0.587, 0.114, 0.0),\n";
	code += "			vec4(0.299, 0.587, 0.114, 0.0),\n";
	code += "			vec4(0.299, 0.587, 0.114, 0.0),\n";
	code += "			vec4(0.000, 0.000, 0.000, 1.0)) +\n";
	code += "		mat4(vec4(0.701, -0.587, -0.114, 0.0),\n";
	code += "			vec4(-0.299, 0.413, -0.114, 0.0),\n";
	code += "			vec4(-0.300, -0.588, 0.886, 0.0),\n";
	code += "			vec4(0.000, 0.000, 0.000, 0.0)) * hue_rot_c +\n";
	code += "		mat4(vec4(0.168, 0.330, -0.497, 0.0),\n";
	code += "			vec4(-0.328, 0.035,  0.292, 0.0),\n";
	code += "			vec4(1.250, -1.050, -0.203, 0.0),\n";
	code += "			vec4(0.000, 0.000, 0.000, 0.0)) * hue_rot_s;\n";
	if (color_ramp.is_valid()) {
		code += "	COLOR = hue_rot_mat * textureLod(color_ramp, vec2(tv, 0.0), 0.0) * color_value;\n";
	} else {
		code += "	COLOR = hue_rot_mat * color_value;\n";
	}

	if (color_initial_ramp.is_valid()) {
		code += "	vec4 start_color = textureLod(color_initial_ramp, vec2(color_initial_rand, 0.0), 0.0);\n";
		code += "	COLOR *= start_color;\n";
	}
	if (split_color_curves) {
		code += "COLOR.a *= textureLod(alpha_ramp, vec2(tv, 0.0), 0.0).r;\n";
		code += "COLOR.rgb *= 1.0 + textureLod(emission_ramp, vec2(tv, 0.0), 0.0).r;\n";
	}

	if (emission_color_texture.is_valid() && (emission_shape == EMISSION_SHAPE_POINTS || emission_shape == EMISSION_SHAPE_DIRECTED_POINTS)) {
		code += "	COLOR *= texelFetch(emission_texture_color, emission_tex_ofs, 0);\n";
	}
	code += "\n";

	if (particle_flags[PARTICLE_FLAG_DISABLE_Z]) {
		if (particle_flags[PARTICLE_FLAG_ALIGN_Y_TO_VELOCITY]) {
			code += "	if (length(VELOCITY) > 0.0) {\n";
			code += "		TRANSFORM[1].xyz = normalize(VELOCITY);\n";
			code += "	} else {\n";
			code += "		TRANSFORM[1].xyz = normalize(TRANSFORM[1].xyz);\n";
			code += "	}\n";
			code += "	TRANSFORM[0].xyz = normalize(cross(TRANSFORM[1].xyz, TRANSFORM[2].xyz));\n";
			code += "	TRANSFORM[2] = vec4(0.0, 0.0, 1.0, 0.0);\n";
		} else {
			code += "	TRANSFORM[0] = vec4(cos(CUSTOM.x), -sin(CUSTOM.x), 0.0, 0.0);\n";
			code += "	TRANSFORM[1] = vec4(sin(CUSTOM.x), cos(CUSTOM.x), 0.0, 0.0);\n";
			code += "	TRANSFORM[2] = vec4(0.0, 0.0, 1.0, 0.0);\n";
		}

	} else {
		// orient particle Y towards velocity
		if (particle_flags[PARTICLE_FLAG_ALIGN_Y_TO_VELOCITY]) {
			code += "	if (length(VELOCITY) > 0.0) {\n";
			code += "		TRANSFORM[1].xyz = normalize(VELOCITY);\n";
			code += "	} else {\n";
			code += "		TRANSFORM[1].xyz = normalize(TRANSFORM[1].xyz);\n";
			code += "	}\n";
			code += "	if (TRANSFORM[1].xyz == normalize(TRANSFORM[0].xyz)) {\n";
			code += "		TRANSFORM[0].xyz = normalize(cross(normalize(TRANSFORM[1].xyz), normalize(TRANSFORM[2].xyz)));\n";
			code += "		TRANSFORM[2].xyz = normalize(cross(normalize(TRANSFORM[0].xyz), normalize(TRANSFORM[1].xyz)));\n";
			code += "	} else {\n";
			code += "		TRANSFORM[2].xyz = normalize(cross(normalize(TRANSFORM[0].xyz), normalize(TRANSFORM[1].xyz)));\n";
			code += "		TRANSFORM[0].xyz = normalize(cross(normalize(TRANSFORM[1].xyz), normalize(TRANSFORM[2].xyz)));\n";
			code += "	}\n";
		} else {
			code += "	TRANSFORM[0].xyz = normalize(TRANSFORM[0].xyz);\n";
			code += "	TRANSFORM[1].xyz = normalize(TRANSFORM[1].xyz);\n";
			code += "	TRANSFORM[2].xyz = normalize(TRANSFORM[2].xyz);\n";
		}
		// turn particle by rotation in Y
		if (particle_flags[PARTICLE_FLAG_ROTATE_Y]) {
			code += "	vec4 origin = TRANSFORM[3];\n";
			code += "	TRANSFORM = mat4(vec4(cos(CUSTOM.x), 0.0, -sin(CUSTOM.x), 0.0), vec4(0.0, 1.0, 0.0, 0.0), vec4(sin(CUSTOM.x), 0.0, cos(CUSTOM.x), 0.0), vec4(0.0, 0.0, 0.0, 1.0));\n";
			code += "	TRANSFORM[3] = origin;\n";
		}
	}

	if (particle_flags[PARTICLE_FLAG_DISABLE_Z]) {
		code += "	TRANSFORM[3].z = 0.0;\n";
	}

	if (tex_parameters[PARAM_INITIAL_LINEAR_VELOCITY].is_valid()) {
		code += "	VELOCITY = normalize(VELOCITY) * tex_linear_velocity * initial_velocity_magnitude;\n";
	}
	// scale by scale
	code += "	float base_scale = mix(scale_min, scale_max, scale_rand);\n";
	code += "	base_scale = sign(base_scale) * max(abs(base_scale), 0.001);\n";
	code += "	TRANSFORM[0].xyz *= base_scale * sign(tex_scale.r) * max(abs(tex_scale.r), 0.001);\n";
	code += "	TRANSFORM[1].xyz *= base_scale * sign(tex_scale.g) * max(abs(tex_scale.g), 0.001);\n";
	code += "	TRANSFORM[2].xyz *= base_scale * sign(tex_scale.b) * max(abs(tex_scale.b), 0.001);\n";

	if (collision_mode == COLLISION_RIGID) {
		code += "	if (COLLIDED) {\n";
		code += "		if (length(VELOCITY) > 3.0) {\n";
		code += "			TRANSFORM[3].xyz += COLLISION_NORMAL * COLLISION_DEPTH;\n";
		code += "			VELOCITY -= COLLISION_NORMAL * dot(COLLISION_NORMAL, VELOCITY) * (1.0 + collision_bounce);\n";
		code += "			VELOCITY = mix(VELOCITY,vec3(0.0),clamp(collision_friction, 0.0, 1.0));\n";
		code += "		} else {\n";
		code += "			VELOCITY = vec3(0.0);\n";
		// If turbulence is enabled, set the noise direction to up so the turbulence color is "neutral"
		if (turbulence_enabled) {
			code += "			noise_direction = vec3(1.0, 0.0, 0.0);\n";
		}
		code += "		}\n";
		code += "	}\n";
	} else if (collision_mode == COLLISION_HIDE_ON_CONTACT) {
		code += "	if (COLLIDED) {\n";
		code += "		ACTIVE = false;\n";
		code += "	}\n";
	}

	if (sub_emitter_mode != SUB_EMITTER_DISABLED && !RenderingServer::get_singleton()->is_low_end()) {
		code += "	int emit_count = 0;\n";
		switch (sub_emitter_mode) {
			case SUB_EMITTER_CONSTANT: {
				code += "	float interval_from = CUSTOM.y * LIFETIME - DELTA;\n";
				code += "	float interval_rem = sub_emitter_frequency - mod(interval_from,sub_emitter_frequency);\n";
				code += "	if (DELTA >= interval_rem) emit_count = 1;\n";
			} break;
			case SUB_EMITTER_AT_COLLISION: {
				code += "	if (COLLIDED) emit_count = sub_emitter_amount_at_collision;\n";
			} break;
			case SUB_EMITTER_AT_END: {
				code += "	float unit_delta = DELTA/LIFETIME;\n";
				code += "	float end_time = CUSTOM.w * 0.95;\n"; // if we do at the end we might miss it, as it can just get deactivated by emitter
				code += "	if (CUSTOM.y < end_time && (CUSTOM.y + unit_delta) >= end_time) emit_count = sub_emitter_amount_at_end;\n";
			} break;
			default: {
			}
		}
		code += "	for(int i=0;i<emit_count;i++) {\n";
		code += "		uint flags = FLAG_EMIT_POSITION|FLAG_EMIT_ROT_SCALE;\n";
		code += "		if (sub_emitter_keep_velocity) flags|=FLAG_EMIT_VELOCITY;\n";
		code += "		emit_subparticle(TRANSFORM,VELOCITY,vec4(0.0),vec4(0.0),flags);\n";
		code += "	}";
	}

	code += "	if (CUSTOM.y > CUSTOM.w) {\n";
	code += "		ACTIVE = false;\n";
	code += "	}\n";
	code += "}\n";
	code += "\n";

	ShaderData shader_data;
	shader_data.shader = RS::get_singleton()->shader_create();
	shader_data.users = 1;

	RS::get_singleton()->shader_set_code(shader_data.shader, code);

	shader_map[mk] = shader_data;

	RS::get_singleton()->material_set_shader(_get_material(), shader_data.shader);
}

void ParticleProcessMaterial::flush_changes() {
	MutexLock lock(material_mutex);

	while (dirty_materials->first()) {
		dirty_materials->first()->self()->_update_shader();
	}
}

void ParticleProcessMaterial::_queue_shader_change() {
	MutexLock lock(material_mutex);

	if (_is_initialized() && !element.in_list()) {
		dirty_materials->add(&element);
	}
}

bool ParticleProcessMaterial::_is_shader_dirty() const {
	MutexLock lock(material_mutex);

	return element.in_list();
}

void ParticleProcessMaterial::set_direction(Vector3 p_direction) {
	direction = p_direction;
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->direction, direction);
}

Vector3 ParticleProcessMaterial::get_direction() const {
	return direction;
}

void ParticleProcessMaterial::set_spread(float p_spread) {
	spread = p_spread;
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->spread, p_spread);
}

float ParticleProcessMaterial::get_spread() const {
	return spread;
}

void ParticleProcessMaterial::set_flatness(float p_flatness) {
	flatness = p_flatness;
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->flatness, p_flatness);
}

float ParticleProcessMaterial::get_flatness() const {
	return flatness;
}

void ParticleProcessMaterial::set_direction_origin(Vector3 p_direction_origin) {
	direction_origin = p_direction_origin;
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->direction_origin, p_direction_origin);
}
Vector3 ParticleProcessMaterial::get_direction_origin() const {
	return direction_origin;
}

void ParticleProcessMaterial::set_direction_origin_influence(float p_direction_origin_influence) {
	direction_origin_influence = p_direction_origin_influence;
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->direction_origin_influence, p_direction_origin_influence);
}
float ParticleProcessMaterial::get_direction_origin_influence() const {
	return direction_origin_influence;
}

void ParticleProcessMaterial::set_param_min(Parameter p_param, float p_value) {
	ERR_FAIL_INDEX(p_param, PARAM_MAX);

	params_min[p_param] = p_value;
	if (params_min[p_param] > params_max[p_param]) {
		set_param_max(p_param, p_value);
	}

	switch (p_param) {
		case PARAM_INITIAL_LINEAR_VELOCITY: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->initial_linear_velocity_min, p_value);
		} break;
		case PARAM_ANGULAR_VELOCITY: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->angular_velocity_min, p_value);
		} break;
		case PARAM_ORBIT_VELOCITY: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->orbit_velocity_min, p_value);
		} break;
		case PARAM_LINEAR_ACCEL: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->linear_accel_min, p_value);
		} break;
		case PARAM_RADIAL_ACCEL: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->radial_accel_min, p_value);
		} break;
		case PARAM_TANGENTIAL_ACCEL: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->tangent_accel_min, p_value);
		} break;
		case PARAM_DAMPING: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->damping_min, p_value);
		} break;
		case PARAM_ANGLE: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->initial_angle_min, p_value);
		} break;
		case PARAM_SCALE: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->scale_min, p_value);
		} break;
		case PARAM_HUE_VARIATION: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->hue_variation_min, p_value);
		} break;
		case PARAM_ANIM_SPEED: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->anim_speed_min, p_value);
		} break;
		case PARAM_ANIM_OFFSET: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->anim_offset_min, p_value);
		} break;
		case PARAM_TURB_VEL_INFLUENCE: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->turbulence_influence_min, p_value);
		} break;
		case PARAM_TURB_INIT_DISPLACEMENT: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->turbulence_initial_displacement_min, p_value);
		} break;
		case PARAM_TURB_INFLUENCE_OVER_LIFE: {
			// Can't happen, but silences warning
		} break;
		case PARAM_MAX:
			break; // Can't happen, but silences warning
	}
}

float ParticleProcessMaterial::get_param_min(Parameter p_param) const {
	ERR_FAIL_INDEX_V(p_param, PARAM_MAX, 0);

	return params_min[p_param];
}

void ParticleProcessMaterial::set_param_max(Parameter p_param, float p_value) {
	ERR_FAIL_INDEX(p_param, PARAM_MAX);

	params_max[p_param] = p_value;
	if (params_min[p_param] > params_max[p_param]) {
		set_param_min(p_param, p_value);
	}

	switch (p_param) {
		case PARAM_INITIAL_LINEAR_VELOCITY: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->initial_linear_velocity_max, p_value);
		} break;
		case PARAM_ANGULAR_VELOCITY: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->angular_velocity_max, p_value);
		} break;
		case PARAM_ORBIT_VELOCITY: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->orbit_velocity_max, p_value);
		} break;
		case PARAM_LINEAR_ACCEL: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->linear_accel_max, p_value);
		} break;
		case PARAM_RADIAL_ACCEL: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->radial_accel_max, p_value);
		} break;
		case PARAM_TANGENTIAL_ACCEL: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->tangent_accel_max, p_value);
		} break;
		case PARAM_DAMPING: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->damping_max, p_value);
		} break;
		case PARAM_ANGLE: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->initial_angle_max, p_value);
		} break;
		case PARAM_SCALE: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->scale_max, p_value);
		} break;
		case PARAM_HUE_VARIATION: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->hue_variation_max, p_value);
		} break;
		case PARAM_ANIM_SPEED: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->anim_speed_max, p_value);
		} break;
		case PARAM_ANIM_OFFSET: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->anim_offset_max, p_value);
		} break;
		case PARAM_TURB_VEL_INFLUENCE: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->turbulence_influence_max, p_value);
		} break;
		case PARAM_TURB_INIT_DISPLACEMENT: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->turbulence_initial_displacement_max, p_value);
		} break;
		case PARAM_TURB_INFLUENCE_OVER_LIFE: {
			// Can't happen, but silences warning
		} break;
		case PARAM_MAX:
			break; // Can't happen, but silences warning
	}
}

float ParticleProcessMaterial::get_param_max(Parameter p_param) const {
	ERR_FAIL_INDEX_V(p_param, PARAM_MAX, 0);

	return params_max[p_param];
}

static void _adjust_curve_range(const Ref<Texture2D> &p_texture, float p_min, float p_max) {
	Ref<CurveTexture> curve_tex = p_texture;
	if (!curve_tex.is_valid()) {
		return;
	}

	curve_tex->ensure_default_setup(p_min, p_max);
}

void ParticleProcessMaterial::set_param_texture(Parameter p_param, const Ref<Texture2D> &p_texture) {
	ERR_FAIL_INDEX(p_param, PARAM_MAX);

	tex_parameters[p_param] = p_texture;

	RID tex_rid = p_texture.is_valid() ? p_texture->get_rid() : RID();

	switch (p_param) {
		case PARAM_INITIAL_LINEAR_VELOCITY: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->linear_velocity_texture, tex_rid);
			_adjust_curve_range(p_texture, -1, 1);
		} break;
		case PARAM_ANGULAR_VELOCITY: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->angular_velocity_texture, tex_rid);
			_adjust_curve_range(p_texture, -360, 360);
		} break;
		case PARAM_ORBIT_VELOCITY: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->orbit_velocity_texture, tex_rid);
			_adjust_curve_range(p_texture, -500, 500);
		} break;
		case PARAM_LINEAR_ACCEL: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->linear_accel_texture, tex_rid);
			_adjust_curve_range(p_texture, -200, 200);
		} break;
		case PARAM_RADIAL_ACCEL: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->radial_accel_texture, tex_rid);
			_adjust_curve_range(p_texture, -200, 200);
		} break;
		case PARAM_TANGENTIAL_ACCEL: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->tangent_accel_texture, tex_rid);
			_adjust_curve_range(p_texture, -200, 200);
		} break;
		case PARAM_DAMPING: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->damping_texture, tex_rid);
			_adjust_curve_range(p_texture, 0, 100);
		} break;
		case PARAM_ANGLE: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->angle_texture, tex_rid);
			_adjust_curve_range(p_texture, -360, 360);
		} break;
		case PARAM_SCALE: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->scale_texture, tex_rid);
			_adjust_curve_range(p_texture, 0, 1);
		} break;
		case PARAM_HUE_VARIATION: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->hue_variation_texture, tex_rid);
			_adjust_curve_range(p_texture, -1, 1);
		} break;
		case PARAM_ANIM_SPEED: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->anim_speed_texture, tex_rid);
			_adjust_curve_range(p_texture, 0, 200);
		} break;
		case PARAM_ANIM_OFFSET: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->anim_offset_texture, tex_rid);
		} break;
		case PARAM_TURB_INFLUENCE_OVER_LIFE: {
			RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->turbulence_influence_over_life, tex_rid);
			_adjust_curve_range(p_texture, 0, 1);
		} break;
		case PARAM_TURB_VEL_INFLUENCE: {
			// Can't happen, but silences warning
		} break;
		case PARAM_TURB_INIT_DISPLACEMENT: {
			// Can't happen, but silences warning
		} break;
		case PARAM_MAX:
			break; // Can't happen, but silences warning
	}

	_queue_shader_change();
}

Ref<Texture2D> ParticleProcessMaterial::get_param_texture(Parameter p_param) const {
	ERR_FAIL_INDEX_V(p_param, PARAM_MAX, Ref<Texture2D>());

	return tex_parameters[p_param];
}

void ParticleProcessMaterial::set_color(const Color &p_color) {
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->color, p_color);
	color = p_color;
}

Color ParticleProcessMaterial::get_color() const {
	return color;
}

void ParticleProcessMaterial::set_color_ramp(const Ref<Texture2D> &p_texture) {
	color_ramp = p_texture;
	RID tex_rid = p_texture.is_valid() ? p_texture->get_rid() : RID();
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->color_ramp, tex_rid);
	_queue_shader_change();
	notify_property_list_changed();
}

Ref<Texture2D> ParticleProcessMaterial::get_color_ramp() const {
	return color_ramp;
}

void ParticleProcessMaterial::set_split_color_enabled(bool p_color_split_enabled) {
	split_color_curves = p_color_split_enabled;
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->split_color_curves, split_color_curves);
	_queue_shader_change();
	notify_property_list_changed();
}

bool ParticleProcessMaterial::is_split_color_enabled() const {
	return split_color_curves;
}

void ParticleProcessMaterial::set_alpha_ramp(const Ref<Texture2D> &p_texture) {
	alpha_ramp = p_texture;
	RID tex_rid = p_texture.is_valid() ? p_texture->get_rid() : RID();
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->alpha_ramp, tex_rid);
	_queue_shader_change();
	notify_property_list_changed();
}
Ref<Texture2D> ParticleProcessMaterial::get_alpha_ramp() const {
	return alpha_ramp;
}

void ParticleProcessMaterial::set_emission_ramp(const Ref<Texture2D> &p_texture) {
	emission_ramp = p_texture;
	RID tex_rid = p_texture.is_valid() ? p_texture->get_rid() : RID();
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->emission_ramp, tex_rid);
	_queue_shader_change();
	notify_property_list_changed();
}

Ref<Texture2D> ParticleProcessMaterial::get_emission_ramp() const {
	return emission_ramp;
}

void ParticleProcessMaterial::set_color_initial_ramp(const Ref<Texture2D> &p_texture) {
	color_initial_ramp = p_texture;
	RID tex_rid = p_texture.is_valid() ? p_texture->get_rid() : RID();
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->color_initial_ramp, tex_rid);
	_queue_shader_change();
	notify_property_list_changed();
}

Ref<Texture2D> ParticleProcessMaterial::get_color_initial_ramp() const {
	return color_initial_ramp;
}

void ParticleProcessMaterial::set_particle_flag(ParticleFlags p_particle_flag, bool p_enable) {
	ERR_FAIL_INDEX(p_particle_flag, PARTICLE_FLAG_MAX);
	particle_flags[p_particle_flag] = p_enable;
	_queue_shader_change();
	if (p_particle_flag == PARTICLE_FLAG_DISABLE_Z) {
		notify_property_list_changed();
	}
}

bool ParticleProcessMaterial::get_particle_flag(ParticleFlags p_particle_flag) const {
	ERR_FAIL_INDEX_V(p_particle_flag, PARTICLE_FLAG_MAX, false);
	return particle_flags[p_particle_flag];
}

void ParticleProcessMaterial::set_emission_shape(EmissionShape p_shape) {
	ERR_FAIL_INDEX(p_shape, EMISSION_SHAPE_MAX);
	emission_shape = p_shape;
	notify_property_list_changed();
	_queue_shader_change();
}

void ParticleProcessMaterial::set_emission_sphere_radius(real_t p_radius) {
	emission_sphere_radius = p_radius;
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->emission_sphere_radius, p_radius);
}

void ParticleProcessMaterial::set_emission_box_extents(Vector3 p_extents) {
	emission_box_extents = p_extents;
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->emission_box_extents, p_extents);
}

void ParticleProcessMaterial::set_emission_point_texture(const Ref<Texture2D> &p_points) {
	emission_point_texture = p_points;
	RID tex_rid = p_points.is_valid() ? p_points->get_rid() : RID();
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->emission_texture_points, tex_rid);
}

void ParticleProcessMaterial::set_emission_normal_texture(const Ref<Texture2D> &p_normals) {
	emission_normal_texture = p_normals;
	RID tex_rid = p_normals.is_valid() ? p_normals->get_rid() : RID();
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->emission_texture_normal, tex_rid);
}

void ParticleProcessMaterial::set_emission_color_texture(const Ref<Texture2D> &p_colors) {
	emission_color_texture = p_colors;
	RID tex_rid = p_colors.is_valid() ? p_colors->get_rid() : RID();
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->emission_texture_color, tex_rid);
	_queue_shader_change();
}

void ParticleProcessMaterial::set_emission_point_count(int p_count) {
	emission_point_count = p_count;
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->emission_texture_point_count, p_count);
}

void ParticleProcessMaterial::set_emission_ring_axis(Vector3 p_axis) {
	emission_ring_axis = p_axis;
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->emission_ring_axis, p_axis);
}

void ParticleProcessMaterial::set_emission_ring_height(real_t p_height) {
	emission_ring_height = p_height;
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->emission_ring_height, p_height);
}

void ParticleProcessMaterial::set_emission_ring_radius(real_t p_radius) {
	emission_ring_radius = p_radius;
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->emission_ring_radius, p_radius);
}

void ParticleProcessMaterial::set_emission_ring_inner_radius(real_t p_radius) {
	emission_ring_inner_radius = p_radius;
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->emission_ring_inner_radius, p_radius);
}

ParticleProcessMaterial::EmissionShape ParticleProcessMaterial::get_emission_shape() const {
	return emission_shape;
}

real_t ParticleProcessMaterial::get_emission_sphere_radius() const {
	return emission_sphere_radius;
}

Vector3 ParticleProcessMaterial::get_emission_box_extents() const {
	return emission_box_extents;
}

Ref<Texture2D> ParticleProcessMaterial::get_emission_point_texture() const {
	return emission_point_texture;
}

Ref<Texture2D> ParticleProcessMaterial::get_emission_normal_texture() const {
	return emission_normal_texture;
}

Ref<Texture2D> ParticleProcessMaterial::get_emission_color_texture() const {
	return emission_color_texture;
}

int ParticleProcessMaterial::get_emission_point_count() const {
	return emission_point_count;
}

Vector3 ParticleProcessMaterial::get_emission_ring_axis() const {
	return emission_ring_axis;
}

real_t ParticleProcessMaterial::get_emission_ring_height() const {
	return emission_ring_height;
}

real_t ParticleProcessMaterial::get_emission_ring_radius() const {
	return emission_ring_radius;
}

real_t ParticleProcessMaterial::get_emission_ring_inner_radius() const {
	return emission_ring_inner_radius;
}

void ParticleProcessMaterial::set_turbulence_enabled(const bool p_turbulence_enabled) {
	turbulence_enabled = p_turbulence_enabled;
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->turbulence_enabled, turbulence_enabled);
	_queue_shader_change();
	notify_property_list_changed();
}

bool ParticleProcessMaterial::get_turbulence_enabled() const {
	return turbulence_enabled;
}

void ParticleProcessMaterial::set_turbulence_noise_strength(float p_turbulence_noise_strength) {
	turbulence_noise_strength = p_turbulence_noise_strength;
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->turbulence_noise_strength, p_turbulence_noise_strength);
}

float ParticleProcessMaterial::get_turbulence_noise_strength() const {
	return turbulence_noise_strength;
}

void ParticleProcessMaterial::set_turbulence_noise_scale(float p_turbulence_noise_scale) {
	turbulence_noise_scale = p_turbulence_noise_scale;
	const float noise_frequency_when_slider_is_zero = 4.0;
	const float max_slider_value = 10.0;
	const float curve_exponent = 0.25;
	const float curve_rescale = noise_frequency_when_slider_is_zero / pow(max_slider_value, curve_exponent);
	float shader_turbulence_noise_scale = pow(p_turbulence_noise_scale, curve_exponent) * curve_rescale - noise_frequency_when_slider_is_zero;
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->turbulence_noise_scale, shader_turbulence_noise_scale);
}

float ParticleProcessMaterial::get_turbulence_noise_scale() const {
	return turbulence_noise_scale;
}

void ParticleProcessMaterial::set_turbulence_noise_speed_random(float p_turbulence_noise_speed_random) {
	turbulence_noise_speed_random = p_turbulence_noise_speed_random;
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->turbulence_noise_speed_random, p_turbulence_noise_speed_random);
}

float ParticleProcessMaterial::get_turbulence_noise_speed_random() const {
	return turbulence_noise_speed_random;
}

void ParticleProcessMaterial::set_turbulence_noise_speed(const Vector3 &p_turbulence_noise_speed) {
	turbulence_noise_speed = p_turbulence_noise_speed;
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->turbulence_noise_speed, turbulence_noise_speed);
}

Vector3 ParticleProcessMaterial::get_turbulence_noise_speed() const {
	return turbulence_noise_speed;
}

void ParticleProcessMaterial::set_gravity(const Vector3 &p_gravity) {
	gravity = p_gravity;
	Vector3 gset = gravity;
	if (gset == Vector3()) {
		gset = Vector3(0, -0.000001, 0); //as gravity is used as upvector in some calculations
	}
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->gravity, gset);
}

Vector3 ParticleProcessMaterial::get_gravity() const {
	return gravity;
}

void ParticleProcessMaterial::set_lifetime_randomness(double p_lifetime) {
	lifetime_randomness = p_lifetime;
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->lifetime_randomness, lifetime_randomness);
}

double ParticleProcessMaterial::get_lifetime_randomness() const {
	return lifetime_randomness;
}

RID ParticleProcessMaterial::get_shader_rid() const {
	ERR_FAIL_COND_V(!shader_map.has(current_key), RID());
	return shader_map[current_key].shader;
}

void ParticleProcessMaterial::_validate_property(PropertyInfo &p_property) const {
	if (p_property.name == "emission_sphere_radius" && (emission_shape != EMISSION_SHAPE_SPHERE && emission_shape != EMISSION_SHAPE_SPHERE_SURFACE)) {
		p_property.usage = PROPERTY_USAGE_NONE;
	}

	if (p_property.name == "emission_box_extents" && emission_shape != EMISSION_SHAPE_BOX) {
		p_property.usage = PROPERTY_USAGE_NONE;
	}

	if ((p_property.name == "emission_point_texture" || p_property.name == "emission_color_texture") && (emission_shape != EMISSION_SHAPE_POINTS && emission_shape != EMISSION_SHAPE_DIRECTED_POINTS)) {
		p_property.usage = PROPERTY_USAGE_NONE;
	}

	if (p_property.name == "emission_normal_texture" && emission_shape != EMISSION_SHAPE_DIRECTED_POINTS) {
		p_property.usage = PROPERTY_USAGE_NONE;
	}

	if (p_property.name == "emission_point_count" && (emission_shape != EMISSION_SHAPE_POINTS && emission_shape != EMISSION_SHAPE_DIRECTED_POINTS)) {
		p_property.usage = PROPERTY_USAGE_NONE;
	}

	if (p_property.name.begins_with("emission_ring_") && emission_shape != EMISSION_SHAPE_RING) {
		p_property.usage = PROPERTY_USAGE_NONE;
	}

	if (p_property.name == "sub_emitter_frequency" && sub_emitter_mode != SUB_EMITTER_CONSTANT) {
		p_property.usage = PROPERTY_USAGE_NONE;
	}

	if (p_property.name == "sub_emitter_amount_at_end" && sub_emitter_mode != SUB_EMITTER_AT_END) {
		p_property.usage = PROPERTY_USAGE_NONE;
	}

	if (p_property.name == "sub_emitter_amount_at_collision" && sub_emitter_mode != SUB_EMITTER_AT_COLLISION) {
		p_property.usage = PROPERTY_USAGE_NONE;
	}

	if (p_property.name.begins_with("orbit_") && !particle_flags[PARTICLE_FLAG_DISABLE_Z]) {
		p_property.usage = PROPERTY_USAGE_NONE;
	}

	if (!turbulence_enabled) {
		if (p_property.name == "turbulence_noise_strength" ||
				p_property.name == "turbulence_noise_scale" ||
				p_property.name == "turbulence_noise_speed" ||
				p_property.name == "turbulence_noise_speed_random" ||
				p_property.name == "turbulence_influence_over_life" ||
				p_property.name == "turbulence_influence_min" ||
				p_property.name == "turbulence_influence_max" ||
				p_property.name == "turbulence_initial_displacement_min" ||
				p_property.name == "turbulence_initial_displacement_max") {
			p_property.usage = PROPERTY_USAGE_NO_EDITOR;
		}
	}

	if (p_property.name == "alpha_ramp" && !split_color_curves) {
		p_property.usage = PROPERTY_USAGE_NO_EDITOR;
	}
	if (p_property.name == "emission_ramp" && !split_color_curves) {
		p_property.usage = PROPERTY_USAGE_NO_EDITOR;
	}

	if (p_property.name == "collision_friction" && collision_mode != COLLISION_RIGID) {
		p_property.usage = PROPERTY_USAGE_NONE;
	}

	if (p_property.name == "collision_bounce" && collision_mode != COLLISION_RIGID) {
		p_property.usage = PROPERTY_USAGE_NONE;
	}
}

void ParticleProcessMaterial::set_sub_emitter_mode(SubEmitterMode p_sub_emitter_mode) {
	sub_emitter_mode = p_sub_emitter_mode;
	_queue_shader_change();
	notify_property_list_changed();
	if (sub_emitter_mode != SUB_EMITTER_DISABLED && RenderingServer::get_singleton()->is_low_end()) {
		WARN_PRINT_ONCE_ED("Sub-emitter modes other than SUB_EMITTER_DISABLED are not supported in the GL Compatibility rendering backend.");
	}
}

ParticleProcessMaterial::SubEmitterMode ParticleProcessMaterial::get_sub_emitter_mode() const {
	return sub_emitter_mode;
}

void ParticleProcessMaterial::set_sub_emitter_frequency(double p_frequency) {
	sub_emitter_frequency = p_frequency;
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->sub_emitter_frequency, 1.0 / p_frequency); //pass delta instead of frequency, since its easier to compute
}

double ParticleProcessMaterial::get_sub_emitter_frequency() const {
	return sub_emitter_frequency;
}

void ParticleProcessMaterial::set_sub_emitter_amount_at_end(int p_amount) {
	sub_emitter_amount_at_end = p_amount;
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->sub_emitter_amount_at_end, p_amount);
}

int ParticleProcessMaterial::get_sub_emitter_amount_at_end() const {
	return sub_emitter_amount_at_end;
}

void ParticleProcessMaterial::set_sub_emitter_amount_at_collision(int p_amount) {
	sub_emitter_amount_at_collision = p_amount;
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->sub_emitter_amount_at_collision, p_amount);
}

int ParticleProcessMaterial::get_sub_emitter_amount_at_collision() const {
	return sub_emitter_amount_at_collision;
}

void ParticleProcessMaterial::set_sub_emitter_keep_velocity(bool p_enable) {
	sub_emitter_keep_velocity = p_enable;
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->sub_emitter_keep_velocity, p_enable);
}
bool ParticleProcessMaterial::get_sub_emitter_keep_velocity() const {
	return sub_emitter_keep_velocity;
}

void ParticleProcessMaterial::set_attractor_interaction_enabled(bool p_enable) {
	attractor_interaction_enabled = p_enable;
	_queue_shader_change();
}

bool ParticleProcessMaterial::is_attractor_interaction_enabled() const {
	return attractor_interaction_enabled;
}

void ParticleProcessMaterial::set_collision_mode(CollisionMode p_collision_mode) {
	collision_mode = p_collision_mode;
	_queue_shader_change();
	notify_property_list_changed();
}

ParticleProcessMaterial::CollisionMode ParticleProcessMaterial::get_collision_mode() const {
	return collision_mode;
}

void ParticleProcessMaterial::set_collision_use_scale(bool p_scale) {
	collision_scale = p_scale;
	_queue_shader_change();
}

bool ParticleProcessMaterial::is_collision_using_scale() const {
	return collision_scale;
}

void ParticleProcessMaterial::set_collision_friction(float p_friction) {
	collision_friction = p_friction;
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->collision_friction, p_friction);
}

float ParticleProcessMaterial::get_collision_friction() const {
	return collision_friction;
}

void ParticleProcessMaterial::set_collision_bounce(float p_bounce) {
	collision_bounce = p_bounce;
	RenderingServer::get_singleton()->material_set_param(_get_material(), shader_names->collision_bounce, p_bounce);
}

float ParticleProcessMaterial::get_collision_bounce() const {
	return collision_bounce;
}

Shader::Mode ParticleProcessMaterial::get_shader_mode() const {
	return Shader::MODE_PARTICLES;
}

void ParticleProcessMaterial::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_direction", "degrees"), &ParticleProcessMaterial::set_direction);
	ClassDB::bind_method(D_METHOD("get_direction"), &ParticleProcessMaterial::get_direction);

	ClassDB::bind_method(D_METHOD("set_spread", "degrees"), &ParticleProcessMaterial::set_spread);
	ClassDB::bind_method(D_METHOD("get_spread"), &ParticleProcessMaterial::get_spread);

	ClassDB::bind_method(D_METHOD("set_flatness", "amount"), &ParticleProcessMaterial::set_flatness);
	ClassDB::bind_method(D_METHOD("get_flatness"), &ParticleProcessMaterial::get_flatness);

	ClassDB::bind_method(D_METHOD("set_direction_origin", "origin"), &ParticleProcessMaterial::set_direction_origin);
	ClassDB::bind_method(D_METHOD("get_direction_origin"), &ParticleProcessMaterial::get_direction_origin);

	ClassDB::bind_method(D_METHOD("set_direction_origin_influence", "influence"), &ParticleProcessMaterial::set_direction_origin_influence);
	ClassDB::bind_method(D_METHOD("get_direction_origin_influence"), &ParticleProcessMaterial::get_direction_origin_influence);

	ClassDB::bind_method(D_METHOD("set_param_min", "param", "value"), &ParticleProcessMaterial::set_param_min);
	ClassDB::bind_method(D_METHOD("get_param_min", "param"), &ParticleProcessMaterial::get_param_min);

	ClassDB::bind_method(D_METHOD("set_param_max", "param", "value"), &ParticleProcessMaterial::set_param_max);
	ClassDB::bind_method(D_METHOD("get_param_max", "param"), &ParticleProcessMaterial::get_param_max);

	ClassDB::bind_method(D_METHOD("set_param_texture", "param", "texture"), &ParticleProcessMaterial::set_param_texture);
	ClassDB::bind_method(D_METHOD("get_param_texture", "param"), &ParticleProcessMaterial::get_param_texture);

	ClassDB::bind_method(D_METHOD("set_color", "color"), &ParticleProcessMaterial::set_color);
	ClassDB::bind_method(D_METHOD("get_color"), &ParticleProcessMaterial::get_color);

	ClassDB::bind_method(D_METHOD("set_color_ramp", "ramp"), &ParticleProcessMaterial::set_color_ramp);
	ClassDB::bind_method(D_METHOD("get_color_ramp"), &ParticleProcessMaterial::get_color_ramp);

	ClassDB::bind_method(D_METHOD("set_alpha_ramp", "ramp"), &ParticleProcessMaterial::set_alpha_ramp);
	ClassDB::bind_method(D_METHOD("get_alpha_ramp"), &ParticleProcessMaterial::get_alpha_ramp);

	ClassDB::bind_method(D_METHOD("set_emission_ramp", "ramp"), &ParticleProcessMaterial::set_emission_ramp);
	ClassDB::bind_method(D_METHOD("get_emission_ramp"), &ParticleProcessMaterial::get_emission_ramp);

	ClassDB::bind_method(D_METHOD("set_split_color_enabled", "enabled"), &ParticleProcessMaterial::set_split_color_enabled);
	ClassDB::bind_method(D_METHOD("is_split_color_enabled"), &ParticleProcessMaterial::is_split_color_enabled);

	ClassDB::bind_method(D_METHOD("set_color_initial_ramp", "ramp"), &ParticleProcessMaterial::set_color_initial_ramp);
	ClassDB::bind_method(D_METHOD("get_color_initial_ramp"), &ParticleProcessMaterial::get_color_initial_ramp);

	ClassDB::bind_method(D_METHOD("set_particle_flag", "particle_flag", "enable"), &ParticleProcessMaterial::set_particle_flag);
	ClassDB::bind_method(D_METHOD("get_particle_flag", "particle_flag"), &ParticleProcessMaterial::get_particle_flag);

	ClassDB::bind_method(D_METHOD("set_emission_shape", "shape"), &ParticleProcessMaterial::set_emission_shape);
	ClassDB::bind_method(D_METHOD("get_emission_shape"), &ParticleProcessMaterial::get_emission_shape);

	ClassDB::bind_method(D_METHOD("set_emission_sphere_radius", "radius"), &ParticleProcessMaterial::set_emission_sphere_radius);
	ClassDB::bind_method(D_METHOD("get_emission_sphere_radius"), &ParticleProcessMaterial::get_emission_sphere_radius);

	ClassDB::bind_method(D_METHOD("set_emission_box_extents", "extents"), &ParticleProcessMaterial::set_emission_box_extents);
	ClassDB::bind_method(D_METHOD("get_emission_box_extents"), &ParticleProcessMaterial::get_emission_box_extents);

	ClassDB::bind_method(D_METHOD("set_emission_point_texture", "texture"), &ParticleProcessMaterial::set_emission_point_texture);
	ClassDB::bind_method(D_METHOD("get_emission_point_texture"), &ParticleProcessMaterial::get_emission_point_texture);

	ClassDB::bind_method(D_METHOD("set_emission_normal_texture", "texture"), &ParticleProcessMaterial::set_emission_normal_texture);
	ClassDB::bind_method(D_METHOD("get_emission_normal_texture"), &ParticleProcessMaterial::get_emission_normal_texture);

	ClassDB::bind_method(D_METHOD("set_emission_color_texture", "texture"), &ParticleProcessMaterial::set_emission_color_texture);
	ClassDB::bind_method(D_METHOD("get_emission_color_texture"), &ParticleProcessMaterial::get_emission_color_texture);

	ClassDB::bind_method(D_METHOD("set_emission_point_count", "point_count"), &ParticleProcessMaterial::set_emission_point_count);
	ClassDB::bind_method(D_METHOD("get_emission_point_count"), &ParticleProcessMaterial::get_emission_point_count);

	ClassDB::bind_method(D_METHOD("set_emission_ring_axis", "axis"), &ParticleProcessMaterial::set_emission_ring_axis);
	ClassDB::bind_method(D_METHOD("get_emission_ring_axis"), &ParticleProcessMaterial::get_emission_ring_axis);

	ClassDB::bind_method(D_METHOD("set_emission_ring_height", "height"), &ParticleProcessMaterial::set_emission_ring_height);
	ClassDB::bind_method(D_METHOD("get_emission_ring_height"), &ParticleProcessMaterial::get_emission_ring_height);

	ClassDB::bind_method(D_METHOD("set_emission_ring_radius", "radius"), &ParticleProcessMaterial::set_emission_ring_radius);
	ClassDB::bind_method(D_METHOD("get_emission_ring_radius"), &ParticleProcessMaterial::get_emission_ring_radius);

	ClassDB::bind_method(D_METHOD("set_emission_ring_inner_radius", "inner_radius"), &ParticleProcessMaterial::set_emission_ring_inner_radius);
	ClassDB::bind_method(D_METHOD("get_emission_ring_inner_radius"), &ParticleProcessMaterial::get_emission_ring_inner_radius);

	ClassDB::bind_method(D_METHOD("get_turbulence_enabled"), &ParticleProcessMaterial::get_turbulence_enabled);
	ClassDB::bind_method(D_METHOD("set_turbulence_enabled", "turbulence_enabled"), &ParticleProcessMaterial::set_turbulence_enabled);

	ClassDB::bind_method(D_METHOD("get_turbulence_noise_strength"), &ParticleProcessMaterial::get_turbulence_noise_strength);
	ClassDB::bind_method(D_METHOD("set_turbulence_noise_strength", "turbulence_noise_strength"), &ParticleProcessMaterial::set_turbulence_noise_strength);

	ClassDB::bind_method(D_METHOD("get_turbulence_noise_scale"), &ParticleProcessMaterial::get_turbulence_noise_scale);
	ClassDB::bind_method(D_METHOD("set_turbulence_noise_scale", "turbulence_noise_scale"), &ParticleProcessMaterial::set_turbulence_noise_scale);

	ClassDB::bind_method(D_METHOD("get_turbulence_noise_speed_random"), &ParticleProcessMaterial::get_turbulence_noise_speed_random);
	ClassDB::bind_method(D_METHOD("set_turbulence_noise_speed_random", "turbulence_noise_speed_random"), &ParticleProcessMaterial::set_turbulence_noise_speed_random);

	ClassDB::bind_method(D_METHOD("get_turbulence_noise_speed"), &ParticleProcessMaterial::get_turbulence_noise_speed);
	ClassDB::bind_method(D_METHOD("set_turbulence_noise_speed", "turbulence_noise_speed"), &ParticleProcessMaterial::set_turbulence_noise_speed);

	ClassDB::bind_method(D_METHOD("get_gravity"), &ParticleProcessMaterial::get_gravity);
	ClassDB::bind_method(D_METHOD("set_gravity", "accel_vec"), &ParticleProcessMaterial::set_gravity);

	ClassDB::bind_method(D_METHOD("set_lifetime_randomness", "randomness"), &ParticleProcessMaterial::set_lifetime_randomness);
	ClassDB::bind_method(D_METHOD("get_lifetime_randomness"), &ParticleProcessMaterial::get_lifetime_randomness);

	ClassDB::bind_method(D_METHOD("get_sub_emitter_mode"), &ParticleProcessMaterial::get_sub_emitter_mode);
	ClassDB::bind_method(D_METHOD("set_sub_emitter_mode", "mode"), &ParticleProcessMaterial::set_sub_emitter_mode);

	ClassDB::bind_method(D_METHOD("get_sub_emitter_frequency"), &ParticleProcessMaterial::get_sub_emitter_frequency);
	ClassDB::bind_method(D_METHOD("set_sub_emitter_frequency", "hz"), &ParticleProcessMaterial::set_sub_emitter_frequency);

	ClassDB::bind_method(D_METHOD("get_sub_emitter_amount_at_end"), &ParticleProcessMaterial::get_sub_emitter_amount_at_end);
	ClassDB::bind_method(D_METHOD("set_sub_emitter_amount_at_end", "amount"), &ParticleProcessMaterial::set_sub_emitter_amount_at_end);

	ClassDB::bind_method(D_METHOD("get_sub_emitter_amount_at_collision"), &ParticleProcessMaterial::get_sub_emitter_amount_at_collision);
	ClassDB::bind_method(D_METHOD("set_sub_emitter_amount_at_collision", "amount"), &ParticleProcessMaterial::set_sub_emitter_amount_at_collision);

	ClassDB::bind_method(D_METHOD("get_sub_emitter_keep_velocity"), &ParticleProcessMaterial::get_sub_emitter_keep_velocity);
	ClassDB::bind_method(D_METHOD("set_sub_emitter_keep_velocity", "enable"), &ParticleProcessMaterial::set_sub_emitter_keep_velocity);

	ClassDB::bind_method(D_METHOD("set_attractor_interaction_enabled", "enabled"), &ParticleProcessMaterial::set_attractor_interaction_enabled);
	ClassDB::bind_method(D_METHOD("is_attractor_interaction_enabled"), &ParticleProcessMaterial::is_attractor_interaction_enabled);

	ClassDB::bind_method(D_METHOD("set_collision_mode", "mode"), &ParticleProcessMaterial::set_collision_mode);
	ClassDB::bind_method(D_METHOD("get_collision_mode"), &ParticleProcessMaterial::get_collision_mode);

	ClassDB::bind_method(D_METHOD("set_collision_use_scale", "radius"), &ParticleProcessMaterial::set_collision_use_scale);
	ClassDB::bind_method(D_METHOD("is_collision_using_scale"), &ParticleProcessMaterial::is_collision_using_scale);

	ClassDB::bind_method(D_METHOD("set_collision_friction", "friction"), &ParticleProcessMaterial::set_collision_friction);
	ClassDB::bind_method(D_METHOD("get_collision_friction"), &ParticleProcessMaterial::get_collision_friction);

	ClassDB::bind_method(D_METHOD("set_collision_bounce", "bounce"), &ParticleProcessMaterial::set_collision_bounce);
	ClassDB::bind_method(D_METHOD("get_collision_bounce"), &ParticleProcessMaterial::get_collision_bounce);

	ADD_GROUP("Time", "");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "lifetime_randomness", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_lifetime_randomness", "get_lifetime_randomness");

	ADD_GROUP("Emission Shape", "emission_");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "emission_shape", PROPERTY_HINT_ENUM, "Point,Sphere,Sphere Surface,Box,Points,Directed Points,Ring"), "set_emission_shape", "get_emission_shape");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "emission_sphere_radius", PROPERTY_HINT_RANGE, "0.01,128,0.01,or_greater"), "set_emission_sphere_radius", "get_emission_sphere_radius");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "emission_box_extents"), "set_emission_box_extents", "get_emission_box_extents");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "emission_point_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_emission_point_texture", "get_emission_point_texture");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "emission_normal_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_emission_normal_texture", "get_emission_normal_texture");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "emission_color_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_emission_color_texture", "get_emission_color_texture");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "emission_point_count", PROPERTY_HINT_RANGE, "0,1000000,1"), "set_emission_point_count", "get_emission_point_count");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "emission_ring_axis"), "set_emission_ring_axis", "get_emission_ring_axis");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "emission_ring_height"), "set_emission_ring_height", "get_emission_ring_height");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "emission_ring_radius"), "set_emission_ring_radius", "get_emission_ring_radius");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "emission_ring_inner_radius"), "set_emission_ring_inner_radius", "get_emission_ring_inner_radius");
	ADD_GROUP("Particle Flags", "particle_flag_");
	ADD_PROPERTYI(PropertyInfo(Variant::BOOL, "particle_flag_align_y"), "set_particle_flag", "get_particle_flag", PARTICLE_FLAG_ALIGN_Y_TO_VELOCITY);
	ADD_PROPERTYI(PropertyInfo(Variant::BOOL, "particle_flag_rotate_y"), "set_particle_flag", "get_particle_flag", PARTICLE_FLAG_ROTATE_Y);
	ADD_PROPERTYI(PropertyInfo(Variant::BOOL, "particle_flag_disable_z"), "set_particle_flag", "get_particle_flag", PARTICLE_FLAG_DISABLE_Z);
	ADD_GROUP("Direction", "");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "direction"), "set_direction", "get_direction");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "spread", PROPERTY_HINT_RANGE, "0,180,0.01"), "set_spread", "get_spread");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "flatness", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_flatness", "get_flatness");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "direction_origin"), "set_direction_origin", "get_direction_origin");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "direction_origin_influence", PROPERTY_HINT_RANGE, "0,1,0.001"), "set_direction_origin_influence", "get_direction_origin_influence");
	ADD_GROUP("Gravity", "");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "gravity"), "set_gravity", "get_gravity");
	ADD_GROUP("Initial Velocity", "");
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "initial_velocity_min", PROPERTY_HINT_RANGE, "0,1000,0.01,or_less,or_greater"), "set_param_min", "get_param_min", PARAM_INITIAL_LINEAR_VELOCITY);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "initial_velocity_max", PROPERTY_HINT_RANGE, "0,1000,0.01,or_less,or_greater"), "set_param_max", "get_param_max", PARAM_INITIAL_LINEAR_VELOCITY);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "linear_velocity_curve", PROPERTY_HINT_RESOURCE_TYPE, "CurveTexture,CurveXYZTexture"), "set_param_texture", "get_param_texture", PARAM_INITIAL_LINEAR_VELOCITY);
	ADD_GROUP("Angular Velocity", "angular_");
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "angular_velocity_min", PROPERTY_HINT_RANGE, "-720,720,0.01,or_less,or_greater"), "set_param_min", "get_param_min", PARAM_ANGULAR_VELOCITY);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "angular_velocity_max", PROPERTY_HINT_RANGE, "-720,720,0.01,or_less,or_greater"), "set_param_max", "get_param_max", PARAM_ANGULAR_VELOCITY);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "angular_velocity_curve", PROPERTY_HINT_RESOURCE_TYPE, "CurveTexture"), "set_param_texture", "get_param_texture", PARAM_ANGULAR_VELOCITY);
	ADD_GROUP("Orbit Velocity", "orbit_");
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "orbit_velocity_min", PROPERTY_HINT_RANGE, "-1000,1000,0.01,or_less,or_greater"), "set_param_min", "get_param_min", PARAM_ORBIT_VELOCITY);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "orbit_velocity_max", PROPERTY_HINT_RANGE, "-1000,1000,0.01,or_less,or_greater"), "set_param_max", "get_param_max", PARAM_ORBIT_VELOCITY);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "orbit_velocity_curve", PROPERTY_HINT_RESOURCE_TYPE, "CurveTexture"), "set_param_texture", "get_param_texture", PARAM_ORBIT_VELOCITY);
	ADD_GROUP("Linear Accel", "linear_");
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "linear_accel_min", PROPERTY_HINT_RANGE, "-100,100,0.01,or_less,or_greater"), "set_param_min", "get_param_min", PARAM_LINEAR_ACCEL);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "linear_accel_max", PROPERTY_HINT_RANGE, "-100,100,0.01,or_less,or_greater"), "set_param_max", "get_param_max", PARAM_LINEAR_ACCEL);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "linear_accel_curve", PROPERTY_HINT_RESOURCE_TYPE, "CurveTexture"), "set_param_texture", "get_param_texture", PARAM_LINEAR_ACCEL);
	ADD_GROUP("Radial Accel", "radial_");
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "radial_accel_min", PROPERTY_HINT_RANGE, "-100,100,0.01,or_less,or_greater"), "set_param_min", "get_param_min", PARAM_RADIAL_ACCEL);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "radial_accel_max", PROPERTY_HINT_RANGE, "-100,100,0.01,or_less,or_greater"), "set_param_max", "get_param_max", PARAM_RADIAL_ACCEL);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "radial_accel_curve", PROPERTY_HINT_RESOURCE_TYPE, "CurveTexture"), "set_param_texture", "get_param_texture", PARAM_RADIAL_ACCEL);
	ADD_GROUP("Tangential Accel", "tangential_");
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "tangential_accel_min", PROPERTY_HINT_RANGE, "-100,100,0.01,or_less,or_greater"), "set_param_min", "get_param_min", PARAM_TANGENTIAL_ACCEL);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "tangential_accel_max", PROPERTY_HINT_RANGE, "-100,100,0.01,or_less,or_greater"), "set_param_max", "get_param_max", PARAM_TANGENTIAL_ACCEL);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "tangential_accel_curve", PROPERTY_HINT_RESOURCE_TYPE, "CurveTexture"), "set_param_texture", "get_param_texture", PARAM_TANGENTIAL_ACCEL);
	ADD_GROUP("Damping", "");
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "damping_min", PROPERTY_HINT_RANGE, "0,100,0.001,or_greater"), "set_param_min", "get_param_min", PARAM_DAMPING);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "damping_max", PROPERTY_HINT_RANGE, "0,100,0.001,or_greater"), "set_param_max", "get_param_max", PARAM_DAMPING);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "damping_curve", PROPERTY_HINT_RESOURCE_TYPE, "CurveTexture"), "set_param_texture", "get_param_texture", PARAM_DAMPING);
	ADD_GROUP("Angle", "");
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "angle_min", PROPERTY_HINT_RANGE, "-720,720,0.1,or_less,or_greater,degrees"), "set_param_min", "get_param_min", PARAM_ANGLE);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "angle_max", PROPERTY_HINT_RANGE, "-720,720,0.1,or_less,or_greater,degrees"), "set_param_max", "get_param_max", PARAM_ANGLE);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "angle_curve", PROPERTY_HINT_RESOURCE_TYPE, "CurveTexture"), "set_param_texture", "get_param_texture", PARAM_ANGLE);
	ADD_GROUP("Scale", "");
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "scale_min", PROPERTY_HINT_RANGE, "0,1000,0.01,or_greater"), "set_param_min", "get_param_min", PARAM_SCALE);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "scale_max", PROPERTY_HINT_RANGE, "0,1000,0.01,or_greater"), "set_param_max", "get_param_max", PARAM_SCALE);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "scale_curve", PROPERTY_HINT_RESOURCE_TYPE, "CurveTexture,CurveXYZTexture"), "set_param_texture", "get_param_texture", PARAM_SCALE);
	ADD_GROUP("Color", "");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color"), "set_color", "get_color");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "color_ramp", PROPERTY_HINT_RESOURCE_TYPE, "GradientTexture1D"), "set_color_ramp", "get_color_ramp");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "color_initial_ramp", PROPERTY_HINT_RESOURCE_TYPE, "GradientTexture1D"), "set_color_initial_ramp", "get_color_initial_ramp");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "split_color_curves"), "set_split_color_enabled", "is_split_color_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "alpha_ramp", PROPERTY_HINT_RESOURCE_TYPE, "CurveTexture"), "set_alpha_ramp", "get_alpha_ramp");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "emission_ramp", PROPERTY_HINT_RESOURCE_TYPE, "CurveTexture"), "set_emission_ramp", "get_emission_ramp");

	ADD_GROUP("Hue Variation", "hue_");
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "hue_variation_min", PROPERTY_HINT_RANGE, "-1,1,0.01"), "set_param_min", "get_param_min", PARAM_HUE_VARIATION);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "hue_variation_max", PROPERTY_HINT_RANGE, "-1,1,0.01"), "set_param_max", "get_param_max", PARAM_HUE_VARIATION);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "hue_variation_curve", PROPERTY_HINT_RESOURCE_TYPE, "CurveTexture"), "set_param_texture", "get_param_texture", PARAM_HUE_VARIATION);

	ADD_GROUP("Turbulence", "turbulence_");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "turbulence_enabled"), "set_turbulence_enabled", "get_turbulence_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "turbulence_noise_strength", PROPERTY_HINT_RANGE, "0,20,0.01"), "set_turbulence_noise_strength", "get_turbulence_noise_strength");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "turbulence_noise_scale", PROPERTY_HINT_RANGE, "0,10,0.01"), "set_turbulence_noise_scale", "get_turbulence_noise_scale");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "turbulence_noise_speed"), "set_turbulence_noise_speed", "get_turbulence_noise_speed");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "turbulence_noise_speed_random", PROPERTY_HINT_RANGE, "0,4,0.01"), "set_turbulence_noise_speed_random", "get_turbulence_noise_speed_random");
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "turbulence_influence_min", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_param_min", "get_param_min", PARAM_TURB_VEL_INFLUENCE);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "turbulence_influence_max", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_param_max", "get_param_max", PARAM_TURB_VEL_INFLUENCE);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "turbulence_initial_displacement_min", PROPERTY_HINT_RANGE, "-100,100,0.1"), "set_param_min", "get_param_min", PARAM_TURB_INIT_DISPLACEMENT);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "turbulence_initial_displacement_max", PROPERTY_HINT_RANGE, "-100,100,0.1"), "set_param_max", "get_param_max", PARAM_TURB_INIT_DISPLACEMENT);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "turbulence_influence_over_life", PROPERTY_HINT_RESOURCE_TYPE, "CurveTexture"), "set_param_texture", "get_param_texture", PARAM_TURB_INFLUENCE_OVER_LIFE);

	ADD_GROUP("Animation", "anim_");
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "anim_speed_min", PROPERTY_HINT_RANGE, "0,16,0.01,or_less,or_greater"), "set_param_min", "get_param_min", PARAM_ANIM_SPEED);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "anim_speed_max", PROPERTY_HINT_RANGE, "0,16,0.01,or_less,or_greater"), "set_param_max", "get_param_max", PARAM_ANIM_SPEED);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "anim_speed_curve", PROPERTY_HINT_RESOURCE_TYPE, "CurveTexture"), "set_param_texture", "get_param_texture", PARAM_ANIM_SPEED);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "anim_offset_min", PROPERTY_HINT_RANGE, "0,1,0.0001"), "set_param_min", "get_param_min", PARAM_ANIM_OFFSET);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "anim_offset_max", PROPERTY_HINT_RANGE, "0,1,0.0001"), "set_param_max", "get_param_max", PARAM_ANIM_OFFSET);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "anim_offset_curve", PROPERTY_HINT_RESOURCE_TYPE, "CurveTexture"), "set_param_texture", "get_param_texture", PARAM_ANIM_OFFSET);

	ADD_GROUP("Sub Emitter", "sub_emitter_");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "sub_emitter_mode", PROPERTY_HINT_ENUM, "Disabled,Constant,At End,At Collision"), "set_sub_emitter_mode", "get_sub_emitter_mode");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "sub_emitter_frequency", PROPERTY_HINT_RANGE, "0.01,100,0.01,suffix:Hz"), "set_sub_emitter_frequency", "get_sub_emitter_frequency");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "sub_emitter_amount_at_end", PROPERTY_HINT_RANGE, "1,32,1"), "set_sub_emitter_amount_at_end", "get_sub_emitter_amount_at_end");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "sub_emitter_amount_at_collision", PROPERTY_HINT_RANGE, "1,32,1"), "set_sub_emitter_amount_at_collision", "get_sub_emitter_amount_at_collision");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "sub_emitter_keep_velocity"), "set_sub_emitter_keep_velocity", "get_sub_emitter_keep_velocity");

	ADD_GROUP("Attractor Interaction", "attractor_interaction_");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "attractor_interaction_enabled"), "set_attractor_interaction_enabled", "is_attractor_interaction_enabled");
	ADD_GROUP("Collision", "collision_");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_mode", PROPERTY_HINT_ENUM, "Disabled,Rigid,Hide On Contact"), "set_collision_mode", "get_collision_mode");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "collision_friction", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_collision_friction", "get_collision_friction");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "collision_bounce", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_collision_bounce", "get_collision_bounce");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "collision_use_scale"), "set_collision_use_scale", "is_collision_using_scale");

	BIND_ENUM_CONSTANT(PARAM_INITIAL_LINEAR_VELOCITY);
	BIND_ENUM_CONSTANT(PARAM_ANGULAR_VELOCITY);
	BIND_ENUM_CONSTANT(PARAM_ORBIT_VELOCITY);
	BIND_ENUM_CONSTANT(PARAM_LINEAR_ACCEL);
	BIND_ENUM_CONSTANT(PARAM_RADIAL_ACCEL);
	BIND_ENUM_CONSTANT(PARAM_TANGENTIAL_ACCEL);
	BIND_ENUM_CONSTANT(PARAM_DAMPING);
	BIND_ENUM_CONSTANT(PARAM_ANGLE);
	BIND_ENUM_CONSTANT(PARAM_SCALE);
	BIND_ENUM_CONSTANT(PARAM_HUE_VARIATION);
	BIND_ENUM_CONSTANT(PARAM_ANIM_SPEED);
	BIND_ENUM_CONSTANT(PARAM_ANIM_OFFSET);
	BIND_ENUM_CONSTANT(PARAM_MAX);

	BIND_ENUM_CONSTANT(PARTICLE_FLAG_ALIGN_Y_TO_VELOCITY);
	BIND_ENUM_CONSTANT(PARTICLE_FLAG_ROTATE_Y);
	BIND_ENUM_CONSTANT(PARTICLE_FLAG_DISABLE_Z);
	BIND_ENUM_CONSTANT(PARTICLE_FLAG_MAX);

	BIND_ENUM_CONSTANT(EMISSION_SHAPE_POINT);
	BIND_ENUM_CONSTANT(EMISSION_SHAPE_SPHERE);
	BIND_ENUM_CONSTANT(EMISSION_SHAPE_SPHERE_SURFACE);
	BIND_ENUM_CONSTANT(EMISSION_SHAPE_BOX);
	BIND_ENUM_CONSTANT(EMISSION_SHAPE_POINTS);
	BIND_ENUM_CONSTANT(EMISSION_SHAPE_DIRECTED_POINTS);
	BIND_ENUM_CONSTANT(EMISSION_SHAPE_RING);
	BIND_ENUM_CONSTANT(EMISSION_SHAPE_MAX);

	BIND_ENUM_CONSTANT(PARAM_TURB_VEL_INFLUENCE);
	BIND_ENUM_CONSTANT(PARAM_TURB_INIT_DISPLACEMENT);
	BIND_ENUM_CONSTANT(PARAM_TURB_INFLUENCE_OVER_LIFE);

	BIND_ENUM_CONSTANT(SUB_EMITTER_DISABLED);
	BIND_ENUM_CONSTANT(SUB_EMITTER_CONSTANT);
	BIND_ENUM_CONSTANT(SUB_EMITTER_AT_END);
	BIND_ENUM_CONSTANT(SUB_EMITTER_AT_COLLISION);
	BIND_ENUM_CONSTANT(SUB_EMITTER_MAX);

	BIND_ENUM_CONSTANT(COLLISION_DISABLED);
	BIND_ENUM_CONSTANT(COLLISION_RIGID);
	BIND_ENUM_CONSTANT(COLLISION_HIDE_ON_CONTACT);
	BIND_ENUM_CONSTANT(COLLISION_MAX);
}

ParticleProcessMaterial::ParticleProcessMaterial() :
		element(this) {
	set_direction(Vector3(1, 0, 0));
	set_spread(45);
	set_flatness(0);
	set_param_min(PARAM_INITIAL_LINEAR_VELOCITY, 0);
	set_param_min(PARAM_ANGULAR_VELOCITY, 0);
	set_param_min(PARAM_ORBIT_VELOCITY, 0);
	set_param_min(PARAM_LINEAR_ACCEL, 0);
	set_param_min(PARAM_RADIAL_ACCEL, 0);
	set_param_min(PARAM_TANGENTIAL_ACCEL, 0);
	set_param_min(PARAM_DAMPING, 0);
	set_param_min(PARAM_ANGLE, 0);
	set_param_min(PARAM_SCALE, 1);
	set_param_min(PARAM_HUE_VARIATION, 0);
	set_param_min(PARAM_ANIM_SPEED, 0);
	set_param_min(PARAM_ANIM_OFFSET, 0);
	set_param_max(PARAM_INITIAL_LINEAR_VELOCITY, 0);
	set_param_max(PARAM_ANGULAR_VELOCITY, 0);
	set_param_max(PARAM_ORBIT_VELOCITY, 0);
	set_param_max(PARAM_LINEAR_ACCEL, 0);
	set_param_max(PARAM_RADIAL_ACCEL, 0);
	set_param_max(PARAM_TANGENTIAL_ACCEL, 0);
	set_param_max(PARAM_DAMPING, 0);
	set_param_max(PARAM_ANGLE, 0);
	set_param_max(PARAM_SCALE, 1);
	set_param_max(PARAM_HUE_VARIATION, 0);
	set_param_max(PARAM_ANIM_SPEED, 0);
	set_param_max(PARAM_ANIM_OFFSET, 0);
	set_emission_shape(EMISSION_SHAPE_POINT);
	set_emission_sphere_radius(1);
	set_emission_box_extents(Vector3(1, 1, 1));
	set_emission_ring_axis(Vector3(0, 0, 1.0));
	set_emission_ring_height(1);
	set_emission_ring_radius(1);
	set_emission_ring_inner_radius(0);
	set_split_color_enabled(false);

	set_turbulence_enabled(false);
	set_turbulence_noise_speed(Vector3(0.0, 0.0, 0.0));
	set_turbulence_noise_strength(1);
	set_turbulence_noise_scale(9);
	set_turbulence_noise_speed_random(0.2);
	set_param_min(PARAM_TURB_VEL_INFLUENCE, 0.1);
	set_param_max(PARAM_TURB_VEL_INFLUENCE, 0.1);
	set_param_min(PARAM_TURB_INIT_DISPLACEMENT, 0.0);
	set_param_max(PARAM_TURB_INIT_DISPLACEMENT, 0.0);

	set_gravity(Vector3(0, -9.8, 0));
	set_lifetime_randomness(0);

	set_sub_emitter_mode(SUB_EMITTER_DISABLED);
	set_sub_emitter_frequency(4);
	set_sub_emitter_amount_at_end(1);
	set_sub_emitter_amount_at_collision(1);
	set_sub_emitter_keep_velocity(false);

	set_attractor_interaction_enabled(true);
	set_collision_mode(COLLISION_DISABLED);
	set_collision_bounce(0.0);
	set_collision_friction(0.0);
	set_collision_use_scale(false);

	for (int i = 0; i < PARTICLE_FLAG_MAX; i++) {
		particle_flags[i] = false;
	}

	set_color(Color(1, 1, 1, 1));

	current_key.invalid_key = 1;

	_mark_initialized(callable_mp(this, &ParticleProcessMaterial::_queue_shader_change));
}

ParticleProcessMaterial::~ParticleProcessMaterial() {
	ERR_FAIL_NULL(RenderingServer::get_singleton());
	MutexLock lock(material_mutex);

	if (shader_map.has(current_key)) {
		shader_map[current_key].users--;
		if (shader_map[current_key].users == 0) {
			//deallocate shader, as it's no longer in use
			RS::get_singleton()->free(shader_map[current_key].shader);
			shader_map.erase(current_key);
		}

		RS::get_singleton()->material_set_shader(_get_material(), RID());
	}
}
