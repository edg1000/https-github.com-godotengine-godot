/* clang-format off */
[vertex]

#ifdef USE_GLES_OVER_GL
#define lowp
#define mediump
#define highp
#else
precision mediump float;
precision mediump int;
#endif

attribute highp vec4 vertex_attrib; // attrib:0
/* clang-format on */
attribute vec2 uv_in; // attrib:4

varying vec2 uv_interp;

void main() {

	uv_interp = uv_in;
	gl_Position = vertex_attrib;
}

/* clang-format off */
[fragment]

#ifdef USE_GLES_OVER_GL
#define lowp
#define mediump
#define highp
#else
#if defined(USE_HIGHP_PRECISION)
precision highp float;
precision highp int;
#else
precision mediump float;
precision mediump int;
#endif
#endif

uniform highp samplerCube source_cube; //texunit:0
/* clang-format on */
varying vec2 uv_interp;

uniform bool z_flip;
uniform highp float z_far;
uniform highp float z_near;
uniform highp float bias;

void main() {

	highp vec3 normal = vec3(uv_interp * 2.0 - 1.0, 0.0);
	/*
	if (z_flip) {
		normal.z = 0.5 - 0.5 * ((normal.x * normal.x) + (normal.y * normal.y));
	} else {
		normal.z = -0.5 + 0.5 * ((normal.x * normal.x) + (normal.y * normal.y));
	}
	*/

	normal.z = 0.5 - 0.5 * ((normal.x * normal.x) + (normal.y * normal.y));
	normal = normalize(normal);
	/*
	normal.z = 0.5;
	normal = normalize(normal);
	*/

	if (!z_flip) {
		normal.z = -normal.z;
	}

	float depth = textureCube(source_cube, normal).r;

	// absolute values for direction cosines, bigger value equals closer to basis axis
	vec3 unorm = abs(normal);

	if ((unorm.x >= unorm.y) && (unorm.x >= unorm.z)) {
		// x code
		unorm = normal.x > 0.0 ? vec3(1.0, 0.0, 0.0) : vec3(-1.0, 0.0, 0.0);
	} else if ((unorm.y > unorm.x) && (unorm.y >= unorm.z)) {
		// y code
		unorm = normal.y > 0.0 ? vec3(0.0, 1.0, 0.0) : vec3(0.0, -1.0, 0.0);
	} else if ((unorm.z > unorm.x) && (unorm.z > unorm.y)) {
		// z code
		unorm = normal.z > 0.0 ? vec3(0.0, 0.0, 1.0) : vec3(0.0, 0.0, -1.0);
	} else {
		// oh-no we messed up code
		// has to be
		unorm = vec3(1.0, 0.0, 0.0);
	}

	float depth_fix = 1.0 / dot(normal, unorm);

	depth = 2.0 * depth - 1.0;
	float linear_depth = 2.0 * z_near * z_far / (z_far + z_near - depth * (z_far - z_near));
	gl_FragDepth = (linear_depth * depth_fix + bias) / z_far;
}
