#[compute]

#version 450

VERSION_DEFINES

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

#define MAX_CASCADES 8

layout(set = 0, binding = 1) uniform texture3D sdf_cascades[MAX_CASCADES];
layout(set = 0, binding = 2) uniform texture3D light_cascades[MAX_CASCADES];
layout(set = 0, binding = 3) uniform texture3D aniso0_cascades[MAX_CASCADES];
layout(set = 0, binding = 4) uniform texture3D aniso1_cascades[MAX_CASCADES];

layout(set = 0, binding = 6) uniform sampler linear_sampler;

struct CascadeData {
	vec3 offset; //offset of (0,0,0) in world coordinates
	float to_cell; // 1/bounds * grid_size
	ivec3 probe_world_offset;
	uint pad;
};

layout(set = 0, binding = 7, std140) uniform Cascades {
	CascadeData data[MAX_CASCADES];
}
cascades;

layout(r32ui, set = 0, binding = 8) uniform restrict uimage2DArray lightprobe_texture_data;
layout(rgba16i, set = 0, binding = 9) uniform restrict iimage2DArray lightprobe_history_texture;
layout(rgba32i, set = 0, binding = 10) uniform restrict iimage2D lightprobe_average_texture;

//used for scrolling
layout(rgba16i, set = 0, binding = 11) uniform restrict iimage2DArray lightprobe_history_scroll_texture;
layout(rgba32i, set = 0, binding = 12) uniform restrict iimage2D lightprobe_average_scroll_texture;

layout(rgba32i, set = 0, binding = 13) uniform restrict iimage2D lightprobe_average_parent_texture;

layout(set = 1, binding = 0) uniform textureCube sky_irradiance;

layout(set = 1, binding = 1) uniform sampler linear_sampler_mipmaps;

#define HISTORY_BITS 10

#define SKY_MODE_DISABLED 0
#define SKY_MODE_COLOR 1
#define SKY_MODE_SKY 2

layout(push_constant, binding = 0, std430) uniform Params {
	vec3 grid_size;
	uint max_cascades;

	uint probe_axis_size;
	uint cascade;
	uint history_index;
	uint history_size;

	uint ray_count;
	float ray_bias;
	ivec2 image_size;

	ivec3 world_offset;
	uint sky_mode;

	ivec3 scroll;
	float sky_energy;

	vec3 sky_color;
	float y_mult;
}
params;

const float PI = 3.14159265f;
const float GOLDEN_ANGLE = PI * (3.0 - sqrt(5.0));

vec3 vogel_hemisphere(uint p_index, uint p_count, float p_offset) {
	float r = sqrt(float(p_index) + 0.5f) / sqrt(float(p_count));
	float theta = float(p_index) * GOLDEN_ANGLE + p_offset;
	float y = cos(r * PI * 0.5);
	float l = sin(r * PI * 0.5);
	return vec3(l * cos(theta), l * sin(theta), y * (float(p_index & 1) * 2.0 - 1.0));
}

uvec3 hash3(uvec3 x) {
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = (x >> 16) ^ x;
	return x;
}

float hashf3(vec3 co) {
	return fract(sin(dot(co, vec3(12.9898, 78.233, 137.13451))) * 43758.5453);
}

vec3 octahedron_encode(vec2 f) {
	// https://twitter.com/Stubbesaurus/status/937994790553227264
	f = f * 2.0 - 1.0;
	vec3 n = vec3(f.x, f.y, 1.0f - abs(f.x) - abs(f.y));
	float t = clamp(-n.z, 0.0, 1.0);
	n.x += n.x >= 0 ? -t : t;
	n.y += n.y >= 0 ? -t : t;
	return normalize(n);
}

uint rgbe_encode(vec3 color) {
	const float pow2to9 = 512.0f;
	const float B = 15.0f;
	const float N = 9.0f;
	const float LN2 = 0.6931471805599453094172321215;

	float cRed = clamp(color.r, 0.0, 65408.0);
	float cGreen = clamp(color.g, 0.0, 65408.0);
	float cBlue = clamp(color.b, 0.0, 65408.0);

	float cMax = max(cRed, max(cGreen, cBlue));

	float expp = max(-B - 1.0f, floor(log(cMax) / LN2)) + 1.0f + B;

	float sMax = floor((cMax / pow(2.0f, expp - B - N)) + 0.5f);

	float exps = expp + 1.0f;

	if (0.0 <= sMax && sMax < pow2to9) {
		exps = expp;
	}

	float sRed = floor((cRed / pow(2.0f, exps - B - N)) + 0.5f);
	float sGreen = floor((cGreen / pow(2.0f, exps - B - N)) + 0.5f);
	float sBlue = floor((cBlue / pow(2.0f, exps - B - N)) + 0.5f);
	return (uint(sRed) & 0x1FF) | ((uint(sGreen) & 0x1FF) << 9) | ((uint(sBlue) & 0x1FF) << 18) | ((uint(exps) & 0x1F) << 27);
}

void main() {
	ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
	if (any(greaterThanEqual(pos, params.image_size))) { //too large, do nothing
		return;
	}

#ifdef MODE_PROCESS

	float probe_cell_size = float(params.grid_size.x / float(params.probe_axis_size - 1)) / cascades.data[params.cascade].to_cell;

	ivec3 probe_cell;
	probe_cell.x = pos.x % int(params.probe_axis_size);
	probe_cell.y = pos.y;
	probe_cell.z = pos.x / int(params.probe_axis_size);

	vec3 probe_pos = cascades.data[params.cascade].offset + vec3(probe_cell) * probe_cell_size;
	vec3 pos_to_uvw = 1.0 / params.grid_size;

	vec4 probe_sh_accum[SH_SIZE] = vec4[](
			vec4(0.0),
			vec4(0.0),
			vec4(0.0),
			vec4(0.0),
			vec4(0.0),
			vec4(0.0),
			vec4(0.0),
			vec4(0.0),
			vec4(0.0)
#if (SH_SIZE == 16)
					,
			vec4(0.0),
			vec4(0.0),
			vec4(0.0),
			vec4(0.0),
			vec4(0.0),
			vec4(0.0),
			vec4(0.0)
#endif
	);

	// quickly ensure each probe has a different "offset" for the vogel function, based on integer world position
	uvec3 h3 = hash3(uvec3(params.world_offset + probe_cell));
	float offset = hashf3(vec3(h3 & uvec3(0xFFFFF)));

	//for a more homogeneous hemisphere, alternate based on history frames
	uint ray_offset = params.history_index;
	uint ray_mult = params.history_size;
	uint ray_total = ray_mult * params.ray_count;

	for (uint i = 0; i < params.ray_count; i++) {
		vec3 ray_dir = vogel_hemisphere(ray_offset + i * ray_mult, ray_total, offset);
		ray_dir.y *= params.y_mult;
		ray_dir = normalize(ray_dir);

		//needs to be visible
		vec3 ray_pos = probe_pos;
		vec3 inv_dir = 1.0 / ray_dir;

		bool hit = false;
		vec3 hit_normal;
		vec3 hit_light;
		vec3 hit_aniso0;
		vec3 hit_aniso1;

		float bias = params.ray_bias;
		vec3 abs_ray_dir = abs(ray_dir);
		ray_pos += ray_dir * 1.0 / max(abs_ray_dir.x, max(abs_ray_dir.y, abs_ray_dir.z)) * bias / cascades.data[params.cascade].to_cell;

		for (uint j = params.cascade; j < params.max_cascades; j++) {
			//convert to local bounds
			vec3 pos = ray_pos - cascades.data[j].offset;
			pos *= cascades.data[j].to_cell;

			if (any(lessThan(pos, vec3(0.0))) || any(greaterThanEqual(pos, params.grid_size))) {
				continue; //already past bounds for this cascade, goto next
			}

			//find maximum advance distance (until reaching bounds)
			vec3 t0 = -pos * inv_dir;
			vec3 t1 = (params.grid_size - pos) * inv_dir;
			vec3 tmax = max(t0, t1);
			float max_advance = min(tmax.x, min(tmax.y, tmax.z));

			float advance = 0.0;

			vec3 uvw;

			while (advance < max_advance) {
				//read how much to advance from SDF
				uvw = (pos + ray_dir * advance) * pos_to_uvw;

				float distance = texture(sampler3D(sdf_cascades[j], linear_sampler), uvw).r * 255.0 - 1.0;
				if (distance < 0.001) {
					//consider hit
					hit = true;
					break;
				}

				advance += distance;
			}

			if (hit) {
				const float EPSILON = 0.001;
				hit_normal = normalize(vec3(
						texture(sampler3D(sdf_cascades[j], linear_sampler), uvw + vec3(EPSILON, 0.0, 0.0)).r - texture(sampler3D(sdf_cascades[j], linear_sampler), uvw - vec3(EPSILON, 0.0, 0.0)).r,
						texture(sampler3D(sdf_cascades[j], linear_sampler), uvw + vec3(0.0, EPSILON, 0.0)).r - texture(sampler3D(sdf_cascades[j], linear_sampler), uvw - vec3(0.0, EPSILON, 0.0)).r,
						texture(sampler3D(sdf_cascades[j], linear_sampler), uvw + vec3(0.0, 0.0, EPSILON)).r - texture(sampler3D(sdf_cascades[j], linear_sampler), uvw - vec3(0.0, 0.0, EPSILON)).r));

				hit_light = texture(sampler3D(light_cascades[j], linear_sampler), uvw).rgb;
				vec4 aniso0 = texture(sampler3D(aniso0_cascades[j], linear_sampler), uvw);
				hit_aniso0 = aniso0.rgb;
				hit_aniso1 = vec3(aniso0.a, texture(sampler3D(aniso1_cascades[j], linear_sampler), uvw).rg);

				break;
			}

			//change ray origin to collision with bounds
			pos += ray_dir * max_advance;
			pos /= cascades.data[j].to_cell;
			pos += cascades.data[j].offset;
			ray_pos = pos;
		}

		vec4 light;
		if (hit) {
			//one liner magic
			light.rgb = hit_light * (dot(max(vec3(0.0), (hit_normal * hit_aniso0)), vec3(1.0)) + dot(max(vec3(0.0), (-hit_normal * hit_aniso1)), vec3(1.0)));
			light.a = 1.0;
		} else if (params.sky_mode == SKY_MODE_SKY) {
			light.rgb = textureLod(samplerCube(sky_irradiance, linear_sampler_mipmaps), ray_dir, 2.0).rgb; //use second mipmap because we dont usually throw a lot of rays, so this compensates
			light.rgb *= params.sky_energy;
			light.a = 0.0;

		} else if (params.sky_mode == SKY_MODE_COLOR) {
			light.rgb = params.sky_color;
			light.rgb *= params.sky_energy;
			light.a = 0.0;
		} else {
			light = vec4(0, 0, 0, 0);
		}

		vec3 ray_dir2 = ray_dir * ray_dir;
		float c[SH_SIZE] = float[](

				0.282095, //l0
				0.488603 * ray_dir.y, //l1n1
				0.488603 * ray_dir.z, //l1n0
				0.488603 * ray_dir.x, //l1p1
				1.092548 * ray_dir.x * ray_dir.y, //l2n2
				1.092548 * ray_dir.y * ray_dir.z, //l2n1
				0.315392 * (3.0 * ray_dir2.z - 1.0), //l20
				1.092548 * ray_dir.x * ray_dir.z, //l2p1
				0.546274 * (ray_dir2.x - ray_dir2.y) //l2p2
#if (SH_SIZE == 16)
				,
				0.590043 * ray_dir.y * (3.0f * ray_dir2.x - ray_dir2.y),
				2.890611 * ray_dir.y * ray_dir.x * ray_dir.z,
				0.646360 * ray_dir.y * (-1.0f + 5.0f * ray_dir2.z),
				0.373176 * (5.0f * ray_dir2.z * ray_dir.z - 3.0f * ray_dir.z),
				0.457045 * ray_dir.x * (-1.0f + 5.0f * ray_dir2.z),
				1.445305 * (ray_dir2.x - ray_dir2.y) * ray_dir.z,
				0.590043 * ray_dir.x * (ray_dir2.x - 3.0f * ray_dir2.y)

#endif
		);

		for (uint j = 0; j < SH_SIZE; j++) {
			probe_sh_accum[j] += light * c[j];
		}
	}

	for (uint i = 0; i < SH_SIZE; i++) {
		// store in history texture
		ivec3 prev_pos = ivec3(pos.x, pos.y * SH_SIZE + i, int(params.history_index));
		ivec2 average_pos = prev_pos.xy;

		vec4 value = probe_sh_accum[i] * 4.0 / float(params.ray_count);

		ivec4 ivalue = clamp(ivec4(value * float(1 << HISTORY_BITS)), -32768, 32767); //clamp to 16 bits, so higher values don't break average

		ivec4 prev_value = imageLoad(lightprobe_history_texture, prev_pos);
		ivec4 average = imageLoad(lightprobe_average_texture, average_pos);

		average -= prev_value;
		average += ivalue;

		imageStore(lightprobe_history_texture, prev_pos, ivalue);
		imageStore(lightprobe_average_texture, average_pos, average);
	}
#endif // MODE PROCESS

#ifdef MODE_STORE

	// converting to octahedral in this step is requiered because
	// octahedral is much faster to read from the screen than spherical harmonics,
	// despite the very slight quality loss

	ivec2 sh_pos = (pos / OCT_SIZE) * ivec2(1, SH_SIZE);
	ivec2 oct_pos = (pos / OCT_SIZE) * (OCT_SIZE + 2) + ivec2(1);
	ivec2 local_pos = pos % OCT_SIZE;

	//fill the spherical harmonic
	vec4 sh[SH_SIZE];

	for (uint i = 0; i < SH_SIZE; i++) {
		// store in history texture
		ivec2 average_pos = sh_pos + ivec2(0, i);
		ivec4 average = imageLoad(lightprobe_average_texture, average_pos);

		sh[i] = (vec4(average) / float(params.history_size)) / float(1 << HISTORY_BITS);
	}

	//compute the octahedral normal for this texel
	vec3 normal = octahedron_encode(vec2(local_pos) / float(OCT_SIZE));
	/*
	// read the spherical harmonic
	const float c1 = 0.429043;
	const float c2 = 0.511664;
	const float c3 = 0.743125;
	const float c4 = 0.886227;
	const float c5 = 0.247708;
	vec4 light = (c1 * sh[8] * (normal.x * normal.x - normal.y * normal.y) +
					  c3 * sh[6] * normal.z * normal.z +
					  c4 * sh[0] -
					  c5 * sh[6] +
					  2.0 * c1 * sh[4] * normal.x * normal.y +
					  2.0 * c1 * sh[7] * normal.x * normal.z +
					  2.0 * c1 * sh[5] * normal.y * normal.z +
					  2.0 * c2 * sh[3] * normal.x +
					  2.0 * c2 * sh[1] * normal.y +
					  2.0 * c2 * sh[2] * normal.z);
*/
	vec3 normal2 = normal * normal;
	float c[SH_SIZE] = float[](

			0.282095, //l0
			0.488603 * normal.y, //l1n1
			0.488603 * normal.z, //l1n0
			0.488603 * normal.x, //l1p1
			1.092548 * normal.x * normal.y, //l2n2
			1.092548 * normal.y * normal.z, //l2n1
			0.315392 * (3.0 * normal2.z - 1.0), //l20
			1.092548 * normal.x * normal.z, //l2p1
			0.546274 * (normal2.x - normal2.y) //l2p2
#if (SH_SIZE == 16)
			,
			0.590043 * normal.y * (3.0f * normal2.x - normal2.y),
			2.890611 * normal.y * normal.x * normal.z,
			0.646360 * normal.y * (-1.0f + 5.0f * normal2.z),
			0.373176 * (5.0f * normal2.z * normal.z - 3.0f * normal.z),
			0.457045 * normal.x * (-1.0f + 5.0f * normal2.z),
			1.445305 * (normal2.x - normal2.y) * normal.z,
			0.590043 * normal.x * (normal2.x - 3.0f * normal2.y)

#endif
	);

	const float l_mult[SH_SIZE] = float[](
			1.0,
			2.0 / 3.0,
			2.0 / 3.0,
			2.0 / 3.0,
			1.0 / 4.0,
			1.0 / 4.0,
			1.0 / 4.0,
			1.0 / 4.0,
			1.0 / 4.0
#if (SH_SIZE == 16)
			, // l4 does not contribute to irradiance
			0.0,
			0.0,
			0.0,
			0.0,
			0.0,
			0.0,
			0.0
#endif
	);

	vec3 irradiance = vec3(0.0);
	vec3 radiance = vec3(0.0);

	for (uint i = 0; i < SH_SIZE; i++) {
		vec3 m = sh[i].rgb * c[i] * 4.0;
		irradiance += m * l_mult[i];
		radiance += m;
	}

	//encode RGBE9995 for the final texture

	uint irradiance_rgbe = rgbe_encode(irradiance);
	uint radiance_rgbe = rgbe_encode(radiance);

	//store in octahedral map

	ivec3 texture_pos = ivec3(oct_pos, int(params.cascade));
	ivec3 copy_to[4] = ivec3[](ivec3(-2, -2, -2), ivec3(-2, -2, -2), ivec3(-2, -2, -2), ivec3(-2, -2, -2));
	copy_to[0] = texture_pos + ivec3(local_pos, 0);

	if (local_pos == ivec2(0, 0)) {
		copy_to[1] = texture_pos + ivec3(OCT_SIZE - 1, -1, 0);
		copy_to[2] = texture_pos + ivec3(-1, OCT_SIZE - 1, 0);
		copy_to[3] = texture_pos + ivec3(OCT_SIZE, OCT_SIZE, 0);
	} else if (local_pos == ivec2(OCT_SIZE - 1, 0)) {
		copy_to[1] = texture_pos + ivec3(0, -1, 0);
		copy_to[2] = texture_pos + ivec3(OCT_SIZE, OCT_SIZE - 1, 0);
		copy_to[3] = texture_pos + ivec3(-1, OCT_SIZE, 0);
	} else if (local_pos == ivec2(0, OCT_SIZE - 1)) {
		copy_to[1] = texture_pos + ivec3(-1, 0, 0);
		copy_to[2] = texture_pos + ivec3(OCT_SIZE - 1, OCT_SIZE, 0);
		copy_to[3] = texture_pos + ivec3(OCT_SIZE, -1, 0);
	} else if (local_pos == ivec2(OCT_SIZE - 1, OCT_SIZE - 1)) {
		copy_to[1] = texture_pos + ivec3(0, OCT_SIZE, 0);
		copy_to[2] = texture_pos + ivec3(OCT_SIZE, 0, 0);
		copy_to[3] = texture_pos + ivec3(-1, -1, 0);
	} else if (local_pos.y == 0) {
		copy_to[1] = texture_pos + ivec3(OCT_SIZE - local_pos.x - 1, local_pos.y - 1, 0);
	} else if (local_pos.x == 0) {
		copy_to[1] = texture_pos + ivec3(local_pos.x - 1, OCT_SIZE - local_pos.y - 1, 0);
	} else if (local_pos.y == OCT_SIZE - 1) {
		copy_to[1] = texture_pos + ivec3(OCT_SIZE - local_pos.x - 1, local_pos.y + 1, 0);
	} else if (local_pos.x == OCT_SIZE - 1) {
		copy_to[1] = texture_pos + ivec3(local_pos.x + 1, OCT_SIZE - local_pos.y - 1, 0);
	}

	for (int i = 0; i < 4; i++) {
		if (copy_to[i] == ivec3(-2, -2, -2)) {
			continue;
		}
		imageStore(lightprobe_texture_data, copy_to[i], uvec4(irradiance_rgbe));
		imageStore(lightprobe_texture_data, copy_to[i] + ivec3(0, 0, int(params.max_cascades)), uvec4(radiance_rgbe));
	}

#endif

#ifdef MODE_SCROLL

	ivec3 probe_cell;
	probe_cell.x = pos.x % int(params.probe_axis_size);
	probe_cell.y = pos.y;
	probe_cell.z = pos.x / int(params.probe_axis_size);

	ivec3 read_probe = probe_cell - params.scroll;

	if (all(greaterThanEqual(read_probe, ivec3(0))) && all(lessThan(read_probe, ivec3(params.probe_axis_size)))) {
		// can scroll
		ivec2 tex_pos;
		tex_pos = read_probe.xy;
		tex_pos.x += read_probe.z * int(params.probe_axis_size);

		//scroll
		for (uint j = 0; j < params.history_size; j++) {
			for (int i = 0; i < SH_SIZE; i++) {
				// copy from history texture
				ivec3 src_pos = ivec3(tex_pos.x, tex_pos.y * SH_SIZE + i, int(j));
				ivec3 dst_pos = ivec3(pos.x, pos.y * SH_SIZE + i, int(j));
				ivec4 value = imageLoad(lightprobe_history_texture, src_pos);
				imageStore(lightprobe_history_scroll_texture, dst_pos, value);
			}
		}

		for (int i = 0; i < SH_SIZE; i++) {
			// copy from average texture
			ivec2 src_pos = ivec2(tex_pos.x, tex_pos.y * SH_SIZE + i);
			ivec2 dst_pos = ivec2(pos.x, pos.y * SH_SIZE + i);
			ivec4 value = imageLoad(lightprobe_average_texture, src_pos);
			imageStore(lightprobe_average_scroll_texture, dst_pos, value);
		}
	} else if (params.cascade < params.max_cascades - 1) {
		//cant scroll, must look for position in parent cascade

		//to global coords
		float probe_cell_size = float(params.grid_size.x / float(params.probe_axis_size - 1)) / cascades.data[params.cascade].to_cell;
		vec3 probe_pos = cascades.data[params.cascade].offset + vec3(probe_cell) * probe_cell_size;

		//to parent local coords
		probe_pos -= cascades.data[params.cascade + 1].offset;
		probe_pos *= cascades.data[params.cascade + 1].to_cell;
		probe_pos = probe_pos * float(params.probe_axis_size - 1) / float(params.grid_size.x);

		ivec3 probe_posi = ivec3(probe_pos);
		//add up all light, no need to use occlusion here, since occlusion will do its work afterwards

		vec4 average_light[SH_SIZE] = vec4[](vec4(0), vec4(0), vec4(0), vec4(0), vec4(0), vec4(0), vec4(0), vec4(0), vec4(0)
#if (SH_SIZE == 16)
																															 ,
				vec4(0), vec4(0), vec4(0), vec4(0), vec4(0), vec4(0), vec4(0)
#endif
		);
		float total_weight = 0.0;

		for (int i = 0; i < 8; i++) {
			ivec3 offset = probe_posi + ((ivec3(i) >> ivec3(0, 1, 2)) & ivec3(1, 1, 1));

			vec3 trilinear = vec3(1.0) - abs(probe_pos - vec3(offset));
			float weight = trilinear.x * trilinear.y * trilinear.z;

			ivec2 tex_pos;
			tex_pos = offset.xy;
			tex_pos.x += offset.z * int(params.probe_axis_size);

			for (int j = 0; j < SH_SIZE; j++) {
				// copy from history texture
				ivec2 src_pos = ivec2(tex_pos.x, tex_pos.y * SH_SIZE + j);
				ivec4 average = imageLoad(lightprobe_average_parent_texture, src_pos);
				vec4 value = (vec4(average) / float(params.history_size)) / float(1 << HISTORY_BITS);
				average_light[j] += value * weight;
			}

			total_weight += weight;
		}

		if (total_weight > 0.0) {
			total_weight = 1.0 / total_weight;
		}
		//store the averaged values everywhere

		for (int i = 0; i < SH_SIZE; i++) {
			ivec4 ivalue = clamp(ivec4(average_light[i] * total_weight * float(1 << HISTORY_BITS)), ivec4(-32768), ivec4(32767)); //clamp to 16 bits, so higher values don't break average
			// copy from history texture
			ivec3 dst_pos = ivec3(pos.x, pos.y * SH_SIZE + i, 0);
			for (uint j = 0; j < params.history_size; j++) {
				dst_pos.z = int(j);
				imageStore(lightprobe_history_scroll_texture, dst_pos, ivalue);
			}

			ivalue *= int(params.history_size); //average needs to have all history added up
			imageStore(lightprobe_average_scroll_texture, dst_pos.xy, ivalue);
		}

	} else {
		// clear and let it re-raytrace, only for the last cascade, which happens very un-often
		//scroll
		for (uint j = 0; j < params.history_size; j++) {
			for (int i = 0; i < SH_SIZE; i++) {
				// copy from history texture
				ivec3 dst_pos = ivec3(pos.x, pos.y * SH_SIZE + i, int(j));
				imageStore(lightprobe_history_scroll_texture, dst_pos, ivec4(0));
			}
		}

		for (int i = 0; i < SH_SIZE; i++) {
			// copy from average texture
			ivec2 dst_pos = ivec2(pos.x, pos.y * SH_SIZE + i);
			imageStore(lightprobe_average_scroll_texture, dst_pos, ivec4(0));
		}
	}

#endif

#ifdef MODE_SCROLL_STORE

	//do not update probe texture, as these will be updated later

	for (uint j = 0; j < params.history_size; j++) {
		for (int i = 0; i < SH_SIZE; i++) {
			// copy from history texture
			ivec3 spos = ivec3(pos.x, pos.y * SH_SIZE + i, int(j));
			ivec4 value = imageLoad(lightprobe_history_scroll_texture, spos);
			imageStore(lightprobe_history_texture, spos, value);
		}
	}

	for (int i = 0; i < SH_SIZE; i++) {
		// copy from average texture
		ivec2 spos = ivec2(pos.x, pos.y * SH_SIZE + i);
		ivec4 average = imageLoad(lightprobe_average_scroll_texture, spos);
		imageStore(lightprobe_average_texture, spos, average);
	}

#endif
}
