// Functions related to gi/sdfgi for our forward renderer

//standard voxel cone trace
vec4 voxel_cone_trace(texture3D probe, vec3 cell_size, vec3 pos, vec3 direction, float tan_half_angle, float max_distance, float p_bias) {
	float dist = p_bias;
	vec4 color = vec4(0.0);

	while (dist < max_distance && color.a < 0.95) {
		float diameter = max(1.0, 2.0 * tan_half_angle * dist);
		vec3 uvw_pos = (pos + dist * direction) * cell_size;
		float half_diameter = diameter * 0.5;
		//check if outside, then break
		if (any(greaterThan(abs(uvw_pos - 0.5), vec3(0.5f + half_diameter * cell_size)))) {
			break;
		}
		vec4 scolor = textureLod(sampler3D(probe, SAMPLER_LINEAR_WITH_MIPMAPS_CLAMP), uvw_pos, log2(diameter));
		float a = (1.0 - color.a);
		color += a * scolor;
		dist += half_diameter;
	}

	return color;
}

vec4 voxel_cone_trace_45_degrees(texture3D probe, vec3 cell_size, vec3 pos, vec3 direction, float tan_half_angle, float max_distance, float p_bias) {
	float dist = p_bias;
	vec4 color = vec4(0.0);
	float radius = max(0.5, tan_half_angle * dist);
	float lod_level = log2(radius * 2.0);

	while (dist < max_distance && color.a < 0.95) {
		vec3 uvw_pos = (pos + dist * direction) * cell_size;

		//check if outside, then break
		if (any(greaterThan(abs(uvw_pos - 0.5), vec3(0.5f + radius * cell_size)))) {
			break;
		}
		vec4 scolor = textureLod(sampler3D(probe, SAMPLER_LINEAR_WITH_MIPMAPS_CLAMP), uvw_pos, lod_level);
		lod_level += 1.0;

		float a = (1.0 - color.a);
		scolor *= a;
		color += scolor;
		dist += radius;
		radius = max(0.5, tan_half_angle * dist);
	}

	return color;
}

void voxel_gi_compute(uint index, vec3 position, vec3 normal, vec3 ref_vec, mat3 normal_xform, float roughness, vec3 ambient, vec3 environment, inout vec4 out_spec, inout vec4 out_diff) {
	position = (voxel_gi_instances.data[index].xform * vec4(position, 1.0)).xyz;
	ref_vec = normalize((voxel_gi_instances.data[index].xform * vec4(ref_vec, 0.0)).xyz);
	normal = normalize((voxel_gi_instances.data[index].xform * vec4(normal, 0.0)).xyz);

	position += normal * voxel_gi_instances.data[index].normal_bias;

	//this causes corrupted pixels, i have no idea why..
	if (any(bvec2(any(lessThan(position, vec3(0.0))), any(greaterThan(position, voxel_gi_instances.data[index].bounds))))) {
		return;
	}

	vec3 blendv = abs(position / voxel_gi_instances.data[index].bounds * 2.0 - 1.0);
	float blend = clamp(1.0 - max(blendv.x, max(blendv.y, blendv.z)), 0.0, 1.0);
	//float blend=1.0;

	float max_distance = length(voxel_gi_instances.data[index].bounds);
	vec3 cell_size = 1.0 / voxel_gi_instances.data[index].bounds;

	//radiance

#define MAX_CONE_DIRS 4

	vec3 cone_dirs[MAX_CONE_DIRS] = vec3[](
			vec3(0.707107, 0.0, 0.707107),
			vec3(0.0, 0.707107, 0.707107),
			vec3(-0.707107, 0.0, 0.707107),
			vec3(0.0, -0.707107, 0.707107));

	float cone_weights[MAX_CONE_DIRS] = float[](0.25, 0.25, 0.25, 0.25);
	float cone_angle_tan = 0.98269;

	vec3 light = vec3(0.0);

	for (int i = 0; i < MAX_CONE_DIRS; i++) {
		vec3 dir = normalize((voxel_gi_instances.data[index].xform * vec4(normal_xform * cone_dirs[i], 0.0)).xyz);

		vec4 cone_light = voxel_cone_trace_45_degrees(voxel_gi_textures[index], cell_size, position, dir, cone_angle_tan, max_distance, voxel_gi_instances.data[index].bias);

		if (voxel_gi_instances.data[index].blend_ambient) {
			cone_light.rgb = mix(ambient, cone_light.rgb, min(1.0, cone_light.a / 0.95));
		}

		light += cone_weights[i] * cone_light.rgb;
	}

	light *= voxel_gi_instances.data[index].dynamic_range * voxel_gi_instances.data[index].exposure_normalization;
	out_diff += vec4(light * blend, blend);

	//irradiance
	vec4 irr_light = voxel_cone_trace(voxel_gi_textures[index], cell_size, position, ref_vec, tan(roughness * 0.5 * M_PI * 0.99), max_distance, voxel_gi_instances.data[index].bias);
	if (voxel_gi_instances.data[index].blend_ambient) {
		irr_light.rgb = mix(environment, irr_light.rgb, min(1.0, irr_light.a / 0.95));
	}
	irr_light.rgb *= voxel_gi_instances.data[index].dynamic_range * voxel_gi_instances.data[index].exposure_normalization;
	//irr_light=vec3(0.0);

	out_spec += vec4(irr_light.rgb * blend, blend);
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



#define PROBE_CELLS 8
#define OCC16_DISTANCE_MAX 256.0
#define ROUGHNESS_TO_REFLECTION_TRESHOOLD 0.2


ivec3 modi(ivec3 value, ivec3 p_y) {
	return mix( value % p_y, p_y - ((abs(value)-ivec3(1)) % p_y), lessThan(sign(value), ivec3(0)) );
}

ivec2 probe_to_tex(ivec3 local_probe,int p_cascade) {

	ivec3 cell = modi( sdfgi.cascades[p_cascade].region_world_offset + local_probe,sdfgi.probe_axis_size);
	return cell.xy + ivec2(0,cell.z * int(sdfgi.probe_axis_size.y));
}


void sdfvoxel_gi_process(int cascade, vec3 cascade_pos, vec3 cam_pos, vec3 cam_normal, vec3 cam_specular_normal, float roughness, out vec3 diffuse_light, out vec3 specular_light) {

	vec3 posf = cascade_pos + cam_normal * sdfgi.normal_bias;

	ivec3 base_probe = ivec3(posf) / PROBE_CELLS;

	vec3 diffuse_accum = vec3(0.0);
	vec3 specular_accum = vec3(0.0);
	float weight_accum = 0.0;

	vec2 occ_probe_tex_to_uv = 1.0 / vec2( (OCCLUSION_OCT_SIZE+2) * sdfgi.probe_axis_size.x, (OCCLUSION_OCT_SIZE+2) * sdfgi.probe_axis_size.y * sdfgi.probe_axis_size.z );

	vec4 accum_light = vec4(0.0);

	vec2 light_probe_tex_to_uv = 1.0 / vec2( (LIGHTPROBE_OCT_SIZE+2) * sdfgi.probe_axis_size.x, (LIGHTPROBE_OCT_SIZE+2) * sdfgi.probe_axis_size.y * sdfgi.probe_axis_size.z );
	vec2 light_uv = octahedron_encode(vec3(cam_normal)) * float(LIGHTPROBE_OCT_SIZE);
	vec2 light_uv_spec = octahedron_encode(vec3(cam_specular_normal)) * float(LIGHTPROBE_OCT_SIZE);

	for(int i=0;i<8;i++) {
		ivec3 probe = base_probe + ((ivec3(i) >> ivec3(0, 1, 2)) & ivec3(1, 1, 1));

		vec3 probe_pos = vec3(probe * PROBE_CELLS);

		vec3 probe_to_pos = posf - probe_pos;
		vec3 n = normalize(probe_to_pos);
		float d = length(probe_to_pos);

		float weight = 1.0;
		weight *= pow(max(0.0001, (dot(-n, cam_normal) + 1.0) * 0.5),2.0) + 0.2;
		//weight *= max(0.0001, (dot(-n, cam_normal) ));

		ivec2 tex_pos = probe_to_tex(probe,cascade);
		vec2 tex_uv = vec2(ivec2(tex_pos * (OCCLUSION_OCT_SIZE+2) + ivec2(1))) + octahedron_encode(n) * float(OCCLUSION_OCT_SIZE);
		tex_uv *= occ_probe_tex_to_uv;
		vec2 o_o2 = texture(sampler2DArray(sdfgi_occlusion_probes,SAMPLER_LINEAR_CLAMP),vec3(tex_uv,float(cascade))).rg * OCC16_DISTANCE_MAX;

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

		vec2 base_tex_uv = vec2(ivec2(tex_pos * (LIGHTPROBE_OCT_SIZE+2) + ivec2(1)));
		tex_uv = base_tex_uv + light_uv;
		tex_uv *= light_probe_tex_to_uv;

		vec3 probe_light = texture(sampler2DArray(sdfgi_lightprobe_diffuse,SAMPLER_LINEAR_CLAMP),vec3(tex_uv,float(cascade))).rgb;

		diffuse_accum+=probe_light * weight;

		tex_uv = base_tex_uv + light_uv_spec;
		tex_uv *= light_probe_tex_to_uv;

		vec3 probe_ref_light;
		if (roughness < 0.99 && roughness > 0.00 ) {
			probe_ref_light = texture(sampler2DArray(sdfgi_lightprobe_specular,SAMPLER_LINEAR_CLAMP),vec3(tex_uv,float(cascade))).rgb;
		} else {
			probe_ref_light = vec3(0.0);
		}

		vec3 probe_ref_full_light;
		if (roughness > ROUGHNESS_TO_REFLECTION_TRESHOOLD) {
			probe_ref_full_light = texture(sampler2DArray(sdfgi_lightprobe_diffuse,SAMPLER_LINEAR_CLAMP),vec3(tex_uv,float(cascade))).rgb;
		} else {
			probe_ref_full_light = vec3(0.0);
		}

		probe_ref_light = mix(probe_ref_light,probe_ref_full_light,smoothstep(ROUGHNESS_TO_REFLECTION_TRESHOOLD,1.0,roughness));

		specular_accum+=probe_ref_light * weight;

		weight_accum += weight;
	}

	diffuse_light = diffuse_accum / weight_accum;
	specular_light = specular_accum / weight_accum;

}

void sdfgi_process(vec3 vertex, vec3 normal, vec3 reflection, float roughness, out vec4 ambient_light, out vec4 reflection_light) {

	//make vertex orientation the world one, but still align to camera
	vertex.y *= sdfgi.y_mult;
	normal.y *= sdfgi.y_mult;
	reflection.y *= sdfgi.y_mult;

	//renormalize
	normal = normalize(normal);
	reflection = normalize(reflection);

	vec3 cam_pos = vertex;
	vec3 cam_normal = normal;

	vec4 light_accum = vec4(0.0);
	float weight_accum = 0.0;

	vec4 light_blend_accum = vec4(0.0);
	float weight_blend_accum = 0.0;

	float blend = -1.0;

	// helper constants, compute once

	int cascade = 0x7FFFFFFF;
	vec3 cascade_pos;
	vec3 cascade_normal;

	for (int i = 0; i < sdfgi.max_cascades; i++) {
		cascade_pos = (cam_pos - sdfgi.cascades[i].position) * sdfgi.cascades[i].to_cell;

		if (any(lessThan(cascade_pos, vec3(0.0))) || any(greaterThanEqual(cascade_pos, vec3(sdfgi.grid_size)))) {
			continue; //skip cascade
		}

		cascade = i;
		break;
	}

	if (cascade < SDFGI_MAX_CASCADES) {
		ambient_light = vec4(0, 0, 0, 1);
		reflection_light = vec4(0, 0, 0, 1);

		float blend;
		vec3 diffuse, specular;
		sdfvoxel_gi_process(cascade, cascade_pos, cam_pos, cam_normal, reflection, roughness, diffuse, specular);

		{
			//process blend
			vec3 blend_from = ((vec3(sdfgi.probe_axis_size) - 1) / 2.0);

			vec3 inner_pos = cam_pos * sdfgi.cascades[cascade].to_probe;

			vec3 inner_dist = blend_from - abs(inner_pos);

			float min_d = min(inner_dist.x,min(inner_dist.y,inner_dist.z));

			blend = clamp(1.0 - smoothstep(0.5,2.5,min_d),0,1);
		}

		if (blend > 0.0) {

#if 0
// debug
			const vec3 to_color[SDFGI_MAX_CASCADES] = vec3[] (
				vec3(1,0,0),vec3(0,1,0),vec3(0,0,1),vec3(1,1,0),vec3(1,0,1),vec3(0,1,1),vec3(0,0,0),vec3(1,1,1) );

			diffuse = mix(diffuse,to_color[cascade],blend);
			specular = mix(specular,to_color[cascade],blend);
#else

			if (cascade == sdfgi.max_cascades - 1) {
				ambient_light.a = 1.0 - blend;
				reflection_light.a = 1.0 - blend;

			} else {
				vec3 diffuse2, specular2;
				cascade_pos = (cam_pos - sdfgi.cascades[cascade + 1].position) * sdfgi.cascades[cascade + 1].to_cell;

				sdfvoxel_gi_process(cascade + 1, cascade_pos, cam_pos, cam_normal, reflection, roughness, diffuse2, specular2);

				diffuse = mix(diffuse, diffuse2, blend);
				specular = mix(specular, specular2, blend);
			}
#endif
		}

		ambient_light.rgb = diffuse;

		if (roughness < 0.2) {
			reflection_light.rgb = specular;

			ambient_light.rgb *= sdfgi.energy;
			reflection_light.rgb *= sdfgi.energy;
		}
	} else {
		ambient_light = vec4(0);
		reflection_light = vec4(0);
	}
}
