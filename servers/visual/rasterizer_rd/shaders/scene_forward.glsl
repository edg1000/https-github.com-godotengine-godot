/* clang-format off */
[vertex]
/* clang-format on */

#version 450

/* clang-format off */
VERSION_DEFINES
/* clang-format on */

#include "scene_forward_inc.glsl"


/* INPUT ATTRIBS */

layout(location = 0) in vec3 vertex_attrib;
/* clang-format on */
layout(location = 1) in vec3 normal_attrib;
#if defined(TANGENT_USED) || defined(NORMALMAP_USED) || defined(LIGHT_ANISOTROPY_USED)
layout(location = 2) in vec4 tangent_attrib;
#endif

#if defined(COLOR_USED)
layout(location = 3) in vec4 color_attrib;
#endif

#if defined(UV_USED)
layout(location = 4) in vec2 uv_attrib;
#endif

#if defined(UV2_USED) || defined(USE_LIGHTMAP)
layout(location = 5) in vec2 uv2_attrib;
#endif

layout(location = 6) in uvec4 bone_attrib; // always bound, even if unused

/* Varyings */

layout(location = 0) out vec3 vertex_interp;
layout(location = 1) out vec3 normal_interp;

#if defined(COLOR_USED)
layout(location = 2) out vec4 color_interp;
#endif

#if defined(UV_USED)
layout(location = 3) out vec2 uv_interp;
#endif

#if defined(UV2_USED) || defined(USE_LIGHTMAP)
layout(location = 4) out vec2 uv2_interp;
#endif

#if defined(TANGENT_USED) || defined(NORMALMAP_USED) || defined(LIGHT_ANISOTROPY_USED)
layout(location = 5) out vec3 tangent_interp;
layout(location = 6) out vec3 binormal_interp;
#endif

#ifdef USE_MATERIAL_UNIFORMS
layout(set = 2, binding = 0, std140) uniform MaterialUniforms {
/* clang-format off */
MATERIAL_UNIFORMS
/* clang-format on */
} material;
#endif


/* clang-format off */

VERTEX_SHADER_GLOBALS

/* clang-format on */

// FIXME: This triggers a Mesa bug that breaks rendering, so disabled for now.
// See GH-13450 and https://bugs.freedesktop.org/show_bug.cgi?id=100316
//invariant gl_Position;

layout(location =7) flat out uint instance_index;

#ifdef MODE_DUAL_PARABOLOID

layout(location =8) out float dp_clip;

#endif

void main() {

	instance_index = draw_call.instance_index;
	vec4 instance_custom = vec4(0.0);
#if defined(COLOR_USED)
	color_interp = color_attrib;
#endif

	mat4 world_matrix = instances.data[instance_index].transform;
	mat3 world_normal_matrix= mat3(instances.data[instance_index].normal_transform);

	if (bool(instances.data[instance_index].flags & INSTANCE_FLAGS_MULTIMESH)) {
		//multimesh, instances are for it
		uint offset = (instances.data[instance_index].flags>>INSTANCE_FLAGS_MULTIMESH_STRIDE_SHIFT)&INSTANCE_FLAGS_MULTIMESH_STRIDE_MASK;
		offset*=gl_InstanceIndex;


		mat4 matrix;
		if (bool(instances.data[instance_index].flags & INSTANCE_FLAGS_MULTIMESH_FORMAT_2D)) {
			mat4 matrix = mat4(transforms.data[offset+0],transforms.data[offset+1],vec4(0.0,0.0,1.0,0.0),vec4(0.0,0.0,0.0,1.0));
			offset+=2;
		} else {
			mat4 matrix = mat4(transforms.data[offset+0],transforms.data[offset+1],transforms.data[offset+2],vec4(0.0,0.0,0.0,1.0));
			offset+=3;
		}

		if (bool(instances.data[instance_index].flags & INSTANCE_FLAGS_MULTIMESH_HAS_COLOR)) {
#ifdef COLOR_USED
			color_interp *= transforms.data[offset];
#endif
			offset+=1;
		}

		if (bool(instances.data[instance_index].flags & INSTANCE_FLAGS_MULTIMESH_HAS_CUSTOM_DATA)) {
			instance_custom = transforms.data[offset];
		}

		//transposed, so multiply in opposite order
		world_matrix = matrix * world_matrix;
		world_normal_matrix = mat3(matrix) * world_normal_matrix;

	} else {
		//not a multimesh, instances are for multiple draw calls
		instance_index += gl_InstanceIndex;
	}

	vec3 vertex = vertex_attrib;
	vec3 normal = normal_attrib;

#if defined(TANGENT_USED) || defined(NORMALMAP_USED) || defined(LIGHT_ANISOTROPY_USED)
	vec3 tangent = tangent_attrib.xyz;
	float binormalf = tangent_attrib.a;
#endif

#if defined(TANGENT_USED) || defined(NORMALMAP_USED) || defined(LIGHT_ANISOTROPY_USED)

	vec3 binormal = normalize(cross(normal, tangent) * binormalf);
#endif

#if defined(UV_USED)
	uv_interp = uv_attrib;
#endif

#if defined(UV2_USED) || defined(USE_LIGHTMAP)
	uv2_interp = uv2_attrib;
#endif

#ifdef USE_OVERRIDE_POSITION
	vec4 position;
#endif



	mat4 projection_matrix = scene_data.projection_matrix;

//using world coordinates
#if !defined(SKIP_TRANSFORM_USED) && defined(VERTEX_WORLD_COORDS_USED)

	vertex = (world_matrix * vec4(vertex,1.0)).xyz;

	normal = world_normal_matrix * normal;

#if defined(TANGENT_USED) || defined(NORMALMAP_USED) || defined(LIGHT_ANISOTROPY_USED)

	tangent = world_normal_matrix * tangent;
	binormal = world_normal_matrix * binormal;

#endif
#endif

	float roughness = 1.0;

	mat4 modelview = scene_data.inv_camera_matrix * world_matrix;
	mat3 modelview_normal = mat3(scene_data.inv_camera_matrix) * world_normal_matrix;

	{
		/* clang-format off */

VERTEX_SHADER_CODE

		/* clang-format on */
	}

// using local coordinates (default)
#if !defined(SKIP_TRANSFORM_USED) && !defined(VERTEX_WORLD_COORDS_USED)

	vertex = (modelview * vec4(vertex,1.0)).xyz;
	normal = modelview_normal * normal;
#endif

#if defined(TANGENT_USED) || defined(NORMALMAP_USED) || defined(LIGHT_ANISOTROPY_USED)

	binormal = modelview_normal * binormal;
	tangent = modelview_normal * tangent;
#endif


//using world coordinates
#if !defined(SKIP_TRANSFORM_USED) && defined(VERTEX_WORLD_COORDS_USED)

	vertex = (scene_data.inv_camera_matrix * vec4(vertex,1.0)).xyz;
	normal = mat3(scene_data.inverse_normal_matrix) * normal;

#if defined(TANGENT_USED) || defined(NORMALMAP_USED) || defined(LIGHT_ANISOTROPY_USED)

	binormal = mat3(scene_data.camera_inverse_binormal_matrix) * binormal;
	tangent = mat3(scene_data.camera_inverse_tangent_matrix) * tangent;
#endif
#endif

	vertex_interp = vertex;
	normal_interp = normal;

#if defined(TANGENT_USED) || defined(NORMALMAP_USED) || defined(LIGHT_ANISOTROPY_USED)
	tangent_interp = tangent;
	binormal_interp = binormal;
#endif

#ifdef MODE_RENDER_DEPTH

#ifdef MODE_DUAL_PARABOLOID

	vertex_interp.z *= scene_data.dual_paraboloid_side;
	normal_interp.z *= scene_data.dual_paraboloid_side;

	dp_clip = vertex_interp.z; //this attempts to avoid noise caused by objects sent to the other parabolloid side due to bias

	//for dual paraboloid shadow mapping, this is the fastest but least correct way, as it curves straight edges

	vec3 vtx = vertex_interp + normalize(vertex_interp) * scene_data.z_offset;
	float distance = length(vtx);
	vtx = normalize(vtx);
	vtx.xy /= 1.0 - vtx.z;
	vtx.z = (distance / scene_data.z_far);
	vtx.z = vtx.z * 2.0 - 1.0;

	vertex_interp = vtx;
#else

	float z_ofs = scene_data.z_offset;
	z_ofs += (1.0 - abs(normal_interp.z)) * scene_data.z_slope_scale;
	vertex_interp.z -= z_ofs;

#endif

#endif //MODE_RENDER_DEPTH

#ifdef USE_OVERRIDE_POSITION
	gl_Position = position;;
#else
	gl_Position = projection_matrix * vec4(vertex_interp, 1.0);
#endif

}

/* clang-format off */
[fragment]
/* clang-format on */

#version 450

/* clang-format off */
VERSION_DEFINES
/* clang-format on */

#include "scene_forward_inc.glsl"

/* Varyings */

layout(location = 0) in vec3 vertex_interp;
layout(location = 1) in vec3 normal_interp;

#if defined(COLOR_USED)
layout(location = 2) in vec4 color_interp;
#endif

#if defined(UV_USED)
layout(location = 3) in vec2 uv_interp;
#endif

#if defined(UV2_USED) || defined(USE_LIGHTMAP)
layout(location = 4) in vec2 uv2_interp;
#endif

#if defined(TANGENT_USED) || defined(NORMALMAP_USED) || defined(LIGHT_ANISOTROPY_USED)
layout(location = 5) in vec3 tangent_interp;
layout(location = 6) in vec3 binormal_interp;
#endif

layout(location =7) flat in uint instance_index;

#ifdef MODE_DUAL_PARABOLOID

layout(location =8) in float dp_clip;

#endif


//defines to keep compatibility with vertex

#define world_matrix instances.data[instance_index].transform;
#define world_normal_matrix instances.data[instance_index].normal_transform;
#define projection_matrix scene_data.projection_matrix;

#ifdef USE_MATERIAL_UNIFORMS
layout(set = 2, binding = 0, std140) uniform MaterialUniforms {
/* clang-format off */
MATERIAL_UNIFORMS
/* clang-format on */
} material;
#endif

/* clang-format off */

FRAGMENT_SHADER_GLOBALS

/* clang-format on */

#ifdef MODE_MULTIPLE_RENDER_TARGETS

layout(location = 0) out vec4 diffuse_buffer; //diffuse (rgb) and roughness
layout(location = 1) out vec4 specular_buffer; //specular and SSS (subsurface scatter)
#else

#ifndef MODE_RENDER_DEPTH
layout(location = 0) out vec4 frag_color;
#endif

#endif



// This returns the G_GGX function divided by 2 cos_theta_m, where in practice cos_theta_m is either N.L or N.V.
// We're dividing this factor off because the overall term we'll end up looks like
// (see, for example, the first unnumbered equation in B. Burley, "Physically Based Shading at Disney", SIGGRAPH 2012):
//
//   F(L.V) D(N.H) G(N.L) G(N.V) / (4 N.L N.V)
//
// We're basically regouping this as
//
//   F(L.V) D(N.H) [G(N.L)/(2 N.L)] [G(N.V) / (2 N.V)]
//
// and thus, this function implements the [G(N.m)/(2 N.m)] part with m = L or V.
//
// The contents of the D and G (G1) functions (GGX) are taken from
// E. Heitz, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs", J. Comp. Graph. Tech. 3 (2) (2014).
// Eqns 71-72 and 85-86 (see also Eqns 43 and 80).

float G_GGX_2cos(float cos_theta_m, float alpha) {
	// Schlick's approximation
	// C. Schlick, "An Inexpensive BRDF Model for Physically-based Rendering", Computer Graphics Forum. 13 (3): 233 (1994)
	// Eq. (19), although see Heitz (2014) the about the problems with his derivation.
	// It nevertheless approximates GGX well with k = alpha/2.
	float k = 0.5 * alpha;
	return 0.5 / (cos_theta_m * (1.0 - k) + k);

	// float cos2 = cos_theta_m * cos_theta_m;
	// float sin2 = (1.0 - cos2);
	// return 1.0 / (cos_theta_m + sqrt(cos2 + alpha * alpha * sin2));
}

float D_GGX(float cos_theta_m, float alpha) {
	float alpha2 = alpha * alpha;
	float d = 1.0 + (alpha2 - 1.0) * cos_theta_m * cos_theta_m;
	return alpha2 / (M_PI * d * d);
}

float G_GGX_anisotropic_2cos(float cos_theta_m, float alpha_x, float alpha_y, float cos_phi, float sin_phi) {
	float cos2 = cos_theta_m * cos_theta_m;
	float sin2 = (1.0 - cos2);
	float s_x = alpha_x * cos_phi;
	float s_y = alpha_y * sin_phi;
	return 1.0 / max(cos_theta_m + sqrt(cos2 + (s_x * s_x + s_y * s_y) * sin2), 0.001);
}

float D_GGX_anisotropic(float cos_theta_m, float alpha_x, float alpha_y, float cos_phi, float sin_phi) {
	float cos2 = cos_theta_m * cos_theta_m;
	float sin2 = (1.0 - cos2);
	float r_x = cos_phi / alpha_x;
	float r_y = sin_phi / alpha_y;
	float d = cos2 + sin2 * (r_x * r_x + r_y * r_y);
	return 1.0 / max(M_PI * alpha_x * alpha_y * d * d, 0.001);
}

float SchlickFresnel(float u) {
	float m = 1.0 - u;
	float m2 = m * m;
	return m2 * m2 * m; // pow(m,5)
}

float GTR1(float NdotH, float a) {
	if (a >= 1.0) return 1.0 / M_PI;
	float a2 = a * a;
	float t = 1.0 + (a2 - 1.0) * NdotH * NdotH;
	return (a2 - 1.0) / (M_PI * log(a2) * t);
}

vec3 F0(float metallic, float specular, vec3 albedo) {
	float dielectric = 0.16 * specular * specular;
	// use albedo * metallic as colored specular reflectance at 0 angle for metallic materials;
	// see https://google.github.io/filament/Filament.md.html
	return mix(vec3(dielectric), albedo, vec3(metallic));
}

void light_compute(vec3 N, vec3 L, vec3 V, vec3 light_color, vec3 attenuation, vec3 diffuse_color,float roughness, float metallic, float specular,float specular_blob_intensity,
#ifdef LIGHT_TRANSMISSION_USED
		vec3 transmission,
#endif
#ifdef LIGHT_RIM_USED
		float rim, float rim_tint,
#endif
#ifdef LIGHT_CLEARCOAT_USED
		float clearcoat, float clearcoat_gloss,
#endif
#ifdef LIGHT_ANISOTROPY_USED
		vec3 B, vec3 T,float anisotropy,
#endif
#ifdef USE_SHADOW_TO_OPACITY
		inout float alpha,
#endif
		inout vec3 diffuse_light, inout vec3 specular_light
		   ) {

#if defined(USE_LIGHT_SHADER_CODE)
	// light is written by the light shader

	vec3 normal = N;
	vec3 albedo = diffuse_color;
	vec3 light = L;
	vec3 view = V;

	/* clang-format off */

LIGHT_SHADER_CODE

	/* clang-format on */

#else
	float NdotL = dot(N, L);
	float cNdotL = max(NdotL, 0.0); // clamped NdotL
	float NdotV = dot(N, V);
	float cNdotV = max(NdotV, 0.0);

#if defined(DIFFUSE_BURLEY) || defined(SPECULAR_BLINN) || defined(SPECULAR_SCHLICK_GGX) || defined(LIGHT_CLEARCOAT_USED)
	vec3 H = normalize(V + L);
#endif

#if defined(SPECULAR_BLINN) || defined(SPECULAR_SCHLICK_GGX) || defined(LIGHT_CLEARCOAT_USED)
	float cNdotH = max(dot(N, H), 0.0);
#endif

#if defined(DIFFUSE_BURLEY) || defined(SPECULAR_SCHLICK_GGX) || defined(LIGHT_CLEARCOAT_USED)
	float cLdotH = max(dot(L, H), 0.0);
#endif

	if (metallic < 1.0) {
#if defined(DIFFUSE_OREN_NAYAR)
		vec3 diffuse_brdf_NL;
#else
		float diffuse_brdf_NL; // BRDF times N.L for calculating diffuse radiance
#endif

#if defined(DIFFUSE_LAMBERT_WRAP)
		// energy conserving lambert wrap shader
		diffuse_brdf_NL = max(0.0, (NdotL + roughness) / ((1.0 + roughness) * (1.0 + roughness)));

#elif defined(DIFFUSE_OREN_NAYAR)

		{
			// see http://mimosa-pudica.net/improved-oren-nayar.html
			float LdotV = dot(L, V);

			float s = LdotV - NdotL * NdotV;
			float t = mix(1.0, max(NdotL, NdotV), step(0.0, s));

			float sigma2 = roughness * roughness; // TODO: this needs checking
			vec3 A = 1.0 + sigma2 * (-0.5 / (sigma2 + 0.33) + 0.17 * diffuse_color / (sigma2 + 0.13));
			float B = 0.45 * sigma2 / (sigma2 + 0.09);

			diffuse_brdf_NL = cNdotL * (A + vec3(B) * s / t) * (1.0 / M_PI);
		}

#elif defined(DIFFUSE_TOON)

		diffuse_brdf_NL = smoothstep(-roughness, max(roughness, 0.01), NdotL);

#elif defined(DIFFUSE_BURLEY)

		{
			float FD90_minus_1 = 2.0 * cLdotH * cLdotH * roughness - 0.5;
			float FdV = 1.0 + FD90_minus_1 * SchlickFresnel(cNdotV);
			float FdL = 1.0 + FD90_minus_1 * SchlickFresnel(cNdotL);
			diffuse_brdf_NL = (1.0 / M_PI) * FdV * FdL * cNdotL;
			/*
			float energyBias = mix(roughness, 0.0, 0.5);
			float energyFactor = mix(roughness, 1.0, 1.0 / 1.51);
			float fd90 = energyBias + 2.0 * VoH * VoH * roughness;
			float f0 = 1.0;
			float lightScatter = f0 + (fd90 - f0) * pow(1.0 - cNdotL, 5.0);
			float viewScatter = f0 + (fd90 - f0) * pow(1.0 - cNdotV, 5.0);

			diffuse_brdf_NL = lightScatter * viewScatter * energyFactor;
			*/
		}
#else
		// lambert
		diffuse_brdf_NL = cNdotL * (1.0 / M_PI);
#endif

		diffuse_light += light_color * diffuse_color * diffuse_brdf_NL * attenuation;

#if defined(LIGHT_TRANSMISSION_USED)
		diffuse_light += light_color * diffuse_color * (vec3(1.0 / M_PI) - diffuse_brdf_NL) * transmission * attenuation;
#endif

#if defined(LIGHT_RIM_USED)
		float rim_light = pow(max(0.0, 1.0 - cNdotV), max(0.0, (1.0 - roughness) * 16.0));
		diffuse_light += rim_light * rim * mix(vec3(1.0), diffuse_color, rim_tint) * light_color;
#endif
	}

	if (roughness > 0.0) { // FIXME: roughness == 0 should not disable specular light entirely

		// D

#if defined(SPECULAR_BLINN)

		//normalized blinn
		float shininess = exp2(15.0 * (1.0 - roughness) + 1.0) * 0.25;
		float blinn = pow(cNdotH, shininess);
		blinn *= (shininess + 8.0) * (1.0 / (8.0 * M_PI));
		float intensity = (blinn) / max(4.0 * cNdotV * cNdotL, 0.75);

		specular_light += light_color * intensity * specular_blob_intensity * attenuation;

#elif defined(SPECULAR_PHONG)

		vec3 R = normalize(-reflect(L, N));
		float cRdotV = max(0.0, dot(R, V));
		float shininess = exp2(15.0 * (1.0 - roughness) + 1.0) * 0.25;
		float phong = pow(cRdotV, shininess);
		phong *= (shininess + 8.0) * (1.0 / (8.0 * M_PI));
		float intensity = (phong) / max(4.0 * cNdotV * cNdotL, 0.75);

		specular_light += light_color * intensity * specular_blob_intensity * attenuation;

#elif defined(SPECULAR_TOON)

		vec3 R = normalize(-reflect(L, N));
		float RdotV = dot(R, V);
		float mid = 1.0 - roughness;
		mid *= mid;
		float intensity = smoothstep(mid - roughness * 0.5, mid + roughness * 0.5, RdotV) * mid;
		diffuse_light += light_color * intensity * specular_blob_intensity * attenuation; // write to diffuse_light, as in toon shading you generally want no reflection

#elif defined(SPECULAR_DISABLED)
		// none..

#elif defined(SPECULAR_SCHLICK_GGX)
		// shlick+ggx as default

#if defined(LIGHT_ANISOTROPY_USED)

		float alpha_ggx = roughness * roughness;
		float aspect = sqrt(1.0 - anisotropy * 0.9);
		float ax = alpha_ggx / aspect;
		float ay = alpha_ggx * aspect;
		float XdotH = dot(T, H);
		float YdotH = dot(B, H);
		float D = D_GGX_anisotropic(cNdotH, ax, ay, XdotH, YdotH);
		float G = G_GGX_anisotropic_2cos(cNdotL, ax, ay, XdotH, YdotH) * G_GGX_anisotropic_2cos(cNdotV, ax, ay, XdotH, YdotH);

#else
		float alpha_ggx = roughness * roughness;
		float D = D_GGX(cNdotH, alpha_ggx);
		float G = G_GGX_2cos(cNdotL, alpha_ggx) * G_GGX_2cos(cNdotV, alpha_ggx);
#endif
		// F
		vec3 f0 = F0(metallic, specular, diffuse_color);
		float cLdotH5 = SchlickFresnel(cLdotH);
		vec3 F = mix(vec3(cLdotH5), vec3(1.0), f0);

		vec3 specular_brdf_NL = cNdotL * D * F * G;

		specular_light += specular_brdf_NL * light_color * specular_blob_intensity * attenuation;
#endif

#if defined(LIGHT_CLEARCOAT_USED)

#if !defined(SPECULAR_SCHLICK_GGX)
		float cLdotH5 = SchlickFresnel(cLdotH);
#endif
		float Dr = GTR1(cNdotH, mix(.1, .001, clearcoat_gloss));
		float Fr = mix(.04, 1.0, cLdotH5);
		float Gr = G_GGX_2cos(cNdotL, .25) * G_GGX_2cos(cNdotV, .25);

		float clearcoat_specular_brdf_NL = 0.25 * clearcoat * Gr * Fr * Dr * cNdotL;

		specular_light += clearcoat_specular_brdf_NL * light_color * specular_blob_intensity * attenuation;
#endif
	}

#ifdef USE_SHADOW_TO_OPACITY
	alpha = min(alpha, clamp(1.0 - length(attenuation), 0.0, 1.0));
#endif

#endif //defined(USE_LIGHT_SHADER_CODE)
}

#ifndef USE_NO_SHADOWS

float sample_shadow(texture2D shadow, vec2 shadow_pixel_size, vec4 coord) {

	//todo optimize
	vec2 pos = coord.xy;
	float depth = coord.z;

#ifdef SHADOW_MODE_PCF_13

	float avg = textureProj(shadow, vec4(pos, depth, 1.0));
	avg += textureProj(sampler2DShadow(shadow,shadow_sampler), vec4(pos + vec2(shadow_pixel_size.x, 0.0), depth, 1.0));
	avg += textureProj(sampler2DShadow(shadow,shadow_sampler), vec4(pos + vec2(-shadow_pixel_size.x, 0.0), depth, 1.0));
	avg += textureProj(sampler2DShadow(shadow,shadow_sampler), vec4(pos + vec2(0.0, shadow_pixel_size.y), depth, 1.0));
	avg += textureProj(sampler2DShadow(shadow,shadow_sampler), vec4(pos + vec2(0.0, -shadow_pixel_size.y), depth, 1.0));
	avg += textureProj(sampler2DShadow(shadow,shadow_sampler), vec4(pos + vec2(shadow_pixel_size.x, shadow_pixel_size.y), depth, 1.0));
	avg += textureProj(sampler2DShadow(shadow,shadow_sampler), vec4(pos + vec2(-shadow_pixel_size.x, shadow_pixel_size.y), depth, 1.0));
	avg += textureProj(sampler2DShadow(shadow,shadow_sampler), vec4(pos + vec2(shadow_pixel_size.x, -shadow_pixel_size.y), depth, 1.0));
	avg += textureProj(sampler2DShadow(shadow,shadow_sampler), vec4(pos + vec2(-shadow_pixel_size.x, -shadow_pixel_size.y), depth, 1.0));
	avg += textureProj(sampler2DShadow(shadow,shadow_sampler), vec4(pos + vec2(shadow_pixel_size.x * 2.0, 0.0), depth, 1.0));
	avg += textureProj(sampler2DShadow(shadow,shadow_sampler), vec4(pos + vec2(-shadow_pixel_size.x * 2.0, 0.0), depth, 1.0));
	avg += textureProj(sampler2DShadow(shadow,shadow_sampler), vec4(pos + vec2(0.0, shadow_pixel_size.y * 2.0), depth, 1.0));
	avg += textureProj(sampler2DShadow(shadow,shadow_sampler), vec4(pos + vec2(0.0, -shadow_pixel_size.y * 2.0), depth, 1.0));
	return avg * (1.0 / 13.0);
#endif

#ifdef SHADOW_MODE_PCF_5

	float avg = textureProj(sampler2DShadow(shadow,shadow_sampler), vec4(pos, depth, 1.0));
	avg += textureProj(sampler2DShadow(shadow,shadow_sampler), vec4(pos + vec2(shadow_pixel_size.x, 0.0), depth, 1.0));
	avg += textureProj(sampler2DShadow(shadow,shadow_sampler), vec4(pos + vec2(-shadow_pixel_size.x, 0.0), depth, 1.0));
	avg += textureProj(sampler2DShadow(shadow,shadow_sampler), vec4(pos + vec2(0.0, shadow_pixel_size.y), depth, 1.0));
	avg += textureProj(sampler2DShadow(shadow,shadow_sampler), vec4(pos + vec2(0.0, -shadow_pixel_size.y), depth, 1.0));
	return avg * (1.0 / 5.0);

#endif

#if !defined(SHADOW_MODE_PCF_5) || !defined(SHADOW_MODE_PCF_13)

	return textureProj(sampler2DShadow(shadow,shadow_sampler), vec4(pos, depth, 1.0));

#endif
}

#endif //USE_NO_SHADOWS


void light_process_omni(uint idx, vec3 vertex, vec3 eye_vec, vec3 normal, vec3 albedo,float roughness, float metallic, float specular,float p_blob_intensity,
#ifdef LIGHT_TRANSMISSION_USED
	vec3 transmission,
#endif
#ifdef LIGHT_RIM_USED
	float rim, float rim_tint,
#endif
#ifdef LIGHT_CLEARCOAT_USED
	float clearcoat, float clearcoat_gloss,
#endif
#ifdef LIGHT_ANISOTROPY_USED
	vec3 binormal, vec3 tangent, float anisotropy,
#endif
#ifdef USE_SHADOW_TO_OPACITY
	inout float alpha,
#endif
	inout vec3 diffuse_light, inout vec3 specular_light) {

	vec3 light_rel_vec = lights.data[idx].position - vertex;
	float light_length = length(light_rel_vec);
	float normalized_distance = light_length * lights.data[idx].inv_radius;
	vec2 attenuation_energy = unpackHalf2x16(lights.data[idx].attenuation_energy);
	float omni_attenuation = pow(max(1.0 - normalized_distance, 0.0), attenuation_energy.x);
	vec3 light_attenuation = vec3(omni_attenuation);
	vec4 color_specular = unpackUnorm4x8(lights.data[idx].color_specular);
	color_specular.rgb*=attenuation_energy.y;

#ifndef USE_NO_SHADOWS
	vec4 shadow_color_enabled = unpackUnorm4x8(lights.data[idx].shadow_color_enabled);
	if (shadow_color_enabled.w > 0.5) {
		// there is a shadowmap

		vec4 splane = (lights.data[idx].shadow_matrix * vec4(vertex, 1.0));
		float shadow_len = length(splane);
		splane = normalize(splane);
		vec4 clamp_rect = lights.data[idx].atlas_rect;

		if (splane.z >= 0.0) {

			splane.z += 1.0;

			clamp_rect.y += clamp_rect.w;

		} else {

			splane.z = 1.0 - splane.z;

		}

		splane.xy /= splane.z;
		splane.xy = splane.xy * 0.5 + 0.5;
		splane.z = shadow_len * lights.data[idx].inv_radius;
		splane.xy = clamp_rect.xy + splane.xy * clamp_rect.zw;
		splane.w = 1.0; //needed? i think it should be 1 already
		float shadow = sample_shadow(shadow_atlas, scene_data.shadow_atlas_pixel_size, splane);

		light_attenuation *= mix(shadow_color_enabled.rgb, vec3(1.0), shadow);
	}
#endif //USE_NO_SHADOWS

	light_compute(normal, normalize(light_rel_vec), eye_vec, color_specular.rgb, light_attenuation, albedo, roughness, metallic, specular,color_specular.a * p_blob_intensity,
#ifdef LIGHT_TRANSMISSION_USED
		transmission,
#endif
#ifdef LIGHT_RIM_USED
		rim * omni_attenuation, rim_tint,
#endif
#ifdef LIGHT_CLEARCOAT_USED
		clearcoat, clearcoat_gloss,
#endif
#ifdef LIGHT_ANISOTROPY_USED
		binormal, tangent, anisotropy,
#endif
#ifdef USE_SHADOW_TO_OPACITY
		alpha
#endif
		diffuse_light, specular_light);
}




void light_process_spot(uint idx, vec3 vertex, vec3 eye_vec, vec3 normal, vec3 albedo,float roughness, float metallic, float specular,float p_blob_intensity,
#ifdef LIGHT_TRANSMISSION_USED
	vec3 transmission,
#endif
#ifdef LIGHT_RIM_USED
	float rim, float rim_tint,
#endif
#ifdef LIGHT_CLEARCOAT_USED
	float clearcoat, float clearcoat_gloss,
#endif
#ifdef LIGHT_ANISOTROPY_USED
	vec3 binormal, vec3 tangent, float anisotropy,
#endif
#ifdef USE_SHADOW_TO_OPACITY
	inout float alpha
#endif
	inout vec3 diffuse_light, inout vec3 specular_light) {

	vec3 light_rel_vec = lights.data[idx].position - vertex;
	float light_length = length(light_rel_vec);
	float normalized_distance = light_length * lights.data[idx].inv_radius;
	vec2 attenuation_energy = unpackHalf2x16(lights.data[idx].attenuation_energy);
	float spot_attenuation = pow(max(1.0 - normalized_distance, 0.001), attenuation_energy.x);
	vec3 spot_dir = lights.data[idx].direction;
	vec2 spot_att_angle = unpackHalf2x16(lights.data[idx].cone_attenuation_angle);
	float scos = max(dot(-normalize(light_rel_vec), spot_dir), spot_att_angle.y);
	float spot_rim = max(0.0001, (1.0 - scos) / (1.0 - spot_att_angle.y));
	spot_attenuation *= 1.0 - pow(spot_rim, spot_att_angle.x);
	vec3 light_attenuation = vec3(spot_attenuation);
	vec4 color_specular = unpackUnorm4x8(lights.data[idx].color_specular);
	color_specular.rgb*=attenuation_energy.y;



/*
	if (lights.data[idx].atlas_rect!=vec4(0.0)) {
		//use projector texture
	}
	*/
#ifndef USE_NO_SHADOWS
	vec4 shadow_color_enabled = unpackUnorm4x8(lights.data[idx].shadow_color_enabled);
	if (shadow_color_enabled.w > 0.5) {
		//there is a shadowmap
		vec4 splane = (lights.data[idx].shadow_matrix * vec4(vertex, 1.0));
		splane /= splane.w;
		float shadow = sample_shadow(shadow_atlas, scene_data.shadow_atlas_pixel_size, splane);

		light_attenuation *= mix(shadow_color_enabled.rgb, vec3(1.0), shadow);
	}

#endif //USE_NO_SHADOWS

	light_compute(normal, normalize(light_rel_vec), eye_vec, color_specular.rgb, light_attenuation, albedo, roughness, metallic, specular,color_specular.a * p_blob_intensity,
#ifdef LIGHT_TRANSMISSION_USED
		transmission,
#endif
#ifdef LIGHT_RIM_USED
		rim * spot_attenuation, rim_tint,
#endif
#ifdef LIGHT_CLEARCOAT_USED
		clearcoat, clearcoat_gloss,
#endif
#ifdef LIGHT_ANISOTROPY_USED
		binormal, tangent, anisotropy,
#endif
#ifdef USE_SHADOW_TO_OPACITY
		alpha,
#endif
		diffuse_light, specular_light);
}

void reflection_process(uint ref_index, vec3 vertex, vec3 normal,float roughness,vec3 ambient_light,vec3 specular_light,inout vec4 ambient_accum, inout vec4 reflection_accum) {

	vec3 box_extents = reflections.data[ref_index].box_extents;
	vec3 local_pos = (reflections.data[ref_index].local_matrix * vec4(vertex, 1.0)).xyz;

	if (any(greaterThan(abs(local_pos), box_extents))) { //out of the reflection box
		return;
	}

	vec3 ref_vec = normalize(reflect(vertex, normal));

	vec3 inner_pos = abs(local_pos / box_extents);
	float blend = max(inner_pos.x, max(inner_pos.y, inner_pos.z));
	//make blend more rounded
	blend = mix(length(inner_pos), blend, blend);
	blend *= blend;
	blend = max(0.0, 1.0 - blend);

	if (reflections.data[ref_index].params.x > 0.0) { // compute reflection

		vec3 local_ref_vec = (reflections.data[ref_index].local_matrix * vec4(ref_vec, 0.0)).xyz;

		if (reflections.data[ref_index].params.w > 0.5) { //box project

			vec3 nrdir = normalize(local_ref_vec);
			vec3 rbmax = (box_extents - local_pos) / nrdir;
			vec3 rbmin = (-box_extents - local_pos) / nrdir;

			vec3 rbminmax = mix(rbmin, rbmax, greaterThan(nrdir, vec3(0.0, 0.0, 0.0)));

			float fa = min(min(rbminmax.x, rbminmax.y), rbminmax.z);
			vec3 posonbox = local_pos + nrdir * fa;
			local_ref_vec = posonbox - reflections.data[ref_index].box_offset;
		}

		vec4 reflection;

		reflection.rgb = textureLod(samplerCubeArray(reflection_atlas,material_samplers[SAMPLER_LINEAR_WITH_MIPMAPS_CLAMP]), vec4(local_ref_vec,reflections.data[ref_index].index), roughness * MAX_ROUGHNESS_LOD).rgb;

		if (reflections.data[ref_index].params.z < 0.5) {
			reflection.rgb = mix(specular_light, reflection.rgb, blend);
		}

		reflection.rgb *= reflections.data[ref_index].params.x;
		reflection.a = blend;
		reflection.rgb *= reflection.a;

		reflection_accum += reflection;
	}

#ifndef USE_LIGHTMAP
	if (reflections.data[ref_index].ambient.a > 0.0) { //compute ambient using skybox

		vec3 local_amb_vec = (reflections.data[ref_index].local_matrix * vec4(normal, 0.0)).xyz;

		vec4 ambient_out;

		ambient_out.rgb = textureLod(samplerCubeArray(reflection_atlas,material_samplers[SAMPLER_LINEAR_WITH_MIPMAPS_CLAMP]), vec4(local_amb_vec,reflections.data[ref_index].index), MAX_ROUGHNESS_LOD).rgb;

		ambient_out.a = blend;
		ambient_out.rgb = mix(reflections.data[ref_index].ambient.rgb, ambient_out.rgb, reflections.data[ref_index].ambient.a);
		if (reflections.data[ref_index].params.z < 0.5) {
			ambient_out.rgb = mix(ambient_light, ambient_out.rgb, blend);
		}

		ambient_out.rgb *= ambient_out.a;
		ambient_accum += ambient_out;
	} else {

		vec4 ambient_out;
		ambient_out.a = blend;
		ambient_out.rgb = reflections.data[ref_index].ambient.rgb;
		if (reflections.data[ref_index].params.z < 0.5) {
			ambient_out.rgb = mix(ambient_light, ambient_out.rgb, blend);
		}
		ambient_out.rgb *= ambient_out.a;
		ambient_accum += ambient_out;
	}
#endif //USE_LIGHTMAP
}
void main() {

#ifdef MODE_DUAL_PARABOLOID

	if (dp_clip > 0.0)
		discard;
#endif

	//lay out everything, whathever is unused is optimized away anyway
	vec3 vertex = vertex_interp;
	vec3 view = -normalize(vertex_interp);
	vec3 albedo = vec3(1.0);
	vec3 transmission = vec3(0.0);
	float metallic = 0.0;
	float specular = 0.5;
	vec3 emission = vec3(0.0);
	float roughness = 1.0;
	float rim = 0.0;
	float rim_tint = 0.0;
	float clearcoat = 0.0;
	float clearcoat_gloss = 0.0;
	float anisotropy = 0.0;
	vec2 anisotropy_flow = vec2(1.0, 0.0);

#if defined(AO_USED)
	float ao = 1.0;
	float ao_light_affect = 0.0;
#endif

	float alpha = 1.0;

#if defined(ALPHA_SCISSOR_USED)
	float alpha_scissor = 0.5;
#endif

#if defined(TANGENT_USED) || defined(NORMALMAP_USED) || defined(LIGHT_ANISOTROPY_USED)
	vec3 binormal = normalize(binormal_interp);
	vec3 tangent = normalize(tangent_interp);
#else
	vec3 binormal = vec3(0.0);
	vec3 tangent = vec3(0.0);
#endif
	vec3 normal = normalize(normal_interp);

#if defined(DO_SIDE_CHECK)
	if (!gl_FrontFacing) {
		normal = -normal;
	}
#endif

#if defined(UV_USED)
	vec2 uv = uv_interp;
#endif

#if defined(UV2_USED) || defined(USE_LIGHTMAP)
	vec2 uv2 = uv2_interp;
#endif

#if defined(COLOR_USED)
	vec4 color = color_interp;
#endif

#if defined(NORMALMAP_USED)

	vec3 normalmap = vec3(0.5);
#endif

	float normaldepth = 1.0;

#if defined(SCREEN_UV_USED)
	vec2 screen_uv = gl_FragCoord.xy * scene_data.screen_pixel_size;
#endif

	float sss_strength = 0.0;


	{
		/* clang-format off */

FRAGMENT_SHADER_CODE

		/* clang-format on */
	}

#if !defined(USE_SHADOW_TO_OPACITY)

#if defined(ALPHA_SCISSOR_USED)
	if (alpha < alpha_scissor) {
		discard;
	}
#endif // ALPHA_SCISSOR_USED

#ifdef USE_OPAQUE_PREPASS

	if (alpha < opaque_prepass_threshold) {
		discard;
	}

#endif // USE_OPAQUE_PREPASS

#endif // !USE_SHADOW_TO_OPACITY

#if defined(NORMALMAP_USED)

	normalmap.xy = normalmap.xy * 2.0 - 1.0;
	normalmap.z = sqrt(max(0.0, 1.0 - dot(normalmap.xy, normalmap.xy))); //always ignore Z, as it can be RG packed, Z may be pos/neg, etc.

	normal = normalize(mix(normal, tangent * normalmap.x + binormal * normalmap.y + normal * normalmap.z, normaldepth));

#endif

#if defined(LIGHT_ANISOTROPY_USED)

	if (anisotropy > 0.01) {
		//rotation matrix
		mat3 rot = mat3(tangent, binormal, normal);
		//make local to space
		tangent = normalize(rot * vec3(anisotropy_flow.x, anisotropy_flow.y, 0.0));
		binormal = normalize(rot * vec3(-anisotropy_flow.y, anisotropy_flow.x, 0.0));
	}

#endif

#ifdef ENABLE_CLIP_ALPHA
	if (albedo.a < 0.99) {
		//used for doublepass and shadowmapping
		discard;
	}
#endif

	/////////////////////// LIGHTING //////////////////////////////

	//apply energy conservation

	vec3 specular_light = vec3(0.0, 0.0, 0.0);
	vec3 diffuse_light = vec3(0.0, 0.0, 0.0);
	vec3 ambient_light = vec3( 0.0, 0.0, 0.0);

#ifndef MODE_RENDER_DEPTH
	if (scene_data.use_reflection_cubemap){

		vec3 ref_vec = reflect(-view, normal);
		ref_vec = scene_data.radiance_inverse_xform * ref_vec;
#ifdef USE_RADIANCE_CUBEMAP_ARRAY

		float lod,blend;
		blend = modf(roughness * MAX_ROUGHNESS_LOD, lod);
		specular_light = texture(samplerCubeArray(radiance_cubemap,material_samplers[SAMPLER_LINEAR_WITH_MIPMAPS_CLAMP]), vec4(ref_vec, lod)).rgb;
		specular_light = mix(specular_light,texture(samplerCubeArray(radiance_cubemap,material_samplers[SAMPLER_LINEAR_WITH_MIPMAPS_CLAMP]), vec4(ref_vec, lod+1)).rgb,blend);

#else
		specular_light = textureLod(samplerCube(radiance_cubemap,material_samplers[SAMPLER_LINEAR_WITH_MIPMAPS_CLAMP]), ref_vec, roughness * MAX_ROUGHNESS_LOD).rgb;

#endif //USE_RADIANCE_CUBEMAP_ARRAY
		specular_light *= scene_data.ambient_light_color_energy.a;
	}

#ifndef USE_LIGHTMAP
	//lightmap overrides everything
	if (scene_data.use_ambient_light){

		ambient_light = scene_data.ambient_light_color_energy.rgb;

		if (scene_data.use_ambient_cubemap) {
			vec3 ambient_dir = scene_data.radiance_inverse_xform * normal;
#ifdef USE_RADIANCE_CUBEMAP_ARRAY
			vec3 cubemap_ambient = texture(samplerCubeArray(radiance_cubemap,material_samplers[SAMPLER_LINEAR_WITH_MIPMAPS_CLAMP]), vec4(ambient_dir, MAX_ROUGHNESS_LOD)).rgb;
#else
			vec3 cubemap_ambient = textureLod(samplerCube(radiance_cubemap,material_samplers[SAMPLER_LINEAR_WITH_MIPMAPS_CLAMP]), ambient_dir, MAX_ROUGHNESS_LOD).rgb;
#endif //USE_RADIANCE_CUBEMAP_ARRAY

			ambient_light = mix( ambient_light, cubemap_ambient * scene_data.ambient_light_color_energy.a, scene_data.ambient_color_sky_mix );
		}


	}
#endif // USE_LIGHTMAP

#endif // MODE_RENDER_DEPTH

	//radiance

	float specular_blob_intensity = 1.0;

#if defined(SPECULAR_TOON)
	specular_blob_intensity *= specular * 2.0;
#endif

#ifndef MODE_RENDER_DEPTH
	//gi probes

	//lightmap

	//lightmap capture


	{ // process reflections


		vec4 reflection_accum = vec4(0.0, 0.0, 0.0, 0.0);
		vec4 ambient_accum = vec4(0.0, 0.0, 0.0, 0.0);

		uint reflection_probe_count = instances.data[instance_index].flags & INSTANCE_FLAGS_FORWARD_MASK;

		for (uint i = 0; i < reflection_probe_count; i++) {


			uint ref_index = instances.data[instance_index].reflection_probe_indices[i>>1];

			if (bool(i&1)) {
				ref_index>>=16;
			} else {
				ref_index&=0xFFFF;
			}


			reflection_process(ref_index,vertex,normal,roughness,ambient_light,specular_light,ambient_accum,reflection_accum);

		}

		if (reflection_accum.a > 0.0) {
			specular_light = reflection_accum.rgb / reflection_accum.a;
		}

#if !defined(USE_LIGHTMAP)
		if (ambient_accum.a > 0.0) {
			ambient_light = ambient_accum.rgb / ambient_accum.a;
		}
#endif


	}

	{

#if defined(DIFFUSE_TOON)
		//simplify for toon, as
		specular_light *= specular * metallic * albedo * 2.0;
#else

		// scales the specular reflections, needs to be be computed before lighting happens,
		// but after environment, GI, and reflection probes are added
		// Environment brdf approximation (Lazarov 2013)
		// see https://www.unrealengine.com/en-US/blog/physically-based-shading-on-mobile
		const vec4 c0 = vec4(-1.0, -0.0275, -0.572, 0.022);
		const vec4 c1 = vec4(1.0, 0.0425, 1.04, -0.04);
		vec4 r = roughness * c0 + c1;
		float ndotv = clamp(dot(normal, view), 0.0, 1.0);
		float a004 = min(r.x * r.x, exp2(-9.28 * ndotv)) * r.x + r.y;
		vec2 env = vec2(-1.04, 1.04) * a004 + r.zw;

		vec3 f0 = F0(metallic, specular, albedo);
		specular_light *= env.x * f0 + env.y;
#endif
	}

	{ //directional light

		for (uint i = 0; i < scene_data.directional_light_count; i++) {

			if (!bool(directional_lights.data[i].mask&instances.data[instance_index].layer_mask)) {
				continue; //not masked
			}

			vec3 light_attenuation = vec3(1.0);

			if (directional_lights.data[i].shadow_enabled) {
				float depth_z = -vertex.z;

				vec4 pssm_coord;

				if (depth_z < directional_lights.data[i].shadow_split_offsets.x) {
					pssm_coord = (directional_lights.data[i].shadow_matrix1 * vec4(vertex, 1.0));
				} else if (depth_z < directional_lights.data[i].shadow_split_offsets.y) {
					pssm_coord = (directional_lights.data[i].shadow_matrix2 * vec4(vertex, 1.0));
				} else if (depth_z < directional_lights.data[i].shadow_split_offsets.z) {
					pssm_coord = (directional_lights.data[i].shadow_matrix3 * vec4(vertex, 1.0));
				} else {
					pssm_coord = (directional_lights.data[i].shadow_matrix4 * vec4(vertex, 1.0));
				}

				pssm_coord/=pssm_coord.w;

				float shadow = sample_shadow(directional_shadow_atlas, scene_data.directional_shadow_pixel_size, pssm_coord);

				if (directional_lights.data[i].blend_splits) {

					float pssm_blend;

					if (depth_z < directional_lights.data[i].shadow_split_offsets.x) {
						pssm_coord = (directional_lights.data[i].shadow_matrix2 * vec4(vertex, 1.0));
						pssm_blend = smoothstep(0.0, directional_lights.data[i].shadow_split_offsets.x, depth_z);
					} else if (depth_z < directional_lights.data[i].shadow_split_offsets.y) {
						pssm_coord = (directional_lights.data[i].shadow_matrix3 * vec4(vertex, 1.0));
						pssm_blend = smoothstep(directional_lights.data[i].shadow_split_offsets.x, directional_lights.data[i].shadow_split_offsets.y, depth_z);
					} else if (depth_z < directional_lights.data[i].shadow_split_offsets.z) {
						pssm_coord = (directional_lights.data[i].shadow_matrix4 * vec4(vertex, 1.0));
						pssm_blend = smoothstep(directional_lights.data[i].shadow_split_offsets.y, directional_lights.data[i].shadow_split_offsets.z, depth_z);
					} else {
						pssm_blend = 0.0; //if no blend, same coord will be used (divide by z will result in same value, and already cached)
					}

					pssm_coord/=pssm_coord.w;

					float shadow2 = sample_shadow(directional_shadow_atlas, scene_data.directional_shadow_pixel_size, pssm_coord);
					shadow = mix(shadow,shadow2,pssm_blend);

				}

				shadow = mix(shadow,1.0,smoothstep(directional_lights.data[i].fade_from,directional_lights.data[i].fade_to,vertex.z)); //done with negative values for performance

				light_attenuation = mix(directional_lights.data[i].shadow_color, vec3(1.0), shadow);
			}


			light_compute(normal, directional_lights.data[i].direction, normalize(view), directional_lights.data[i].color * directional_lights.data[i].energy, light_attenuation, albedo, roughness, metallic, specular,directional_lights.data[i].specular * specular_blob_intensity,
#ifdef LIGHT_TRANSMISSION_USED
				transmission,
#endif
#ifdef LIGHT_RIM_USED
				rim, rim_tint,
#endif
#ifdef LIGHT_CLEARCOAT_USED
				clearcoat, clearcoat_gloss,
#endif
#ifdef LIGHT_ANISOTROPY_USED
				binormal, tangent, anisotropy,
#endif
#ifdef USE_SHADOW_TO_OPACITY
				alpha
#endif
				diffuse_light, specular_light);
		}
	}

	{ //omni lights
		uint omni_light_count = (instances.data[instance_index].flags >> INSTANCE_FLAGS_FORWARD_OMNI_LIGHT_SHIFT) & INSTANCE_FLAGS_FORWARD_MASK;
		for (uint i = 0; i < omni_light_count; i++) {

			uint light_index = instances.data[instance_index].omni_light_indices[i>>1];

			if (bool(i&1)) {
				light_index>>=16;
			} else {
				light_index&=0xFFFF;
			}

			//this is done on CPU, so no need to do it here
			//if (!bool(lights.data[light_index].mask&instances.data[instance_index].layer_mask)) {
			//	continue; //not masked
			//}

			light_process_omni(light_index, vertex, view, normal, albedo, roughness, metallic, specular,specular_blob_intensity,
#ifdef LIGHT_TRANSMISSION_USED
					   transmission,
#endif
#ifdef LIGHT_RIM_USED
					   rim,
					   rim_tint,
#endif
#ifdef LIGHT_CLEARCOAT_USED
					   clearcoat, clearcoat_gloss,
#endif
#ifdef LIGHT_ANISOTROPY_USED
					   tangent, binormal, anisotropy,
#endif
#ifdef USE_SHADOW_TO_OPACITY
					   alpha,
#endif
					   diffuse_light, specular_light);
		}

	}

	{ //spot lights
		uint spot_light_count = (instances.data[instance_index].flags >> INSTANCE_FLAGS_FORWARD_SPOT_LIGHT_SHIFT) & INSTANCE_FLAGS_FORWARD_MASK;
		for (uint i = 0; i < spot_light_count; i++) {

			uint light_index = instances.data[instance_index].spot_light_indices[i>>1];

			if (bool(i&1)) {
				light_index>>=16;
			} else {
				light_index&=0xFFFF;
			}

			//this is done on CPU, so no need to do it here
			//if (!bool(lights.data[light_index].mask&instances.data[instance_index].layer_mask)) {
			//	continue; //not masked
			//}

			light_process_spot(light_index, vertex, view, normal, albedo, roughness, metallic, specular,specular_blob_intensity,
#ifdef LIGHT_TRANSMISSION_USED
					   transmission,
#endif
#ifdef LIGHT_RIM_USED
					   rim,
					   rim_tint,
#endif
#ifdef LIGHT_CLEARCOAT_USED
					   clearcoat, clearcoat_gloss,
#endif
#ifdef LIGHT_ANISOTROPY_USED
					   tangent, binormal, anisotropy,
#endif
#ifdef USE_SHADOW_TO_OPACITY
					   alpha,
#endif
					   diffuse_light, specular_light);
		}

	}


#endif //!MODE_RENDER_DEPTH

#ifdef USE_SHADOW_TO_OPACITY
	alpha = min(alpha, clamp(length(ambient_light), 0.0, 1.0));

#if defined(ALPHA_SCISSOR_USED)
	if (alpha < alpha_scissor) {
		discard;
	}
#endif // ALPHA_SCISSOR_USED

#ifdef USE_OPAQUE_PREPASS

	if (alpha < opaque_prepass_threshold) {
		discard;
	}

#endif // USE_OPAQUE_PREPASS

#endif // USE_SHADOW_TO_OPACITY



#ifdef MODE_RENDER_DEPTH
//nothing happens, so a tree-ssa optimizer will result in no fragment shader :)
#else

	specular_light *= scene_data.reflection_multiplier;
	ambient_light *= albedo; //ambient must be multiplied by albedo at the end

#if defined(AO_USED)
	ambient_light *= ao;
	ao_light_affect = mix(1.0, ao, ao_light_affect);
	specular_light *= ao_light_affect;
	diffuse_light *= ao_light_affect;
#endif

	// base color remapping
	diffuse_light *= 1.0 - metallic; // TODO: avoid all diffuse and ambient light calculations when metallic == 1 up to this point
	ambient_light *= 1.0 - metallic;

	//fog

#ifdef MODE_MULTIPLE_RENDER_TARGETS

#ifdef USE_NO_SHADING
	diffuse_buffer = vec4(albedo.rgb, 0.0);
	specular_buffer = vec4(0.0);

#else

	diffuse_buffer = vec4(emission + diffuse_light + ambient_light, sss_strength);
	specular_buffer = vec4(specular_light, metallic);

#endif

#else //MODE_MULTIPLE_RENDER_TARGETS

#ifdef USE_NO_SHADING
	frag_color = vec4(albedo, alpha);
#else
	frag_color = vec4(emission + ambient_light + diffuse_light + specular_light, alpha);
	//frag_color = vec4(1.0);

#endif //USE_NO_SHADING

#endif //MODE_MULTIPLE_RENDER_TARGETS

#endif //MODE_RENDER_DEPTH
}
