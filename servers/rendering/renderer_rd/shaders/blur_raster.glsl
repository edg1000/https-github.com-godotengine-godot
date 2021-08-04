/* clang-format off */
#[vertex]

#version 450

#VERSION_DEFINES

#include "blur_raster_inc.glsl"

layout(location = 0) out vec2 uv_interp;
/* clang-format on */

void main() {
	vec2 base_arr[4] = vec2[](vec2(0.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 1.0), vec2(1.0, 0.0));
	uv_interp = base_arr[gl_VertexIndex];

	gl_Position = vec4(uv_interp * 2.0 - 1.0, 0.0, 1.0);
}

/* clang-format off */
#[fragment]

#version 450

#VERSION_DEFINES

#include "blur_raster_inc.glsl"

layout(location = 0) in vec2 uv_interp;
/* clang-format on */

layout(set = 0, binding = 0) uniform sampler2D source_color;

#ifdef GLOW_USE_AUTO_EXPOSURE
layout(set = 1, binding = 0) uniform sampler2D source_auto_exposure;
#endif

layout(location = 0) out vec4 frag_color;

//DOF
#ifdef MODE_DOF_BLUR

layout(set = 1, binding = 0) uniform sampler2D dof_source_depth;

#ifdef DOF_QUALITY_LOW
const int dof_kernel_size = 5;
const int dof_kernel_from = 2;
const float dof_kernel[5] = float[](0.153388, 0.221461, 0.250301, 0.221461, 0.153388);
#endif

#ifdef DOF_QUALITY_MEDIUM
const int dof_kernel_size = 11;
const int dof_kernel_from = 5;
const float dof_kernel[11] = float[](0.055037, 0.072806, 0.090506, 0.105726, 0.116061, 0.119726, 0.116061, 0.105726, 0.090506, 0.072806, 0.055037);

#endif

#ifdef DOF_QUALITY_HIGH
const int dof_kernel_size = 21;
const int dof_kernel_from = 10;
const float dof_kernel[21] = float[](0.028174, 0.032676, 0.037311, 0.041944, 0.046421, 0.050582, 0.054261, 0.057307, 0.059587, 0.060998, 0.061476, 0.060998, 0.059587, 0.057307, 0.054261, 0.050582, 0.046421, 0.041944, 0.037311, 0.032676, 0.028174);
#endif

#endif

void main() {
#ifdef MODE_MIPMAP

	vec2 pix_size = blur.pixel_size;
	vec4 color = texture(source_color, uv_interp + vec2(-0.5, -0.5) * pix_size);
	color += texture(source_color, uv_interp + vec2(0.5, -0.5) * pix_size);
	color += texture(source_color, uv_interp + vec2(0.5, 0.5) * pix_size);
	color += texture(source_color, uv_interp + vec2(-0.5, 0.5) * pix_size);
	frag_color = color / 4.0;

#endif

#ifdef MODE_GAUSSIAN_BLUR

	//Simpler blur uses SIGMA2 for the gaussian kernel for a stronger effect

	if (bool(blur.flags & FLAG_HORIZONTAL)) {
		vec2 pix_size = blur.pixel_size;
		pix_size *= 0.5; //reading from larger buffer, so use more samples
		vec4 color = texture(source_color, uv_interp + vec2(0.0, 0.0) * pix_size) * 0.214607;
		color += texture(source_color, uv_interp + vec2(1.0, 0.0) * pix_size) * 0.189879;
		color += texture(source_color, uv_interp + vec2(2.0, 0.0) * pix_size) * 0.131514;
		color += texture(source_color, uv_interp + vec2(3.0, 0.0) * pix_size) * 0.071303;
		color += texture(source_color, uv_interp + vec2(-1.0, 0.0) * pix_size) * 0.189879;
		color += texture(source_color, uv_interp + vec2(-2.0, 0.0) * pix_size) * 0.131514;
		color += texture(source_color, uv_interp + vec2(-3.0, 0.0) * pix_size) * 0.071303;
		frag_color = color;
	} else {
		vec2 pix_size = blur.pixel_size;
		vec4 color = texture(source_color, uv_interp + vec2(0.0, 0.0) * pix_size) * 0.38774;
		color += texture(source_color, uv_interp + vec2(0.0, 1.0) * pix_size) * 0.24477;
		color += texture(source_color, uv_interp + vec2(0.0, 2.0) * pix_size) * 0.06136;
		color += texture(source_color, uv_interp + vec2(0.0, -1.0) * pix_size) * 0.24477;
		color += texture(source_color, uv_interp + vec2(0.0, -2.0) * pix_size) * 0.06136;
		frag_color = color;
	}
#endif

#ifdef MODE_GAUSSIAN_GLOW

	//Glow uses larger sigma 1 for a more rounded blur effect

#define GLOW_ADD(m_ofs, m_mult)                                                  \
	{                                                                            \
		vec2 ofs = uv_interp + m_ofs * pix_size;                                 \
		vec4 c = texture(source_color, ofs) * m_mult;                            \
		if (any(lessThan(ofs, vec2(0.0))) || any(greaterThan(ofs, vec2(1.0)))) { \
			c *= 0.0;                                                            \
		}                                                                        \
		color += c;                                                              \
	}

	if (bool(blur.flags & FLAG_HORIZONTAL)) {
		vec2 pix_size = blur.pixel_size;
		pix_size *= 0.5; //reading from larger buffer, so use more samples
		vec4 color = texture(source_color, uv_interp + vec2(0.0, 0.0) * pix_size) * 0.174938;
		GLOW_ADD(vec2(1.0, 0.0), 0.165569);
		GLOW_ADD(vec2(2.0, 0.0), 0.140367);
		GLOW_ADD(vec2(3.0, 0.0), 0.106595);
		GLOW_ADD(vec2(-1.0, 0.0), 0.165569);
		GLOW_ADD(vec2(-2.0, 0.0), 0.140367);
		GLOW_ADD(vec2(-3.0, 0.0), 0.106595);
		color *= blur.glow_strength;
		frag_color = color;
	} else {
		vec2 pix_size = blur.pixel_size;
		vec4 color = texture(source_color, uv_interp + vec2(0.0, 0.0) * pix_size) * 0.288713;
		GLOW_ADD(vec2(0.0, 1.0), 0.233062);
		GLOW_ADD(vec2(0.0, 2.0), 0.122581);
		GLOW_ADD(vec2(0.0, -1.0), 0.233062);
		GLOW_ADD(vec2(0.0, -2.0), 0.122581);
		color *= blur.glow_strength;
		frag_color = color;
	}

#undef GLOW_ADD

	if (bool(blur.flags & FLAG_GLOW_FIRST_PASS)) {
#ifdef GLOW_USE_AUTO_EXPOSURE

		frag_color /= texelFetch(source_auto_exposure, ivec2(0, 0), 0).r / blur.glow_auto_exposure_grey;
#endif
		frag_color *= blur.glow_exposure;

		float luminance = max(frag_color.r, max(frag_color.g, frag_color.b));
		float feedback = max(smoothstep(blur.glow_hdr_threshold, blur.glow_hdr_threshold + blur.glow_hdr_scale, luminance), blur.glow_bloom);

		frag_color = min(frag_color * feedback, vec4(blur.glow_luminance_cap));
	}

#endif

#ifdef MODE_DOF_BLUR

	vec4 color_accum = vec4(0.0);

	float depth = texture(dof_source_depth, uv_interp, 0.0).r;
	depth = depth * 2.0 - 1.0;

	if (bool(blur.flags & FLAG_USE_ORTHOGONAL_PROJECTION)) {
		depth = ((depth + (blur.camera_z_far + blur.camera_z_near) / (blur.camera_z_far - blur.camera_z_near)) * (blur.camera_z_far - blur.camera_z_near)) / 2.0;
	} else {
		depth = 2.0 * blur.camera_z_near * blur.camera_z_far / (blur.camera_z_far + blur.camera_z_near - depth * (blur.camera_z_far - blur.camera_z_near));
	}

	// mix near and far blur amount
	float amount = 1.0;
	if (bool(blur.flags & FLAG_DOF_FAR)) {
		amount *= 1.0 - smoothstep(blur.dof_far_begin, blur.dof_far_end, depth);
	}
	if (bool(blur.flags & FLAG_DOF_NEAR)) {
		amount *= smoothstep(blur.dof_near_end, blur.dof_near_begin, depth);
	}
	amount = 1.0 - amount;

	if (amount > 0.0) {
		float k_accum = 0.0;

		for (int i = 0; i < dof_kernel_size; i++) {
			int int_ofs = i - dof_kernel_from;
			vec2 tap_uv = uv_interp + blur.dof_dir * float(int_ofs) * amount * blur.dof_radius;

			float tap_k = dof_kernel[i];

			float tap_depth = texture(dof_source_depth, tap_uv, 0.0).r;
			tap_depth = tap_depth * 2.0 - 1.0;

			if (bool(blur.flags & FLAG_USE_ORTHOGONAL_PROJECTION)) {
				tap_depth = ((tap_depth + (blur.camera_z_far + blur.camera_z_near) / (blur.camera_z_far - blur.camera_z_near)) * (blur.camera_z_far - blur.camera_z_near)) / 2.0;
			} else {
				tap_depth = 2.0 * blur.camera_z_near * blur.camera_z_far / (blur.camera_z_far + blur.camera_z_near - tap_depth * (blur.camera_z_far - blur.camera_z_near));
			}

			// mix near and far blur amount
			float tap_amount = 1.0;
			if (bool(blur.flags & FLAG_DOF_FAR)) {
				tap_amount *= mix(1.0 - smoothstep(blur.dof_far_begin, blur.dof_far_end, tap_depth), 0.0, int_ofs == 0);
			}
			if (bool(blur.flags & FLAG_DOF_NEAR)) {
				tap_amount *= mix(smoothstep(blur.dof_near_end, blur.dof_near_begin, tap_depth), 0.0, int_ofs == 0);
			}
			tap_amount = 1.0 - tap_amount;

			tap_amount *= tap_amount * tap_amount; //prevent undesired glow effect

			vec4 tap_color = texture(source_color, tap_uv, 0.0) * tap_k;

			k_accum += tap_k * tap_amount;
			color_accum += tap_color * tap_amount;
		}

		if (k_accum > 0.0) {
			color_accum /= k_accum;
		}

		frag_color = color_accum; ///k_accum;
	} else {
		// we are in focus, don't waste time
		frag_color = texture(source_color, uv_interp, 0.0);
	}

#endif
}
