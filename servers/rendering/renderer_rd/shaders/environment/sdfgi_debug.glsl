#[compute]

#version 450

#VERSION_DEFINES

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

#define MAX_CASCADES 8
#define REGION_SIZE 8


layout(rg32ui, set = 0, binding = 1) uniform restrict readonly uimage3D voxel_cascades;
layout(r8ui, set = 0, binding = 2) uniform restrict readonly uimage3D voxel_region_cascades;

layout(set = 0, binding = 3) uniform texture3D light_cascades;

layout(set = 0, binding = 4) uniform sampler linear_sampler;
layout(set = 0, binding = 5) uniform sampler nearest_sampler;

layout(set = 0, binding = 6) uniform texture2DArray light_probes;
layout(set = 0, binding = 7) uniform texture2DArray occlusion_probes;


struct CascadeData {
	vec3 offset; //offset of (0,0,0) in world coordinates
	float to_cell; // 1/bounds * grid_size
	ivec3 region_world_offset;
	uint pad;
	vec4 pad2;
};

layout(set = 0, binding = 8, std140) restrict readonly uniform Cascades {
	CascadeData data[MAX_CASCADES];
}
cascades;

layout(rgba16f, set = 0, binding = 9) uniform restrict writeonly image2D screen_buffer;


layout(push_constant, std430) uniform Params {
	vec3 grid_size;
	uint max_cascades;

	ivec2 screen_size;
	float y_mult;

	float z_near;

	mat3x4 inv_projection;
	// We pack these more tightly than mat3 and vec3, which will require some reconstruction trickery.
	float cam_basis[3][3];
	float cam_origin[3];
}
params;

vec3 linear_to_srgb(vec3 color) {
	//if going to srgb, clamp from 0 to 1.
	color = clamp(color, vec3(0.0), vec3(1.0));
	const vec3 a = vec3(0.055f);
	return mix((vec3(1.0f) + a) * pow(color.rgb, vec3(1.0f / 2.4f)) - a, 12.92f * color.rgb, lessThan(color.rgb, vec3(0.0031308f)));
}

ivec3 mul64(ivec3 a, ivec3 b,int fp_shift) {

	ivec3 lsb, msb;
	imulExtended(a,b,msb,lsb);
	lsb = ivec3(uvec3(lsb)>>fp_shift);
	lsb |= msb << (32 - fp_shift);
	return lsb;
}

bool trace_ray_hdda(vec3 ray_pos, vec3 ray_dir,int p_cascade, out ivec3 r_cell,out ivec3 r_side, out int r_cascade) {

	const int LEVEL_CASCADE = -1;
	const int LEVEL_REGION = 0;
	const int LEVEL_BLOCK = 1;
	const int LEVEL_VOXEL = 2;
	const int MAX_LEVEL = 3;

//#define HQ_RAY

#ifdef HQ_RAY
	const int fp_bits = 16;
#else
	const int fp_bits = 8;
#endif
	const int fp_block_bits = fp_bits + 2;
	const int fp_region_bits = fp_block_bits + 1;
	const int fp_cascade_bits = fp_region_bits + 4;

	bvec3 limit_dir = greaterThan(ray_dir,vec3(0.0));
	ivec3 step = mix(ivec3(0),ivec3(1),limit_dir);
	ivec3 ray_sign = ivec3(sign(ray_dir));

	ivec3 ray_dir_fp = ivec3(ray_dir * float(1<<fp_bits));

#ifdef HQ_RAY
	const float limit = 1.0/65535.0;
#else
	const float limit = 1.0/127.0;
#endif
	bvec3 ray_zero = lessThan(abs(ray_dir),vec3(limit));
	ivec3 inv_ray_dir_fp = ivec3( float(1<<fp_bits) / ray_dir );

	const ivec3 level_masks[MAX_LEVEL]=ivec3[](
		ivec3(1<<fp_region_bits) - ivec3(1),
		ivec3(1<<fp_block_bits) - ivec3(1),
		ivec3(1<<fp_bits) - ivec3(1)
	);

	ivec3 region_offset_mask = (ivec3(params.grid_size) / REGION_SIZE) - ivec3(1);

	ivec3 limits[MAX_LEVEL];

	limits[LEVEL_REGION] = ((ivec3(params.grid_size) << fp_bits) - ivec3(1)) * step; // Region limit does not change, so initialize now.

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
			if (any(lessThan(pos,ivec3(0))) || any(greaterThanEqual(pos,ivec3(params.grid_size)<<fp_bits))) {
				// Outside this cascade, go to next.
				continue;
			}

			cascade_base = ivec3(0,int(params.grid_size.y/REGION_SIZE) * cascade , 0);
			level = LEVEL_REGION;
			continue;
		}

		// Fixed point, multi-level DDA.

		ivec3 mask = level_masks[level];
		ivec3 box = mask * step;
		ivec3 pos_diff = box - (pos & mask);
#ifdef HQ_RAY
		ivec3 mul_res = mul64(pos_diff,inv_ray_dir_fp,fp_bits);
#else
		ivec3 mul_res = (pos_diff * inv_ray_dir_fp) >> fp_bits;
#endif
		ivec3 tv = mix(mul_res,ivec3(0x7FFFFFFF),ray_zero);
		int t = min(tv.x,min(tv.y,tv.z));

		// The general idea here is that we _always_ need to increment to the closest next cell
		// (this is a DDA after all), so adv_box forces this increment for the minimum axis.

		ivec3 adv_box = pos_diff + ray_sign;
#ifdef HQ_RAY
		ivec3 adv_t = mul64(ray_dir_fp, ivec3(t), fp_bits);
#else
		ivec3 adv_t = (ray_dir_fp * t) >> fp_bits;
#endif
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
#ifdef HQ_RAY
		ivec3 mul_res = mul64(pos_diff,-inv_ray_dir_fp,fp_bits);
#else
		ivec3 mul_res = (pos_diff * -inv_ray_dir_fp);
#endif

		ivec3 tv = mix(mul_res,ivec3(0x7FFFFFFF),ray_zero);

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


#define PROBE_CELLS 8


ivec3 modi(ivec3 value, ivec3 p_y) {
	return ((value % p_y) + p_y) % p_y;
}

ivec2 probe_to_tex(ivec3 local_probe,int p_cascade) {

	ivec3 probe_axis_size = ivec3(params.grid_size) / PROBE_CELLS + ivec3(1);
	ivec3 cell = modi( cascades.data[p_cascade].region_world_offset + local_probe,probe_axis_size);
	return cell.xy + ivec2(0,cell.z * int(probe_axis_size.y));

}

vec2 octahedron_wrap(vec2 v) {
	vec2 signVal;
	signVal.x = v.x >= 0.0 ? 1.0 : -1.0;
	signVal.y = v.y >= 0.0 ? 1.0 : -1.0;
	return (1.0 - abs(v.yx)) * signVal;
}

vec2 octahedron_encode(vec3 n) {
	// https://twitter.com/Stubbesaurus/status/937994790553227264
	n /= (abs(n.x) + abs(n.y) + abs(n.z));
	n.xy = n.z >= 0.0 ? n.xy : octahedron_wrap(n.xy);
	n.xy = n.xy * 0.5 + 0.5;
	return n.xy;
}

#define OCC8_DISTANCE_MAX 15.0
#define OCC16_DISTANCE_MAX 256.0

void main() {
	// Pixel being shaded
	ivec2 screen_pos = ivec2(gl_GlobalInvocationID.xy);
	if (any(greaterThanEqual(screen_pos, params.screen_size))) { //too large, do nothing
		return;
	}

	vec3 ray_pos;
	vec3 ray_dir;
	vec3 cam_dir;
	{
		ray_pos = vec3(params.cam_origin[0], params.cam_origin[1], params.cam_origin[2]);

		ray_dir.xy = ((vec2(screen_pos) / vec2(params.screen_size)) * 2.0 - 1.0);
		ray_dir.z = params.z_near;

		ray_dir = (vec4(ray_dir, 1.0) * mat4(params.inv_projection)).xyz;

		mat3 cam_basis;
		{
			vec3 c0 = vec3(params.cam_basis[0][0], params.cam_basis[0][1], params.cam_basis[0][2]);
			vec3 c1 = vec3(params.cam_basis[1][0], params.cam_basis[1][1], params.cam_basis[1][2]);
			vec3 c2 = vec3(params.cam_basis[2][0], params.cam_basis[2][1], params.cam_basis[2][2]);
			cam_basis = mat3(c0, c1, c2);
		}
		ray_dir = normalize(cam_basis * ray_dir);
		cam_dir = vec3(params.cam_basis[2][0],params.cam_basis[2][1],params.cam_basis[2][2]);
	}


	ray_pos.y *= params.y_mult;
	ray_dir.y *= params.y_mult;
	ray_dir = normalize(ray_dir);


	vec3 light = vec3(0.0);
	ivec3 hit_cell;
	vec3 hit_uvw;
	int hit_cascade = 0;

	ivec3 hit_face;

	if (trace_ray_hdda(ray_pos, ray_dir,0,hit_cell,hit_face,hit_cascade)) {

		ivec3 read_cell = (hit_cell + hit_face + (cascades.data[hit_cascade].region_world_offset * REGION_SIZE)) & (ivec3(params.grid_size) - 1);
		light = texelFetch(sampler3D(light_cascades, linear_sampler), read_cell + ivec3(0,(params.grid_size.y * hit_cascade),0), 0).rgb;
		//light = vec3(abs(hit_face));

		if (false) {
			// compute occlusion

			int cascade = hit_cascade;
			hit_cell.y%=int(params.grid_size.y);

			ivec3 pos = hit_cell + hit_face;

			ivec3 base_probe = pos / PROBE_CELLS;
			vec3 posf = vec3(pos) + 0.5; // Actual point in the center of the box.

			ivec3 probe_axis_size = ivec3(params.grid_size) / PROBE_CELLS + ivec3(1);

			vec2 occ_probe_tex_to_uv = 1.0 / vec2( (OCCLUSION_OCT_SIZE+2) * probe_axis_size.x, (OCCLUSION_OCT_SIZE+2) * probe_axis_size.y * probe_axis_size.z );

			vec4 accum_light = vec4(0.0);

			vec2 light_probe_tex_to_uv = 1.0 / vec2( (LIGHTPROBE_OCT_SIZE+2) * probe_axis_size.x, (LIGHTPROBE_OCT_SIZE+2) * probe_axis_size.y * probe_axis_size.z );
			vec2 light_uv = octahedron_encode(vec3(hit_face)) * float(LIGHTPROBE_OCT_SIZE);

			for(int i=0;i<8;i++) {
				ivec3 probe = base_probe + ((ivec3(i) >> ivec3(0, 1, 2)) & ivec3(1, 1, 1));

				vec3 probe_pos = vec3(probe * PROBE_CELLS);



				vec3 probe_to_pos = posf - probe_pos;
				vec3 n = normalize(probe_to_pos);
				float d = length(probe_to_pos);

				float weight = 1.0;
				weight *= pow(max(0.0001, (dot(-n, vec3(hit_face)) + 1.0) * 0.5),2.0) + 0.2;


				ivec2 tex_pos = probe_to_tex(probe,cascade);
				vec2 tex_uv = vec2(ivec2(tex_pos * (OCCLUSION_OCT_SIZE+2) + ivec2(1))) + octahedron_encode(n) * float(OCCLUSION_OCT_SIZE);
				tex_uv *= occ_probe_tex_to_uv;
				vec2 o_o2 = texture(sampler2DArray(occlusion_probes,linear_sampler),vec3(tex_uv,float(cascade))).rg * OCC16_DISTANCE_MAX;

				float mean = o_o2.x;
				float variance = abs((mean*mean) - o_o2.y);

				 // http://www.punkuser.net/vsm/vsm_paper.pdf; equation 5
				 // Need the max in the denominator because biasing can cause a negative displacement
				float dmean = max(d - mean, 0.0);
				float chebyshev_weight = variance / (variance + dmean*dmean);

				chebyshev_weight = max(pow(chebyshev_weight,3.0), 0.0);

				weight *= (d <= mean) ? 1.0 : chebyshev_weight;

				weight = max(0.000001, weight); // make sure not zero (only trilinear can be zero)

				const float crushThreshold = 0.2;
				if (weight < crushThreshold) {
				      weight *= weight * weight * (1.0 / pow(crushThreshold,2.0));
				}

				vec3 trilinear = vec3(1.0) - abs(probe_to_pos / float(PROBE_CELLS));

				weight *= trilinear.x * trilinear.y * trilinear.z;

				tex_uv = vec2(ivec2(tex_pos * (LIGHTPROBE_OCT_SIZE+2) + ivec2(1))) + light_uv;
				tex_uv *= light_probe_tex_to_uv;

				vec3 probe_light = texture(sampler2DArray(light_probes,linear_sampler),vec3(tex_uv,float(cascade))).rgb;

				accum_light+=vec4(probe_light,1.0) * weight;
			}

			light += accum_light.rgb / accum_light.a;

		}

		//light = abs(hit_normal);//texelFetch(sampler3D(light_cascades, linear_sampler), hit_cell + normal_ofs,0).rgb;
	}


	imageStore(screen_buffer, screen_pos, vec4(linear_to_srgb(light), 1.0));
}
