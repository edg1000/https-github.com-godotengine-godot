[vertex]

layout (location = 0) in highp vec4 vertex_attrib;
layout (location = 4) in vec2 uv_in;

out vec2 uv_interp;

void main()
{
	gl_Position = vertex_attrib;

	uv_interp = uv_in;

	#ifdef V_FLIP
		uv_interp.y = 1.0f - uv_interp.y;
	#endif
}

[fragment]

#if !defined(GLES_OVER_GL)
	precision mediump float;
#endif

in vec2 uv_interp;

uniform highp sampler2D source; //texunit:0

uniform float exposure;
uniform float white;

#ifdef USE_AUTO_EXPOSURE
	uniform highp sampler2D source_auto_exposure; //texunit:1
	uniform highp float auto_exposure_grey;
#endif

#if defined(USE_GLOW_LINEAR_ADD) || defined(USE_GLOW_LINEAR_MIX)
	#define BLEND_GLOW_LINEAR // use "linear blending" codepath, otherwise blend using srgb modes
#endif

#if defined(USE_GLOW_LEVEL1) || defined(USE_GLOW_LEVEL2) || defined(USE_GLOW_LEVEL3) || defined(USE_GLOW_LEVEL4) || defined(USE_GLOW_LEVEL5) || defined(USE_GLOW_LEVEL6) || defined(USE_GLOW_LEVEL7)
	#define USING_GLOW // only use glow when at least one glow level is selected

	uniform float glow_normalization_factor;

	uniform highp sampler2D source_glow; //texunit:2
	uniform highp float glow_blend_intensity;
#endif

#if defined(USE_REINHARD_TONEMAPPER) || defined(USE_FILMIC_TONEMAPPER) || defined(USE_ACES_TONEMAPPER) || defined(USE_FILMIC_SIMPLE_TONEMAPPER) || defined(USE_UNCHARTED_TONEMAPPER) // any tonemapping?
	#define USE_TONEMAPPER
#endif

#ifdef USE_BCS
	uniform vec3 bcs;
#endif

#ifdef USE_COLOR_CORRECTION
	uniform sampler2D color_correction; //texunit:3
#endif

layout (location = 0) out vec4 frag_color;

#ifdef USE_GLOW_FILTER_BICUBIC
	// w0, w1, w2, and w3 are the four cubic B-spline basis functions
	float w0(float a)
	{
		return (1.0f / 6.0f) * (a * (a * (-a + 3.0f) - 3.0f) + 1.0f);
	}

	float w1(float a)
	{
		return (1.0f / 6.0f) * (a * a * (3.0f * a - 6.0f) + 4.0f);
	}

	float w2(float a)
	{
		return (1.0f / 6.0f) * (a * (a * (-3.0f * a + 3.0f) + 3.0f) + 1.0f);
	}

	float w3(float a)
	{
		return (1.0f / 6.0f) * (a * a * a);
	}

	// g0 and g1 are the two amplitude functions
	float g0(float a)
	{
		return w0(a) + w1(a);
	}

	float g1(float a)
	{
		return w2(a) + w3(a);
	}

	// h0 and h1 are the two offset functions
	float h0(float a)
	{
		return -1.0f + w1(a) / (w0(a) + w1(a));
	}

	float h1(float a)
	{
		return 1.0f + w3(a) / (w2(a) + w3(a));
	}

	uniform ivec2 glow_texture_size;

	vec4 texture2D_bicubic(sampler2D tex, vec2 uv, int p_lod)
	{
		float lod = float(p_lod);
		vec2 tex_size = vec2(glow_texture_size >> p_lod);
		vec2 pixel_size = vec2(1.0f) / tex_size;

		uv = uv * tex_size + vec2(0.5f);

		vec2 iuv = floor(uv);
		vec2 fuv = fract(uv);

		float g0x = g0(fuv.x);
		float g1x = g1(fuv.x);
		float h0x = h0(fuv.x);
		float h1x = h1(fuv.x);
		float h0y = h0(fuv.y);
		float h1y = h1(fuv.y);

		vec2 p0 = (vec2(iuv.x + h0x, iuv.y + h0y) - vec2(0.5f)) * pixel_size;
		vec2 p1 = (vec2(iuv.x + h1x, iuv.y + h0y) - vec2(0.5f)) * pixel_size;
		vec2 p2 = (vec2(iuv.x + h0x, iuv.y + h1y) - vec2(0.5f)) * pixel_size;
		vec2 p3 = (vec2(iuv.x + h1x, iuv.y + h1y) - vec2(0.5f)) * pixel_size;

		return g0(fuv.y) * (g0x * textureLod(tex, p0,lod) +
			g1x * textureLod(tex, p1,lod)) +
			g1(fuv.y) * (g0x * textureLod(tex, p2,lod) +
			g1x * textureLod(tex, p3,lod));
	}

	#define GLOW_TEXTURE_SAMPLE(m_tex, m_uv, m_lod) texture2D_bicubic(m_tex, m_uv, m_lod)
#else
	#define GLOW_TEXTURE_SAMPLE(m_tex, m_uv, m_lod) textureLod(m_tex, m_uv, float(m_lod))
#endif

vec3 linear_to_srgb(vec3 color) // convert linear rgb to srgb, assumes clamped input in range [0;1]
{
	const vec3 a = vec3(0.055f);
	return mix((vec3(1.0f) + a) * pow(color.rgb, vec3(1.0f / 2.4f)) - a, 12.92f * color.rgb, lessThan(color.rgb, vec3(0.0031308f)));
}

vec3 tonemap_filmic(vec3 color, float white)
{
	const float A = 0.15f;
	const float B = 0.50f;
	const float C = 0.10f;
	const float D = 0.20f;
	const float E = 0.02f;
	const float F = 0.30f;
	const float W = 11.2f;
	const float exposure_bias = 20.0f; // rather aggressive but needed to make use of the filmic curve

	color *= exposure_bias;
	white *= exposure_bias;

	vec3 a_color = A * color;
	float a_white = A * white;

	vec3 color_tonemapped = ((color * (a_color + C * B) + D * E) / (color * (a_color + B) + D * F)) - E / F;
	float white_tonemapped = ((white * (a_white + C * B) + D * E) / (white * (a_white + B) + D * F)) - E / F;

	return clamp(color_tonemapped / white_tonemapped, vec3(0.0f), vec3(1.0f)); // srgb conversion seems to be baked into the constants
}

vec3 tonemap_aces(vec3 color, float white)
{
	const float A = 2.51f;
	const float B = 0.03f;
	const float C = 2.43f;
	const float D = 0.59f;
	const float E = 0.14f;

	vec3 color_tonemapped = (color * (A * color + B)) / (color * (C * color + D) + E);
	float white_tonemapped = (white * (A * white + B)) / (white * (C * white + D) + E);

	return linear_to_srgb(clamp(color_tonemapped / white_tonemapped, vec3(0.0f), vec3(1.0f)));
}

vec3 tonemap_filmic_simple(vec3 color, float white) // simplified algorithm from the uncharted 2 paper, does not require gamma correction!
{
	const float A = 6.2f;
	const float B = 0.5f;
	const float C = 1.7f;
	const float D = 0.06f;

	color = max(color - vec3(0.004f), 0.0f); // black clip

	vec3 a_color = A * color;
	float a_white = A * white;

	vec3 color_tonemapped = (color * (a_color + B)) / (color * (a_color + C) + D);
	float white_tonemapped = (white * (a_white + B)) / (white * (a_white + C) + D);

	return clamp(color_tonemapped / white_tonemapped, vec3(0.0f), vec3(1.0f));
}

vec3 tonemap_uncharted(vec3 color, float white) // widely used uncharted 2 filmic tonemapping
{
	const float A = 0.22f;
	const float B = 0.30f;
	const float C = 0.10f;
	const float D = 0.20f;
	const float E = 0.01f;
	const float F = 0.30f;

	const float ExposureBias = 2.0f;

	color *= vec3(ExposureBias);
	white *= ExposureBias;

	vec3 color_tonemapped = ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
	float white_tonemapped = ((white * (A * white + C * B) + D * E) / (white * (A * white + B) + D * F)) - E / F;

	return linear_to_srgb(clamp(color_tonemapped / white_tonemapped, vec3(0.0f), vec3(1.0f)));
}

vec3 tonemap_reinhard(vec3 color, float white)
{
	return linear_to_srgb(clamp((white + vec3(1.0f)) * color / ((color + vec3(1.0f)) * white), vec3(0.0f), vec3(1.0f)));
}

vec3 tonemap_linear(vec3 color)
{
	return linear_to_srgb(clamp(color, 0.0f, 1.0f));
}

vec3 apply_tonemapping(vec3 color, float white) // inputs are LINEAR, always outputs clamped [0;1] color
{
	#ifdef USE_TONEMAPPER
		#ifdef USE_FILMIC_SATURATION
			const float filmic_sat_desaturate = 0.05f;
			const float filmic_sat_resaturate = -0.125f; // overexaggerate a little to compensate for saturation loss on tonemap

			color = mix(color, vec3((color.r + color.g + color.b) * 0.33333f), filmic_sat_desaturate);
		#endif

		#ifdef USE_REINHARD_TONEMAPPER
			color = tonemap_reinhard(color, white);
		#endif

		#ifdef USE_FILMIC_TONEMAPPER
			color = tonemap_filmic(color, white);
		#endif

		#ifdef USE_FILMIC_SIMPLE_TONEMAPPER
			color = tonemap_filmic_simple(color, white);
		#endif

		#ifdef USE_ACES_TONEMAPPER
			color = tonemap_aces(color, white);
		#endif

		#ifdef USE_UNCHARTED_TONEMAPPER
			color = tonemap_uncharted(color, white);
		#endif

		#ifdef BLEND_GLOW_LINEAR
			// don't resaturate for srgb blend modes (because tonemapping is done twice there, this would result in too much saturation)

			#ifdef USE_FILMIC_SATURATION
				return mix(color, vec3((color.r + color.g + color.b) * 0.33333f), filmic_sat_resaturate);
			#endif
		#endif
	#else
		color = tonemap_linear(color); // no other seleced -> linear
	#endif

	return color;
}

#ifdef USING_GLOW
	vec3 gather_glow(sampler2D tex, vec2 uv) // sample all selected glow levels
	{
		vec3 glow = vec3(0.0f);

		#ifdef USE_GLOW_LEVEL1
			glow += GLOW_TEXTURE_SAMPLE(tex, uv, 1).rgb;
		#endif

		#ifdef USE_GLOW_LEVEL2
			glow += GLOW_TEXTURE_SAMPLE(tex, uv, 2).rgb;
		#endif

		#ifdef USE_GLOW_LEVEL3
			glow += GLOW_TEXTURE_SAMPLE(tex, uv, 3).rgb;
		#endif

		#ifdef USE_GLOW_LEVEL4
			glow += GLOW_TEXTURE_SAMPLE(tex, uv, 4).rgb;
		#endif

		#ifdef USE_GLOW_LEVEL5
			glow += GLOW_TEXTURE_SAMPLE(tex, uv, 5).rgb;
		#endif

		#ifdef USE_GLOW_LEVEL6
			glow += GLOW_TEXTURE_SAMPLE(tex, uv, 6).rgb;
		#endif

		#ifdef USE_GLOW_LEVEL7
			glow += GLOW_TEXTURE_SAMPLE(tex, uv, 7).rgb;
		#endif

		return glow * glow_normalization_factor;
	}

	vec3 apply_glow(vec3 color, vec3 glow) // apply srgb glow using the selected blending mode
	{
		#ifdef USE_GLOW_REPLACE
			color = glow;
		#endif

		#ifdef USE_GLOW_SCREEN
			color = max((color + glow) - (color * glow), vec3(0.0));
		#endif

		#ifdef USE_GLOW_SOFTLIGHT
			/* old mode, replaced because it's fairly complex and uses a lot of branching
			glow = glow * vec3(0.5f) + vec3(0.5f);

			color.r = (glow.r <= 0.5f) ? (color.r - (1.0f - 2.0f * glow.r) * color.r * (1.0f - color.r)) : (((glow.r > 0.5f) && (color.r <= 0.25f)) ? (color.r + (2.0f * glow.r - 1.0f) * (4.0f * color.r * (4.0f * color.r + 1.0f) * (color.r - 1.0f) + 7.0f * color.r)) : (color.r + (2.0f * glow.r - 1.0f) * (sqrt(color.r) - color.r)));
			color.g = (glow.g <= 0.5f) ? (color.g - (1.0f - 2.0f * glow.g) * color.g * (1.0f - color.g)) : (((glow.g > 0.5f) && (color.g <= 0.25f)) ? (color.g + (2.0f * glow.g - 1.0f) * (4.0f * color.g * (4.0f * color.g + 1.0f) * (color.g - 1.0f) + 7.0f * color.g)) : (color.g + (2.0f * glow.g - 1.0f) * (sqrt(color.g) - color.g)));
			color.b = (glow.b <= 0.5f) ? (color.b - (1.0f - 2.0f * glow.b) * color.b * (1.0f - color.b)) : (((glow.b > 0.5f) && (color.b <= 0.25f)) ? (color.b + (2.0f * glow.b - 1.0f) * (4.0f * color.b * (4.0f * color.b + 1.0f) * (color.b - 1.0f) + 7.0f * color.b)) : (color.b + (2.0f * glow.b - 1.0f) * (sqrt(color.b) - color.b)));
			*/

			color = color + glow.rgb * (1.0f - color * color) * 0.25f;
		#endif

		#if !defined(USE_GLOW_SCREEN) && !defined(USE_GLOW_SOFTLIGHT) && !defined(USE_GLOW_REPLACE) // no other selected -> additive
			color += glow;
		#endif

		return color;
	}

	vec3 apply_glow_linear(vec3 color, vec3 glow, float glow_blend_intensity) // apply linear glow using the selected blending mode (requires glow intensity param because of "mix"-mode
	{
		#ifdef USE_GLOW_LINEAR_ADD
			color = color + glow * vec3(glow_blend_intensity);
		#endif

		#ifdef USE_GLOW_LINEAR_MIX
			color = mix(color, glow, vec3(glow_blend_intensity));
		#endif

		return color;
	}
#endif

vec3 apply_bcs(vec3 color, vec3 bcs)
{
	color = mix(vec3(0.0f), color, bcs.x);
	color = mix(vec3(0.5f), color, bcs.y);
	color = mix(vec3(dot(vec3(1.0f), color) * 0.33333f), color, bcs.z);

	return color;
}

vec3 apply_color_correction(vec3 color, sampler2D correction_tex)
{
	color.r = texture(correction_tex, vec2(color.r, 0.0f)).r;
	color.g = texture(correction_tex, vec2(color.g, 0.0f)).g;
	color.b = texture(correction_tex, vec2(color.b, 0.0f)).b;

	return color;
}

void main()
{
	vec3 color = textureLod(source, uv_interp, 0.0f).rgb;

	// Exposure

	#ifdef USE_AUTO_EXPOSURE
		color /= texelFetch(source_auto_exposure, ivec2(0, 0), 0).r / auto_exposure_grey;
	#endif

	color *= exposure;

	#ifndef BLEND_GLOW_LINEAR
		// tonemap and then blend glow in srgb space

		// Early Tonemap & SRGB Conversion

		#ifdef KEEP_3D_LINEAR
			// leave color as is (-> don't convert to SRGB)
		#else
			color = apply_tonemapping(color, white);
		#endif

		// Glow

		#ifdef USING_GLOW
			vec3 glow = gather_glow(source_glow, uv_interp) * glow_blend_intensity;

			// high dynamic range -> SRGB
			glow = apply_tonemapping(glow, white);

			color = apply_glow(color, glow);
		#endif
	#else
		// blend glow in linear space and tonemap result afterwards ("physically correct")

		// Glow

		#ifdef USING_GLOW
			vec3 glow = gather_glow(source_glow, uv_interp);

			color = apply_glow_linear(color, glow, glow_blend_intensity);
		#endif

		// Late Tonemap & SRGB Conversion

		#ifdef KEEP_3D_LINEAR
			// leave color as is (-> don't convert to SRGB)
		#else
			color = apply_tonemapping(color, white);
		#endif
	#endif

	// Additional effects

	#ifdef USE_BCS
		color = apply_bcs(color, bcs);
	#endif

	#ifdef USE_COLOR_CORRECTION
		color = apply_color_correction(color, color_correction);
	#endif

	frag_color = vec4(color, 1.0f);
}
