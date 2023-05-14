/* clang-format off */
#[modes]

mode_color = #define BASE_PASS
mode_color_instancing = #define BASE_PASS \n#define USE_INSTANCING
mode_additive = #define USE_ADDITIVE_LIGHTING
mode_additive_instancing = #define USE_ADDITIVE_LIGHTING \n#define USE_INSTANCING
mode_depth = #define MODE_RENDER_DEPTH
mode_depth_instancing = #define MODE_RENDER_DEPTH \n#define USE_INSTANCING

#[specializations]

DISABLE_LIGHTMAP = false
DISABLE_LIGHT_DIRECTIONAL = false
DISABLE_LIGHT_OMNI = false
DISABLE_LIGHT_SPOT = false
DISABLE_FOG = false
USE_RADIANCE_MAP = true
USE_MULTIVIEW = false


#[vertex]

#define M_PI 3.14159265359

#define SHADER_IS_SRGB true

#include "stdlib_inc.glsl"

#if !defined(MODE_RENDER_DEPTH) || defined(TANGENT_USED) || defined(NORMAL_MAP_USED) || defined(LIGHT_ANISOTROPY_USED) ||defined(LIGHT_CLEARCOAT_USED)
#ifndef NORMAL_USED
#define NORMAL_USED
#endif
#endif

/*
from RenderingServer:
ARRAY_VERTEX = 0, // RG32F or RGB32F (depending on 2D bit)
ARRAY_NORMAL = 1, // RG16 octahedral compression
ARRAY_TANGENT = 2, // RG16 octahedral compression, sign stored in sign of G
ARRAY_COLOR = 3, // RGBA8
ARRAY_TEX_UV = 4, // RG32F
ARRAY_TEX_UV2 = 5, // RG32F
ARRAY_CUSTOM0 = 6, // Depends on ArrayCustomFormat.
ARRAY_CUSTOM1 = 7,
ARRAY_CUSTOM2 = 8,
ARRAY_CUSTOM3 = 9,
ARRAY_BONES = 10, // RGBA16UI (x2 if 8 weights)
ARRAY_WEIGHTS = 11, // RGBA16UNORM (x2 if 8 weights)
*/

/* INPUT ATTRIBS */

layout(location = 0) in highp vec3 vertex_attrib;
/* clang-format on */

#ifdef NORMAL_USED
layout(location = 1) in vec2 normal_attrib;
#endif

#if defined(TANGENT_USED) || defined(NORMAL_MAP_USED) || defined(LIGHT_ANISOTROPY_USED)
layout(location = 2) in vec2 tangent_attrib;
#endif

#if defined(COLOR_USED)
layout(location = 3) in vec4 color_attrib;
#endif

#ifdef UV_USED
layout(location = 4) in vec2 uv_attrib;
#endif

#if defined(UV2_USED) || defined(USE_LIGHTMAP)
layout(location = 5) in vec2 uv2_attrib;
#endif

#if defined(CUSTOM0_USED)
layout(location = 6) in vec4 custom0_attrib;
#endif

#if defined(CUSTOM1_USED)
layout(location = 7) in vec4 custom1_attrib;
#endif

#if defined(CUSTOM2_USED)
layout(location = 8) in vec4 custom2_attrib;
#endif

#if defined(CUSTOM3_USED)
layout(location = 9) in vec4 custom3_attrib;
#endif

#if defined(BONES_USED)
layout(location = 10) in uvec4 bone_attrib;
#endif

#if defined(WEIGHTS_USED)
layout(location = 11) in vec4 weight_attrib;
#endif

vec3 oct_to_vec3(vec2 e) {
	vec3 v = vec3(e.xy, 1.0 - abs(e.x) - abs(e.y));
	float t = max(-v.z, 0.0);
	v.xy += t * -sign(v.xy);
	return normalize(v);
}

#ifdef USE_INSTANCING
layout(location = 12) in highp vec4 instance_xform0;
layout(location = 13) in highp vec4 instance_xform1;
layout(location = 14) in highp vec4 instance_xform2;
layout(location = 15) in highp uvec4 instance_color_custom_data; // Color packed into xy, Custom data into zw.
#endif

layout(std140) uniform GlobalShaderUniformData { //ubo:1
	vec4 global_shader_uniforms[MAX_GLOBAL_SHADER_UNIFORMS];
};

layout(std140) uniform SceneData { // ubo:2
	highp mat4 projection_matrix;
	highp mat4 inv_projection_matrix;
	highp mat4 inv_view_matrix;
	highp mat4 view_matrix;

	vec2 viewport_size;
	vec2 screen_pixel_size;

	mediump vec4 ambient_light_color_energy;

	mediump float ambient_color_sky_mix;
	bool material_uv2_mode;
	float emissive_exposure_normalization;
	bool use_ambient_light;
	bool use_ambient_cubemap;
	bool use_reflection_cubemap;

	float fog_aerial_perspective;
	float time;

	mat3 radiance_inverse_xform;

	uint directional_light_count;
	float z_far;
	float z_near;
	float IBL_exposure_normalization;

	bool fog_enabled;
	float fog_density;
	float fog_height;
	float fog_height_density;

	vec3 fog_light_color;
	float fog_sun_scatter;
	uint camera_visible_layers;
	uint pad3;
	uint pad4;
	uint pad5;
}
scene_data;

#ifdef USE_MULTIVIEW
layout(std140) uniform MultiviewData { // ubo:8
	highp mat4 projection_matrix_view[MAX_VIEWS];
	highp mat4 inv_projection_matrix_view[MAX_VIEWS];
	highp vec4 eye_offset[MAX_VIEWS];
}
multiview_data;
#endif

uniform highp mat4 world_transform;

#ifdef USE_LIGHTMAP
uniform highp vec4 lightmap_uv_rect;
#endif

/* Varyings */

out highp vec3 vertex_interp;
#ifdef NORMAL_USED
out vec3 normal_interp;
#endif

#if defined(COLOR_USED)
out vec4 color_interp;
#endif

#if defined(UV_USED)
out vec2 uv_interp;
#endif

#if defined(UV2_USED)
out vec2 uv2_interp;
#else
#ifdef USE_LIGHTMAP
out vec2 uv2_interp;
#endif
#endif

#if defined(TANGENT_USED) || defined(NORMAL_MAP_USED) || defined(LIGHT_ANISOTROPY_USED)
out vec3 tangent_interp;
out vec3 binormal_interp;
#endif

#ifdef MATERIAL_UNIFORMS_USED

/* clang-format off */
layout(std140) uniform MaterialUniforms { // ubo:3

#MATERIAL_UNIFORMS

};
/* clang-format on */

#endif

/* clang-format off */

#GLOBALS

/* clang-format on */
invariant gl_Position;

void main() {
	highp vec3 vertex = vertex_attrib;

	highp mat4 model_matrix = world_transform;
#ifdef USE_INSTANCING
	highp mat4 m = mat4(instance_xform0, instance_xform1, instance_xform2, vec4(0.0, 0.0, 0.0, 1.0));
	model_matrix = model_matrix * transpose(m);
#endif

#ifdef NORMAL_USED
	vec3 normal = oct_to_vec3(normal_attrib * 2.0 - 1.0);
#endif
	highp mat3 model_normal_matrix = mat3(model_matrix);

#if defined(TANGENT_USED) || defined(NORMAL_MAP_USED) || defined(LIGHT_ANISOTROPY_USED)
	vec2 signed_tangent_attrib = tangent_attrib * 2.0 - 1.0;
	vec3 tangent = oct_to_vec3(vec2(signed_tangent_attrib.x, abs(signed_tangent_attrib.y) * 2.0 - 1.0));
	float binormalf = sign(signed_tangent_attrib.y);
	vec3 binormal = normalize(cross(normal, tangent) * binormalf);
#endif

#if defined(COLOR_USED)
	color_interp = color_attrib;
#ifdef USE_INSTANCING
	vec4 instance_color = vec4(unpackHalf2x16(instance_color_custom_data.x), unpackHalf2x16(instance_color_custom_data.y));
	color_interp *= instance_color;
#endif
#endif

#if defined(UV_USED)
	uv_interp = uv_attrib;
#endif

#ifdef USE_LIGHTMAP
	uv2_interp = lightmap_uv_rect.zw * uv2_attrib + lightmap_uv_rect.xy;
#else
#if defined(UV2_USED)
	uv2_interp = uv2_attrib;
#endif
#endif

#if defined(OVERRIDE_POSITION)
	highp vec4 position;
#endif

#ifdef USE_MULTIVIEW
	mat4 projection_matrix = multiview_data.projection_matrix_view[ViewIndex];
	mat4 inv_projection_matrix = multiview_data.inv_projection_matrix_view[ViewIndex];
	vec3 eye_offset = multiview_data.eye_offset[ViewIndex].xyz;
#else
	mat4 projection_matrix = scene_data.projection_matrix;
	mat4 inv_projection_matrix = scene_data.inv_projection_matrix;
	vec3 eye_offset = vec3(0.0, 0.0, 0.0);
#endif //USE_MULTIVIEW

#ifdef USE_INSTANCING
	vec4 instance_custom = vec4(unpackHalf2x16(instance_color_custom_data.z), unpackHalf2x16(instance_color_custom_data.w));
#else
	vec4 instance_custom = vec4(0.0);
#endif

	// Using world coordinates
#if !defined(SKIP_TRANSFORM_USED) && defined(VERTEX_WORLD_COORDS_USED)

	vertex = (model_matrix * vec4(vertex, 1.0)).xyz;

#ifdef NORMAL_USED
	normal = model_normal_matrix * normal;
#endif

#if defined(TANGENT_USED) || defined(NORMAL_MAP_USED) || defined(LIGHT_ANISOTROPY_USED)

	tangent = model_normal_matrix * tangent;
	binormal = model_normal_matrix * binormal;

#endif
#endif

	float roughness = 1.0;

	highp mat4 modelview = scene_data.view_matrix * model_matrix;
	highp mat3 modelview_normal = mat3(scene_data.view_matrix) * model_normal_matrix;

	float point_size = 1.0;

	{
#CODE : VERTEX
	}

	gl_PointSize = point_size;

	// Using local coordinates (default)
#if !defined(SKIP_TRANSFORM_USED) && !defined(VERTEX_WORLD_COORDS_USED)

	vertex = (modelview * vec4(vertex, 1.0)).xyz;
#ifdef NORMAL_USED
	normal = modelview_normal * normal;
#endif

#endif

#if defined(TANGENT_USED) || defined(NORMAL_MAP_USED) || defined(LIGHT_ANISOTROPY_USED)

	binormal = modelview_normal * binormal;
	tangent = modelview_normal * tangent;
#endif

	// Using world coordinates
#if !defined(SKIP_TRANSFORM_USED) && defined(VERTEX_WORLD_COORDS_USED)

	vertex = (scene_data.view_matrix * vec4(vertex, 1.0)).xyz;
#ifdef NORMAL_USED
	normal = (scene_data.view_matrix * vec4(normal, 0.0)).xyz;
#endif

#if defined(TANGENT_USED) || defined(NORMAL_MAP_USED) || defined(LIGHT_ANISOTROPY_USED)
	binormal = (scene_data.view_matrix * vec4(binormal, 0.0)).xyz;
	tangent = (scene_data.view_matrix * vec4(tangent, 0.0)).xyz;
#endif
#endif

	vertex_interp = vertex;
#ifdef NORMAL_USED
	normal_interp = normal;
#endif

#if defined(TANGENT_USED) || defined(NORMAL_MAP_USED) || defined(LIGHT_ANISOTROPY_USED)
	tangent_interp = tangent;
	binormal_interp = binormal;
#endif

#if defined(OVERRIDE_POSITION)
	gl_Position = position;
#else
	gl_Position = projection_matrix * vec4(vertex_interp, 1.0);
#endif
}

/* clang-format off */
#[fragment]

// Default to SPECULAR_SCHLICK_GGX.
#if !defined(SPECULAR_DISABLED) && !defined(SPECULAR_SCHLICK_GGX) && !defined(SPECULAR_TOON)
#define SPECULAR_SCHLICK_GGX
#endif

#if !defined(MODE_RENDER_DEPTH) || defined(TANGENT_USED) || defined(NORMAL_MAP_USED) || defined(LIGHT_ANISOTROPY_USED) ||defined(LIGHT_CLEARCOAT_USED)
#ifndef NORMAL_USED
#define NORMAL_USED
#endif
#endif

#ifndef MODE_RENDER_DEPTH
#include "tonemap_inc.glsl"
#endif
#include "stdlib_inc.glsl"

/* texture unit usage, N is max_texture_unity-N

1-color correction // In tonemap_inc.glsl
2-radiance
3-directional_shadow
4-positional_shadow
5-screen
6-depth

*/

#define M_PI 3.14159265359
/* clang-format on */

#define SHADER_IS_SRGB true

/* Varyings */

#if defined(COLOR_USED)
in vec4 color_interp;
#endif

#if defined(UV_USED)
in vec2 uv_interp;
#endif

#if defined(UV2_USED)
in vec2 uv2_interp;
#else
#ifdef USE_LIGHTMAP
in vec2 uv2_interp;
#endif
#endif

#if defined(TANGENT_USED) || defined(NORMAL_MAP_USED) || defined(LIGHT_ANISOTROPY_USED)
in vec3 tangent_interp;
in vec3 binormal_interp;
#endif

#ifdef NORMAL_USED
in vec3 normal_interp;
#endif

in highp vec3 vertex_interp;

#ifdef USE_RADIANCE_MAP

#define RADIANCE_MAX_LOD 5.0

uniform samplerCube radiance_map; // texunit:-2

#endif

layout(std140) uniform GlobalShaderUniformData { //ubo:1
	vec4 global_shader_uniforms[MAX_GLOBAL_SHADER_UNIFORMS];
};

	/* Material Uniforms */

#ifdef MATERIAL_UNIFORMS_USED

/* clang-format off */
layout(std140) uniform MaterialUniforms { // ubo:3

#MATERIAL_UNIFORMS

};
/* clang-format on */

#endif

layout(std140) uniform SceneData { // ubo:2
	highp mat4 projection_matrix;
	highp mat4 inv_projection_matrix;
	highp mat4 inv_view_matrix;
	highp mat4 view_matrix;

	vec2 viewport_size;
	vec2 screen_pixel_size;

	mediump vec4 ambient_light_color_energy;

	mediump float ambient_color_sky_mix;
	bool material_uv2_mode;
	float emissive_exposure_normalization;
	bool use_ambient_light;
	bool use_ambient_cubemap;
	bool use_reflection_cubemap;

	float fog_aerial_perspective;
	float time;

	mat3 radiance_inverse_xform;

	uint directional_light_count;
	float z_far;
	float z_near;
	float IBL_exposure_normalization;

	bool fog_enabled;
	float fog_density;
	float fog_height;
	float fog_height_density;

	vec3 fog_light_color;
	float fog_sun_scatter;
	uint camera_visible_layers;
	uint pad3;
	uint pad4;
	uint pad5;
}
scene_data;

#ifdef USE_MULTIVIEW
layout(std140) uniform MultiviewData { // ubo:8
	highp mat4 projection_matrix_view[MAX_VIEWS];
	highp mat4 inv_projection_matrix_view[MAX_VIEWS];
	highp vec4 eye_offset[MAX_VIEWS];
}
multiview_data;
#endif

/* clang-format off */

#GLOBALS

/* clang-format on */

// Directional light data.
#ifndef DISABLE_LIGHT_DIRECTIONAL

struct DirectionalLightData {
	mediump vec3 direction;
	mediump float energy;
	mediump vec3 color;
	mediump float size;
	mediump vec3 pad;
	mediump float specular;
};

layout(std140) uniform DirectionalLights { // ubo:7
	DirectionalLightData directional_lights[MAX_DIRECTIONAL_LIGHT_DATA_STRUCTS];
};

#endif // !DISABLE_LIGHT_DIRECTIONAL

// Omni and spot light data.
#if !defined(DISABLE_LIGHT_OMNI) || !defined(DISABLE_LIGHT_SPOT)

struct LightData { // This structure needs to be as packed as possible.
	highp vec3 position;
	highp float inv_radius;

	mediump vec3 direction;
	highp float size;

	mediump vec3 color;
	mediump float attenuation;

	mediump float cone_attenuation;
	mediump float cone_angle;
	mediump float specular_amount;
	mediump float shadow_opacity;
};

#ifndef DISABLE_LIGHT_OMNI
layout(std140) uniform OmniLightData { // ubo:5
	LightData omni_lights[MAX_LIGHT_DATA_STRUCTS];
};
uniform uint omni_light_indices[MAX_FORWARD_LIGHTS];
uniform uint omni_light_count;
#endif

#ifndef DISABLE_LIGHT_SPOT
layout(std140) uniform SpotLightData { // ubo:6
	LightData spot_lights[MAX_LIGHT_DATA_STRUCTS];
};
uniform uint spot_light_indices[MAX_FORWARD_LIGHTS];
uniform uint spot_light_count;
#endif

#ifdef USE_ADDITIVE_LIGHTING
uniform highp samplerCubeShadow positional_shadow; // texunit:-4
#endif

#endif // !defined(DISABLE_LIGHT_OMNI) || !defined(DISABLE_LIGHT_SPOT)

#ifdef USE_MULTIVIEW
uniform highp sampler2DArray depth_buffer; // texunit:-6
uniform highp sampler2DArray color_buffer; // texunit:-5
vec3 multiview_uv(vec2 uv) {
	return vec3(uv, ViewIndex);
}
#else
uniform highp sampler2D depth_buffer; // texunit:-6
uniform highp sampler2D color_buffer; // texunit:-5
vec2 multiview_uv(vec2 uv) {
	return uv;
}
#endif

uniform highp mat4 world_transform;
uniform mediump float opaque_prepass_threshold;

layout(location = 0) out vec4 frag_color;

vec3 F0(float metallic, float specular, vec3 albedo) {
	float dielectric = 0.16 * specular * specular;
	// use albedo * metallic as colored specular reflectance at 0 angle for metallic materials;
	// see https://google.github.io/filament/Filament.md.html
	return mix(vec3(dielectric), albedo, vec3(metallic));
}

#if !defined(DISABLE_LIGHT_DIRECTIONAL) || !defined(DISABLE_LIGHT_OMNI) || !defined(DISABLE_LIGHT_SPOT)

float D_GGX(float cos_theta_m, float alpha) {
	float a = cos_theta_m * alpha;
	float k = alpha / (1.0 - cos_theta_m * cos_theta_m + a * a);
	return k * k * (1.0 / M_PI);
}

// From Earl Hammon, Jr. "PBR Diffuse Lighting for GGX+Smith Microsurfaces" https://www.gdcvault.com/play/1024478/PBR-Diffuse-Lighting-for-GGX
float V_GGX(float NdotL, float NdotV, float alpha) {
	return 0.5 / mix(2.0 * NdotL * NdotV, NdotL + NdotV, alpha);
}

float D_GGX_anisotropic(float cos_theta_m, float alpha_x, float alpha_y, float cos_phi, float sin_phi) {
	float alpha2 = alpha_x * alpha_y;
	highp vec3 v = vec3(alpha_y * cos_phi, alpha_x * sin_phi, alpha2 * cos_theta_m);
	highp float v2 = dot(v, v);
	float w2 = alpha2 / v2;
	float D = alpha2 * w2 * w2 * (1.0 / M_PI);
	return D;
}

float V_GGX_anisotropic(float alpha_x, float alpha_y, float TdotV, float TdotL, float BdotV, float BdotL, float NdotV, float NdotL) {
	float Lambda_V = NdotL * length(vec3(alpha_x * TdotV, alpha_y * BdotV, NdotV));
	float Lambda_L = NdotV * length(vec3(alpha_x * TdotL, alpha_y * BdotL, NdotL));
	return 0.5 / (Lambda_V + Lambda_L);
}

float SchlickFresnel(float u) {
	float m = 1.0 - u;
	float m2 = m * m;
	return m2 * m2 * m; // pow(m,5)
}

void light_compute(vec3 N, vec3 L, vec3 V, float A, vec3 light_color, bool is_directional, float attenuation, vec3 f0, float roughness, float metallic, float specular_amount, vec3 albedo, inout float alpha,
#ifdef LIGHT_BACKLIGHT_USED
		vec3 backlight,
#endif
#ifdef LIGHT_RIM_USED
		float rim, float rim_tint,
#endif
#ifdef LIGHT_CLEARCOAT_USED
		float clearcoat, float clearcoat_roughness, vec3 vertex_normal,
#endif
#ifdef LIGHT_ANISOTROPY_USED
		vec3 B, vec3 T, float anisotropy,
#endif
		inout vec3 diffuse_light, inout vec3 specular_light) {

#if defined(USE_LIGHT_SHADER_CODE)
	// light is written by the light shader

	vec3 normal = N;
	vec3 light = L;
	vec3 view = V;

	/* clang-format off */

#CODE : LIGHT

	/* clang-format on */

#else
	float NdotL = min(A + dot(N, L), 1.0);
	float cNdotL = max(NdotL, 0.0); // clamped NdotL
	float NdotV = dot(N, V);
	float cNdotV = max(NdotV, 1e-4);

#if defined(DIFFUSE_BURLEY) || defined(SPECULAR_SCHLICK_GGX) || defined(LIGHT_CLEARCOAT_USED)
	vec3 H = normalize(V + L);
#endif

#if defined(SPECULAR_SCHLICK_GGX)
	float cNdotH = clamp(A + dot(N, H), 0.0, 1.0);
#endif

#if defined(DIFFUSE_BURLEY) || defined(SPECULAR_SCHLICK_GGX) || defined(LIGHT_CLEARCOAT_USED)
	float cLdotH = clamp(A + dot(L, H), 0.0, 1.0);
#endif

	if (metallic < 1.0) {
		float diffuse_brdf_NL; // BRDF times N.L for calculating diffuse radiance

#if defined(DIFFUSE_LAMBERT_WRAP)
		// Energy conserving lambert wrap shader.
		// https://web.archive.org/web/20210228210901/http://blog.stevemcauley.com/2011/12/03/energy-conserving-wrapped-diffuse/
		diffuse_brdf_NL = max(0.0, (NdotL + roughness) / ((1.0 + roughness) * (1.0 + roughness))) * (1.0 / M_PI);
#elif defined(DIFFUSE_TOON)
		diffuse_brdf_NL = smoothstep(-roughness, max(roughness, 0.01), NdotL) * (1.0 / M_PI);
#elif defined(DIFFUSE_BURLEY)
		{
			float FD90_minus_1 = 2.0 * cLdotH * cLdotH * roughness - 0.5;
			float FdV = 1.0 + FD90_minus_1 * SchlickFresnel(cNdotV);
			float FdL = 1.0 + FD90_minus_1 * SchlickFresnel(cNdotL);
			diffuse_brdf_NL = (1.0 / M_PI) * FdV * FdL * cNdotL;
		}
#else
		// Lambert
		diffuse_brdf_NL = cNdotL * (1.0 / M_PI);
#endif

		diffuse_light += light_color * diffuse_brdf_NL * attenuation;

#if defined(LIGHT_BACKLIGHT_USED)
		diffuse_light += light_color * (vec3(1.0 / M_PI) - diffuse_brdf_NL) * backlight * attenuation;
#endif

#if defined(LIGHT_RIM_USED)
		// Epsilon min to prevent pow(0, 0) singularity which results in undefined behavior.
		float rim_light = pow(max(1e-4, 1.0 - cNdotV), max(0.0, (1.0 - roughness) * 16.0));
		diffuse_light += rim_light * rim * mix(vec3(1.0), albedo, rim_tint) * light_color;
#endif
	}

	if (roughness > 0.0) { // FIXME: roughness == 0 should not disable specular light entirely

		// D

#if defined(SPECULAR_TOON)

		vec3 R = normalize(-reflect(L, N));
		float RdotV = dot(R, V);
		float mid = 1.0 - roughness;
		mid *= mid;
		float intensity = smoothstep(mid - roughness * 0.5, mid + roughness * 0.5, RdotV) * mid;
		diffuse_light += light_color * intensity * attenuation * specular_amount; // write to diffuse_light, as in toon shading you generally want no reflection

#elif defined(SPECULAR_DISABLED)
		// none..

#elif defined(SPECULAR_SCHLICK_GGX)
		// shlick+ggx as default
		float alpha_ggx = roughness * roughness;
#if defined(LIGHT_ANISOTROPY_USED)
		float aspect = sqrt(1.0 - anisotropy * 0.9);
		float ax = alpha_ggx / aspect;
		float ay = alpha_ggx * aspect;
		float XdotH = dot(T, H);
		float YdotH = dot(B, H);
		float D = D_GGX_anisotropic(cNdotH, ax, ay, XdotH, YdotH);
		float G = V_GGX_anisotropic(ax, ay, dot(T, V), dot(T, L), dot(B, V), dot(B, L), cNdotV, cNdotL);
#else
		float D = D_GGX(cNdotH, alpha_ggx);
		float G = V_GGX(cNdotL, cNdotV, alpha_ggx);
#endif // LIGHT_ANISOTROPY_USED
	   // F
		float cLdotH5 = SchlickFresnel(cLdotH);
		// Calculate Fresnel using cheap approximate specular occlusion term from Filament:
		// https://google.github.io/filament/Filament.html#lighting/occlusion/specularocclusion
		float f90 = clamp(50.0 * f0.g, 0.0, 1.0);
		vec3 F = f0 + (f90 - f0) * cLdotH5;

		vec3 specular_brdf_NL = cNdotL * D * F * G;

		specular_light += specular_brdf_NL * light_color * attenuation * specular_amount;
#endif

#if defined(LIGHT_CLEARCOAT_USED)
		// Clearcoat ignores normal_map, use vertex normal instead
		float ccNdotL = max(min(A + dot(vertex_normal, L), 1.0), 0.0);
		float ccNdotH = clamp(A + dot(vertex_normal, H), 0.0, 1.0);
		float ccNdotV = max(dot(vertex_normal, V), 1e-4);

#if !defined(SPECULAR_SCHLICK_GGX)
		float cLdotH5 = SchlickFresnel(cLdotH);
#endif
		float Dr = D_GGX(ccNdotH, mix(0.001, 0.1, clearcoat_roughness));
		float Gr = 0.25 / (cLdotH * cLdotH);
		float Fr = mix(.04, 1.0, cLdotH5);
		float clearcoat_specular_brdf_NL = clearcoat * Gr * Fr * Dr * cNdotL;

		specular_light += clearcoat_specular_brdf_NL * light_color * attenuation * specular_amount;
		// TODO: Clearcoat adds light to the scene right now (it is non-energy conserving), both diffuse and specular need to be scaled by (1.0 - FR)
		// but to do so we need to rearrange this entire function
#endif // LIGHT_CLEARCOAT_USED
	}

#ifdef USE_SHADOW_TO_OPACITY
	alpha = min(alpha, clamp(1.0 - attenuation, 0.0, 1.0));
#endif

#endif // USE_LIGHT_SHADER_CODE
}

float get_omni_spot_attenuation(float distance, float inv_range, float decay) {
	float nd = distance * inv_range;
	nd *= nd;
	nd *= nd; // nd^4
	nd = max(1.0 - nd, 0.0);
	nd *= nd; // nd^2
	return nd * pow(max(distance, 0.0001), -decay);
}

#ifndef DISABLE_LIGHT_OMNI
void light_process_omni(uint idx, vec3 vertex, vec3 eye_vec, vec3 normal, vec3 f0, float roughness, float metallic, float shadow, vec3 albedo, inout float alpha,
#ifdef LIGHT_BACKLIGHT_USED
		vec3 backlight,
#endif
#ifdef LIGHT_RIM_USED
		float rim, float rim_tint,
#endif
#ifdef LIGHT_CLEARCOAT_USED
		float clearcoat, float clearcoat_roughness, vec3 vertex_normal,
#endif
#ifdef LIGHT_ANISOTROPY_USED
		vec3 binormal, vec3 tangent, float anisotropy,
#endif
		inout vec3 diffuse_light, inout vec3 specular_light) {
	vec3 light_rel_vec = omni_lights[idx].position - vertex;
	float light_length = length(light_rel_vec);
	float omni_attenuation = get_omni_spot_attenuation(light_length, omni_lights[idx].inv_radius, omni_lights[idx].attenuation);
	vec3 color = omni_lights[idx].color;
	float size_A = 0.0;

	if (omni_lights[idx].size > 0.0) {
		float t = omni_lights[idx].size / max(0.001, light_length);
		size_A = max(0.0, 1.0 - 1.0 / sqrt(1.0 + t * t));
	}

	light_compute(normal, normalize(light_rel_vec), eye_vec, size_A, color, false, omni_attenuation, f0, roughness, metallic, omni_lights[idx].specular_amount, albedo, alpha,
#ifdef LIGHT_BACKLIGHT_USED
			backlight,
#endif
#ifdef LIGHT_RIM_USED
			rim * omni_attenuation, rim_tint,
#endif
#ifdef LIGHT_CLEARCOAT_USED
			clearcoat, clearcoat_roughness, vertex_normal,
#endif
#ifdef LIGHT_ANISOTROPY_USED
			binormal, tangent, anisotropy,
#endif
			diffuse_light,
			specular_light);
}
#endif // !DISABLE_LIGHT_OMNI

#ifndef DISABLE_LIGHT_SPOT
void light_process_spot(uint idx, vec3 vertex, vec3 eye_vec, vec3 normal, vec3 f0, float roughness, float metallic, float shadow, vec3 albedo, inout float alpha,
#ifdef LIGHT_BACKLIGHT_USED
		vec3 backlight,
#endif
#ifdef LIGHT_RIM_USED
		float rim, float rim_tint,
#endif
#ifdef LIGHT_CLEARCOAT_USED
		float clearcoat, float clearcoat_roughness, vec3 vertex_normal,
#endif
#ifdef LIGHT_ANISOTROPY_USED
		vec3 binormal, vec3 tangent, float anisotropy,
#endif
		inout vec3 diffuse_light,
		inout vec3 specular_light) {

	vec3 light_rel_vec = spot_lights[idx].position - vertex;
	float light_length = length(light_rel_vec);
	float spot_attenuation = get_omni_spot_attenuation(light_length, spot_lights[idx].inv_radius, spot_lights[idx].attenuation);
	vec3 spot_dir = spot_lights[idx].direction;
	float scos = max(dot(-normalize(light_rel_vec), spot_dir), spot_lights[idx].cone_angle);
	float spot_rim = max(0.0001, (1.0 - scos) / (1.0 - spot_lights[idx].cone_angle));
	spot_attenuation *= 1.0 - pow(spot_rim, spot_lights[idx].cone_attenuation);
	vec3 color = spot_lights[idx].color;

	float size_A = 0.0;

	if (spot_lights[idx].size > 0.0) {
		float t = spot_lights[idx].size / max(0.001, light_length);
		size_A = max(0.0, 1.0 - 1.0 / sqrt(1.0 + t * t));
	}

	light_compute(normal, normalize(light_rel_vec), eye_vec, size_A, color, false, spot_attenuation, f0, roughness, metallic, spot_lights[idx].specular_amount, albedo, alpha,
#ifdef LIGHT_BACKLIGHT_USED
			backlight,
#endif
#ifdef LIGHT_RIM_USED
			rim * spot_attenuation, rim_tint,
#endif
#ifdef LIGHT_CLEARCOAT_USED
			clearcoat, clearcoat_roughness, vertex_normal,
#endif
#ifdef LIGHT_ANISOTROPY_USED
			binormal, tangent, anisotropy,
#endif
			diffuse_light, specular_light);
}
#endif // !DISABLE_LIGHT_SPOT

#endif // !defined(DISABLE_LIGHT_DIRECTIONAL) || !defined(DISABLE_LIGHT_OMNI) || !defined(DISABLE_LIGHT_SPOT)

#ifndef MODE_RENDER_DEPTH
vec4 fog_process(vec3 vertex) {
	vec3 fog_color = scene_data.fog_light_color;

#ifdef USE_RADIANCE_MAP
/*
		if (scene_data.fog_aerial_perspective > 0.0) {
		vec3 sky_fog_color = vec3(0.0);
		vec3 cube_view = scene_data.radiance_inverse_xform * vertex;
		// mip_level always reads from the second mipmap and higher so the fog is always slightly blurred
		float mip_level = mix(1.0 / MAX_ROUGHNESS_LOD, 1.0, 1.0 - (abs(vertex.z) - scene_data.z_near) / (scene_data.z_far - scene_data.z_near));

		sky_fog_color = textureLod(radiance_map, cube_view, mip_level * RADIANCE_MAX_LOD).rgb;

		fog_color = mix(fog_color, sky_fog_color, scene_data.fog_aerial_perspective);
	}
	*/
#endif

#ifndef DISABLE_LIGHT_DIRECTIONAL
	if (scene_data.fog_sun_scatter > 0.001) {
		vec4 sun_scatter = vec4(0.0);
		float sun_total = 0.0;
		vec3 view = normalize(vertex);
		for (uint i = uint(0); i < scene_data.directional_light_count; i++) {
			vec3 light_color = directional_lights[i].color * directional_lights[i].energy;
			float light_amount = pow(max(dot(view, directional_lights[i].direction), 0.0), 8.0);
			fog_color += light_color * light_amount * scene_data.fog_sun_scatter;
		}
	}
#endif // !DISABLE_LIGHT_DIRECTIONAL

	float fog_amount = 1.0 - exp(min(0.0, -length(vertex) * scene_data.fog_density));

	if (abs(scene_data.fog_height_density) >= 0.0001) {
		float y = (scene_data.inv_view_matrix * vec4(vertex, 1.0)).y;

		float y_dist = y - scene_data.fog_height;

		float vfog_amount = 1.0 - exp(min(0.0, y_dist * scene_data.fog_height_density));

		fog_amount = max(vfog_amount, fog_amount);
	}

	return vec4(fog_color, fog_amount);
}

#endif // !MODE_RENDER_DEPTH

void main() {
	//lay out everything, whatever is unused is optimized away anyway
	vec3 vertex = vertex_interp;
#ifdef USE_MULTIVIEW
	vec3 eye_offset = multiview_data.eye_offset[ViewIndex].xyz;
	vec3 view = -normalize(vertex_interp - eye_offset);
	mat4 projection_matrix = multiview_data.projection_matrix_view[ViewIndex];
	mat4 inv_projection_matrix = multiview_data.inv_projection_matrix_view[ViewIndex];
#else
	vec3 eye_offset = vec3(0.0, 0.0, 0.0);
	vec3 view = -normalize(vertex_interp);
	mat4 projection_matrix = scene_data.projection_matrix;
	mat4 inv_projection_matrix = scene_data.inv_projection_matrix;
#endif
	highp mat4 model_matrix = world_transform;
	vec3 albedo = vec3(1.0);
	vec3 backlight = vec3(0.0);
	vec4 transmittance_color = vec4(0.0, 0.0, 0.0, 1.0);
	float transmittance_depth = 0.0;
	float transmittance_boost = 0.0;
	float metallic = 0.0;
	float specular = 0.5;
	vec3 emission = vec3(0.0);
	float roughness = 1.0;
	float rim = 0.0;
	float rim_tint = 0.0;
	float clearcoat = 0.0;
	float clearcoat_roughness = 0.0;
	float anisotropy = 0.0;
	vec2 anisotropy_flow = vec2(1.0, 0.0);
	vec4 fog = vec4(0.0);
#if defined(CUSTOM_RADIANCE_USED)
	vec4 custom_radiance = vec4(0.0);
#endif
#if defined(CUSTOM_IRRADIANCE_USED)
	vec4 custom_irradiance = vec4(0.0);
#endif

	float ao = 1.0;
	float ao_light_affect = 0.0;

	float alpha = 1.0;

#define projection_matrix scene_data.projection_matrix
#define inv_projection_matrix scene_data.inv_projection_matrix

#if defined(TANGENT_USED) || defined(NORMAL_MAP_USED) || defined(LIGHT_ANISOTROPY_USED)
	vec3 binormal = normalize(binormal_interp);
	vec3 tangent = normalize(tangent_interp);
#else
	vec3 binormal = vec3(0.0);
	vec3 tangent = vec3(0.0);
#endif

#ifdef NORMAL_USED
	vec3 normal = normalize(normal_interp);

#if defined(DO_SIDE_CHECK)
	if (!gl_FrontFacing) {
		normal = -normal;
	}
#endif

#endif //NORMAL_USED

#ifdef UV_USED
	vec2 uv = uv_interp;
#endif

#if defined(UV2_USED) || defined(USE_LIGHTMAP)
	vec2 uv2 = uv2_interp;
#endif

#if defined(COLOR_USED)
	vec4 color = color_interp;
#endif

#if defined(NORMAL_MAP_USED)

	vec3 normal_map = vec3(0.5);
#endif

	float normal_map_depth = 1.0;

	vec2 screen_uv = gl_FragCoord.xy * scene_data.screen_pixel_size;

	float sss_strength = 0.0;

#ifdef ALPHA_SCISSOR_USED
	float alpha_scissor_threshold = 1.0;
#endif // ALPHA_SCISSOR_USED

#ifdef ALPHA_HASH_USED
	float alpha_hash_scale = 1.0;
#endif // ALPHA_HASH_USED

#ifdef ALPHA_ANTIALIASING_EDGE_USED
	float alpha_antialiasing_edge = 0.0;
	vec2 alpha_texture_coordinate = vec2(0.0, 0.0);
#endif // ALPHA_ANTIALIASING_EDGE_USED
	{
#CODE : FRAGMENT
	}

// Normalize these only once if the user fragment shader set them
// FIXME: It would be nice to have NORMAL_GET and NORMAL_SET instead to separate the read and write cases
#ifdef NORMAL_USED
	normal = normalize(normal);
#endif
#ifdef TANGENT_USED
	tangent = normalize(tangent);
	binormal = normalize(binormal);
#endif

#ifdef DEPTH_USED
	vec3 ndc = vec3(screen_uv * 2.0 - 1.0, gl_FragDepth);
	vec4 view_pos = inv_projection_matrix * vec4(ndc, 1.0);
	// Vertex is in view space, the rays are not parallel. It is not enough to update the z component!
	vertex = view_pos.xyz / view_pos.w;
	view = -normalize(vertex);
#endif

#ifndef USE_SHADOW_TO_OPACITY

#if defined(ALPHA_SCISSOR_USED)
	if (alpha < alpha_scissor_threshold) {
		discard;
	}
#endif // ALPHA_SCISSOR_USED

#ifdef USE_OPAQUE_PREPASS
#if !defined(ALPHA_SCISSOR_USED)

	if (alpha < opaque_prepass_threshold) {
		discard;
	}

#endif // not ALPHA_SCISSOR_USED
#endif // USE_OPAQUE_PREPASS

#endif // !USE_SHADOW_TO_OPACITY

#ifdef NORMAL_MAP_USED
	// Save the normal vector before applying the normal map.
	// This must be used instead of normal_interp, since the
	// user FRAGMENT code may have set NORMAL.
	vec3 geometric_normal = normal;

	normal_map.xy = normal_map.xy * 2.0 - 1.0;
	normal_map.z = sqrt(max(0.0, 1.0 - dot(normal_map.xy, normal_map.xy))); //always ignore Z, as it can be RG packed, Z may be pos/neg, etc.

	normal = normalize(mix(normal, tangent * normal_map.x + binormal * normal_map.y + normal * normal_map.z, normal_map_depth));
#else
#define geometric_normal normal
#endif // NORMAL_MAP_USED

#ifdef LIGHT_ANISOTROPY_USED

	if (anisotropy > 0.01) {
		//rotation matrix
		mat3 rot = mat3(tangent, binormal, normal);
		//make local to space
		tangent = normalize(rot * vec3(anisotropy_flow.x, anisotropy_flow.y, 0.0));
		binormal = normalize(rot * vec3(-anisotropy_flow.y, anisotropy_flow.x, 0.0));
	}

#endif

#ifndef MODE_RENDER_DEPTH

#ifndef CUSTOM_FOG_USED
#ifndef DISABLE_FOG
	// fog must be processed as early as possible and then packed.
	// to maximize VGPR usage

	if (scene_data.fog_enabled) {
		fog = fog_process(vertex);
	}
#endif // !DISABLE_FOG
#endif // !CUSTOM_FOG_USED

	uint fog_rg = packHalf2x16(fog.rg);
	uint fog_ba = packHalf2x16(fog.ba);

	// Convert colors to linear
	albedo = srgb_to_linear(albedo);
	emission = srgb_to_linear(emission);
	// TODO Backlight and transmittance when used
#ifndef MODE_UNSHADED
	vec3 f0 = F0(metallic, specular, albedo);
	vec3 specular_light = vec3(0.0, 0.0, 0.0);
	vec3 diffuse_light = vec3(0.0, 0.0, 0.0);
	vec3 ambient_light = vec3(0.0, 0.0, 0.0);

#ifdef BASE_PASS
	/////////////////////// LIGHTING //////////////////////////////

	// IBL precalculations
	float ndotv = clamp(dot(normal, view), 0.0, 1.0);
	vec3 F = f0 + (max(vec3(1.0 - roughness), f0) - f0) * pow(1.0 - ndotv, 5.0);

#ifdef USE_RADIANCE_MAP
	if (scene_data.use_reflection_cubemap) {
#ifdef LIGHT_ANISOTROPY_USED
		// https://google.github.io/filament/Filament.html#lighting/imagebasedlights/anisotropy
		vec3 anisotropic_direction = anisotropy >= 0.0 ? binormal : tangent;
		vec3 anisotropic_tangent = cross(anisotropic_direction, view);
		vec3 anisotropic_normal = cross(anisotropic_tangent, anisotropic_direction);
		vec3 bent_normal = normalize(mix(normal, anisotropic_normal, abs(anisotropy) * clamp(5.0 * roughness, 0.0, 1.0)));
		vec3 ref_vec = reflect(-view, bent_normal);
#else
		vec3 ref_vec = reflect(-view, normal);
#endif
		ref_vec = mix(ref_vec, normal, roughness * roughness);
		float horizon = min(1.0 + dot(ref_vec, normal), 1.0);
		ref_vec = scene_data.radiance_inverse_xform * ref_vec;
		specular_light = textureLod(radiance_map, ref_vec, sqrt(roughness) * RADIANCE_MAX_LOD).rgb;
		specular_light = srgb_to_linear(specular_light);
		specular_light *= horizon * horizon;
		specular_light *= scene_data.ambient_light_color_energy.a;
	}
#endif

	// Calculate Reflection probes
	// Calculate Lightmaps

#if defined(CUSTOM_RADIANCE_USED)
	specular_light = mix(specular_light, custom_radiance.rgb, custom_radiance.a);
#endif // CUSTOM_RADIANCE_USED

#ifndef USE_LIGHTMAP
	//lightmap overrides everything
	if (scene_data.use_ambient_light) {
		ambient_light = scene_data.ambient_light_color_energy.rgb;
#ifdef USE_RADIANCE_MAP
		if (scene_data.use_ambient_cubemap) {
			vec3 ambient_dir = scene_data.radiance_inverse_xform * normal;
			vec3 cubemap_ambient = textureLod(radiance_map, ambient_dir, RADIANCE_MAX_LOD).rgb;
			cubemap_ambient = srgb_to_linear(cubemap_ambient);
			ambient_light = mix(ambient_light, cubemap_ambient * scene_data.ambient_light_color_energy.a, scene_data.ambient_color_sky_mix);
		}
#endif
	}
#endif // USE_LIGHTMAP

#if defined(CUSTOM_IRRADIANCE_USED)
	ambient_light = mix(ambient_light, custom_irradiance.rgb, custom_irradiance.a);
#endif // CUSTOM_IRRADIANCE_USED

	{
#if defined(AMBIENT_LIGHT_DISABLED)
		ambient_light = vec3(0.0, 0.0, 0.0);
#else
		ambient_light *= albedo.rgb;
		ambient_light *= ao;
#endif // AMBIENT_LIGHT_DISABLED
	}

	// convert ao to direct light ao
	ao = mix(1.0, ao, ao_light_affect);

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
		specular_light *= env.x * f0 + env.y * clamp(50.0 * f0.g, metallic, 1.0);
#endif
	}

#endif // BASE_PASS

#ifndef DISABLE_LIGHT_DIRECTIONAL
	//diffuse_light = normal; //
	for (uint i = uint(0); i < scene_data.directional_light_count; i++) {
		light_compute(normal, normalize(directional_lights[i].direction), view, directional_lights[i].size, directional_lights[i].color * directional_lights[i].energy, true, 1.0, f0, roughness, metallic, 1.0, albedo, alpha,
#ifdef LIGHT_BACKLIGHT_USED
				backlight,
#endif
#ifdef LIGHT_RIM_USED
				rim, rim_tint,
#endif
#ifdef LIGHT_CLEARCOAT_USED
				clearcoat, clearcoat_roughness, geometric_normal,
#endif
#ifdef LIGHT_ANISOTROPY_USED
				binormal,
				tangent, anisotropy,
#endif
				diffuse_light,
				specular_light);
	}
#endif // !DISABLE_LIGHT_DIRECTIONAL

#ifndef DISABLE_LIGHT_OMNI
	for (uint i = 0u; i < MAX_FORWARD_LIGHTS; i++) {
		if (i >= omni_light_count) {
			break;
		}
		light_process_omni(omni_light_indices[i], vertex, view, normal, f0, roughness, metallic, 0.0, albedo, alpha,
#ifdef LIGHT_BACKLIGHT_USED
				backlight,
#endif
#ifdef LIGHT_RIM_USED
				rim,
				rim_tint,
#endif
#ifdef LIGHT_CLEARCOAT_USED
				clearcoat, clearcoat_roughness, geometric_normal,
#endif
#ifdef LIGHT_ANISOTROPY_USED
				binormal, tangent, anisotropy,
#endif
				diffuse_light, specular_light);
	}
#endif // !DISABLE_LIGHT_OMNI

#ifndef DISABLE_LIGHT_SPOT
	for (uint i = 0u; i < MAX_FORWARD_LIGHTS; i++) {
		if (i >= spot_light_count) {
			break;
		}
		light_process_spot(spot_light_indices[i], vertex, view, normal, f0, roughness, metallic, 0.0, albedo, alpha,
#ifdef LIGHT_BACKLIGHT_USED
				backlight,
#endif
#ifdef LIGHT_RIM_USED
				rim,
				rim_tint,
#endif
#ifdef LIGHT_CLEARCOAT_USED
				clearcoat, clearcoat_roughness, geometric_normal,
#endif
#ifdef LIGHT_ANISOTROPY_USED
				tangent,
				binormal, anisotropy,
#endif
				diffuse_light, specular_light);
	}
#endif // !DISABLE_LIGHT_SPOT

#endif // !MODE_UNSHADED

#endif // !MODE_RENDER_DEPTH

#if defined(USE_SHADOW_TO_OPACITY)
	alpha = min(alpha, clamp(length(ambient_light), 0.0, 1.0));

#if defined(ALPHA_SCISSOR_USED)
	if (alpha < alpha_scissor) {
		discard;
	}
#endif // ALPHA_SCISSOR_USED

#ifdef USE_OPAQUE_PREPASS
#if !defined(ALPHA_SCISSOR_USED)

	if (alpha < opaque_prepass_threshold) {
		discard;
	}

#endif // not ALPHA_SCISSOR_USED
#endif // USE_OPAQUE_PREPASS

#endif // USE_SHADOW_TO_OPACITY

#ifdef MODE_RENDER_DEPTH
//nothing happens, so a tree-ssa optimizer will result in no fragment shader :)
#else // !MODE_RENDER_DEPTH

#ifdef MODE_UNSHADED
	frag_color = vec4(albedo, alpha);
#else

	diffuse_light *= albedo;

	diffuse_light *= 1.0 - metallic;
	ambient_light *= 1.0 - metallic;

	frag_color = vec4(diffuse_light + specular_light, alpha);
#ifdef BASE_PASS
	frag_color.rgb += emission + ambient_light;
#endif
#endif //MODE_UNSHADED
	fog = vec4(unpackHalf2x16(fog_rg), unpackHalf2x16(fog_ba));

#ifndef DISABLE_FOG
	if (scene_data.fog_enabled) {
#ifdef BASE_PASS
		frag_color.rgb = mix(frag_color.rgb, fog.rgb, fog.a);
#else
		frag_color.rgb *= (1.0 - fog.a);
#endif // BASE_PASS
	}
#endif

	// Tonemap before writing as we are writing to an sRGB framebuffer
	frag_color.rgb *= exposure;
	frag_color.rgb = apply_tonemapping(frag_color.rgb, white);
	frag_color.rgb = linear_to_srgb(frag_color.rgb);

#ifdef USE_BCS
	frag_color.rgb = apply_bcs(frag_color.rgb, bcs);
#endif

#ifdef USE_COLOR_CORRECTION
	frag_color.rgb = apply_color_correction(frag_color.rgb, color_correction);
#endif

#endif //!MODE_RENDER_DEPTH
}
