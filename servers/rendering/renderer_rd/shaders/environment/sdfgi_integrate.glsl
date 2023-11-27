#[compute]

#version 450

#VERSION_DEFINES

#ifndef MOLTENVK_USED
#if defined(has_GL_KHR_shader_subgroup_ballot) && defined(has_GL_KHR_shader_subgroup_arithmetic)

#extension GL_KHR_shader_subgroup_ballot : enable
#extension GL_KHR_shader_subgroup_arithmetic : enable

#define USE_SUBGROUPS
#endif
#endif // MOLTENVK_USED


#define REGION_SIZE 8

#define CACHE_IS_VALID 0x80000000
#define CACHE_IS_HIT 0x40000000

layout(local_size_x = LIGHTPROBE_OCT_SIZE, local_size_y = LIGHTPROBE_OCT_SIZE, local_size_z = 1) in;

#define MAX_CASCADES 8

#ifdef MODE_PROCESS

layout(rg32ui, set = 0, binding = 1) uniform restrict readonly uimage3D voxel_cascades;
layout(r8ui, set = 0, binding = 2) uniform restrict readonly uimage3D voxel_region_cascades;

layout(set = 0, binding = 3) uniform texture3D light_cascades;
layout(set = 0, binding = 4) uniform sampler linear_sampler;
layout(r32ui, set = 0, binding = 5) uniform restrict uimage2DArray lightprobe_texture_data;
layout(r32ui, set = 0, binding = 6) uniform restrict writeonly uimage2DArray lightprobe_diffuse_data;
layout(r32ui, set = 0, binding = 7) uniform restrict writeonly uimage2DArray lightprobe_ambient_data;
layout(r32ui, set = 0, binding = 8) uniform restrict uimage2DArray ray_hit_cache;
layout(r16ui, set = 0, binding = 9) uniform restrict uimage2DArray ray_hit_cache_version;
layout(r16ui, set = 0, binding = 10) uniform restrict uimage3D region_versions;
layout(rgba16ui, set = 0, binding = 11) uniform restrict uimage2DArray lightprobe_moving_average_history;
layout(rgba16ui, set = 0, binding = 12) uniform restrict uimage2DArray lightprobe_moving_average;


#ifdef USE_CUBEMAP_ARRAY
layout(set = 1, binding = 0) uniform textureCubeArray sky_irradiance;
#else
layout(set = 1, binding = 0) uniform textureCube sky_irradiance;
#endif
layout(set = 1, binding = 1) uniform sampler linear_sampler_mipmaps;

#define SKY_MODE_DISABLED 0
#define SKY_MODE_COLOR 1
#define SKY_MODE_SKY 2

struct CascadeData {
	vec3 offset; //offset of (0,0,0) in world coordinates
	float to_cell; // 1/bounds * grid_size
	ivec3 region_world_offset;
	uint pad;
	vec4 pad2;
};

layout(set = 0, binding = 13, std140) uniform Cascades {
	CascadeData data[MAX_CASCADES];
}
cascades;

// MODE_PROCESS
#endif


#ifdef MODE_FILTER

layout(r32ui, set = 0, binding = 1) uniform restrict readonly uimage2DArray lightprobe_src_diffuse_data;
layout(r32ui, set = 0, binding = 2) uniform restrict writeonly uimage2DArray lightprobe_dst_diffuse_data;
layout(r8ui, set = 0, binding = 3) uniform restrict readonly uimage2DArray lightprobe_neighbours;

#endif

layout(push_constant, std430) uniform Params {
	ivec3 grid_size;
	int max_cascades;

	float ray_bias;
	int cascade;
	int history_index;
	int history_size;

	ivec3 world_offset;
	uint sky_mode;

	ivec3 scroll;
	float sky_energy;

	vec3 sky_color;
	float y_mult;

	ivec3 probe_axis_size;
	bool store_ambient_texture;

	uvec3 pad;
	uint motion_accum; // Motion that happened since last update (bit 0 in X, bit 1 in Y, bit 2 in Z).
}
params;

#define HISTORY_FRACT_BITS 10
#define HISTORY_BITS 16

uvec3 history_encode_16(vec3 p_color) {
	return uvec3(clamp(p_color * float(1<<HISTORY_FRACT_BITS),vec3(0.0),vec3(float((1<<HISTORY_BITS)-1))));
}

vec3 history_decode_16(uvec3 p_color) {
	return vec3(p_color) / vec3(1<<HISTORY_FRACT_BITS);
}

uvec4 history_encode_21(uvec3 p_color16) {
	uvec4 enc;
	const uint mask = ((1<<HISTORY_BITS)-1);
	p_color16 = min(p_color16,uvec3( (1<<21) -1 ));
	enc.rgb = p_color16 & uvec3(mask); // lower 16
	enc.a = (p_color16.r >> HISTORY_BITS) | ((p_color16.g&~mask) >> (HISTORY_BITS-5)) | ((p_color16.b&~mask) >> (HISTORY_BITS-10));
	return enc;
}

uvec3 history_decode_21(uvec4 p_color21) {
	uvec3 dec;
	dec = p_color21.rgb;
	dec |= ((uvec3(p_color21.a) >> uvec3(0,5,10)) & uvec3(0x1F)) << uvec3(16);
	return dec;
}

uvec3 hash3(uvec3 x) {
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = (x >> 16) ^ x;
	return x;
}

uint hash(uint x) {
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = (x >> 16) ^ x;
	return x;
}

float hashf3(vec3 co) {
	return fract(sin(dot(co, vec3(12.9898, 78.233, 137.13451))) * 43758.5453);
}

vec3 octahedron_decode(vec2 f) {
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


vec3 rgbe_decode(uint p_rgbe) {
	vec4 rgbef = vec4((uvec4(p_rgbe) >> uvec4(0,9,18,27)) & uvec4(0x1FF,0x1FF,0x1FF,0x1F));
	return rgbef.rgb * pow( 2.0, rgbef.a - 15.0 - 9.0 );
}

#ifdef MODE_PROCESS

bool trace_ray_hdda(vec3 ray_pos, vec3 ray_dir,int p_cascade, out ivec3 r_cell,out ivec3 r_side, out int r_cascade) {

	const int LEVEL_CASCADE = -1;
	const int LEVEL_REGION = 0;
	const int LEVEL_BLOCK = 1;
	const int LEVEL_VOXEL = 2;
	const int MAX_LEVEL = 3;


	const int fp_bits = 8;
	const int fp_block_bits = fp_bits + 2;
	const int fp_region_bits = fp_block_bits + 1;
	const int fp_cascade_bits = fp_region_bits + 4;

	bvec3 limit_dir = greaterThan(ray_dir,vec3(0.0));
	ivec3 step = mix(ivec3(0),ivec3(1),limit_dir);
	ivec3 ray_sign = ivec3(sign(ray_dir));

	ivec3 ray_dir_fp = ivec3(ray_dir * float(1<<fp_bits));

	bvec3 ray_zero = lessThan(abs(ray_dir),vec3(1.0/127.0));
	ivec3 inv_ray_dir_fp = ivec3( float(1<<fp_bits) / ray_dir );

	const ivec3 level_masks[MAX_LEVEL]=ivec3[](
		ivec3(1<<fp_region_bits) - ivec3(1),
		ivec3(1<<fp_block_bits) - ivec3(1),
		ivec3(1<<fp_bits) - ivec3(1)
	);

	ivec3 region_offset_mask = (params.grid_size / REGION_SIZE) - ivec3(1);

	ivec3 limits[MAX_LEVEL];

	limits[LEVEL_REGION] = ((params.grid_size << fp_bits) - ivec3(1)) * step; // Region limit does not change, so initialize now.

	// Initialize to cascade
	int level = LEVEL_CASCADE;
	int cascade = p_cascade - 1;

	ivec3 cascade_base;
	ivec3 region_base;
	uvec2 block;
	bool hit = false;

	ivec3 pos;

	while(true) {
		// This loop is written so there is only one single main interation.
		// This ensures that different compute threads working on different
		// levels can still run together without blocking each other.

		if (level == LEVEL_VOXEL) {
			// The first level should be (in a worst case scenario) the most used
			// so it needs to appear first. The rest of the levels go from more to least used order.

			ivec3 block_local = (pos & level_masks[LEVEL_BLOCK]) >> fp_bits;
			uint block_index = uint(block_local.z * 16 + block_local.y * 4 + block_local.x);
			if (block_index < 32) {
				// Low 32 bits.
				if (bool(block.x & uint(1<<block_index))) {
					hit=true;
					break;
				}
			} else {
				// High 32 bits.
				block_index-=32;
				if (bool(block.y & uint(1<<block_index))) {
					hit=true;
					break;
				}
			}
		} else if (level == LEVEL_BLOCK) {
			ivec3 block_local = (pos & level_masks[LEVEL_REGION]) >> fp_block_bits;
			block = imageLoad(voxel_cascades,region_base + block_local).rg;
			if (block != uvec2(0)) {
				// Have voxels inside
				level = LEVEL_VOXEL;
				limits[LEVEL_VOXEL]= pos - (pos & level_masks[LEVEL_BLOCK]) + step * (level_masks[LEVEL_BLOCK] + ivec3(1));
				continue;
			}
		} else if (level == LEVEL_REGION) {
			ivec3 region = pos >> fp_region_bits;
			region = (cascades.data[cascade].region_world_offset + region) & region_offset_mask; // Scroll to world
			region += cascade_base;
			bool region_used = imageLoad(voxel_region_cascades,region).r > 0;

			if (region_used) {
				// The region has contents.
				region_base = (region<<1);
				level = LEVEL_BLOCK;
				limits[LEVEL_BLOCK]= pos - (pos & level_masks[LEVEL_REGION]) + step * (level_masks[LEVEL_REGION] + ivec3(1));
				continue;
			}
		} else if (level == LEVEL_CASCADE) {
			// Return to global
			if (cascade >= p_cascade) {
				ray_pos = vec3(pos) / float(1<<fp_bits);
				ray_pos /= cascades.data[cascade].to_cell;
				ray_pos += cascades.data[cascade].offset;
			}

			cascade++;
			if (cascade == params.max_cascades) {
				break;
			}

			ray_pos -= cascades.data[cascade].offset;
			ray_pos *= cascades.data[cascade].to_cell;
			pos = ivec3(ray_pos * float(1<<fp_bits));
			if (any(lessThan(pos,ivec3(0))) || any(greaterThanEqual(pos,params.grid_size<<fp_bits))) {
				// Outside this cascade, go to next.
				continue;
			}

			cascade_base = ivec3(0,params.grid_size.y/REGION_SIZE * cascade , 0);
			level = LEVEL_REGION;
			continue;
		}

		// Fixed point, multi-level DDA.

		ivec3 mask = level_masks[level];
		ivec3 box = mask * step;
		ivec3 pos_diff = box - (pos & mask);
		ivec3 tv = mix((pos_diff * inv_ray_dir_fp),ivec3(0x7FFFFFFF),ray_zero) >> fp_bits;
		int t = min(tv.x,min(tv.y,tv.z));

		// The general idea here is that we _always_ need to increment to the closest next cell
		// (this is a DDA after all), so adv_box forces this increment for the minimum axis.

		ivec3 adv_box = pos_diff + ray_sign;
		ivec3 adv_t = (ray_dir_fp * t) >> fp_bits;

		pos += mix(adv_t,adv_box,equal(ivec3(t),tv));

		while(true) {
			bvec3 limit = lessThan(pos,limits[level]);
			bool inside = all(equal(limit,limit_dir));
			if (inside) {
				break;
			}
			level-=1;
			if (level == LEVEL_CASCADE) {
				break;
			}
		}
	}

	if (hit) {

		ivec3 mask = level_masks[LEVEL_VOXEL];
		ivec3 box = mask * (step ^ ivec3(1));
		ivec3 pos_diff = box - (pos & mask);
		ivec3 tv = mix((pos_diff * -inv_ray_dir_fp),ivec3(0x7FFFFFFF),ray_zero);

		int m;
		if (tv.x < tv.y) {
			r_side = ivec3(1,0,0);
			m = tv.x;
		} else {
			r_side = ivec3(0,1,0);
			m = tv.y;
		}
		if (tv.z < m) {
			r_side = ivec3(0,0,1);
		}

		r_side *= -ray_sign;

		r_cell = pos >> fp_bits;

		r_cascade = cascade;
	}

	return hit;
}

#if LIGHTPROBE_OCT_SIZE == 4
const uint neighbour_max_weights = 8;
const uint neighbour_weights[128]= uint[](15544, 73563, 135085, 206971, 270171, 528301, 796795, 988221, 8569, 82130, 144347, 200892, 272100, 336249, 397500, 0, 4284, 78811, 147666, 205177, 331964, 401785, 468708, 0, 10363, 69549, 139099, 212152, 466779, 724909, 791613, 993403, 8569, 75492, 278738, 336249, 537563, 594108, 790716, 0, 73563, 135085, 270171, 343224, 403579, 528301, 600187, 660541, 69549, 139099, 338043, 408760, 466779, 595005, 665723, 724909, 141028, 205177, 401785, 475346, 659644, 734171, 987324, 0, 4284, 275419, 331964, 540882, 598393, 795001, 861924, 0, 266157, 338043, 398397, 532315, 605368, 665723, 859995, 921517, 332861, 403579, 462765, 600187, 670904, 728923, 855981, 925531, 200892, 397500, 472027, 663929, 737490, 927460, 991609, 0, 10363, 201789, 266157, 532315, 801976, 859995, 921517, 993403, 534244, 598393, 659644, 795001, 868562, 930779, 987324, 0, 594108, 663929, 730852, 790716, 865243, 934098, 991609, 0, 5181, 206971, 462765, 728923, 796795, 855981, 925531, 998584);
#endif

#if LIGHTPROBE_OCT_SIZE == 5
const uint neighbour_max_weights = 15;
const uint neighbour_weights[375]= uint[](11139, 72624, 131886, 201671, 271258, 334768, 394335, 590836, 656174, 988103, 1319834, 1377268, 1579952, 0, 0, 6839, 76283, 139717, 205401, 267029, 334519, 400776, 461448, 527528, 590801, 657717, 984017, 1311697, 0, 0, 778, 74103, 141723, 205175, 262922, 330016, 400965, 466633, 532037, 592160, 655986, 723045, 789015, 854117, 918130, 4885, 74329, 139717, 207355, 268983, 328657, 396456, 461448, 531848, 596663, 919861, 1246161, 1573841, 0, 0, 9114, 70599, 131886, 203696, 273283, 328692, 525407, 596912, 918318, 1250247, 1317808, 1508340, 1581978, 0, 0, 6839, 72375, 133429, 197585, 263121, 338427, 400776, 664005, 723592, 991833, 1051816, 1315605, 1377233, 0, 0, 1026, 72722, 138504, 199687, 334866, 403430, 465362, 525422, 662792, 727506, 789836, 986119, 1049710, 0, 0, 68049, 138485, 199121, 399699, 468771, 530771, 657381, 727832, 794768, 858904, 919525, 1117965, 0, 0, 0, 68615, 138504, 203794, 263170, 394350, 465362, 534502, 597010, 789836, 858578, 924936, 1180782, 1248263, 0, 0, 977, 66513, 133429, 203447, 268983, 531848, 600571, 854664, 926149, 1182888, 1253977, 1508305, 1577749, 0, 0, 778, 67872, 131698, 336247, 400965, 460901, 666011, 728777, 789015, 991607, 1056325, 1116261, 1311498, 1378592, 1442418, 133093, 330193, 399699, 465688, 662773, 730915, 794768, 855821, 985553, 1055059, 1121048, 1443813, 0, 0, 0, 133468, 396510, 466974, 527582, 657756, 729118, 796314, 860190, 919900, 1051870, 1122334, 1182942, 1444188, 0, 0, 133093, 465688, 530771, 592337, 724749, 794768, 861987, 924917, 1121048, 1186131, 1247697, 1443813, 0, 0, 0, 131698, 198944, 262922, 460901, 532037, 598391, 789015, 859849, 928155, 1116261, 1187397, 1253751, 1442418, 1509664, 1573642, 4885, 66513, 336473, 396456, 664005, 723592, 993787, 1056136, 1317559, 1383095, 1444149, 1508305, 1573841, 0, 0, 330759, 394350, 662792, 727506, 789836, 990226, 1058790, 1120722, 1180782, 1311746, 1383442, 1449224, 1510407, 0, 0, 462605, 657381, 727832, 794768, 858904, 919525, 1055059, 1124131, 1186131, 1378769, 1449205, 1509841, 0, 0, 0, 525422, 592903, 789836, 858578, 924936, 1049710, 1120722, 1189862, 1252370, 1379335, 1449224, 1514514, 1573890, 0, 0, 197585, 267029, 527528, 598617, 854664, 926149, 1187208, 1255931, 1311697, 1377233, 1444149, 1514167, 1579703, 0, 0, 9114, 66548, 269232, 332743, 656174, 990128, 1049695, 1246196, 1321859, 1383344, 1442606, 1512391, 1581978, 0, 0, 977, 328657, 657717, 989879, 1056136, 1116808, 1182888, 1246161, 1317559, 1387003, 1450437, 1516121, 1577749, 0, 0, 655986, 723045, 789015, 854117, 918130, 985376, 1056325, 1121993, 1187397, 1247520, 1311498, 1384823, 1452443, 1515895, 1573642, 263121, 590801, 919861, 984017, 1051816, 1116808, 1187208, 1252023, 1315605, 1385049, 1450437, 1518075, 1579703, 0, 0, 7088, 197620, 271258, 594887, 918318, 984052, 1180767, 1252272, 1319834, 1381319, 1442606, 1514416, 1584003, 0, 0);
#endif

shared vec3 neighbours[LIGHTPROBE_OCT_SIZE*LIGHTPROBE_OCT_SIZE];

#endif

ivec3 modi(ivec3 value, ivec3 p_y) {
	return mix( value % p_y, p_y - ((abs(value)-ivec3(1)) % p_y), lessThan(sign(value), ivec3(0)) );
}


void main() {

#ifdef MODE_PROCESS

	ivec2 pos = ivec2(gl_WorkGroupID.xy);
	ivec2 local_pos = ivec2(gl_LocalInvocationID.xy);
	uint probe_index = gl_LocalInvocationID.x + gl_LocalInvocationID.y * LIGHTPROBE_OCT_SIZE;

	float probe_cell_size = float(params.grid_size.x) / float(params.probe_axis_size.x - 1) / cascades.data[params.cascade].to_cell;

	ivec3 probe_cell;
	probe_cell.x = pos.x;
	probe_cell.y = pos.y % params.probe_axis_size.y;
	probe_cell.z = pos.y / params.probe_axis_size.y;

	vec3 ray_pos = cascades.data[params.cascade].offset + vec3(probe_cell) * probe_cell_size;

	ivec3 probe_world_pos = params.world_offset + probe_cell;

	// Ensure a unique hash that includes the probe world position, the local octahedron pixel, and the history frame index
	uvec3 h3 = hash3(uvec3((uvec3(probe_world_pos) * LIGHTPROBE_OCT_SIZE * LIGHTPROBE_OCT_SIZE + uvec3(probe_index)) * uvec3(params.history_size) + uvec3(params.history_index)));
	uint h = (h3.x ^ h3.y) ^ h3.z;
	vec2 sample_ofs = vec2(ivec2(h>>16,h&0xFFFF)) / vec2(0xFFFF);

	vec3 ray_dir = octahedron_decode( (vec2(local_pos) + sample_ofs) / vec2(LIGHTPROBE_OCT_SIZE) );
	ray_dir.y *= params.y_mult;
	ray_dir = normalize(ray_dir);

	// Apply bias (by a cell)
	float bias = params.ray_bias;
	vec3 abs_ray_dir = abs(ray_dir);
	ray_pos += ray_dir * 1.0 / max(abs_ray_dir.x, max(abs_ray_dir.y, abs_ray_dir.z)) * bias / cascades.data[params.cascade].to_cell;

	ivec3 probe_scroll_pos = modi(probe_world_pos, params.probe_axis_size);
	ivec3 probe_texture_pos = ivec3( (probe_scroll_pos.xy + ivec2(0,probe_scroll_pos.z * params.probe_axis_size.y)), params.cascade);
	ivec3 cache_texture_pos = ivec3(probe_texture_pos.xy * LIGHTPROBE_OCT_SIZE + local_pos, probe_texture_pos.z * params.history_size + params.history_index);
	uint cache_entry = imageLoad(ray_hit_cache,cache_texture_pos).r;

	bool hit;
	ivec3 hit_cell;
	int hit_cascade;

	bool cache_valid = bool(cache_entry & CACHE_IS_VALID);

	vec3 cache_invalidated_debug = vec3(0.0);

	if (cache_valid) {
		// Make sure the cache is really valid
		hit = bool(cache_entry & CACHE_IS_HIT);
		uvec4 uhit = (uvec4(cache_entry) >> uvec4(0,8,16,24)) & uvec4(0xFF,0xFF,0xFF,0x7);
		hit_cell = ivec3(uhit.xyz);
		hit_cascade = int(uhit.w);
		uint axis = (cache_entry >> 27) & 0x3;
		if (bool((1<<axis) & params.motion_accum)) {
			// There was motion in this axis, cache is no longer valid.
			cache_valid=false;
			cache_invalidated_debug = vec3(0,0,4.0);
		} else if (hit) {
			// Check if the region pointed to is still valid.
			uint version = imageLoad(ray_hit_cache_version,cache_texture_pos).r;
			uint region_version = imageLoad(region_versions, (hit_cell / REGION_SIZE) + ivec3(0,hit_cascade * (params.grid_size.y / REGION_SIZE),0)).r;

			if (region_version != version) {
				cache_valid = false;
				cache_invalidated_debug = (hit_cascade==params.cascade) ? vec3(0.0,4.00,0.0) : vec3(4.0,0,0.0);
			}
		}
	}


	if (!cache_valid) {
		ivec3 hit_face;
		hit = trace_ray_hdda(ray_pos, ray_dir,params.cascade, hit_cell,hit_face, hit_cascade);
		if (hit) {
			hit_cell += hit_face;

			ivec3 reg_cell_offset = cascades.data[hit_cascade].region_world_offset * REGION_SIZE;
			hit_cell = (hit_cell + reg_cell_offset) & (params.grid_size-1); // Read from wrapped world cordinates
		}
	}

	vec3 light;

	if (hit) {
		ivec3 spos = hit_cell;
		spos.y += hit_cascade * params.grid_size.y;
		light = texelFetch(sampler3D(light_cascades, linear_sampler), spos,0).rgb;
	} else if (params.sky_mode == SKY_MODE_SKY) {
#ifdef USE_CUBEMAP_ARRAY
		light = textureLod(samplerCubeArray(sky_irradiance, linear_sampler_mipmaps), vec4(ray_dir, 0.0), 2.0).rgb; // Use second mipmap because we don't usually throw a lot of rays, so this compensates.
#else
		light = textureLod(samplerCube(sky_irradiance, linear_sampler_mipmaps), ray_dir, 2.0).rgb; // Use second mipmap because we don't usually throw a lot of rays, so this compensates.
#endif
		light *= params.sky_energy;
	} else if (params.sky_mode == SKY_MODE_COLOR) {
		light = params.sky_color;
		light *= params.sky_energy;
	} else {
		light = vec3(0);
	}


	if (!cache_valid) {

		cache_entry = CACHE_IS_VALID;
		if (hit) {
			// Determine the side of the cascade box this ray exited through, this is important for invalidation purposes.

			vec3 unit_pos = ray_pos - cascades.data[params.cascade].offset;
			unit_pos *= cascades.data[params.cascade].to_cell;

			vec3 t0 = -unit_pos / ray_dir;
			vec3 t1 = (vec3(params.grid_size) - unit_pos) / ray_dir;
			vec3 tmax = max(t0, t1);

			uint axis;
			float m;
			if (tmax.x < tmax.y) {
				axis = 0;
				m=tmax.x;
			} else {
				axis = 1;
				m=tmax.y;
			}
			if (tmax.z < m) {
				axis = 2;
			}

			uvec3 ucell = (uvec3(hit_cell) & uvec3(0xFF)) << uvec3(0,8,16);
			cache_entry |= CACHE_IS_HIT | ucell.x | ucell.y | ucell.z | (uint(min(7,hit_cascade))<<24) | (axis << 27);

			uint region_version = imageLoad(region_versions, (hit_cell >> REGION_SIZE) + ivec3(0,hit_cascade * (params.grid_size.y / REGION_SIZE),0)).r;

			imageStore(ray_hit_cache_version, cache_texture_pos, uvec4(region_version));

		}

		imageStore(ray_hit_cache, cache_texture_pos, uvec4(cache_entry));
	}


	// Blend with existing light


	{
		// Do moving average

		uvec3 moving_avg = history_decode_21( imageLoad(lightprobe_moving_average,ivec3(cache_texture_pos.xy,params.cascade)) );
		uvec3 prev_val16 = imageLoad(lightprobe_moving_average_history,cache_texture_pos).rgb;
		moving_avg -= prev_val16;
		uvec3 new_val = history_encode_16(light);
		imageStore(lightprobe_moving_average_history,cache_texture_pos,uvec4(new_val,0));
		moving_avg+=new_val;
		imageStore(lightprobe_moving_average,ivec3(cache_texture_pos.xy,params.cascade), history_encode_21(moving_avg) );
		light = history_decode_16( moving_avg / uvec3(params.history_size) );
	}

	probe_texture_pos = ivec3(probe_texture_pos.xy * (LIGHTPROBE_OCT_SIZE + 2) + ivec2(1),probe_texture_pos.z);
	ivec3 probe_read_pos = probe_texture_pos + ivec3(local_pos,0);

	neighbours[ probe_index ] = light;


	groupMemoryBarrier();
	barrier();

	// Filter with neighbours
	vec3 diffuse_light = vec3(0.0);
	for(uint i=0;i<neighbour_max_weights;i++) {
		uint n = neighbour_weights[ probe_index * neighbour_max_weights + i];
		uint index = n>>16;
		float weight = float(n&0xFFFF) / float(0xFFFF);
		diffuse_light += neighbours[index] * weight;
	}

	//if (cache_invalidated_debug!=vec3(0.0)) {
	//	diffuse_light = cache_invalidated_debug;
	//}

	vec3 ambient_light = vec3(0);
	if (params.store_ambient_texture) {
#ifdef USE_SUBGROUPS
		ambient_light = subgroupAdd(diffuse_light) / float(LIGHTPROBE_OCT_SIZE*LIGHTPROBE_OCT_SIZE);
#else
		neighbours[ probe_index ] = diffuse_light;
		groupMemoryBarrier();
		barrier();
		for(uint i=0;i<LIGHTPROBE_OCT_SIZE*LIGHTPROBE_OCT_SIZE;i++) {
			ambient_light+=neighbours[ i ];
		}
		ambient_light /= float(LIGHTPROBE_OCT_SIZE*LIGHTPROBE_OCT_SIZE);
#endif
	}

//	diffuse_light = vec3(probe_cache_pos) / vec3(17.0);

	// Store in octahedral map

	ivec3 copy_to[4] = ivec3[](ivec3(-2, -2, -2), ivec3(-2, -2, -2), ivec3(-2, -2, -2), ivec3(-2, -2, -2));
	copy_to[0] = probe_read_pos;

	if (local_pos == ivec2(0, 0)) {
		copy_to[1] = probe_texture_pos + ivec3(LIGHTPROBE_OCT_SIZE - 1, -1, 0);
		copy_to[2] = probe_texture_pos + ivec3(-1, LIGHTPROBE_OCT_SIZE - 1, 0);
		copy_to[3] = probe_texture_pos + ivec3(LIGHTPROBE_OCT_SIZE, LIGHTPROBE_OCT_SIZE, 0);
	} else if (local_pos == ivec2(LIGHTPROBE_OCT_SIZE - 1, 0)) {
		copy_to[1] = probe_texture_pos + ivec3(0, -1, 0);
		copy_to[2] = probe_texture_pos + ivec3(LIGHTPROBE_OCT_SIZE, LIGHTPROBE_OCT_SIZE - 1, 0);
		copy_to[3] = probe_texture_pos + ivec3(-1, LIGHTPROBE_OCT_SIZE, 0);
	} else if (local_pos == ivec2(0, LIGHTPROBE_OCT_SIZE - 1)) {
		copy_to[1] = probe_texture_pos + ivec3(-1, 0, 0);
		copy_to[2] = probe_texture_pos + ivec3(LIGHTPROBE_OCT_SIZE - 1, LIGHTPROBE_OCT_SIZE, 0);
		copy_to[3] = probe_texture_pos + ivec3(LIGHTPROBE_OCT_SIZE, -1, 0);
	} else if (local_pos == ivec2(LIGHTPROBE_OCT_SIZE - 1, LIGHTPROBE_OCT_SIZE - 1)) {
		copy_to[1] = probe_texture_pos + ivec3(0, LIGHTPROBE_OCT_SIZE, 0);
		copy_to[2] = probe_texture_pos + ivec3(LIGHTPROBE_OCT_SIZE, 0, 0);
		copy_to[3] = probe_texture_pos + ivec3(-1, -1, 0);
	} else if (local_pos.y == 0) {
		copy_to[1] = probe_texture_pos + ivec3(LIGHTPROBE_OCT_SIZE - local_pos.x - 1, local_pos.y - 1, 0);
	} else if (local_pos.x == 0) {
		copy_to[1] = probe_texture_pos + ivec3(local_pos.x - 1, LIGHTPROBE_OCT_SIZE - local_pos.y - 1, 0);
	} else if (local_pos.y == LIGHTPROBE_OCT_SIZE - 1) {
		copy_to[1] = probe_texture_pos + ivec3(LIGHTPROBE_OCT_SIZE - local_pos.x - 1, local_pos.y + 1, 0);
	} else if (local_pos.x == LIGHTPROBE_OCT_SIZE - 1) {
		copy_to[1] = probe_texture_pos + ivec3(local_pos.x + 1, LIGHTPROBE_OCT_SIZE - local_pos.y - 1, 0);
	}

	uint light_rgbe = rgbe_encode(light);
	uint diffuse_rgbe = rgbe_encode(diffuse_light);
	uint ambient_rgbe;
	if (params.store_ambient_texture) {
		ambient_rgbe = rgbe_encode(ambient_light);
	}
	for (int i = 0; i < 4; i++) {
		if (copy_to[i] == ivec3(-2, -2, -2)) {
			continue;
		}
		imageStore(lightprobe_texture_data, copy_to[i], uvec4(light_rgbe));
		imageStore(lightprobe_diffuse_data, copy_to[i], uvec4(diffuse_rgbe));
		if (params.store_ambient_texture) {
			imageStore(lightprobe_ambient_data, copy_to[i], uvec4(ambient_rgbe));
		}
		// also to diffuse
	}

#endif

#ifdef MODE_FILTER


	ivec2 pos = ivec2(gl_WorkGroupID.xy);
	ivec2 local_pos = ivec2(gl_LocalInvocationID.xy);

	ivec3 probe_cell;
	probe_cell.x = pos.x;
	probe_cell.y = pos.y % params.probe_axis_size.y;
	probe_cell.z = pos.y / params.probe_axis_size.y;

	ivec3 probe_world_pos = params.world_offset + probe_cell;

	ivec3 probe_scroll_pos = modi(probe_world_pos, params.probe_axis_size);
	ivec3 probe_base_pos = ivec3( (probe_scroll_pos.xy + ivec2(0,probe_scroll_pos.z * params.probe_axis_size.y)), params.cascade);

	ivec3 probe_texture_pos = ivec3(probe_base_pos.xy * (LIGHTPROBE_OCT_SIZE + 2) + ivec2(1),probe_base_pos.z);
	ivec3 probe_read_pos = probe_texture_pos + ivec3(local_pos,0);

	vec4 light;
	light.rgb = rgbe_decode(imageLoad(lightprobe_src_diffuse_data,probe_read_pos).r);
	light.a = 1.0;

	const vec3 aniso_dir[6] = vec3[](
			vec3(-1, 0, 0),
			vec3(1, 0, 0),
			vec3(0, -1, 0),
			vec3(0, 1, 0),
			vec3(0, 0, -1),
			vec3(0, 0, 1));

	uint neighbours = imageLoad( lightprobe_neighbours,probe_base_pos ).r;

	for(int i=0;i<6;i++) {

		if (!bool(neighbours & (1<<i))) {
			continue; // un-neighboured
		}

		ivec3 neighbour_probe = probe_cell + ivec3(aniso_dir[i]);
		if (any(lessThan(neighbour_probe,ivec3(0))) || any(greaterThanEqual(neighbour_probe,params.probe_axis_size))) {
			continue; // Outside range.
		}

		ivec3 probe_world_pos2 = params.world_offset + neighbour_probe;
		ivec3 probe_scroll_pos2 = modi(probe_world_pos2, params.probe_axis_size);
		ivec3 probe_base_pos2 = ivec3( (probe_scroll_pos2.xy + ivec2(0,probe_scroll_pos2.z * params.probe_axis_size.y)), params.cascade);

		ivec3 probe_texture_pos2 = ivec3(probe_base_pos2.xy * (LIGHTPROBE_OCT_SIZE + 2) + ivec2(1),probe_base_pos2.z);
		ivec3 probe_read_pos2 = probe_texture_pos2 + ivec3(local_pos,0);

		vec4 light2;
		light2.rgb = rgbe_decode(imageLoad(lightprobe_src_diffuse_data,probe_read_pos2).r);
		light2.a = 1.0;

		light += light2 * 0.7;
	}

	light.rgb /= light.a;

	ivec3 copy_to[4] = ivec3[](ivec3(-2, -2, -2), ivec3(-2, -2, -2), ivec3(-2, -2, -2), ivec3(-2, -2, -2));
	copy_to[0] = probe_read_pos;

	if (local_pos == ivec2(0, 0)) {
		copy_to[1] = probe_texture_pos + ivec3(LIGHTPROBE_OCT_SIZE - 1, -1, 0);
		copy_to[2] = probe_texture_pos + ivec3(-1, LIGHTPROBE_OCT_SIZE - 1, 0);
		copy_to[3] = probe_texture_pos + ivec3(LIGHTPROBE_OCT_SIZE, LIGHTPROBE_OCT_SIZE, 0);
	} else if (local_pos == ivec2(LIGHTPROBE_OCT_SIZE - 1, 0)) {
		copy_to[1] = probe_texture_pos + ivec3(0, -1, 0);
		copy_to[2] = probe_texture_pos + ivec3(LIGHTPROBE_OCT_SIZE, LIGHTPROBE_OCT_SIZE - 1, 0);
		copy_to[3] = probe_texture_pos + ivec3(-1, LIGHTPROBE_OCT_SIZE, 0);
	} else if (local_pos == ivec2(0, LIGHTPROBE_OCT_SIZE - 1)) {
		copy_to[1] = probe_texture_pos + ivec3(-1, 0, 0);
		copy_to[2] = probe_texture_pos + ivec3(LIGHTPROBE_OCT_SIZE - 1, LIGHTPROBE_OCT_SIZE, 0);
		copy_to[3] = probe_texture_pos + ivec3(LIGHTPROBE_OCT_SIZE, -1, 0);
	} else if (local_pos == ivec2(LIGHTPROBE_OCT_SIZE - 1, LIGHTPROBE_OCT_SIZE - 1)) {
		copy_to[1] = probe_texture_pos + ivec3(0, LIGHTPROBE_OCT_SIZE, 0);
		copy_to[2] = probe_texture_pos + ivec3(LIGHTPROBE_OCT_SIZE, 0, 0);
		copy_to[3] = probe_texture_pos + ivec3(-1, -1, 0);
	} else if (local_pos.y == 0) {
		copy_to[1] = probe_texture_pos + ivec3(LIGHTPROBE_OCT_SIZE - local_pos.x - 1, local_pos.y - 1, 0);
	} else if (local_pos.x == 0) {
		copy_to[1] = probe_texture_pos + ivec3(local_pos.x - 1, LIGHTPROBE_OCT_SIZE - local_pos.y - 1, 0);
	} else if (local_pos.y == LIGHTPROBE_OCT_SIZE - 1) {
		copy_to[1] = probe_texture_pos + ivec3(LIGHTPROBE_OCT_SIZE - local_pos.x - 1, local_pos.y + 1, 0);
	} else if (local_pos.x == LIGHTPROBE_OCT_SIZE - 1) {
		copy_to[1] = probe_texture_pos + ivec3(local_pos.x + 1, LIGHTPROBE_OCT_SIZE - local_pos.y - 1, 0);
	}

	uint light_rgbe = rgbe_encode(light.rgb);

	for (int i = 0; i < 4; i++) {
		if (copy_to[i] == ivec3(-2, -2, -2)) {
			continue;
		}
		imageStore(lightprobe_dst_diffuse_data, copy_to[i], uvec4(light_rgbe));
	}

#endif

}
