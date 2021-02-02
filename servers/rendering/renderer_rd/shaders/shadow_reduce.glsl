#[compute]

#version 450

VERSION_DEFINES

#define BLOCK_SIZE 8

#ifdef MODE_REDUCE_SUBGROUP

#extension GL_KHR_shader_subgroup_ballot : enable
#extension GL_KHR_shader_subgroup_arithmetic : enable

//nvidia friendly, max 32
layout(local_size_x = 8, local_size_y = 4, local_size_z = 1) in;

#else

layout(local_size_x = BLOCK_SIZE, local_size_y = BLOCK_SIZE, local_size_z = 1) in;

#endif

#ifdef MODE_REDUCE

shared float tmp_data[BLOCK_SIZE * BLOCK_SIZE];
const uint swizzle_table[BLOCK_SIZE] = uint[](0, 4, 2, 6, 1, 5, 3, 7);
const uint unswizzle_table[BLOCK_SIZE] = uint[](0, 0, 0, 1, 0, 2, 1, 3);

#endif

#if defined(MODE_REDUCE) || defined(MODE_REDUCE_SUBGROUP)
layout(set = 0, binding = 0) uniform sampler2D source_depth;
#else
layout(r16, set = 0, binding = 0) uniform restrict readonly image2D source_depth;
#endif
layout(r16, set = 1, binding = 0) uniform restrict writeonly image2D dst_depth;

layout(push_constant, binding = 1, std430) uniform Params {
	ivec2 source_size;
	ivec2 source_offset;
	uint min_size;
	uint gaussian_kernel_version;
	ivec2 filter_dir;
}
params;

void main() {
#ifdef MODE_REDUCE_SUBGROUP

	uvec2 local_pos = gl_LocalInvocationID.xy;
	ivec2 image_offset = params.source_offset;
	ivec2 image_pos = image_offset + ivec2(gl_GlobalInvocationID.xy * ivec2(1, 2));

	float depth = texelFetch(source_depth, min(image_pos, params.source_size - ivec2(1)), 0).r;
	depth += texelFetch(source_depth, min(image_pos + ivec2(0, 1), params.source_size - ivec2(1)), 0).r;
	depth *= 0.5;

#ifdef MODE_REDUCE_8
	//fast version, reduce all
	float depth_average = subgroupAdd(depth) / 32.0;
	if (local_pos == uvec2(0)) {
		imageStore(dst_depth, image_pos / 8, vec4(depth_average));
	}
#else
	//bit slower version, reduce by regions
	uint group_size = (8 / params.min_size);
	uvec2 group_id = local_pos / (8 / params.min_size);

	uvec4 mask;
	float depth_average = 0;

	while (true) {
		uvec2 first = subgroupBroadcastFirst(group_id);
		mask = subgroupBallot(first == group_id);
		if (first == group_id) {
			depth_average = subgroupAdd(depth);
			break;
		}
	}

	depth_average /= float(group_size * group_size);

	if (local_pos == group_id) {
		imageStore(dst_depth, image_pos / int(group_size), vec4(depth_average));
	}
#endif

#endif

#ifdef MODE_REDUCE

	uvec2 pos = gl_LocalInvocationID.xy;

	ivec2 image_offset = params.source_offset;
	ivec2 image_pos = image_offset + ivec2(gl_GlobalInvocationID.xy);
	uint dst_t = swizzle_table[pos.y] * BLOCK_SIZE + swizzle_table[pos.x];
	tmp_data[dst_t] = texelFetch(source_depth, min(image_pos, params.source_size - ivec2(1)), 0).r;
	ivec2 image_size = params.source_size;

	uint t = pos.y * BLOCK_SIZE + pos.x;

	//neighbours
	uint size = BLOCK_SIZE;

	do {
		groupMemoryBarrier();
		barrier();

		size >>= 1;
		image_size >>= 1;
		image_offset >>= 1;

		if (all(lessThan(pos, uvec2(size)))) {
			uint nx = t + size;
			uint ny = t + (BLOCK_SIZE * size);
			uint nxy = ny + size;

			tmp_data[t] += tmp_data[nx];
			tmp_data[t] += tmp_data[ny];
			tmp_data[t] += tmp_data[nxy];
			tmp_data[t] /= 4.0;
		}

	} while (size > params.min_size);

	if (all(lessThan(pos, uvec2(size)))) {
		image_pos = ivec2(unswizzle_table[size + pos.x], unswizzle_table[size + pos.y]);
		image_pos += image_offset + ivec2(gl_WorkGroupID.xy) * int(size);

		image_size = max(ivec2(1), image_size); //in case image size became 0

		if (all(lessThan(image_pos, uvec2(image_size)))) {
			imageStore(dst_depth, image_pos, vec4(tmp_data[t]));
		}
	}
#endif

#ifdef MODE_FILTER

	ivec2 image_pos = params.source_offset + ivec2(gl_GlobalInvocationID.xy);
	if (any(greaterThanEqual(image_pos, params.source_size))) {
		return;
	}

	ivec2 clamp_min = ivec2(params.source_offset);
	ivec2 clamp_max = ivec2(params.source_size) - 1;

	//gaussian kernel, size 9, sigma 4
	const int kernel_size = 9;
	const float gaussian_kernel[kernel_size * 3] = float[](
			0.000229, 0.005977, 0.060598, 0.241732, 0.382928, 0.241732, 0.060598, 0.005977, 0.000229,
			0.028532, 0.067234, 0.124009, 0.179044, 0.20236, 0.179044, 0.124009, 0.067234, 0.028532,
			0.081812, 0.101701, 0.118804, 0.130417, 0.134535, 0.130417, 0.118804, 0.101701, 0.081812);
	float accum = 0.0;
	for (int i = 0; i < kernel_size; i++) {
		ivec2 ofs = clamp(image_pos + params.filter_dir * (i - kernel_size / 2), clamp_min, clamp_max);
		accum += imageLoad(source_depth, ofs).r * gaussian_kernel[params.gaussian_kernel_version + i];
	}

	imageStore(dst_depth, image_pos, vec4(accum));

#endif
}
