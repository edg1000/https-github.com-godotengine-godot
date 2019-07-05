
/* SET0: Per draw primitive settings */

#define M_PI 3.14159265359

#define MAX_LIGHT_TEXTURES 1024
#define MAX_RENDER_LIGHTS 256

#define FLAGS_INSTANCING_STRIDE_MASK 0xF
#define FLAGS_INSTANCING_ENABLED (1<<4)
#define FLAGS_INSTANCING_HAS_COLORS (1 << 5)
#define FLAGS_INSTANCING_COLOR_8BIT (1 << 6)
#define FLAGS_INSTANCING_HAS_CUSTOM_DATA (1 << 7)
#define FLAGS_INSTANCING_CUSTOM_DATA_8_BIT (1 << 8)

#define FLAGS_CLIP_RECT_UV (1 << 9)
#define FLAGS_TRANSPOSE_RECT (1 << 10)
#define FLAGS_USING_LIGHT_MASK (1 << 11)
#define FLAGS_NINEPACH_DRAW_CENTER (1 << 12)
#define FLAGS_USING_PARTICLES (1 << 13)
#define FLAGS_USE_PIXEL_SNAP (1 << 14)

#define FLAGS_NINEPATCH_H_MODE_SHIFT 16
#define FLAGS_NINEPATCH_V_MODE_SHIFT 18

#define FLAGS_LIGHT_COUNT_SHIFT 20

#define FLAGS_DEFAULT_NORMAL_MAP_USED (1 << 26)
#define FLAGS_DEFAULT_SPECULAR_MAP_USED (1 << 27)

layout(push_constant, binding = 0, std430) uniform DrawData {
	vec2 world_x;
	vec2 world_y;
	vec2 world_ofs;
	uint flags;
	uint specular_shininess;
#ifdef USE_PRIMITIVE
	vec2 points[3];
	vec2 uvs[3];
	uint colors[6];
#else
	vec4 modulation;
	vec4 ninepatch_margins;
	vec4 dst_rect; //for built-in rect and UV
	vec4 src_rect;
	vec2 pad;

#endif
	vec2 color_texture_pixel_size;
	uint lights[4];

} draw_data;

// The values passed per draw primitives are cached within it

layout(set = 0, binding = 1) uniform texture2D color_texture;
layout(set = 0, binding = 2) uniform texture2D normal_texture;
layout(set = 0, binding = 3) uniform texture2D specular_texture;
layout(set = 0, binding = 4) uniform sampler texture_sampler;

layout(set = 0, binding = 5) uniform textureBuffer instancing_buffer;

/* SET1: Is reserved for the material */

//

/* SET2: Is the skeleton */

#ifdef USE_ATTRIBUTES

layout(set = 2, binding = 0) uniform textureBuffer skeleton_buffer;

layout(set = 2, binding = 1, std140) uniform SkeletonData {
	mat4 skeleton_transform; //in world coordinates
	mat4 skeleton_transform_inverse;
} skeleton_data;

#endif

/* SET3: Per Scene settings */

layout(set = 3, binding = 0, std140) uniform CanvasData {
	mat4 canvas_transform;
	mat4 screen_transform;
	mat4 canvas_normal_transform;
	vec4 canvas_modulation;
	//uint light_count;
} canvas_data;

#define LIGHT_FLAGS_TEXTURE_MASK 0xFFFF
#define LIGHT_FLAGS_BLEND_MASK (3<<16)
#define LIGHT_FLAGS_BLEND_MODE_ADD (0<<16)
#define LIGHT_FLAGS_BLEND_MODE_SUB (1<<16)
#define LIGHT_FLAGS_BLEND_MODE_MIX (2<<16)
#define LIGHT_FLAGS_BLEND_MODE_MASK (3<<16)


struct Light {
	mat2x4 matrix; //light to texture coordinate matrix
	vec4 color;
	vec4 shadow_color;
	vec2 position;
	uint flags; //index to light texture
	float height;
	float shadow_softness;
	float shadow_pixel_size;
	float pad0;
	float pad1;
};

layout(set = 3, binding = 1, std140) uniform LightData {
	Light data[MAX_RENDER_LIGHTS];
} light_array;

layout(set = 3, binding = 2) uniform texture2D light_textures[MAX_LIGHT_TEXTURES];
layout(set = 3, binding = 3) uniform texture2D shadow_textures[MAX_LIGHT_TEXTURES];
