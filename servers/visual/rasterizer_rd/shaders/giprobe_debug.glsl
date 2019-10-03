[vertex]

#version 450

VERSION_DEFINES

struct CellData {
	uint position; // xyz 10 bits
	uint albedo; //rgb albedo
	uint emission; //rgb normalized with e as multiplier
	uint normal; //RGB normal encoded
};

layout(set=0,binding=1,std140) buffer CellDataBuffer {
    CellData data[];
} cell_data;

layout (set=0,binding=2) uniform texture3D color_tex;

layout (set=0,binding=3) uniform sampler tex_sampler;

#ifdef USE_ANISOTROPY
layout (set=0,binding=4) uniform texture3D aniso_pos_tex;
layout (set=0,binding=5) uniform texture3D aniso_neg_tex;
#endif


layout(push_constant, binding = 0, std430) uniform Params {

	mat4 projection;
	uint cell_offset;
	float dynamic_range;
	float alpha;
	uint level;

} params;

layout(location=0) out vec4 color_interp;

void main() {

	const vec3 cube_triangles[36] = vec3[](
	    vec3(-1.0f,-1.0f,-1.0f),
	    vec3(-1.0f,-1.0f, 1.0f),
	    vec3(-1.0f, 1.0f, 1.0f),
	    vec3(1.0f, 1.0f,-1.0f),
	    vec3(-1.0f,-1.0f,-1.0f),
	    vec3(-1.0f, 1.0f,-1.0f),
	    vec3(1.0f,-1.0f, 1.0f),
	    vec3(-1.0f,-1.0f,-1.0f),
	    vec3(1.0f,-1.0f,-1.0f),
	    vec3(1.0f, 1.0f,-1.0f),
	    vec3(1.0f,-1.0f,-1.0f),
	    vec3(-1.0f,-1.0f,-1.0f),
	    vec3(-1.0f,-1.0f,-1.0f),
	    vec3(-1.0f, 1.0f, 1.0f),
	    vec3(-1.0f, 1.0f,-1.0f),
	    vec3(1.0f,-1.0f, 1.0f),
	    vec3(-1.0f,-1.0f, 1.0f),
	    vec3(-1.0f,-1.0f,-1.0f),
	    vec3(-1.0f, 1.0f, 1.0f),
	    vec3(-1.0f,-1.0f, 1.0f),
	    vec3(1.0f,-1.0f, 1.0f),
	    vec3(1.0f, 1.0f, 1.0f),
	    vec3(1.0f,-1.0f,-1.0f),
	    vec3(1.0f, 1.0f,-1.0f),
	    vec3(1.0f,-1.0f,-1.0f),
	    vec3(1.0f, 1.0f, 1.0f),
	    vec3(1.0f,-1.0f, 1.0f),
	    vec3(1.0f, 1.0f, 1.0f),
	    vec3(1.0f, 1.0f,-1.0f),
	    vec3(-1.0f, 1.0f,-1.0f),
	    vec3(1.0f, 1.0f, 1.0f),
	    vec3(-1.0f, 1.0f,-1.0f),
	    vec3(-1.0f, 1.0f, 1.0f),
	    vec3(1.0f, 1.0f, 1.0f),
	    vec3(-1.0f, 1.0f, 1.0f),
	    vec3(1.0f,-1.0f, 1.0f)
	);


	vec3 vertex = cube_triangles[gl_VertexIndex] * 0.5 + 0.5;

	uint cell_index = gl_InstanceIndex + params.cell_offset;

	uvec3 posu = uvec3(cell_data.data[cell_index].position&0x7FF,(cell_data.data[cell_index].position>>11)&0x3FF,cell_data.data[cell_index].position>>21);

#ifdef MODE_DEBUG_COLOR
	color_interp.xyz = unpackUnorm4x8(cell_data.data[cell_index].albedo).xyz;
#endif
#ifdef MODE_DEBUG_LIGHT

#ifdef USE_ANISOTROPY

#define POS_X 0
#define POS_Y 1
#define POS_Z 2
#define NEG_X 3
#define NEG_Y 4
#define NEG_Z 5

	const uint triangle_aniso[12] = uint[](
			NEG_X,
			NEG_Z,
			NEG_Y,
			NEG_Z,
			NEG_X,
			NEG_Y,
			POS_Z,
			POS_X,
			POS_X,
			POS_Y,
			POS_Y,
			POS_Z
	);

	color_interp.xyz = texelFetch(sampler3D(color_tex,tex_sampler),ivec3(posu),int(params.level)).xyz * params.dynamic_range;
	vec3 aniso_pos =  texelFetch(sampler3D(aniso_pos_tex,tex_sampler),ivec3(posu),int(params.level)).xyz;
	vec3 aniso_neg =  texelFetch(sampler3D(aniso_neg_tex,tex_sampler),ivec3(posu),int(params.level)).xyz;
	uint side = triangle_aniso[gl_VertexIndex/3];

	float strength = 0.0;
	switch(side) {
	case POS_X: strength = aniso_pos.x; break;
	case POS_Y: strength = aniso_pos.y; break;
	case POS_Z: strength = aniso_pos.z; break;
	case NEG_X: strength = aniso_neg.x; break;
	case NEG_Y: strength = aniso_neg.y; break;
	case NEG_Z: strength = aniso_neg.z; break;

	}

	color_interp.xyz *= strength;

#else
	color_interp.xyz = texelFetch(sampler3D(color_tex,tex_sampler),ivec3(posu),int(params.level)).xyz * params.dynamic_range;

#endif

#endif
	float scale = (1<<params.level);
	color_interp.a = params.alpha;

	gl_Position = params.projection * vec4((vec3(posu)+vertex)*scale,1.0);

}

[fragment]

#version 450

VERSION_DEFINES

layout(location=0) in vec4 color_interp;
layout(location=0) out vec4 frag_color;

void main() {

	frag_color = color_interp;
}
