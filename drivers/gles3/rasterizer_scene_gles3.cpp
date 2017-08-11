/*************************************************************************/
/*  rasterizer_storage_gles3.cpp                                         */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2017 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2017 Godot Engine contributors (cf. AUTHORS.md)    */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
#include "rasterizer_scene_gles3.h"

#include "os/os.h"
#include "project_settings.h"
#include "rasterizer_canvas_gles3.h"

#ifndef GLES_OVER_GL
#define glClearDepth glClearDepthf
#endif

static const GLenum _cube_side_enum[6] = {

	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,

};

static _FORCE_INLINE_ void store_transform2d(const Transform2D &p_mtx, float *p_array) {

	p_array[0] = p_mtx.elements[0][0];
	p_array[1] = p_mtx.elements[0][1];
	p_array[2] = 0;
	p_array[3] = 0;
	p_array[4] = p_mtx.elements[1][0];
	p_array[5] = p_mtx.elements[1][1];
	p_array[6] = 0;
	p_array[7] = 0;
	p_array[8] = 0;
	p_array[9] = 0;
	p_array[10] = 1;
	p_array[11] = 0;
	p_array[12] = p_mtx.elements[2][0];
	p_array[13] = p_mtx.elements[2][1];
	p_array[14] = 0;
	p_array[15] = 1;
}

static _FORCE_INLINE_ void store_transform(const Transform &p_mtx, float *p_array) {
	p_array[0] = p_mtx.basis.elements[0][0];
	p_array[1] = p_mtx.basis.elements[1][0];
	p_array[2] = p_mtx.basis.elements[2][0];
	p_array[3] = 0;
	p_array[4] = p_mtx.basis.elements[0][1];
	p_array[5] = p_mtx.basis.elements[1][1];
	p_array[6] = p_mtx.basis.elements[2][1];
	p_array[7] = 0;
	p_array[8] = p_mtx.basis.elements[0][2];
	p_array[9] = p_mtx.basis.elements[1][2];
	p_array[10] = p_mtx.basis.elements[2][2];
	p_array[11] = 0;
	p_array[12] = p_mtx.origin.x;
	p_array[13] = p_mtx.origin.y;
	p_array[14] = p_mtx.origin.z;
	p_array[15] = 1;
}

static _FORCE_INLINE_ void store_camera(const CameraMatrix &p_mtx, float *p_array) {

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {

			p_array[i * 4 + j] = p_mtx.matrix[i][j];
		}
	}
}

/* SHADOW ATLAS API */

RID RasterizerSceneGLES3::shadow_atlas_create() {

	ShadowAtlas *shadow_atlas = memnew(ShadowAtlas);
	shadow_atlas->fbo = 0;
	shadow_atlas->depth = 0;
	shadow_atlas->size = 0;
	shadow_atlas->smallest_subdiv = 0;

	for (int i = 0; i < 4; i++) {
		shadow_atlas->size_order[i] = i;
	}

	return shadow_atlas_owner.make_rid(shadow_atlas);
}

void RasterizerSceneGLES3::shadow_atlas_set_size(RID p_atlas, int p_size) {

	ShadowAtlas *shadow_atlas = shadow_atlas_owner.getornull(p_atlas);
	ERR_FAIL_COND(!shadow_atlas);
	ERR_FAIL_COND(p_size < 0);

	p_size = nearest_power_of_2(p_size);

	if (p_size == shadow_atlas->size)
		return;

	// erasing atlas
	if (shadow_atlas->fbo) {
		glDeleteTextures(1, &shadow_atlas->depth);
		glDeleteFramebuffers(1, &shadow_atlas->fbo);

		shadow_atlas->depth = 0;
		shadow_atlas->fbo = 0;
	}
	for (int i = 0; i < 4; i++) {
		//clear subdivisions
		shadow_atlas->quadrants[i].shadows.resize(0);
		shadow_atlas->quadrants[i].shadows.resize(1 << shadow_atlas->quadrants[i].subdivision);
	}

	//erase shadow atlas reference from lights
	for (Map<RID, uint32_t>::Element *E = shadow_atlas->shadow_owners.front(); E; E = E->next()) {
		LightInstance *li = light_instance_owner.getornull(E->key());
		ERR_CONTINUE(!li);
		li->shadow_atlases.erase(p_atlas);
	}

	//clear owners
	shadow_atlas->shadow_owners.clear();

	shadow_atlas->size = p_size;

	if (shadow_atlas->size) {
		glGenFramebuffers(1, &shadow_atlas->fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, shadow_atlas->fbo);

		// Create a texture for storing the depth
		glActiveTexture(GL_TEXTURE0);
		glGenTextures(1, &shadow_atlas->depth);
		glBindTexture(GL_TEXTURE_2D, shadow_atlas->depth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, shadow_atlas->size, shadow_atlas->size, 0,
				GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
				GL_TEXTURE_2D, shadow_atlas->depth, 0);

		glViewport(0, 0, shadow_atlas->size, shadow_atlas->size);
		glClearDepth(0.0f);
		glClear(GL_DEPTH_BUFFER_BIT);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void RasterizerSceneGLES3::shadow_atlas_set_quadrant_subdivision(RID p_atlas, int p_quadrant, int p_subdivision) {

	ShadowAtlas *shadow_atlas = shadow_atlas_owner.getornull(p_atlas);
	ERR_FAIL_COND(!shadow_atlas);
	ERR_FAIL_INDEX(p_quadrant, 4);
	ERR_FAIL_INDEX(p_subdivision, 16384);

	uint32_t subdiv = nearest_power_of_2(p_subdivision);
	if (subdiv & 0xaaaaaaaa) { //sqrt(subdiv) must be integer
		subdiv <<= 1;
	}

	subdiv = int(Math::sqrt((float)subdiv));

	//obtain the number that will be x*x

	if (shadow_atlas->quadrants[p_quadrant].subdivision == subdiv)
		return;

	//erase all data from quadrant
	for (int i = 0; i < shadow_atlas->quadrants[p_quadrant].shadows.size(); i++) {

		if (shadow_atlas->quadrants[p_quadrant].shadows[i].owner.is_valid()) {
			shadow_atlas->shadow_owners.erase(shadow_atlas->quadrants[p_quadrant].shadows[i].owner);
			LightInstance *li = light_instance_owner.getornull(shadow_atlas->quadrants[p_quadrant].shadows[i].owner);
			ERR_CONTINUE(!li);
			li->shadow_atlases.erase(p_atlas);
		}
	}

	shadow_atlas->quadrants[p_quadrant].shadows.resize(0);
	shadow_atlas->quadrants[p_quadrant].shadows.resize(subdiv * subdiv);
	shadow_atlas->quadrants[p_quadrant].subdivision = subdiv;

	//cache the smallest subdiv (for faster allocation in light update)

	shadow_atlas->smallest_subdiv = 1 << 30;

	for (int i = 0; i < 4; i++) {
		if (shadow_atlas->quadrants[i].subdivision) {
			shadow_atlas->smallest_subdiv = MIN(shadow_atlas->smallest_subdiv, shadow_atlas->quadrants[i].subdivision);
		}
	}

	if (shadow_atlas->smallest_subdiv == 1 << 30) {
		shadow_atlas->smallest_subdiv = 0;
	}

	//resort the size orders, simple bublesort for 4 elements..

	int swaps = 0;
	do {
		swaps = 0;

		for (int i = 0; i < 3; i++) {
			if (shadow_atlas->quadrants[shadow_atlas->size_order[i]].subdivision < shadow_atlas->quadrants[shadow_atlas->size_order[i + 1]].subdivision) {
				SWAP(shadow_atlas->size_order[i], shadow_atlas->size_order[i + 1]);
				swaps++;
			}
		}
	} while (swaps > 0);
}

bool RasterizerSceneGLES3::_shadow_atlas_find_shadow(ShadowAtlas *shadow_atlas, int *p_in_quadrants, int p_quadrant_count, int p_current_subdiv, uint64_t p_tick, int &r_quadrant, int &r_shadow) {

	for (int i = p_quadrant_count - 1; i >= 0; i--) {

		int qidx = p_in_quadrants[i];

		if (shadow_atlas->quadrants[qidx].subdivision == p_current_subdiv) {
			return false;
		}

		//look for an empty space
		int sc = shadow_atlas->quadrants[qidx].shadows.size();
		ShadowAtlas::Quadrant::Shadow *sarr = shadow_atlas->quadrants[qidx].shadows.ptr();

		int found_free_idx = -1; //found a free one
		int found_used_idx = -1; //found existing one, must steal it
		uint64_t min_pass; // pass of the existing one, try to use the least recently used one (LRU fashion)

		for (int j = 0; j < sc; j++) {
			if (!sarr[j].owner.is_valid()) {
				found_free_idx = j;
				break;
			}

			LightInstance *sli = light_instance_owner.getornull(sarr[j].owner);
			ERR_CONTINUE(!sli);

			if (sli->last_scene_pass != scene_pass) {

				//was just allocated, don't kill it so soon, wait a bit..
				if (p_tick - sarr[j].alloc_tick < shadow_atlas_realloc_tolerance_msec)
					continue;

				if (found_used_idx == -1 || sli->last_scene_pass < min_pass) {
					found_used_idx = j;
					min_pass = sli->last_scene_pass;
				}
			}
		}

		if (found_free_idx == -1 && found_used_idx == -1)
			continue; //nothing found

		if (found_free_idx == -1 && found_used_idx != -1) {
			found_free_idx = found_used_idx;
		}

		r_quadrant = qidx;
		r_shadow = found_free_idx;

		return true;
	}

	return false;
}

bool RasterizerSceneGLES3::shadow_atlas_update_light(RID p_atlas, RID p_light_intance, float p_coverage, uint64_t p_light_version) {

	ShadowAtlas *shadow_atlas = shadow_atlas_owner.getornull(p_atlas);
	ERR_FAIL_COND_V(!shadow_atlas, false);

	LightInstance *li = light_instance_owner.getornull(p_light_intance);
	ERR_FAIL_COND_V(!li, false);

	if (shadow_atlas->size == 0 || shadow_atlas->smallest_subdiv == 0) {
		return false;
	}

	uint32_t quad_size = shadow_atlas->size >> 1;
	int desired_fit = MIN(quad_size / shadow_atlas->smallest_subdiv, nearest_power_of_2(quad_size * p_coverage));

	int valid_quadrants[4];
	int valid_quadrant_count = 0;
	int best_size = -1; //best size found
	int best_subdiv = -1; //subdiv for the best size

	//find the quadrants this fits into, and the best possible size it can fit into
	for (int i = 0; i < 4; i++) {
		int q = shadow_atlas->size_order[i];
		int sd = shadow_atlas->quadrants[q].subdivision;
		if (sd == 0)
			continue; //unused

		int max_fit = quad_size / sd;

		if (best_size != -1 && max_fit > best_size)
			break; //too large

		valid_quadrants[valid_quadrant_count++] = q;
		best_subdiv = sd;

		if (max_fit >= desired_fit) {
			best_size = max_fit;
		}
	}

	ERR_FAIL_COND_V(valid_quadrant_count == 0, false);

	uint64_t tick = OS::get_singleton()->get_ticks_msec();

	//see if it already exists

	if (shadow_atlas->shadow_owners.has(p_light_intance)) {
		//it does!
		uint32_t key = shadow_atlas->shadow_owners[p_light_intance];
		uint32_t q = (key >> ShadowAtlas::QUADRANT_SHIFT) & 0x3;
		uint32_t s = key & ShadowAtlas::SHADOW_INDEX_MASK;

		bool should_realloc = shadow_atlas->quadrants[q].subdivision != best_subdiv && (shadow_atlas->quadrants[q].shadows[s].alloc_tick - tick > shadow_atlas_realloc_tolerance_msec);
		bool should_redraw = shadow_atlas->quadrants[q].shadows[s].version != p_light_version;

		if (!should_realloc) {
			shadow_atlas->quadrants[q].shadows[s].version = p_light_version;
			//already existing, see if it should redraw or it's just OK
			return should_redraw;
		}

		int new_quadrant, new_shadow;

		//find a better place
		if (_shadow_atlas_find_shadow(shadow_atlas, valid_quadrants, valid_quadrant_count, shadow_atlas->quadrants[q].subdivision, tick, new_quadrant, new_shadow)) {
			//found a better place!
			ShadowAtlas::Quadrant::Shadow *sh = &shadow_atlas->quadrants[new_quadrant].shadows[new_shadow];
			if (sh->owner.is_valid()) {
				//is taken, but is invalid, erasing it
				shadow_atlas->shadow_owners.erase(sh->owner);
				LightInstance *sli = light_instance_owner.get(sh->owner);
				sli->shadow_atlases.erase(p_atlas);
			}

			//erase previous
			shadow_atlas->quadrants[q].shadows[s].version = 0;
			shadow_atlas->quadrants[q].shadows[s].owner = RID();

			sh->owner = p_light_intance;
			sh->alloc_tick = tick;
			sh->version = p_light_version;

			//make new key
			key = new_quadrant << ShadowAtlas::QUADRANT_SHIFT;
			key |= new_shadow;
			//update it in map
			shadow_atlas->shadow_owners[p_light_intance] = key;
			//make it dirty, as it should redraw anyway
			return true;
		}

		//no better place for this shadow found, keep current

		//already existing, see if it should redraw or it's just OK

		shadow_atlas->quadrants[q].shadows[s].version = p_light_version;

		return should_redraw;
	}

	int new_quadrant, new_shadow;

	//find a better place
	if (_shadow_atlas_find_shadow(shadow_atlas, valid_quadrants, valid_quadrant_count, -1, tick, new_quadrant, new_shadow)) {
		//found a better place!
		ShadowAtlas::Quadrant::Shadow *sh = &shadow_atlas->quadrants[new_quadrant].shadows[new_shadow];
		if (sh->owner.is_valid()) {
			//is taken, but is invalid, erasing it
			shadow_atlas->shadow_owners.erase(sh->owner);
			LightInstance *sli = light_instance_owner.get(sh->owner);
			sli->shadow_atlases.erase(p_atlas);
		}

		sh->owner = p_light_intance;
		sh->alloc_tick = tick;
		sh->version = p_light_version;

		//make new key
		uint32_t key = new_quadrant << ShadowAtlas::QUADRANT_SHIFT;
		key |= new_shadow;
		//update it in map
		shadow_atlas->shadow_owners[p_light_intance] = key;
		//make it dirty, as it should redraw anyway

		return true;
	}

	//no place to allocate this light, apologies

	return false;
}

void RasterizerSceneGLES3::set_directional_shadow_count(int p_count) {

	directional_shadow.light_count = p_count;
	directional_shadow.current_light = 0;
}

int RasterizerSceneGLES3::get_directional_light_shadow_size(RID p_light_intance) {

	ERR_FAIL_COND_V(directional_shadow.light_count == 0, 0);

	int shadow_size;

	if (directional_shadow.light_count == 1) {
		shadow_size = directional_shadow.size;
	} else {
		shadow_size = directional_shadow.size / 2; //more than 4 not supported anyway
	}

	LightInstance *light_instance = light_instance_owner.getornull(p_light_intance);
	ERR_FAIL_COND_V(!light_instance, 0);

	switch (light_instance->light_ptr->directional_shadow_mode) {
		case VS::LIGHT_DIRECTIONAL_SHADOW_ORTHOGONAL:
			break; //none
		case VS::LIGHT_DIRECTIONAL_SHADOW_PARALLEL_2_SPLITS:
		case VS::LIGHT_DIRECTIONAL_SHADOW_PARALLEL_4_SPLITS: shadow_size /= 2; break;
	}

	return shadow_size;
}
//////////////////////////////////////////////////////

RID RasterizerSceneGLES3::reflection_atlas_create() {

	ReflectionAtlas *reflection_atlas = memnew(ReflectionAtlas);
	reflection_atlas->subdiv = 0;
	reflection_atlas->color = 0;
	reflection_atlas->size = 0;
	for (int i = 0; i < 6; i++) {
		reflection_atlas->fbo[i] = 0;
	}

	return reflection_atlas_owner.make_rid(reflection_atlas);
}

void RasterizerSceneGLES3::reflection_atlas_set_size(RID p_ref_atlas, int p_size) {

	ReflectionAtlas *reflection_atlas = reflection_atlas_owner.getornull(p_ref_atlas);
	ERR_FAIL_COND(!reflection_atlas);

	int size = nearest_power_of_2(p_size);

	if (size == reflection_atlas->size)
		return;
	if (reflection_atlas->size) {
		for (int i = 0; i < 6; i++) {
			glDeleteFramebuffers(1, &reflection_atlas->fbo[i]);
			reflection_atlas->fbo[i] = 0;
		}
		glDeleteTextures(1, &reflection_atlas->color);
		reflection_atlas->color = 0;
	}

	reflection_atlas->size = size;

	for (int i = 0; i < reflection_atlas->reflections.size(); i++) {
		//erase probes reference to this
		if (reflection_atlas->reflections[i].owner.is_valid()) {
			ReflectionProbeInstance *reflection_probe_instance = reflection_probe_instance_owner.getornull(reflection_atlas->reflections[i].owner);
			reflection_atlas->reflections[i].owner = RID();

			ERR_CONTINUE(!reflection_probe_instance);
			reflection_probe_instance->reflection_atlas_index = -1;
			reflection_probe_instance->atlas = RID();
			reflection_probe_instance->render_step = -1;
		}
	}

	if (reflection_atlas->size) {

		bool use_float = true;

		GLenum internal_format = use_float ? GL_RGBA16F : GL_RGB10_A2;
		GLenum format = GL_RGBA;
		GLenum type = use_float ? GL_HALF_FLOAT : GL_UNSIGNED_INT_2_10_10_10_REV;

		// Create a texture for storing the color
		glActiveTexture(GL_TEXTURE0);
		glGenTextures(1, &reflection_atlas->color);
		glBindTexture(GL_TEXTURE_2D, reflection_atlas->color);

		int mmsize = reflection_atlas->size;
		glTexStorage2DCustom(GL_TEXTURE_2D, 6, internal_format, mmsize, mmsize, format, type);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 5);

		for (int i = 0; i < 6; i++) {
			glGenFramebuffers(1, &reflection_atlas->fbo[i]);
			glBindFramebuffer(GL_FRAMEBUFFER, reflection_atlas->fbo[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, reflection_atlas->color, i);

			GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			ERR_CONTINUE(status != GL_FRAMEBUFFER_COMPLETE);

			glDisable(GL_SCISSOR_TEST);
			glViewport(0, 0, mmsize, mmsize);
			glClearColor(0, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT); //it needs to be cleared, to avoid generating garbage

			mmsize >>= 1;
		}
	}
}

void RasterizerSceneGLES3::reflection_atlas_set_subdivision(RID p_ref_atlas, int p_subdiv) {

	ReflectionAtlas *reflection_atlas = reflection_atlas_owner.getornull(p_ref_atlas);
	ERR_FAIL_COND(!reflection_atlas);

	uint32_t subdiv = nearest_power_of_2(p_subdiv);
	if (subdiv & 0xaaaaaaaa) { //sqrt(subdiv) must be integer
		subdiv <<= 1;
	}

	subdiv = int(Math::sqrt((float)subdiv));

	if (reflection_atlas->subdiv == subdiv)
		return;

	if (subdiv) {

		for (int i = 0; i < reflection_atlas->reflections.size(); i++) {
			//erase probes reference to this
			if (reflection_atlas->reflections[i].owner.is_valid()) {
				ReflectionProbeInstance *reflection_probe_instance = reflection_probe_instance_owner.getornull(reflection_atlas->reflections[i].owner);
				reflection_atlas->reflections[i].owner = RID();

				ERR_CONTINUE(!reflection_probe_instance);
				reflection_probe_instance->reflection_atlas_index = -1;
				reflection_probe_instance->atlas = RID();
				reflection_probe_instance->render_step = -1;
			}
		}
	}

	reflection_atlas->subdiv = subdiv;

	reflection_atlas->reflections.resize(subdiv * subdiv);
}

////////////////////////////////////////////////////

RID RasterizerSceneGLES3::reflection_probe_instance_create(RID p_probe) {

	RasterizerStorageGLES3::ReflectionProbe *probe = storage->reflection_probe_owner.getornull(p_probe);
	ERR_FAIL_COND_V(!probe, RID());

	ReflectionProbeInstance *rpi = memnew(ReflectionProbeInstance);

	rpi->probe_ptr = probe;
	rpi->self = reflection_probe_instance_owner.make_rid(rpi);
	rpi->probe = p_probe;
	rpi->reflection_atlas_index = -1;
	rpi->render_step = -1;
	rpi->last_pass = 0;

	return rpi->self;
}

void RasterizerSceneGLES3::reflection_probe_instance_set_transform(RID p_instance, const Transform &p_transform) {

	ReflectionProbeInstance *rpi = reflection_probe_instance_owner.getornull(p_instance);
	ERR_FAIL_COND(!rpi);
	rpi->transform = p_transform;
}

void RasterizerSceneGLES3::reflection_probe_release_atlas_index(RID p_instance) {

	ReflectionProbeInstance *rpi = reflection_probe_instance_owner.getornull(p_instance);
	ERR_FAIL_COND(!rpi);
	if (rpi->reflection_atlas_index == -1)
		return;

	ReflectionAtlas *reflection_atlas = reflection_atlas_owner.getornull(rpi->atlas);
	ERR_FAIL_COND(!reflection_atlas);

	ERR_FAIL_INDEX(rpi->reflection_atlas_index, reflection_atlas->reflections.size());

	ERR_FAIL_COND(reflection_atlas->reflections[rpi->reflection_atlas_index].owner != rpi->self);

	reflection_atlas->reflections[rpi->reflection_atlas_index].owner = RID();

	rpi->reflection_atlas_index = -1;
	rpi->atlas = RID();
	rpi->render_step = -1;
}

bool RasterizerSceneGLES3::reflection_probe_instance_needs_redraw(RID p_instance) {

	ReflectionProbeInstance *rpi = reflection_probe_instance_owner.getornull(p_instance);
	ERR_FAIL_COND_V(!rpi, false);

	return rpi->reflection_atlas_index == -1 || rpi->probe_ptr->update_mode == VS::REFLECTION_PROBE_UPDATE_ALWAYS;
}

bool RasterizerSceneGLES3::reflection_probe_instance_has_reflection(RID p_instance) {

	ReflectionProbeInstance *rpi = reflection_probe_instance_owner.getornull(p_instance);
	ERR_FAIL_COND_V(!rpi, false);

	return rpi->reflection_atlas_index != -1;
}

bool RasterizerSceneGLES3::reflection_probe_instance_begin_render(RID p_instance, RID p_reflection_atlas) {

	ReflectionProbeInstance *rpi = reflection_probe_instance_owner.getornull(p_instance);
	ERR_FAIL_COND_V(!rpi, false);

	rpi->render_step = 0;

	if (rpi->reflection_atlas_index != -1) {
		return true; //got one already
	}

	ReflectionAtlas *reflection_atlas = reflection_atlas_owner.getornull(p_reflection_atlas);
	ERR_FAIL_COND_V(!reflection_atlas, false);

	if (reflection_atlas->size == 0 || reflection_atlas->subdiv == 0) {
		return false;
	}

	int best_free = -1;
	int best_used = -1;
	uint64_t best_used_frame;

	for (int i = 0; i < reflection_atlas->reflections.size(); i++) {
		if (reflection_atlas->reflections[i].owner == RID()) {
			best_free = i;
			break;
		}

		if (rpi->render_step < 0 && reflection_atlas->reflections[i].last_frame < storage->frame.count &&
				(best_used == -1 || reflection_atlas->reflections[i].last_frame < best_used_frame)) {
			best_used = i;
			best_used_frame = reflection_atlas->reflections[i].last_frame;
		}
	}

	if (best_free == -1 && best_used == -1) {
		return false; // sorry, can not do. Try again next frame.
	}

	if (best_free == -1) {
		//find best from what is used
		best_free = best_used;

		ReflectionProbeInstance *victim_rpi = reflection_probe_instance_owner.getornull(reflection_atlas->reflections[best_free].owner);
		ERR_FAIL_COND_V(!victim_rpi, false);
		victim_rpi->atlas = RID();
		victim_rpi->reflection_atlas_index = -1;
	}

	reflection_atlas->reflections[best_free].owner = p_instance;
	reflection_atlas->reflections[best_free].last_frame = storage->frame.count;

	rpi->reflection_atlas_index = best_free;
	rpi->atlas = p_reflection_atlas;
	rpi->render_step = 0;

	return true;
}

bool RasterizerSceneGLES3::reflection_probe_instance_postprocess_step(RID p_instance) {

	ReflectionProbeInstance *rpi = reflection_probe_instance_owner.getornull(p_instance);
	ERR_FAIL_COND_V(!rpi, true);

	ReflectionAtlas *reflection_atlas = reflection_atlas_owner.getornull(rpi->atlas);
	ERR_FAIL_COND_V(!reflection_atlas, false);

	ERR_FAIL_COND_V(rpi->render_step >= 6, true);

	glBindFramebuffer(GL_FRAMEBUFFER, reflection_atlas->fbo[rpi->render_step]);
	state.cube_to_dp_shader.bind();

	int target_size = reflection_atlas->size / reflection_atlas->subdiv;

	int cubemap_index = reflection_cubemaps.size() - 1;

	for (int i = reflection_cubemaps.size() - 1; i >= 0; i--) {
		//find appropriate cubemap to render to
		if (reflection_cubemaps[i].size > target_size * 2)
			break;

		cubemap_index = i;
	}

	glDisable(GL_BLEND);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, reflection_cubemaps[cubemap_index].cubemap);
	glDisable(GL_CULL_FACE);

	storage->shaders.cubemap_filter.set_conditional(CubemapFilterShaderGLES3::USE_DUAL_PARABOLOID, true);
	storage->shaders.cubemap_filter.bind();

	int cell_size = reflection_atlas->size / reflection_atlas->subdiv;
	for (int i = 0; i < rpi->render_step; i++) {
		cell_size >>= 1; //mipmaps!
	}
	int x = (rpi->reflection_atlas_index % reflection_atlas->subdiv) * cell_size;
	int y = (rpi->reflection_atlas_index / reflection_atlas->subdiv) * cell_size;
	int width = cell_size;
	int height = cell_size;

	storage->shaders.cubemap_filter.set_conditional(CubemapFilterShaderGLES3::USE_DIRECT_WRITE, rpi->render_step == 0);
	storage->shaders.cubemap_filter.set_conditional(CubemapFilterShaderGLES3::LOW_QUALITY, rpi->probe_ptr->update_mode == VS::REFLECTION_PROBE_UPDATE_ALWAYS);
	for (int i = 0; i < 2; i++) {

		storage->shaders.cubemap_filter.set_uniform(CubemapFilterShaderGLES3::Z_FLIP, i == 0);
		storage->shaders.cubemap_filter.set_uniform(CubemapFilterShaderGLES3::ROUGHNESS, rpi->render_step / 5.0);

		uint32_t local_width = width, local_height = height;
		uint32_t local_x = x, local_y = y;

		local_height /= 2;
		local_y += i * local_height;

		glViewport(local_x, local_y, local_width, local_height);

		_copy_screen();
	}
	storage->shaders.cubemap_filter.set_conditional(CubemapFilterShaderGLES3::USE_DIRECT_WRITE, false);
	storage->shaders.cubemap_filter.set_conditional(CubemapFilterShaderGLES3::LOW_QUALITY, false);

	rpi->render_step++;

	return rpi->render_step == 6;
}

/* ENVIRONMENT API */

RID RasterizerSceneGLES3::environment_create() {

	Environment *env = memnew(Environment);

	return environment_owner.make_rid(env);
}

void RasterizerSceneGLES3::environment_set_background(RID p_env, VS::EnvironmentBG p_bg) {

	Environment *env = environment_owner.getornull(p_env);
	ERR_FAIL_COND(!env);
	env->bg_mode = p_bg;
}

void RasterizerSceneGLES3::environment_set_sky(RID p_env, RID p_sky) {

	Environment *env = environment_owner.getornull(p_env);
	ERR_FAIL_COND(!env);

	env->sky = p_sky;
}

void RasterizerSceneGLES3::environment_set_sky_scale(RID p_env, float p_scale) {

	Environment *env = environment_owner.getornull(p_env);
	ERR_FAIL_COND(!env);

	env->sky_scale = p_scale;
}

void RasterizerSceneGLES3::environment_set_bg_color(RID p_env, const Color &p_color) {

	Environment *env = environment_owner.getornull(p_env);
	ERR_FAIL_COND(!env);

	env->bg_color = p_color;
}
void RasterizerSceneGLES3::environment_set_bg_energy(RID p_env, float p_energy) {

	Environment *env = environment_owner.getornull(p_env);
	ERR_FAIL_COND(!env);

	env->bg_energy = p_energy;
}

void RasterizerSceneGLES3::environment_set_canvas_max_layer(RID p_env, int p_max_layer) {

	Environment *env = environment_owner.getornull(p_env);
	ERR_FAIL_COND(!env);

	env->canvas_max_layer = p_max_layer;
}
void RasterizerSceneGLES3::environment_set_ambient_light(RID p_env, const Color &p_color, float p_energy, float p_sky_contribution) {

	Environment *env = environment_owner.getornull(p_env);
	ERR_FAIL_COND(!env);

	env->ambient_color = p_color;
	env->ambient_energy = p_energy;
	env->ambient_sky_contribution = p_sky_contribution;
}

void RasterizerSceneGLES3::environment_set_dof_blur_far(RID p_env, bool p_enable, float p_distance, float p_transition, float p_amount, VS::EnvironmentDOFBlurQuality p_quality) {

	Environment *env = environment_owner.getornull(p_env);
	ERR_FAIL_COND(!env);

	env->dof_blur_far_enabled = p_enable;
	env->dof_blur_far_distance = p_distance;
	env->dof_blur_far_transition = p_transition;
	env->dof_blur_far_amount = p_amount;
	env->dof_blur_far_quality = p_quality;
}

void RasterizerSceneGLES3::environment_set_dof_blur_near(RID p_env, bool p_enable, float p_distance, float p_transition, float p_amount, VS::EnvironmentDOFBlurQuality p_quality) {

	Environment *env = environment_owner.getornull(p_env);
	ERR_FAIL_COND(!env);

	env->dof_blur_near_enabled = p_enable;
	env->dof_blur_near_distance = p_distance;
	env->dof_blur_near_transition = p_transition;
	env->dof_blur_near_amount = p_amount;
	env->dof_blur_near_quality = p_quality;
}
void RasterizerSceneGLES3::environment_set_glow(RID p_env, bool p_enable, int p_level_flags, float p_intensity, float p_strength, float p_bloom_threshold, VS::EnvironmentGlowBlendMode p_blend_mode, float p_hdr_bleed_threshold, float p_hdr_bleed_scale, bool p_bicubic_upscale) {

	Environment *env = environment_owner.getornull(p_env);
	ERR_FAIL_COND(!env);

	env->glow_enabled = p_enable;
	env->glow_levels = p_level_flags;
	env->glow_intensity = p_intensity;
	env->glow_strength = p_strength;
	env->glow_bloom = p_bloom_threshold;
	env->glow_blend_mode = p_blend_mode;
	env->glow_hdr_bleed_threshold = p_hdr_bleed_threshold;
	env->glow_hdr_bleed_scale = p_hdr_bleed_scale;
	env->glow_bicubic_upscale = p_bicubic_upscale;
}
void RasterizerSceneGLES3::environment_set_fog(RID p_env, bool p_enable, float p_begin, float p_end, RID p_gradient_texture) {
}

void RasterizerSceneGLES3::environment_set_ssr(RID p_env, bool p_enable, int p_max_steps, float p_fade_in, float p_fade_out, float p_depth_tolerance, bool p_roughness) {

	Environment *env = environment_owner.getornull(p_env);
	ERR_FAIL_COND(!env);

	env->ssr_enabled = p_enable;
	env->ssr_max_steps = p_max_steps;
	env->ssr_fade_in = p_fade_in;
	env->ssr_fade_out = p_fade_out;
	env->ssr_depth_tolerance = p_depth_tolerance;
	env->ssr_roughness = p_roughness;
}

void RasterizerSceneGLES3::environment_set_ssao(RID p_env, bool p_enable, float p_radius, float p_intensity, float p_radius2, float p_intensity2, float p_bias, float p_light_affect, const Color &p_color, bool p_blur) {

	Environment *env = environment_owner.getornull(p_env);
	ERR_FAIL_COND(!env);

	env->ssao_enabled = p_enable;
	env->ssao_radius = p_radius;
	env->ssao_intensity = p_intensity;
	env->ssao_radius2 = p_radius2;
	env->ssao_intensity2 = p_intensity2;
	env->ssao_bias = p_bias;
	env->ssao_light_affect = p_light_affect;
	env->ssao_color = p_color;
	env->ssao_filter = p_blur;
}

void RasterizerSceneGLES3::environment_set_tonemap(RID p_env, VS::EnvironmentToneMapper p_tone_mapper, float p_exposure, float p_white, bool p_auto_exposure, float p_min_luminance, float p_max_luminance, float p_auto_exp_speed, float p_auto_exp_scale) {

	Environment *env = environment_owner.getornull(p_env);
	ERR_FAIL_COND(!env);

	env->tone_mapper = p_tone_mapper;
	env->tone_mapper_exposure = p_exposure;
	env->tone_mapper_exposure_white = p_white;
	env->auto_exposure = p_auto_exposure;
	env->auto_exposure_speed = p_auto_exp_speed;
	env->auto_exposure_min = p_min_luminance;
	env->auto_exposure_max = p_max_luminance;
	env->auto_exposure_grey = p_auto_exp_scale;
}

void RasterizerSceneGLES3::environment_set_adjustment(RID p_env, bool p_enable, float p_brightness, float p_contrast, float p_saturation, RID p_ramp) {

	Environment *env = environment_owner.getornull(p_env);
	ERR_FAIL_COND(!env);

	env->adjustments_enabled = p_enable;
	env->adjustments_brightness = p_brightness;
	env->adjustments_contrast = p_contrast;
	env->adjustments_saturation = p_saturation;
	env->color_correction = p_ramp;
}

void RasterizerSceneGLES3::environment_set_fog(RID p_env, bool p_enable, const Color &p_color, const Color &p_sun_color, float p_sun_amount) {

	Environment *env = environment_owner.getornull(p_env);
	ERR_FAIL_COND(!env);

	env->fog_enabled = p_enable;
	env->fog_color = p_color;
	env->fog_sun_color = p_sun_color;
	env->fog_sun_amount = p_sun_amount;
}

void RasterizerSceneGLES3::environment_set_fog_depth(RID p_env, bool p_enable, float p_depth_begin, float p_depth_curve, bool p_transmit, float p_transmit_curve) {

	Environment *env = environment_owner.getornull(p_env);
	ERR_FAIL_COND(!env);

	env->fog_depth_enabled = p_enable;
	env->fog_depth_begin = p_depth_begin;
	env->fog_depth_curve = p_depth_curve;
	env->fog_transmit_enabled = p_transmit;
	env->fog_transmit_curve = p_transmit_curve;
}

void RasterizerSceneGLES3::environment_set_fog_height(RID p_env, bool p_enable, float p_min_height, float p_max_height, float p_height_curve) {

	Environment *env = environment_owner.getornull(p_env);
	ERR_FAIL_COND(!env);

	env->fog_height_enabled = p_enable;
	env->fog_height_min = p_min_height;
	env->fog_height_max = p_max_height;
	env->fog_height_curve = p_height_curve;
}

bool RasterizerSceneGLES3::is_environment(RID p_env) {

	return environment_owner.owns(p_env);
}

VS::EnvironmentBG RasterizerSceneGLES3::environment_get_background(RID p_env) {

	const Environment *env = environment_owner.getornull(p_env);
	ERR_FAIL_COND_V(!env, VS::ENV_BG_MAX);

	return env->bg_mode;
}

int RasterizerSceneGLES3::environment_get_canvas_max_layer(RID p_env) {

	const Environment *env = environment_owner.getornull(p_env);
	ERR_FAIL_COND_V(!env, -1);

	return env->canvas_max_layer;
}

RID RasterizerSceneGLES3::light_instance_create(RID p_light) {

	LightInstance *light_instance = memnew(LightInstance);

	light_instance->last_pass = 0;
	light_instance->last_scene_pass = 0;
	light_instance->last_scene_shadow_pass = 0;

	light_instance->light = p_light;
	light_instance->light_ptr = storage->light_owner.getornull(p_light);

	ERR_FAIL_COND_V(!light_instance->light_ptr, RID());

	light_instance->self = light_instance_owner.make_rid(light_instance);

	return light_instance->self;
}

void RasterizerSceneGLES3::light_instance_set_transform(RID p_light_instance, const Transform &p_transform) {

	LightInstance *light_instance = light_instance_owner.getornull(p_light_instance);
	ERR_FAIL_COND(!light_instance);

	light_instance->transform = p_transform;
}

void RasterizerSceneGLES3::light_instance_set_shadow_transform(RID p_light_instance, const CameraMatrix &p_projection, const Transform &p_transform, float p_far, float p_split, int p_pass, float p_bias_scale) {

	LightInstance *light_instance = light_instance_owner.getornull(p_light_instance);
	ERR_FAIL_COND(!light_instance);

	if (light_instance->light_ptr->type != VS::LIGHT_DIRECTIONAL) {
		p_pass = 0;
	}

	ERR_FAIL_INDEX(p_pass, 4);

	light_instance->shadow_transform[p_pass].camera = p_projection;
	light_instance->shadow_transform[p_pass].transform = p_transform;
	light_instance->shadow_transform[p_pass].farplane = p_far;
	light_instance->shadow_transform[p_pass].split = p_split;
	light_instance->shadow_transform[p_pass].bias_scale = p_bias_scale;
}

void RasterizerSceneGLES3::light_instance_mark_visible(RID p_light_instance) {

	LightInstance *light_instance = light_instance_owner.getornull(p_light_instance);
	ERR_FAIL_COND(!light_instance);

	light_instance->last_scene_pass = scene_pass;
}

//////////////////////

RID RasterizerSceneGLES3::gi_probe_instance_create() {

	GIProbeInstance *gipi = memnew(GIProbeInstance);

	return gi_probe_instance_owner.make_rid(gipi);
}

void RasterizerSceneGLES3::gi_probe_instance_set_light_data(RID p_probe, RID p_base, RID p_data) {

	GIProbeInstance *gipi = gi_probe_instance_owner.getornull(p_probe);
	ERR_FAIL_COND(!gipi);
	gipi->data = p_data;
	gipi->probe = storage->gi_probe_owner.getornull(p_base);
	if (p_data.is_valid()) {
		RasterizerStorageGLES3::GIProbeData *gipd = storage->gi_probe_data_owner.getornull(p_data);
		ERR_FAIL_COND(!gipd);
		if (gipd) {
			gipi->tex_cache = gipd->tex_id;
			gipi->cell_size_cache.x = 1.0 / gipd->width;
			gipi->cell_size_cache.y = 1.0 / gipd->height;
			gipi->cell_size_cache.z = 1.0 / gipd->depth;
		}
	}
}
void RasterizerSceneGLES3::gi_probe_instance_set_transform_to_data(RID p_probe, const Transform &p_xform) {

	GIProbeInstance *gipi = gi_probe_instance_owner.getornull(p_probe);
	ERR_FAIL_COND(!gipi);
	gipi->transform_to_data = p_xform;
}

void RasterizerSceneGLES3::gi_probe_instance_set_bounds(RID p_probe, const Vector3 &p_bounds) {

	GIProbeInstance *gipi = gi_probe_instance_owner.getornull(p_probe);
	ERR_FAIL_COND(!gipi);
	gipi->bounds = p_bounds;
}

////////////////////////////
////////////////////////////
////////////////////////////

bool RasterizerSceneGLES3::_setup_material(RasterizerStorageGLES3::Material *p_material, bool p_alpha_pass) {

	if (p_material->shader->spatial.cull_mode == RasterizerStorageGLES3::Shader::Spatial::CULL_MODE_DISABLED) {
		glDisable(GL_CULL_FACE);
	} else {
		glEnable(GL_CULL_FACE);
	}

	if (state.current_line_width != p_material->line_width) {
		//glLineWidth(MAX(p_material->line_width,1.0));
		state.current_line_width = p_material->line_width;
	}

	if (state.current_depth_test != (!p_material->shader->spatial.ontop)) {
		if (p_material->shader->spatial.ontop) {
			glDisable(GL_DEPTH_TEST);

		} else {
			glEnable(GL_DEPTH_TEST);
		}

		state.current_depth_test = !p_material->shader->spatial.ontop;
	}

	if (state.current_depth_draw != p_material->shader->spatial.depth_draw_mode) {
		switch (p_material->shader->spatial.depth_draw_mode) {
			case RasterizerStorageGLES3::Shader::Spatial::DEPTH_DRAW_ALPHA_PREPASS:
			case RasterizerStorageGLES3::Shader::Spatial::DEPTH_DRAW_OPAQUE: {

				glDepthMask(!p_alpha_pass);
			} break;
			case RasterizerStorageGLES3::Shader::Spatial::DEPTH_DRAW_ALWAYS: {
				glDepthMask(GL_TRUE);
			} break;
			case RasterizerStorageGLES3::Shader::Spatial::DEPTH_DRAW_NEVER: {
				glDepthMask(GL_FALSE);
			} break;
		}

		state.current_depth_draw = p_material->shader->spatial.depth_draw_mode;
	}

//glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);

/*
	if (p_material->flags[VS::MATERIAL_FLAG_WIREFRAME])
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

	if (p_material->line_width)
		glLineWidth(p_material->line_width);
	*/

#if 0
	//blend mode
	if (state.current_blend_mode!=p_material->shader->spatial.blend_mode) {

		switch(p_material->shader->spatial.blend_mode) {

			 case RasterizerStorageGLES3::Shader::Spatial::BLEND_MODE_MIX: {
				glBlendEquation(GL_FUNC_ADD);
				if (storage->frame.current_rt->flags[RasterizerStorage::RENDER_TARGET_TRANSPARENT]) {
					glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				} else {
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				}

			 } break;
			 case RasterizerStorageGLES3::Shader::Spatial::BLEND_MODE_ADD: {

				glBlendEquation(GL_FUNC_ADD);
				glBlendFunc(p_alpha_pass?GL_SRC_ALPHA:GL_ONE,GL_ONE);

			 } break;
			 case RasterizerStorageGLES3::Shader::Spatial::BLEND_MODE_SUB: {

				glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
				glBlendFunc(GL_SRC_ALPHA,GL_ONE);
			 } break;
			case RasterizerStorageGLES3::Shader::Spatial::BLEND_MODE_MUL: {
				glBlendEquation(GL_FUNC_ADD);
				if (storage->frame.current_rt->flags[RasterizerStorage::RENDER_TARGET_TRANSPARENT]) {
					glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				} else {
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				}

			} break;
		}

		state.current_blend_mode=p_material->shader->spatial.blend_mode;

	}
#endif
	//material parameters

	state.scene_shader.set_custom_shader(p_material->shader->custom_code_id);
	bool rebind = state.scene_shader.bind();

	if (p_material->ubo_id) {

		glBindBufferBase(GL_UNIFORM_BUFFER, 1, p_material->ubo_id);
	}

	int tc = p_material->textures.size();
	RID *textures = p_material->textures.ptr();
	ShaderLanguage::ShaderNode::Uniform::Hint *texture_hints = p_material->shader->texture_hints.ptr();

	state.current_main_tex = 0;

	for (int i = 0; i < tc; i++) {

		glActiveTexture(GL_TEXTURE0 + i);

		GLenum target;
		GLuint tex;

		RasterizerStorageGLES3::Texture *t = storage->texture_owner.getornull(textures[i]);

		if (!t) {
			//check hints
			target = GL_TEXTURE_2D;

			switch (texture_hints[i]) {
				case ShaderLanguage::ShaderNode::Uniform::HINT_BLACK_ALBEDO:
				case ShaderLanguage::ShaderNode::Uniform::HINT_BLACK: {
					tex = storage->resources.black_tex;
				} break;
				case ShaderLanguage::ShaderNode::Uniform::HINT_ANISO: {
					tex = storage->resources.aniso_tex;
				} break;
				case ShaderLanguage::ShaderNode::Uniform::HINT_NORMAL: {
					tex = storage->resources.normal_tex;

				} break;
				default: {
					tex = storage->resources.white_tex;
				} break;
			}

		} else {

#ifdef TOOLS_ENABLED
			if (t->detect_3d) {
				t->detect_3d(t->detect_3d_ud);
			}
#endif

#ifdef TOOLS_ENABLED
			if (t->detect_normal && texture_hints[i] == ShaderLanguage::ShaderNode::Uniform::HINT_NORMAL) {
				t->detect_normal(t->detect_normal_ud);
			}
#endif
			if (t->render_target)
				t->render_target->used_in_frame = true;

			target = t->target;
			tex = t->tex_id;
		}

		glBindTexture(target, tex);

		if (t && storage->config.srgb_decode_supported) {
			//if SRGB decode extension is present, simply switch the texture to whathever is needed
			bool must_srgb = false;

			if (t->srgb && (texture_hints[i] == ShaderLanguage::ShaderNode::Uniform::HINT_ALBEDO || texture_hints[i] == ShaderLanguage::ShaderNode::Uniform::HINT_BLACK_ALBEDO)) {
				must_srgb = true;
			}

			if (t->using_srgb != must_srgb) {
				if (must_srgb) {
					glTexParameteri(t->target, _TEXTURE_SRGB_DECODE_EXT, _DECODE_EXT);
#ifdef TOOLS_ENABLED
					if (t->detect_srgb) {
						t->detect_srgb(t->detect_srgb_ud);
					}
#endif

				} else {
					glTexParameteri(t->target, _TEXTURE_SRGB_DECODE_EXT, _SKIP_DECODE_EXT);
				}
				t->using_srgb = must_srgb;
			}
		}

		if (i == 0) {
			state.current_main_tex = tex;
		}
	}

	return rebind;
}

struct RasterizerGLES3Particle {

	float color[4];
	float velocity_active[4];
	float custom[4];
	float xform_1[4];
	float xform_2[4];
	float xform_3[4];
};

struct RasterizerGLES3ParticleSort {

	Vector3 z_dir;
	bool operator()(const RasterizerGLES3Particle &p_a, const RasterizerGLES3Particle &p_b) const {

		return z_dir.dot(Vector3(p_a.xform_1[3], p_a.xform_2[3], p_a.xform_3[3])) < z_dir.dot(Vector3(p_b.xform_1[3], p_b.xform_2[3], p_b.xform_3[3]));
	}
};

void RasterizerSceneGLES3::_setup_geometry(RenderList::Element *e, const Transform &p_view_transform) {

	switch (e->instance->base_type) {

		case VS::INSTANCE_MESH: {

			RasterizerStorageGLES3::Surface *s = static_cast<RasterizerStorageGLES3::Surface *>(e->geometry);

			if (s->blend_shapes.size() && e->instance->blend_values.size()) {
				//blend shapes, use transform feedback
				storage->mesh_render_blend_shapes(s, e->instance->blend_values.ptr());
				//rebind shader
				state.scene_shader.bind();
#ifdef DEBUG_ENABLED
			} else if (state.debug_draw == VS::VIEWPORT_DEBUG_DRAW_WIREFRAME && s->array_wireframe_id) {
				glBindVertexArray(s->array_wireframe_id); // everything is so easy nowadays
#endif
			} else {
				glBindVertexArray(s->array_id); // everything is so easy nowadays
			}

		} break;

		case VS::INSTANCE_MULTIMESH: {

			RasterizerStorageGLES3::MultiMesh *multi_mesh = static_cast<RasterizerStorageGLES3::MultiMesh *>(e->owner);
			RasterizerStorageGLES3::Surface *s = static_cast<RasterizerStorageGLES3::Surface *>(e->geometry);
#ifdef DEBUG_ENABLED
			if (state.debug_draw == VS::VIEWPORT_DEBUG_DRAW_WIREFRAME && s->instancing_array_wireframe_id) {

				glBindVertexArray(s->instancing_array_wireframe_id); // use the instancing array ID
			} else
#endif
			{
				glBindVertexArray(s->instancing_array_id); // use the instancing array ID
			}

			glBindBuffer(GL_ARRAY_BUFFER, multi_mesh->buffer); //modify the buffer

			int stride = (multi_mesh->xform_floats + multi_mesh->color_floats) * 4;
			glEnableVertexAttribArray(8);
			glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, stride, ((uint8_t *)NULL) + 0);
			glVertexAttribDivisor(8, 1);
			glEnableVertexAttribArray(9);
			glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, stride, ((uint8_t *)NULL) + 4 * 4);
			glVertexAttribDivisor(9, 1);

			int color_ofs;

			if (multi_mesh->transform_format == VS::MULTIMESH_TRANSFORM_3D) {
				glEnableVertexAttribArray(10);
				glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, stride, ((uint8_t *)NULL) + 8 * 4);
				glVertexAttribDivisor(10, 1);
				color_ofs = 12 * 4;
			} else {
				glDisableVertexAttribArray(10);
				glVertexAttrib4f(10, 0, 0, 1, 0);
				color_ofs = 8 * 4;
			}

			switch (multi_mesh->color_format) {

				case VS::MULTIMESH_COLOR_NONE: {
					glDisableVertexAttribArray(11);
					glVertexAttrib4f(11, 1, 1, 1, 1);
				} break;
				case VS::MULTIMESH_COLOR_8BIT: {
					glEnableVertexAttribArray(11);
					glVertexAttribPointer(11, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, ((uint8_t *)NULL) + color_ofs);
					glVertexAttribDivisor(11, 1);

				} break;
				case VS::MULTIMESH_COLOR_FLOAT: {
					glEnableVertexAttribArray(11);
					glVertexAttribPointer(11, 4, GL_FLOAT, GL_FALSE, stride, ((uint8_t *)NULL) + color_ofs);
					glVertexAttribDivisor(11, 1);
				} break;
			}

		} break;
		case VS::INSTANCE_PARTICLES: {

			RasterizerStorageGLES3::Particles *particles = static_cast<RasterizerStorageGLES3::Particles *>(e->owner);
			RasterizerStorageGLES3::Surface *s = static_cast<RasterizerStorageGLES3::Surface *>(e->geometry);

			if (particles->draw_order == VS::PARTICLES_DRAW_ORDER_VIEW_DEPTH && particles->particle_valid_histories[1]) {

				glBindBuffer(GL_ARRAY_BUFFER, particles->particle_buffer_histories[1]); //modify the buffer, this was used 2 frames ago so it should be good enough for flushing
				RasterizerGLES3Particle *particle_array = (RasterizerGLES3Particle *)glMapBufferRange(GL_ARRAY_BUFFER, 0, particles->amount * 24 * sizeof(float), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);

				SortArray<RasterizerGLES3Particle, RasterizerGLES3ParticleSort> sorter;

				if (particles->use_local_coords) {
					sorter.compare.z_dir = e->instance->transform.affine_inverse().xform(p_view_transform.basis.get_axis(2)).normalized();
				} else {
					sorter.compare.z_dir = p_view_transform.basis.get_axis(2).normalized();
				}

				sorter.sort(particle_array, particles->amount);

				glUnmapBuffer(GL_ARRAY_BUFFER);
#ifdef DEBUG_ENABLED
				if (state.debug_draw == VS::VIEWPORT_DEBUG_DRAW_WIREFRAME && s->instancing_array_wireframe_id) {
					glBindVertexArray(s->instancing_array_wireframe_id); // use the wireframe instancing array ID
				} else
#endif
				{

					glBindVertexArray(s->instancing_array_id); // use the instancing array ID
				}
				glBindBuffer(GL_ARRAY_BUFFER, particles->particle_buffer_histories[1]); //modify the buffer

			} else {
#ifdef DEBUG_ENABLED
				if (state.debug_draw == VS::VIEWPORT_DEBUG_DRAW_WIREFRAME && s->instancing_array_wireframe_id) {
					glBindVertexArray(s->instancing_array_wireframe_id); // use the wireframe instancing array ID
				} else
#endif
				{
					glBindVertexArray(s->instancing_array_id); // use the instancing array ID
				}
				glBindBuffer(GL_ARRAY_BUFFER, particles->particle_buffers[0]); //modify the buffer
			}

			int stride = sizeof(float) * 4 * 6;

			//transform

			if (particles->draw_order != VS::PARTICLES_DRAW_ORDER_LIFETIME) {

				glEnableVertexAttribArray(8); //xform x
				glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, stride, ((uint8_t *)NULL) + sizeof(float) * 4 * 3);
				glVertexAttribDivisor(8, 1);
				glEnableVertexAttribArray(9); //xform y
				glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, stride, ((uint8_t *)NULL) + sizeof(float) * 4 * 4);
				glVertexAttribDivisor(9, 1);
				glEnableVertexAttribArray(10); //xform z
				glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, stride, ((uint8_t *)NULL) + sizeof(float) * 4 * 5);
				glVertexAttribDivisor(10, 1);
				glEnableVertexAttribArray(11); //color
				glVertexAttribPointer(11, 4, GL_FLOAT, GL_FALSE, stride, ((uint8_t *)NULL) + 0);
				glVertexAttribDivisor(11, 1);
				glEnableVertexAttribArray(12); //custom
				glVertexAttribPointer(12, 4, GL_FLOAT, GL_FALSE, stride, ((uint8_t *)NULL) + sizeof(float) * 4 * 2);
				glVertexAttribDivisor(12, 1);
			}

		} break;
	}
}

static const GLenum gl_primitive[] = {
	GL_POINTS,
	GL_LINES,
	GL_LINE_STRIP,
	GL_LINE_LOOP,
	GL_TRIANGLES,
	GL_TRIANGLE_STRIP,
	GL_TRIANGLE_FAN
};

void RasterizerSceneGLES3::_render_geometry(RenderList::Element *e) {

	switch (e->instance->base_type) {

		case VS::INSTANCE_MESH: {

			RasterizerStorageGLES3::Surface *s = static_cast<RasterizerStorageGLES3::Surface *>(e->geometry);

#ifdef DEBUG_ENABLED

			if (state.debug_draw == VS::VIEWPORT_DEBUG_DRAW_WIREFRAME && s->array_wireframe_id) {

				glDrawElements(GL_LINES, s->index_wireframe_len, GL_UNSIGNED_INT, 0);
				storage->info.render.vertices_count += s->index_array_len;
			} else
#endif
					if (s->index_array_len > 0) {

				glDrawElements(gl_primitive[s->primitive], s->index_array_len, (s->array_len >= (1 << 16)) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, 0);

				storage->info.render.vertices_count += s->index_array_len;

			} else {

				glDrawArrays(gl_primitive[s->primitive], 0, s->array_len);

				storage->info.render.vertices_count += s->array_len;
			}

		} break;
		case VS::INSTANCE_MULTIMESH: {

			RasterizerStorageGLES3::MultiMesh *multi_mesh = static_cast<RasterizerStorageGLES3::MultiMesh *>(e->owner);
			RasterizerStorageGLES3::Surface *s = static_cast<RasterizerStorageGLES3::Surface *>(e->geometry);

			int amount = MAX(multi_mesh->size, multi_mesh->visible_instances);

#ifdef DEBUG_ENABLED

			if (state.debug_draw == VS::VIEWPORT_DEBUG_DRAW_WIREFRAME && s->array_wireframe_id) {

				glDrawElementsInstanced(GL_LINES, s->index_wireframe_len, GL_UNSIGNED_INT, 0, amount);
				storage->info.render.vertices_count += s->index_array_len * amount;
			} else
#endif
					if (s->index_array_len > 0) {

				glDrawElementsInstanced(gl_primitive[s->primitive], s->index_array_len, (s->array_len >= (1 << 16)) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, 0, amount);

				storage->info.render.vertices_count += s->index_array_len * amount;

			} else {

				glDrawArraysInstanced(gl_primitive[s->primitive], 0, s->array_len, amount);

				storage->info.render.vertices_count += s->array_len * amount;
			}

		} break;
		case VS::INSTANCE_IMMEDIATE: {

			bool restore_tex = false;
			const RasterizerStorageGLES3::Immediate *im = static_cast<const RasterizerStorageGLES3::Immediate *>(e->geometry);

			if (im->building) {
				return;
			}

			glBindBuffer(GL_ARRAY_BUFFER, state.immediate_buffer);
			glBindVertexArray(state.immediate_array);

			for (const List<RasterizerStorageGLES3::Immediate::Chunk>::Element *E = im->chunks.front(); E; E = E->next()) {

				const RasterizerStorageGLES3::Immediate::Chunk &c = E->get();
				if (c.vertices.empty()) {
					continue;
				}

				int vertices = c.vertices.size();
				uint32_t buf_ofs = 0;

				storage->info.render.vertices_count += vertices;

				if (c.texture.is_valid() && storage->texture_owner.owns(c.texture)) {

					const RasterizerStorageGLES3::Texture *t = storage->texture_owner.get(c.texture);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(t->target, t->tex_id);
					restore_tex = true;

				} else if (restore_tex) {

					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, state.current_main_tex);
					restore_tex = false;
				}

				if (!c.normals.empty()) {

					glEnableVertexAttribArray(VS::ARRAY_NORMAL);
					glBufferSubData(GL_ARRAY_BUFFER, buf_ofs, sizeof(Vector3) * vertices, c.normals.ptr());
					glVertexAttribPointer(VS::ARRAY_NORMAL, 3, GL_FLOAT, false, sizeof(Vector3) * vertices, ((uint8_t *)NULL) + buf_ofs);
					buf_ofs += sizeof(Vector3) * vertices;

				} else {

					glDisableVertexAttribArray(VS::ARRAY_NORMAL);
				}

				if (!c.tangents.empty()) {

					glEnableVertexAttribArray(VS::ARRAY_TANGENT);
					glBufferSubData(GL_ARRAY_BUFFER, buf_ofs, sizeof(Plane) * vertices, c.tangents.ptr());
					glVertexAttribPointer(VS::ARRAY_TANGENT, 4, GL_FLOAT, false, sizeof(Plane) * vertices, ((uint8_t *)NULL) + buf_ofs);
					buf_ofs += sizeof(Plane) * vertices;

				} else {

					glDisableVertexAttribArray(VS::ARRAY_TANGENT);
				}

				if (!c.colors.empty()) {

					glEnableVertexAttribArray(VS::ARRAY_COLOR);
					glBufferSubData(GL_ARRAY_BUFFER, buf_ofs, sizeof(Color) * vertices, c.colors.ptr());
					glVertexAttribPointer(VS::ARRAY_COLOR, 4, GL_FLOAT, false, sizeof(Color), ((uint8_t *)NULL) + buf_ofs);
					buf_ofs += sizeof(Color) * vertices;

				} else {

					glDisableVertexAttribArray(VS::ARRAY_COLOR);
					glVertexAttrib4f(VS::ARRAY_COLOR, 1, 1, 1, 1);
				}

				if (!c.uvs.empty()) {

					glEnableVertexAttribArray(VS::ARRAY_TEX_UV);
					glBufferSubData(GL_ARRAY_BUFFER, buf_ofs, sizeof(Vector2) * vertices, c.uvs.ptr());
					glVertexAttribPointer(VS::ARRAY_TEX_UV, 2, GL_FLOAT, false, sizeof(Vector2), ((uint8_t *)NULL) + buf_ofs);
					buf_ofs += sizeof(Vector2) * vertices;

				} else {

					glDisableVertexAttribArray(VS::ARRAY_TEX_UV);
				}

				if (!c.uvs2.empty()) {

					glEnableVertexAttribArray(VS::ARRAY_TEX_UV2);
					glBufferSubData(GL_ARRAY_BUFFER, buf_ofs, sizeof(Vector2) * vertices, c.uvs2.ptr());
					glVertexAttribPointer(VS::ARRAY_TEX_UV2, 2, GL_FLOAT, false, sizeof(Vector2), ((uint8_t *)NULL) + buf_ofs);
					buf_ofs += sizeof(Vector2) * vertices;

				} else {

					glDisableVertexAttribArray(VS::ARRAY_TEX_UV2);
				}

				glEnableVertexAttribArray(VS::ARRAY_VERTEX);
				glBufferSubData(GL_ARRAY_BUFFER, buf_ofs, sizeof(Vector3) * vertices, c.vertices.ptr());
				glVertexAttribPointer(VS::ARRAY_VERTEX, 3, GL_FLOAT, false, sizeof(Vector3), ((uint8_t *)NULL) + buf_ofs);
				glDrawArrays(gl_primitive[c.primitive], 0, c.vertices.size());
			}

			if (restore_tex) {

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, state.current_main_tex);
				restore_tex = false;
			}
		} break;
		case VS::INSTANCE_PARTICLES: {

			RasterizerStorageGLES3::Particles *particles = static_cast<RasterizerStorageGLES3::Particles *>(e->owner);
			RasterizerStorageGLES3::Surface *s = static_cast<RasterizerStorageGLES3::Surface *>(e->geometry);

			if (!particles->use_local_coords) //not using local coordinates? then clear transform..
				state.scene_shader.set_uniform(SceneShaderGLES3::WORLD_TRANSFORM, Transform());

			int amount = particles->amount;

			if (particles->draw_order == VS::PARTICLES_DRAW_ORDER_LIFETIME) {
				//split

				int stride = sizeof(float) * 4 * 6;
				int split = int(Math::ceil(particles->phase * particles->amount));

				if (amount - split > 0) {
					glEnableVertexAttribArray(8); //xform x
					glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, stride, ((uint8_t *)NULL) + stride * split + sizeof(float) * 4 * 3);
					glVertexAttribDivisor(8, 1);
					glEnableVertexAttribArray(9); //xform y
					glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, stride, ((uint8_t *)NULL) + stride * split + sizeof(float) * 4 * 4);
					glVertexAttribDivisor(9, 1);
					glEnableVertexAttribArray(10); //xform z
					glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, stride, ((uint8_t *)NULL) + stride * split + sizeof(float) * 4 * 5);
					glVertexAttribDivisor(10, 1);
					glEnableVertexAttribArray(11); //color
					glVertexAttribPointer(11, 4, GL_FLOAT, GL_FALSE, stride, ((uint8_t *)NULL) + stride * split + 0);
					glVertexAttribDivisor(11, 1);
					glEnableVertexAttribArray(12); //custom
					glVertexAttribPointer(12, 4, GL_FLOAT, GL_FALSE, stride, ((uint8_t *)NULL) + stride * split + sizeof(float) * 4 * 2);
					glVertexAttribDivisor(12, 1);
#ifdef DEBUG_ENABLED

					if (state.debug_draw == VS::VIEWPORT_DEBUG_DRAW_WIREFRAME && s->array_wireframe_id) {

						glDrawElementsInstanced(GL_LINES, s->index_wireframe_len, GL_UNSIGNED_INT, 0, amount - split);
						storage->info.render.vertices_count += s->index_array_len * (amount - split);
					} else
#endif
							if (s->index_array_len > 0) {

						glDrawElementsInstanced(gl_primitive[s->primitive], s->index_array_len, (s->array_len >= (1 << 16)) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, 0, amount - split);

						storage->info.render.vertices_count += s->index_array_len * (amount - split);

					} else {

						glDrawArraysInstanced(gl_primitive[s->primitive], 0, s->array_len, amount - split);

						storage->info.render.vertices_count += s->array_len * (amount - split);
					}
				}

				if (split > 0) {
					glEnableVertexAttribArray(8); //xform x
					glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, stride, ((uint8_t *)NULL) + sizeof(float) * 4 * 3);
					glVertexAttribDivisor(8, 1);
					glEnableVertexAttribArray(9); //xform y
					glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, stride, ((uint8_t *)NULL) + sizeof(float) * 4 * 4);
					glVertexAttribDivisor(9, 1);
					glEnableVertexAttribArray(10); //xform z
					glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, stride, ((uint8_t *)NULL) + sizeof(float) * 4 * 5);
					glVertexAttribDivisor(10, 1);
					glEnableVertexAttribArray(11); //color
					glVertexAttribPointer(11, 4, GL_FLOAT, GL_FALSE, stride, ((uint8_t *)NULL) + 0);
					glVertexAttribDivisor(11, 1);
					glEnableVertexAttribArray(12); //custom
					glVertexAttribPointer(12, 4, GL_FLOAT, GL_FALSE, stride, ((uint8_t *)NULL) + sizeof(float) * 4 * 2);
					glVertexAttribDivisor(12, 1);
#ifdef DEBUG_ENABLED

					if (state.debug_draw == VS::VIEWPORT_DEBUG_DRAW_WIREFRAME && s->array_wireframe_id) {

						glDrawElementsInstanced(GL_LINES, s->index_wireframe_len, GL_UNSIGNED_INT, 0, split);
						storage->info.render.vertices_count += s->index_array_len * split;
					} else
#endif
							if (s->index_array_len > 0) {

						glDrawElementsInstanced(gl_primitive[s->primitive], s->index_array_len, (s->array_len >= (1 << 16)) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, 0, split);

						storage->info.render.vertices_count += s->index_array_len * split;

					} else {

						glDrawArraysInstanced(gl_primitive[s->primitive], 0, s->array_len, split);

						storage->info.render.vertices_count += s->array_len * split;
					}
				}

			} else {

#ifdef DEBUG_ENABLED

				if (state.debug_draw == VS::VIEWPORT_DEBUG_DRAW_WIREFRAME && s->array_wireframe_id) {

					glDrawElementsInstanced(GL_LINES, s->index_wireframe_len, GL_UNSIGNED_INT, 0, amount);
					storage->info.render.vertices_count += s->index_array_len * amount;
				} else
#endif
						if (s->index_array_len > 0) {

					glDrawElementsInstanced(gl_primitive[s->primitive], s->index_array_len, (s->array_len >= (1 << 16)) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, 0, amount);

					storage->info.render.vertices_count += s->index_array_len * amount;

				} else {

					glDrawArraysInstanced(gl_primitive[s->primitive], 0, s->array_len, amount);

					storage->info.render.vertices_count += s->array_len * amount;
				}
			}

		} break;
	}
}

void RasterizerSceneGLES3::_setup_light(RenderList::Element *e, const Transform &p_view_transform) {

	int omni_indices[16];
	int omni_count = 0;
	int spot_indices[16];
	int spot_count = 0;
	int reflection_indices[16];
	int reflection_count = 0;

	int maxobj = MIN(16, state.max_forward_lights_per_object);

	int lc = e->instance->light_instances.size();
	if (lc) {

		const RID *lights = e->instance->light_instances.ptr();

		for (int i = 0; i < lc; i++) {
			LightInstance *li = light_instance_owner.getptr(lights[i]);
			if (li->last_pass != render_pass) //not visible
				continue;

			if (li->light_ptr->type == VS::LIGHT_OMNI) {
				if (omni_count < maxobj && e->instance->layer_mask & li->light_ptr->cull_mask) {
					omni_indices[omni_count++] = li->light_index;
				}
			}

			if (li->light_ptr->type == VS::LIGHT_SPOT) {
				if (spot_count < maxobj && e->instance->layer_mask & li->light_ptr->cull_mask) {
					spot_indices[spot_count++] = li->light_index;
				}
			}
		}
	}

	state.scene_shader.set_uniform(SceneShaderGLES3::OMNI_LIGHT_COUNT, omni_count);

	if (omni_count) {
		glUniform1iv(state.scene_shader.get_uniform(SceneShaderGLES3::OMNI_LIGHT_INDICES), omni_count, omni_indices);
	}

	state.scene_shader.set_uniform(SceneShaderGLES3::SPOT_LIGHT_COUNT, spot_count);
	if (spot_count) {
		glUniform1iv(state.scene_shader.get_uniform(SceneShaderGLES3::SPOT_LIGHT_INDICES), spot_count, spot_indices);
	}

	int rc = e->instance->reflection_probe_instances.size();

	if (rc) {

		const RID *reflections = e->instance->reflection_probe_instances.ptr();

		for (int i = 0; i < rc; i++) {
			ReflectionProbeInstance *rpi = reflection_probe_instance_owner.getptr(reflections[i]);
			if (rpi->last_pass != render_pass) //not visible
				continue;

			if (reflection_count < maxobj) {
				reflection_indices[reflection_count++] = rpi->reflection_index;
			}
		}
	}

	state.scene_shader.set_uniform(SceneShaderGLES3::REFLECTION_COUNT, reflection_count);
	if (reflection_count) {
		glUniform1iv(state.scene_shader.get_uniform(SceneShaderGLES3::REFLECTION_INDICES), reflection_count, reflection_indices);
	}

	int gi_probe_count = e->instance->gi_probe_instances.size();
	if (gi_probe_count) {
		const RID *ridp = e->instance->gi_probe_instances.ptr();

		GIProbeInstance *gipi = gi_probe_instance_owner.getptr(ridp[0]);

		float bias_scale = e->instance->baked_light ? 1 : 0;
		glActiveTexture(GL_TEXTURE0 + storage->config.max_texture_image_units - 9);
		glBindTexture(GL_TEXTURE_3D, gipi->tex_cache);
		state.scene_shader.set_uniform(SceneShaderGLES3::GI_PROBE_XFORM1, gipi->transform_to_data * p_view_transform);
		state.scene_shader.set_uniform(SceneShaderGLES3::GI_PROBE_BOUNDS1, gipi->bounds);
		state.scene_shader.set_uniform(SceneShaderGLES3::GI_PROBE_MULTIPLIER1, gipi->probe ? gipi->probe->dynamic_range * gipi->probe->energy : 0.0);
		state.scene_shader.set_uniform(SceneShaderGLES3::GI_PROBE_BIAS1, gipi->probe ? gipi->probe->bias * bias_scale : 0.0);
		state.scene_shader.set_uniform(SceneShaderGLES3::GI_PROBE_NORMAL_BIAS1, gipi->probe ? gipi->probe->normal_bias * bias_scale : 0.0);
		state.scene_shader.set_uniform(SceneShaderGLES3::GI_PROBE_BLEND_AMBIENT1, gipi->probe ? !gipi->probe->interior : false);
		state.scene_shader.set_uniform(SceneShaderGLES3::GI_PROBE_CELL_SIZE1, gipi->cell_size_cache);
		if (gi_probe_count > 1) {

			GIProbeInstance *gipi2 = gi_probe_instance_owner.getptr(ridp[1]);

			glActiveTexture(GL_TEXTURE0 + storage->config.max_texture_image_units - 10);
			glBindTexture(GL_TEXTURE_3D, gipi2->tex_cache);
			state.scene_shader.set_uniform(SceneShaderGLES3::GI_PROBE_XFORM2, gipi2->transform_to_data * p_view_transform);
			state.scene_shader.set_uniform(SceneShaderGLES3::GI_PROBE_BOUNDS2, gipi2->bounds);
			state.scene_shader.set_uniform(SceneShaderGLES3::GI_PROBE_CELL_SIZE2, gipi2->cell_size_cache);
			state.scene_shader.set_uniform(SceneShaderGLES3::GI_PROBE_MULTIPLIER2, gipi2->probe ? gipi2->probe->dynamic_range * gipi2->probe->energy : 0.0);
			state.scene_shader.set_uniform(SceneShaderGLES3::GI_PROBE_BIAS2, gipi2->probe ? gipi2->probe->bias * bias_scale : 0.0);
			state.scene_shader.set_uniform(SceneShaderGLES3::GI_PROBE_NORMAL_BIAS2, gipi2->probe ? gipi2->probe->normal_bias * bias_scale : 0.0);
			state.scene_shader.set_uniform(SceneShaderGLES3::GI_PROBE_BLEND_AMBIENT2, gipi2->probe ? !gipi2->probe->interior : false);
			state.scene_shader.set_uniform(SceneShaderGLES3::GI_PROBE2_ENABLED, true);
		} else {

			state.scene_shader.set_uniform(SceneShaderGLES3::GI_PROBE2_ENABLED, false);
		}
	}
}

void RasterizerSceneGLES3::_set_cull(bool p_front, bool p_reverse_cull) {

	bool front = p_front;
	if (p_reverse_cull)
		front = !front;

	if (front != state.cull_front) {

		glCullFace(front ? GL_FRONT : GL_BACK);
		state.cull_front = front;
	}
}

void RasterizerSceneGLES3::_render_list(RenderList::Element **p_elements, int p_element_count, const Transform &p_view_transform, const CameraMatrix &p_projection, GLuint p_base_env, bool p_reverse_cull, bool p_alpha_pass, bool p_shadow, bool p_directional_add, bool p_directional_shadows) {

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, state.scene_ubo); //bind globals ubo

	bool use_radiance_map = false;
	if (!p_shadow && !p_directional_add) {
		glBindBufferBase(GL_UNIFORM_BUFFER, 2, state.env_radiance_ubo); //bind environment radiance info

		if (p_base_env) {
			glActiveTexture(GL_TEXTURE0 + storage->config.max_texture_image_units - 2);
			if (storage->config.use_texture_array_environment) {
				glBindTexture(GL_TEXTURE_2D_ARRAY, p_base_env);
			} else {
				glBindTexture(GL_TEXTURE_2D, p_base_env);
			}

			state.scene_shader.set_conditional(SceneShaderGLES3::USE_RADIANCE_MAP, true);
			state.scene_shader.set_conditional(SceneShaderGLES3::USE_RADIANCE_MAP_ARRAY, storage->config.use_texture_array_environment);
			use_radiance_map = true;
		} else {
			state.scene_shader.set_conditional(SceneShaderGLES3::USE_RADIANCE_MAP, false);
			state.scene_shader.set_conditional(SceneShaderGLES3::USE_RADIANCE_MAP_ARRAY, false);
		}
	} else {

		state.scene_shader.set_conditional(SceneShaderGLES3::USE_RADIANCE_MAP, false);
		state.scene_shader.set_conditional(SceneShaderGLES3::USE_RADIANCE_MAP_ARRAY, false);
	}

	state.cull_front = false;
	glCullFace(GL_BACK);

	state.current_depth_test = true;
	glEnable(GL_DEPTH_TEST);

	state.scene_shader.set_conditional(SceneShaderGLES3::USE_SKELETON, false);

	state.current_blend_mode = -1;
	state.current_line_width = -1;
	state.current_depth_draw = -1;

	RasterizerStorageGLES3::Material *prev_material = NULL;
	RasterizerStorageGLES3::Geometry *prev_geometry = NULL;
	RasterizerStorageGLES3::GeometryOwner *prev_owner = NULL;
	VS::InstanceType prev_base_type = VS::INSTANCE_MAX;

	int current_blend_mode = -1;

	int prev_shading = -1;
	RID prev_skeleton;

	state.scene_shader.set_conditional(SceneShaderGLES3::SHADELESS, true); //by default unshaded (easier to set)

	bool first = true;
	bool prev_use_instancing = false;

	storage->info.render.draw_call_count += p_element_count;

	for (int i = 0; i < p_element_count; i++) {

		RenderList::Element *e = p_elements[i];
		RasterizerStorageGLES3::Material *material = e->material;
		RID skeleton = e->instance->skeleton;

		bool rebind = first;

		int shading = (e->sort_key >> RenderList::SORT_KEY_SHADING_SHIFT) & RenderList::SORT_KEY_SHADING_MASK;

		if (!p_shadow) {

			if (p_directional_add) {
				if (e->sort_key & SORT_KEY_UNSHADED_FLAG || !(e->instance->layer_mask & directional_light->light_ptr->cull_mask)) {
					continue;
				}

				shading &= ~1; //ignore the ignore directional for base pass
			}

			if (shading != prev_shading) {

				if (e->sort_key & SORT_KEY_UNSHADED_FLAG) {

					state.scene_shader.set_conditional(SceneShaderGLES3::SHADELESS, true);
					state.scene_shader.set_conditional(SceneShaderGLES3::USE_FORWARD_LIGHTING, false);
					state.scene_shader.set_conditional(SceneShaderGLES3::USE_VERTEX_LIGHTING, false);
					state.scene_shader.set_conditional(SceneShaderGLES3::USE_LIGHT_DIRECTIONAL, false);
					state.scene_shader.set_conditional(SceneShaderGLES3::LIGHT_DIRECTIONAL_SHADOW, false);
					state.scene_shader.set_conditional(SceneShaderGLES3::LIGHT_USE_PSSM4, false);
					state.scene_shader.set_conditional(SceneShaderGLES3::LIGHT_USE_PSSM2, false);
					state.scene_shader.set_conditional(SceneShaderGLES3::LIGHT_USE_PSSM_BLEND, false);
					state.scene_shader.set_conditional(SceneShaderGLES3::LIGHT_USE_PSSM_BLEND, false);
					state.scene_shader.set_conditional(SceneShaderGLES3::SHADOW_MODE_PCF_5, false);
					state.scene_shader.set_conditional(SceneShaderGLES3::SHADOW_MODE_PCF_13, false);
					state.scene_shader.set_conditional(SceneShaderGLES3::USE_GI_PROBES, false);
					state.scene_shader.set_conditional(SceneShaderGLES3::USE_RADIANCE_MAP, false);
					state.scene_shader.set_conditional(SceneShaderGLES3::USE_CONTACT_SHADOWS, false);

					//state.scene_shader.set_conditional(SceneShaderGLES3::SHADELESS,true);
				} else {

					state.scene_shader.set_conditional(SceneShaderGLES3::USE_GI_PROBES, e->instance->gi_probe_instances.size() > 0);

					state.scene_shader.set_conditional(SceneShaderGLES3::SHADELESS, false);

					state.scene_shader.set_conditional(SceneShaderGLES3::USE_FORWARD_LIGHTING, !p_directional_add);
					state.scene_shader.set_conditional(SceneShaderGLES3::USE_VERTEX_LIGHTING, (e->sort_key & SORT_KEY_VERTEX_LIT_FLAG));

					state.scene_shader.set_conditional(SceneShaderGLES3::USE_LIGHT_DIRECTIONAL, false);
					state.scene_shader.set_conditional(SceneShaderGLES3::LIGHT_DIRECTIONAL_SHADOW, false);
					state.scene_shader.set_conditional(SceneShaderGLES3::LIGHT_USE_PSSM4, false);
					state.scene_shader.set_conditional(SceneShaderGLES3::LIGHT_USE_PSSM2, false);
					state.scene_shader.set_conditional(SceneShaderGLES3::LIGHT_USE_PSSM_BLEND, false);
					state.scene_shader.set_conditional(SceneShaderGLES3::SHADOW_MODE_PCF_5, shadow_filter_mode == SHADOW_FILTER_PCF5);
					state.scene_shader.set_conditional(SceneShaderGLES3::SHADOW_MODE_PCF_13, shadow_filter_mode == SHADOW_FILTER_PCF13);
					state.scene_shader.set_conditional(SceneShaderGLES3::USE_RADIANCE_MAP, use_radiance_map);
					state.scene_shader.set_conditional(SceneShaderGLES3::USE_CONTACT_SHADOWS, state.used_contact_shadows);

					if (p_directional_add || (directional_light && (e->sort_key & SORT_KEY_NO_DIRECTIONAL_FLAG) == 0)) {
						state.scene_shader.set_conditional(SceneShaderGLES3::USE_LIGHT_DIRECTIONAL, true);

						if (p_directional_shadows && directional_light->light_ptr->shadow) {
							state.scene_shader.set_conditional(SceneShaderGLES3::LIGHT_DIRECTIONAL_SHADOW, true);

							switch (directional_light->light_ptr->directional_shadow_mode) {
								case VS::LIGHT_DIRECTIONAL_SHADOW_ORTHOGONAL:
									break; //none
								case VS::LIGHT_DIRECTIONAL_SHADOW_PARALLEL_2_SPLITS:
									state.scene_shader.set_conditional(SceneShaderGLES3::LIGHT_USE_PSSM2, true);
									state.scene_shader.set_conditional(SceneShaderGLES3::LIGHT_USE_PSSM_BLEND, directional_light->light_ptr->directional_blend_splits);
									break;
								case VS::LIGHT_DIRECTIONAL_SHADOW_PARALLEL_4_SPLITS:
									state.scene_shader.set_conditional(SceneShaderGLES3::LIGHT_USE_PSSM4, true);
									state.scene_shader.set_conditional(SceneShaderGLES3::LIGHT_USE_PSSM_BLEND, directional_light->light_ptr->directional_blend_splits);
									break;
							}
						}
					}
				}

				rebind = true;
			}

			if (p_alpha_pass || p_directional_add) {
				int desired_blend_mode;
				if (p_directional_add) {
					desired_blend_mode = RasterizerStorageGLES3::Shader::Spatial::BLEND_MODE_ADD;
				} else {
					desired_blend_mode = material->shader->spatial.blend_mode;
				}

				if (desired_blend_mode != current_blend_mode) {

					switch (desired_blend_mode) {

						case RasterizerStorageGLES3::Shader::Spatial::BLEND_MODE_MIX: {
							glBlendEquation(GL_FUNC_ADD);
							if (storage->frame.current_rt && storage->frame.current_rt->flags[RasterizerStorage::RENDER_TARGET_TRANSPARENT]) {
								glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
							} else {
								glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
							}

						} break;
						case RasterizerStorageGLES3::Shader::Spatial::BLEND_MODE_ADD: {

							glBlendEquation(GL_FUNC_ADD);
							glBlendFunc(p_alpha_pass ? GL_SRC_ALPHA : GL_ONE, GL_ONE);

						} break;
						case RasterizerStorageGLES3::Shader::Spatial::BLEND_MODE_SUB: {

							glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
							glBlendFunc(GL_SRC_ALPHA, GL_ONE);
						} break;
						case RasterizerStorageGLES3::Shader::Spatial::BLEND_MODE_MUL: {
							glBlendEquation(GL_FUNC_ADD);
							if (storage->frame.current_rt && storage->frame.current_rt->flags[RasterizerStorage::RENDER_TARGET_TRANSPARENT]) {
								glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
							} else {
								glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
							}

						} break;
					}

					current_blend_mode = desired_blend_mode;
				}
			}
		}

		bool use_instancing = e->instance->base_type == VS::INSTANCE_MULTIMESH || e->instance->base_type == VS::INSTANCE_PARTICLES;

		if (use_instancing != prev_use_instancing) {
			state.scene_shader.set_conditional(SceneShaderGLES3::USE_INSTANCING, use_instancing);
			rebind = true;
		}

		if (prev_skeleton != skeleton) {
			if (prev_skeleton.is_valid() != skeleton.is_valid()) {
				state.scene_shader.set_conditional(SceneShaderGLES3::USE_SKELETON, skeleton.is_valid());
				rebind = true;
			}

			if (skeleton.is_valid()) {
				RasterizerStorageGLES3::Skeleton *sk = storage->skeleton_owner.getornull(skeleton);
				glActiveTexture(GL_TEXTURE0 + storage->config.max_texture_image_units - 1);
				glBindTexture(GL_TEXTURE_2D, sk->texture);
			}
		}

		if (material != prev_material || rebind) {

			storage->info.render.material_switch_count++;

			rebind = _setup_material(material, p_alpha_pass);

			if (rebind) {
				storage->info.render.shader_rebind_count++;
			}
		}

		if (!(e->sort_key & SORT_KEY_UNSHADED_FLAG) && !p_directional_add && !p_shadow) {
			_setup_light(e, p_view_transform);
		}

		if (e->owner != prev_owner || prev_base_type != e->instance->base_type || prev_geometry != e->geometry) {

			_setup_geometry(e, p_view_transform);
			storage->info.render.surface_switch_count++;
		}

		_set_cull(e->sort_key & RenderList::SORT_KEY_MIRROR_FLAG, p_reverse_cull);

		state.scene_shader.set_uniform(SceneShaderGLES3::NORMAL_MULT, e->instance->mirror ? -1.0 : 1.0);
		state.scene_shader.set_uniform(SceneShaderGLES3::WORLD_TRANSFORM, e->instance->transform);

		_render_geometry(e);

		prev_material = material;
		prev_base_type = e->instance->base_type;
		prev_geometry = e->geometry;
		prev_owner = e->owner;
		prev_shading = shading;
		prev_skeleton = skeleton;
		prev_use_instancing = use_instancing;
		first = false;
	}

	glFrontFace(GL_CW);
	glBindVertexArray(0);

	state.scene_shader.set_conditional(SceneShaderGLES3::USE_INSTANCING, false);
	state.scene_shader.set_conditional(SceneShaderGLES3::USE_SKELETON, false);
	state.scene_shader.set_conditional(SceneShaderGLES3::USE_RADIANCE_MAP, false);
	state.scene_shader.set_conditional(SceneShaderGLES3::USE_FORWARD_LIGHTING, false);
	state.scene_shader.set_conditional(SceneShaderGLES3::USE_LIGHT_DIRECTIONAL, false);
	state.scene_shader.set_conditional(SceneShaderGLES3::LIGHT_DIRECTIONAL_SHADOW, false);
	state.scene_shader.set_conditional(SceneShaderGLES3::LIGHT_USE_PSSM4, false);
	state.scene_shader.set_conditional(SceneShaderGLES3::LIGHT_USE_PSSM2, false);
	state.scene_shader.set_conditional(SceneShaderGLES3::LIGHT_USE_PSSM_BLEND, false);
	state.scene_shader.set_conditional(SceneShaderGLES3::SHADELESS, false);
	state.scene_shader.set_conditional(SceneShaderGLES3::SHADOW_MODE_PCF_5, false);
	state.scene_shader.set_conditional(SceneShaderGLES3::SHADOW_MODE_PCF_13, false);
	state.scene_shader.set_conditional(SceneShaderGLES3::USE_GI_PROBES, false);
	state.scene_shader.set_conditional(SceneShaderGLES3::USE_CONTACT_SHADOWS, false);
	state.scene_shader.set_conditional(SceneShaderGLES3::USE_VERTEX_LIGHTING, false);
}

void RasterizerSceneGLES3::_add_geometry(RasterizerStorageGLES3::Geometry *p_geometry, InstanceBase *p_instance, RasterizerStorageGLES3::GeometryOwner *p_owner, int p_material, bool p_shadow) {

	RasterizerStorageGLES3::Material *m = NULL;
	RID m_src = p_instance->material_override.is_valid() ? p_instance->material_override : (p_material >= 0 ? p_instance->materials[p_material] : p_geometry->material);

	if (state.debug_draw == VS::VIEWPORT_DEBUG_DRAW_OVERDRAW) {
		m_src = default_overdraw_material;
	}

	/*
#ifdef DEBUG_ENABLED
	if (current_debug==VS::SCENARIO_DEBUG_OVERDRAW) {
		m_src=overdraw_material;
	}

#endif
*/

	if (m_src.is_valid()) {
		m = storage->material_owner.getornull(m_src);

		if (!m->shader || !m->shader->valid) {
			m = NULL;
		}
	}

	if (!m) {
		m = storage->material_owner.getptr(default_material);
	}

	ERR_FAIL_COND(!m);

	_add_geometry_with_material(p_geometry, p_instance, p_owner, m, p_shadow);

	while (m->next_pass.is_valid()) {
		m = storage->material_owner.getornull(m->next_pass);
		if (!m)
			break;
		_add_geometry_with_material(p_geometry, p_instance, p_owner, m, p_shadow);
	}
}

void RasterizerSceneGLES3::_add_geometry_with_material(RasterizerStorageGLES3::Geometry *p_geometry, InstanceBase *p_instance, RasterizerStorageGLES3::GeometryOwner *p_owner, RasterizerStorageGLES3::Material *p_material, bool p_shadow) {

	bool has_base_alpha = (p_material->shader->spatial.uses_alpha && !p_material->shader->spatial.uses_alpha_scissor) || p_material->shader->spatial.uses_screen_texture || p_material->shader->spatial.unshaded;
	bool has_blend_alpha = p_material->shader->spatial.blend_mode != RasterizerStorageGLES3::Shader::Spatial::BLEND_MODE_MIX || p_material->shader->spatial.ontop;
	bool has_alpha = has_base_alpha || has_blend_alpha;
	bool shadow = false;

	bool mirror = p_instance->mirror;

	if (p_material->shader->spatial.cull_mode == RasterizerStorageGLES3::Shader::Spatial::CULL_MODE_FRONT) {
		mirror = !mirror;
	}

	if (p_material->shader->spatial.uses_sss) {
		state.used_sss = true;
	}

	if (p_material->shader->spatial.uses_screen_texture) {
		state.used_screen_texture = true;
	}

	if (p_shadow) {

		if (has_blend_alpha || (has_base_alpha && p_material->shader->spatial.depth_draw_mode != RasterizerStorageGLES3::Shader::Spatial::DEPTH_DRAW_ALPHA_PREPASS))
			return; //bye

		if (!p_material->shader->spatial.uses_alpha_scissor && !p_material->shader->spatial.writes_modelview_or_projection && !p_material->shader->spatial.uses_vertex && !p_material->shader->spatial.uses_discard && p_material->shader->spatial.depth_draw_mode != RasterizerStorageGLES3::Shader::Spatial::DEPTH_DRAW_ALPHA_PREPASS) {
			//shader does not use discard and does not write a vertex position, use generic material
			if (p_instance->cast_shadows == VS::SHADOW_CASTING_SETTING_DOUBLE_SIDED)
				p_material = storage->material_owner.getptr(default_material_twosided);
			else
				p_material = storage->material_owner.getptr(default_material);
		}

		has_alpha = false;
	}

	RenderList::Element *e = has_alpha ? render_list.add_alpha_element() : render_list.add_element();

	if (!e)
		return;

	e->geometry = p_geometry;
	e->material = p_material;
	e->instance = p_instance;
	e->owner = p_owner;
	e->sort_key = 0;

	if (e->geometry->last_pass != render_pass) {
		e->geometry->last_pass = render_pass;
		e->geometry->index = current_geometry_index++;
	}

	if (!p_shadow && directional_light && (directional_light->light_ptr->cull_mask & e->instance->layer_mask) == 0) {
		e->sort_key |= SORT_KEY_NO_DIRECTIONAL_FLAG;
	}

	e->sort_key |= uint64_t(e->geometry->index) << RenderList::SORT_KEY_GEOMETRY_INDEX_SHIFT;
	e->sort_key |= uint64_t(e->instance->base_type) << RenderList::SORT_KEY_GEOMETRY_TYPE_SHIFT;

	if (!p_shadow) {

		if (e->material->last_pass != render_pass) {
			e->material->last_pass = render_pass;
			e->material->index = current_material_index++;
		}

		e->sort_key |= uint64_t(e->material->index) << RenderList::SORT_KEY_MATERIAL_INDEX_SHIFT;
		e->sort_key |= uint64_t(e->instance->depth_layer) << RenderList::SORT_KEY_DEPTH_LAYER_SHIFT;

		if (!has_blend_alpha && has_alpha && p_material->shader->spatial.depth_draw_mode == RasterizerStorageGLES3::Shader::Spatial::DEPTH_DRAW_ALPHA_PREPASS) {

			//if nothing exists, add this element as opaque too
			RenderList::Element *oe = render_list.add_element();

			if (!oe)
				return;

			copymem(oe, e, sizeof(RenderList::Element));
		}

		if (e->instance->gi_probe_instances.size()) {
			e->sort_key |= SORT_KEY_GI_PROBES_FLAG;
		}
	}

	/*
	if (e->geometry->type==RasterizerStorageGLES3::Geometry::GEOMETRY_MULTISURFACE)
		e->sort_flags|=RenderList::SORT_FLAG_INSTANCING;
	*/

	if (mirror) {
		e->sort_key |= RenderList::SORT_KEY_MIRROR_FLAG;
	}

	//e->light_type=0xFF; // no lights!

	if (shadow || p_material->shader->spatial.unshaded || state.debug_draw == VS::VIEWPORT_DEBUG_DRAW_UNSHADED) {

		e->sort_key |= SORT_KEY_UNSHADED_FLAG;
	}

	if (!shadow && (p_material->shader->spatial.uses_vertex_lighting || storage->config.force_vertex_shading)) {

		e->sort_key |= SORT_KEY_VERTEX_LIT_FLAG;
	}
}

void RasterizerSceneGLES3::_draw_sky(RasterizerStorageGLES3::Sky *p_sky, const CameraMatrix &p_projection, const Transform &p_transform, bool p_vflip, float p_scale, float p_energy) {

	if (!p_sky)
		return;

	RasterizerStorageGLES3::Texture *tex = storage->texture_owner.getornull(p_sky->panorama);

	ERR_FAIL_COND(!tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(tex->target, tex->tex_id);

	if (storage->config.srgb_decode_supported && tex->srgb && !tex->using_srgb) {

		glTexParameteri(tex->target, _TEXTURE_SRGB_DECODE_EXT, _DECODE_EXT);
		tex->using_srgb = true;
#ifdef TOOLS_ENABLED
		if (!(tex->flags & VS::TEXTURE_FLAG_CONVERT_TO_LINEAR)) {
			tex->flags |= VS::TEXTURE_FLAG_CONVERT_TO_LINEAR;
			//notify that texture must be set to linear beforehand, so it works in other platforms when exported
		}
#endif
	}

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDepthFunc(GL_LEQUAL);
	glColorMask(1, 1, 1, 1);

	float flip_sign = p_vflip ? -1 : 1;

	Vector3 vertices[8] = {
		Vector3(-1, -1 * flip_sign, 1),
		Vector3(0, 1, 0),
		Vector3(1, -1 * flip_sign, 1),
		Vector3(1, 1, 0),
		Vector3(1, 1 * flip_sign, 1),
		Vector3(1, 0, 0),
		Vector3(-1, 1 * flip_sign, 1),
		Vector3(0, 0, 0)

	};

	//sky uv vectors
	float vw, vh, zn;
	p_projection.get_viewport_size(vw, vh);
	zn = p_projection.get_z_near();

	float scale = p_scale;

	for (int i = 0; i < 4; i++) {

		Vector3 uv = vertices[i * 2 + 1];
		uv.x = (uv.x * 2.0 - 1.0) * vw * scale;
		uv.y = -(uv.y * 2.0 - 1.0) * vh * scale;
		uv.z = -zn;
		vertices[i * 2 + 1] = p_transform.basis.xform(uv).normalized();
		vertices[i * 2 + 1].z = -vertices[i * 2 + 1].z;
	}

	glBindBuffer(GL_ARRAY_BUFFER, state.sky_verts);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vector3) * 8, vertices);
	glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind

	glBindVertexArray(state.sky_array);

	storage->shaders.copy.set_conditional(CopyShaderGLES3::USE_PANORAMA, true);
	storage->shaders.copy.set_conditional(CopyShaderGLES3::USE_MULTIPLIER, true);
	storage->shaders.copy.bind();
	storage->shaders.copy.set_uniform(CopyShaderGLES3::MULTIPLIER, p_energy);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glBindVertexArray(0);
	glColorMask(1, 1, 1, 1);

	storage->shaders.copy.set_conditional(CopyShaderGLES3::USE_MULTIPLIER, false);
	storage->shaders.copy.set_conditional(CopyShaderGLES3::USE_PANORAMA, false);
}

void RasterizerSceneGLES3::_setup_environment(Environment *env, const CameraMatrix &p_cam_projection, const Transform &p_cam_transform) {

	//store camera into ubo
	store_camera(p_cam_projection, state.ubo_data.projection_matrix);
	store_transform(p_cam_transform, state.ubo_data.camera_matrix);
	store_transform(p_cam_transform.affine_inverse(), state.ubo_data.camera_inverse_matrix);

	//time global variables
	state.ubo_data.time = storage->frame.time[0];

	state.ubo_data.z_far = p_cam_projection.get_z_far();
	//bg and ambient
	if (env) {
		state.ubo_data.bg_energy = env->bg_energy;
		state.ubo_data.ambient_energy = env->ambient_energy;
		Color linear_ambient_color = env->ambient_color.to_linear();
		state.ubo_data.ambient_light_color[0] = linear_ambient_color.r;
		state.ubo_data.ambient_light_color[1] = linear_ambient_color.g;
		state.ubo_data.ambient_light_color[2] = linear_ambient_color.b;
		state.ubo_data.ambient_light_color[3] = linear_ambient_color.a;

		Color bg_color;

		switch (env->bg_mode) {
			case VS::ENV_BG_CLEAR_COLOR: {
				bg_color = storage->frame.clear_request_color.to_linear();
			} break;
			case VS::ENV_BG_COLOR: {
				bg_color = env->bg_color.to_linear();
			} break;
			default: {
				bg_color = Color(0, 0, 0, 1);
			} break;
		}

		state.ubo_data.bg_color[0] = bg_color.r;
		state.ubo_data.bg_color[1] = bg_color.g;
		state.ubo_data.bg_color[2] = bg_color.b;
		state.ubo_data.bg_color[3] = bg_color.a;

		state.env_radiance_data.ambient_contribution = env->ambient_sky_contribution;
		state.ubo_data.ambient_occlusion_affect_light = env->ssao_light_affect;

		//fog

		Color linear_fog = env->fog_color.to_linear();
		state.ubo_data.fog_color_enabled[0] = linear_fog.r;
		state.ubo_data.fog_color_enabled[1] = linear_fog.g;
		state.ubo_data.fog_color_enabled[2] = linear_fog.b;
		state.ubo_data.fog_color_enabled[3] = env->fog_enabled ? 1.0 : 0.0;

		Color linear_sun = env->fog_sun_color.to_linear();
		state.ubo_data.fog_sun_color_amount[0] = linear_sun.r;
		state.ubo_data.fog_sun_color_amount[1] = linear_sun.g;
		state.ubo_data.fog_sun_color_amount[2] = linear_sun.b;
		state.ubo_data.fog_sun_color_amount[3] = env->fog_sun_amount;
		state.ubo_data.fog_depth_enabled = env->fog_depth_enabled;
		state.ubo_data.fog_depth_begin = env->fog_depth_begin;
		state.ubo_data.fog_depth_curve = env->fog_depth_curve;
		state.ubo_data.fog_transmit_enabled = env->fog_transmit_enabled;
		state.ubo_data.fog_transmit_curve = env->fog_transmit_curve;
		state.ubo_data.fog_height_enabled = env->fog_height_enabled;
		state.ubo_data.fog_height_min = env->fog_height_min;
		state.ubo_data.fog_height_max = env->fog_height_max;
		state.ubo_data.fog_height_curve = env->fog_height_curve;

	} else {
		state.ubo_data.bg_energy = 1.0;
		state.ubo_data.ambient_energy = 1.0;
		//use from clear color instead, since there is no ambient
		Color linear_ambient_color = storage->frame.clear_request_color.to_linear();
		state.ubo_data.ambient_light_color[0] = linear_ambient_color.r;
		state.ubo_data.ambient_light_color[1] = linear_ambient_color.g;
		state.ubo_data.ambient_light_color[2] = linear_ambient_color.b;
		state.ubo_data.ambient_light_color[3] = linear_ambient_color.a;

		state.ubo_data.bg_color[0] = linear_ambient_color.r;
		state.ubo_data.bg_color[1] = linear_ambient_color.g;
		state.ubo_data.bg_color[2] = linear_ambient_color.b;
		state.ubo_data.bg_color[3] = linear_ambient_color.a;

		state.env_radiance_data.ambient_contribution = 0;
		state.ubo_data.ambient_occlusion_affect_light = 0;

		state.ubo_data.fog_color_enabled[3] = 0.0;
	}

	{
		//directional shadow

		state.ubo_data.shadow_directional_pixel_size[0] = 1.0 / directional_shadow.size;
		state.ubo_data.shadow_directional_pixel_size[1] = 1.0 / directional_shadow.size;

		glActiveTexture(GL_TEXTURE0 + storage->config.max_texture_image_units - 4);
		glBindTexture(GL_TEXTURE_2D, directional_shadow.depth);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
	}

	glBindBuffer(GL_UNIFORM_BUFFER, state.scene_ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(State::SceneDataUBO), &state.ubo_data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	//fill up environment

	store_transform(p_cam_transform, state.env_radiance_data.transform);

	glBindBuffer(GL_UNIFORM_BUFFER, state.env_radiance_ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(State::EnvironmentRadianceUBO), &state.env_radiance_data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void RasterizerSceneGLES3::_setup_directional_light(int p_index, const Transform &p_camera_inverse_transform, bool p_use_shadows) {

	LightInstance *li = directional_lights[p_index];

	LightDataUBO ubo_data; //used for filling

	float sign = li->light_ptr->negative ? -1 : 1;

	Color linear_col = li->light_ptr->color.to_linear();
	ubo_data.light_color_energy[0] = linear_col.r * sign * li->light_ptr->param[VS::LIGHT_PARAM_ENERGY];
	ubo_data.light_color_energy[1] = linear_col.g * sign * li->light_ptr->param[VS::LIGHT_PARAM_ENERGY];
	ubo_data.light_color_energy[2] = linear_col.b * sign * li->light_ptr->param[VS::LIGHT_PARAM_ENERGY];
	ubo_data.light_color_energy[3] = 0;

	//omni, keep at 0
	ubo_data.light_pos_inv_radius[0] = 0.0;
	ubo_data.light_pos_inv_radius[1] = 0.0;
	ubo_data.light_pos_inv_radius[2] = 0.0;
	ubo_data.light_pos_inv_radius[3] = 0.0;

	Vector3 direction = p_camera_inverse_transform.basis.xform(li->transform.basis.xform(Vector3(0, 0, -1))).normalized();
	ubo_data.light_direction_attenuation[0] = direction.x;
	ubo_data.light_direction_attenuation[1] = direction.y;
	ubo_data.light_direction_attenuation[2] = direction.z;
	ubo_data.light_direction_attenuation[3] = 1.0;

	ubo_data.light_params[0] = 0;
	ubo_data.light_params[1] = li->light_ptr->param[VS::LIGHT_PARAM_SPECULAR];
	ubo_data.light_params[2] = 0;
	ubo_data.light_params[3] = 0;

	Color shadow_color = li->light_ptr->shadow_color.to_linear();
	ubo_data.light_shadow_color_contact[0] = shadow_color.r;
	ubo_data.light_shadow_color_contact[1] = shadow_color.g;
	ubo_data.light_shadow_color_contact[2] = shadow_color.b;
	ubo_data.light_shadow_color_contact[3] = li->light_ptr->param[VS::LIGHT_PARAM_CONTACT_SHADOW_SIZE];

	if (p_use_shadows && li->light_ptr->shadow) {

		int shadow_count = 0;

		switch (li->light_ptr->directional_shadow_mode) {
			case VS::LIGHT_DIRECTIONAL_SHADOW_ORTHOGONAL: {
				shadow_count = 1;
			} break;
			case VS::LIGHT_DIRECTIONAL_SHADOW_PARALLEL_2_SPLITS: {
				shadow_count = 2;
			} break;
			case VS::LIGHT_DIRECTIONAL_SHADOW_PARALLEL_4_SPLITS: {
				shadow_count = 4;
			} break;
		}

		for (int j = 0; j < shadow_count; j++) {

			uint32_t x = li->directional_rect.position.x;
			uint32_t y = li->directional_rect.position.y;
			uint32_t width = li->directional_rect.size.x;
			uint32_t height = li->directional_rect.size.y;

			if (li->light_ptr->directional_shadow_mode == VS::LIGHT_DIRECTIONAL_SHADOW_PARALLEL_4_SPLITS) {

				width /= 2;
				height /= 2;

				if (j == 0) {

				} else if (j == 1) {
					x += width;
				} else if (j == 2) {
					y += height;
				} else if (j == 3) {
					x += width;
					y += height;
				}

			} else if (li->light_ptr->directional_shadow_mode == VS::LIGHT_DIRECTIONAL_SHADOW_PARALLEL_2_SPLITS) {

				height /= 2;

				if (j == 0) {

				} else {
					y += height;
				}
			}

			ubo_data.shadow_split_offsets[j] = 1.0 / li->shadow_transform[j].split;

			Transform modelview = (p_camera_inverse_transform * li->shadow_transform[j].transform).inverse();

			CameraMatrix bias;
			bias.set_light_bias();
			CameraMatrix rectm;
			Rect2 atlas_rect = Rect2(float(x) / directional_shadow.size, float(y) / directional_shadow.size, float(width) / directional_shadow.size, float(height) / directional_shadow.size);
			rectm.set_light_atlas_rect(atlas_rect);

			CameraMatrix shadow_mtx = rectm * bias * li->shadow_transform[j].camera * modelview;

			store_camera(shadow_mtx, &ubo_data.shadow_matrix1[16 * j]);

			ubo_data.light_clamp[0] = atlas_rect.position.x;
			ubo_data.light_clamp[1] = atlas_rect.position.y;
			ubo_data.light_clamp[2] = atlas_rect.size.x;
			ubo_data.light_clamp[3] = atlas_rect.size.y;
		}
	}

	glBindBuffer(GL_UNIFORM_BUFFER, state.directional_ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightDataUBO), &ubo_data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	directional_light = li;

	glBindBufferBase(GL_UNIFORM_BUFFER, 3, state.directional_ubo);
}

void RasterizerSceneGLES3::_setup_lights(RID *p_light_cull_result, int p_light_cull_count, const Transform &p_camera_inverse_transform, const CameraMatrix &p_camera_projection, RID p_shadow_atlas) {

	state.omni_light_count = 0;
	state.spot_light_count = 0;
	state.directional_light_count = 0;

	directional_light = NULL;

	ShadowAtlas *shadow_atlas = shadow_atlas_owner.getornull(p_shadow_atlas);

	for (int i = 0; i < p_light_cull_count; i++) {

		ERR_BREAK(i >= RenderList::MAX_LIGHTS);

		LightInstance *li = light_instance_owner.getptr(p_light_cull_result[i]);

		LightDataUBO ubo_data; //used for filling

		switch (li->light_ptr->type) {

			case VS::LIGHT_DIRECTIONAL: {

				if (state.directional_light_count < RenderList::MAX_DIRECTIONAL_LIGHTS) {
					directional_lights[state.directional_light_count++] = li;
				}

			} break;
			case VS::LIGHT_OMNI: {

				float sign = li->light_ptr->negative ? -1 : 1;

				Color linear_col = li->light_ptr->color.to_linear();
				ubo_data.light_color_energy[0] = linear_col.r * sign * li->light_ptr->param[VS::LIGHT_PARAM_ENERGY];
				ubo_data.light_color_energy[1] = linear_col.g * sign * li->light_ptr->param[VS::LIGHT_PARAM_ENERGY];
				ubo_data.light_color_energy[2] = linear_col.b * sign * li->light_ptr->param[VS::LIGHT_PARAM_ENERGY];
				ubo_data.light_color_energy[3] = 0;

				Vector3 pos = p_camera_inverse_transform.xform(li->transform.origin);

				//directional, keep at 0
				ubo_data.light_pos_inv_radius[0] = pos.x;
				ubo_data.light_pos_inv_radius[1] = pos.y;
				ubo_data.light_pos_inv_radius[2] = pos.z;
				ubo_data.light_pos_inv_radius[3] = 1.0 / MAX(0.001, li->light_ptr->param[VS::LIGHT_PARAM_RANGE]);

				ubo_data.light_direction_attenuation[0] = 0;
				ubo_data.light_direction_attenuation[1] = 0;
				ubo_data.light_direction_attenuation[2] = 0;
				ubo_data.light_direction_attenuation[3] = li->light_ptr->param[VS::LIGHT_PARAM_ATTENUATION];

				ubo_data.light_params[0] = 0;
				ubo_data.light_params[1] = 0;
				ubo_data.light_params[2] = li->light_ptr->param[VS::LIGHT_PARAM_SPECULAR];
				ubo_data.light_params[3] = 0;

				Color shadow_color = li->light_ptr->shadow_color.to_linear();
				ubo_data.light_shadow_color_contact[0] = shadow_color.r;
				ubo_data.light_shadow_color_contact[1] = shadow_color.g;
				ubo_data.light_shadow_color_contact[2] = shadow_color.b;
				ubo_data.light_shadow_color_contact[3] = li->light_ptr->param[VS::LIGHT_PARAM_CONTACT_SHADOW_SIZE];

				if (li->light_ptr->shadow && shadow_atlas && shadow_atlas->shadow_owners.has(li->self)) {
					// fill in the shadow information

					uint32_t key = shadow_atlas->shadow_owners[li->self];

					uint32_t quadrant = (key >> ShadowAtlas::QUADRANT_SHIFT) & 0x3;
					uint32_t shadow = key & ShadowAtlas::SHADOW_INDEX_MASK;

					ERR_CONTINUE(shadow >= shadow_atlas->quadrants[quadrant].shadows.size());

					uint32_t atlas_size = shadow_atlas->size;
					uint32_t quadrant_size = atlas_size >> 1;

					uint32_t x = (quadrant & 1) * quadrant_size;
					uint32_t y = (quadrant >> 1) * quadrant_size;

					uint32_t shadow_size = (quadrant_size / shadow_atlas->quadrants[quadrant].subdivision);
					x += (shadow % shadow_atlas->quadrants[quadrant].subdivision) * shadow_size;
					y += (shadow / shadow_atlas->quadrants[quadrant].subdivision) * shadow_size;

					uint32_t width = shadow_size;
					uint32_t height = shadow_size;

					if (li->light_ptr->omni_shadow_detail == VS::LIGHT_OMNI_SHADOW_DETAIL_HORIZONTAL) {

						height /= 2;
					} else {
						width /= 2;
					}

					Transform proj = (p_camera_inverse_transform * li->transform).inverse();

					store_transform(proj, ubo_data.shadow_matrix1);

					ubo_data.light_params[3] = 1.0; //means it has shadow
					ubo_data.light_clamp[0] = float(x) / atlas_size;
					ubo_data.light_clamp[1] = float(y) / atlas_size;
					ubo_data.light_clamp[2] = float(width) / atlas_size;
					ubo_data.light_clamp[3] = float(height) / atlas_size;
				}

				li->light_index = state.omni_light_count;
				copymem(&state.omni_array_tmp[li->light_index * state.ubo_light_size], &ubo_data, state.ubo_light_size);
				state.omni_light_count++;

#if 0
				if (li->light_ptr->shadow_enabled) {
					li->shadow_projection[0] = Transform(camera_transform_inverse * li->transform).inverse();
					lights_use_shadow=true;
				}
#endif
			} break;
			case VS::LIGHT_SPOT: {

				float sign = li->light_ptr->negative ? -1 : 1;

				Color linear_col = li->light_ptr->color.to_linear();
				ubo_data.light_color_energy[0] = linear_col.r * sign * li->light_ptr->param[VS::LIGHT_PARAM_ENERGY];
				ubo_data.light_color_energy[1] = linear_col.g * sign * li->light_ptr->param[VS::LIGHT_PARAM_ENERGY];
				ubo_data.light_color_energy[2] = linear_col.b * sign * li->light_ptr->param[VS::LIGHT_PARAM_ENERGY];
				ubo_data.light_color_energy[3] = 0;

				Vector3 pos = p_camera_inverse_transform.xform(li->transform.origin);

				//directional, keep at 0
				ubo_data.light_pos_inv_radius[0] = pos.x;
				ubo_data.light_pos_inv_radius[1] = pos.y;
				ubo_data.light_pos_inv_radius[2] = pos.z;
				ubo_data.light_pos_inv_radius[3] = 1.0 / MAX(0.001, li->light_ptr->param[VS::LIGHT_PARAM_RANGE]);

				Vector3 direction = p_camera_inverse_transform.basis.xform(li->transform.basis.xform(Vector3(0, 0, -1))).normalized();
				ubo_data.light_direction_attenuation[0] = direction.x;
				ubo_data.light_direction_attenuation[1] = direction.y;
				ubo_data.light_direction_attenuation[2] = direction.z;
				ubo_data.light_direction_attenuation[3] = li->light_ptr->param[VS::LIGHT_PARAM_ATTENUATION];

				ubo_data.light_params[0] = li->light_ptr->param[VS::LIGHT_PARAM_SPOT_ATTENUATION];
				ubo_data.light_params[1] = Math::cos(Math::deg2rad(li->light_ptr->param[VS::LIGHT_PARAM_SPOT_ANGLE]));
				ubo_data.light_params[2] = li->light_ptr->param[VS::LIGHT_PARAM_SPECULAR];
				ubo_data.light_params[3] = 0;

				Color shadow_color = li->light_ptr->shadow_color.to_linear();
				ubo_data.light_shadow_color_contact[0] = shadow_color.r;
				ubo_data.light_shadow_color_contact[1] = shadow_color.g;
				ubo_data.light_shadow_color_contact[2] = shadow_color.b;
				ubo_data.light_shadow_color_contact[3] = li->light_ptr->param[VS::LIGHT_PARAM_CONTACT_SHADOW_SIZE];

				if (li->light_ptr->shadow && shadow_atlas && shadow_atlas->shadow_owners.has(li->self)) {
					// fill in the shadow information

					uint32_t key = shadow_atlas->shadow_owners[li->self];

					uint32_t quadrant = (key >> ShadowAtlas::QUADRANT_SHIFT) & 0x3;
					uint32_t shadow = key & ShadowAtlas::SHADOW_INDEX_MASK;

					ERR_CONTINUE(shadow >= shadow_atlas->quadrants[quadrant].shadows.size());

					uint32_t atlas_size = shadow_atlas->size;
					uint32_t quadrant_size = atlas_size >> 1;

					uint32_t x = (quadrant & 1) * quadrant_size;
					uint32_t y = (quadrant >> 1) * quadrant_size;

					uint32_t shadow_size = (quadrant_size / shadow_atlas->quadrants[quadrant].subdivision);
					x += (shadow % shadow_atlas->quadrants[quadrant].subdivision) * shadow_size;
					y += (shadow / shadow_atlas->quadrants[quadrant].subdivision) * shadow_size;

					uint32_t width = shadow_size;
					uint32_t height = shadow_size;

					Rect2 rect(float(x) / atlas_size, float(y) / atlas_size, float(width) / atlas_size, float(height) / atlas_size);

					ubo_data.light_params[3] = 1.0; //means it has shadow
					ubo_data.light_clamp[0] = rect.position.x;
					ubo_data.light_clamp[1] = rect.position.y;
					ubo_data.light_clamp[2] = rect.size.x;
					ubo_data.light_clamp[3] = rect.size.y;

					Transform modelview = (p_camera_inverse_transform * li->transform).inverse();

					CameraMatrix bias;
					bias.set_light_bias();
					CameraMatrix rectm;
					rectm.set_light_atlas_rect(rect);

					CameraMatrix shadow_mtx = rectm * bias * li->shadow_transform[0].camera * modelview;

					store_camera(shadow_mtx, ubo_data.shadow_matrix1);
				}

				li->light_index = state.spot_light_count;
				copymem(&state.spot_array_tmp[li->light_index * state.ubo_light_size], &ubo_data, state.ubo_light_size);
				state.spot_light_count++;

#if 0
				if (li->light_ptr->shadow_enabled) {
					CameraMatrix bias;
					bias.set_light_bias();
					Transform modelview=Transform(camera_transform_inverse * li->transform).inverse();
					li->shadow_projection[0] = bias * li->projection * modelview;
					lights_use_shadow=true;
				}
#endif
			} break;
		}

		li->last_pass = render_pass;

		//update UBO for forward rendering, blit to texture for clustered
	}

	if (state.omni_light_count) {

		glBindBuffer(GL_UNIFORM_BUFFER, state.omni_array_ubo);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, state.omni_light_count * state.ubo_light_size, state.omni_array_tmp);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	glBindBufferBase(GL_UNIFORM_BUFFER, 4, state.omni_array_ubo);

	if (state.spot_light_count) {

		glBindBuffer(GL_UNIFORM_BUFFER, state.spot_array_ubo);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, state.spot_light_count * state.ubo_light_size, state.spot_array_tmp);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	glBindBufferBase(GL_UNIFORM_BUFFER, 5, state.spot_array_ubo);
}

void RasterizerSceneGLES3::_setup_reflections(RID *p_reflection_probe_cull_result, int p_reflection_probe_cull_count, const Transform &p_camera_inverse_transform, const CameraMatrix &p_camera_projection, RID p_reflection_atlas, Environment *p_env) {

	state.reflection_probe_count = 0;

	for (int i = 0; i < p_reflection_probe_cull_count; i++) {

		ReflectionProbeInstance *rpi = reflection_probe_instance_owner.getornull(p_reflection_probe_cull_result[i]);
		ERR_CONTINUE(!rpi);

		ReflectionAtlas *reflection_atlas = reflection_atlas_owner.getornull(p_reflection_atlas);
		ERR_CONTINUE(!reflection_atlas);

		ERR_CONTINUE(rpi->reflection_atlas_index < 0);

		if (state.reflection_probe_count >= state.max_ubo_reflections)
			break;

		rpi->last_pass = render_pass;

		ReflectionProbeDataUBO reflection_ubo;

		reflection_ubo.box_extents[0] = rpi->probe_ptr->extents.x;
		reflection_ubo.box_extents[1] = rpi->probe_ptr->extents.y;
		reflection_ubo.box_extents[2] = rpi->probe_ptr->extents.z;
		reflection_ubo.box_extents[3] = 0;

		reflection_ubo.box_ofs[0] = rpi->probe_ptr->origin_offset.x;
		reflection_ubo.box_ofs[1] = rpi->probe_ptr->origin_offset.y;
		reflection_ubo.box_ofs[2] = rpi->probe_ptr->origin_offset.z;
		reflection_ubo.box_ofs[3] = 0;

		reflection_ubo.params[0] = rpi->probe_ptr->intensity;
		reflection_ubo.params[1] = 0;
		reflection_ubo.params[2] = rpi->probe_ptr->interior ? 1.0 : 0.0;
		reflection_ubo.params[3] = rpi->probe_ptr->box_projection ? 1.0 : 0.0;

		if (rpi->probe_ptr->interior) {
			Color ambient_linear = rpi->probe_ptr->interior_ambient.to_linear();
			reflection_ubo.ambient[0] = ambient_linear.r * rpi->probe_ptr->interior_ambient_energy;
			reflection_ubo.ambient[1] = ambient_linear.g * rpi->probe_ptr->interior_ambient_energy;
			reflection_ubo.ambient[2] = ambient_linear.b * rpi->probe_ptr->interior_ambient_energy;
			reflection_ubo.ambient[3] = rpi->probe_ptr->interior_ambient_probe_contrib;
		} else {
			Color ambient_linear;
			float contrib = 0;
			if (p_env) {
				ambient_linear = p_env->ambient_color.to_linear();
				ambient_linear.r *= p_env->ambient_energy;
				ambient_linear.g *= p_env->ambient_energy;
				ambient_linear.b *= p_env->ambient_energy;
				contrib = p_env->ambient_sky_contribution;
			}

			reflection_ubo.ambient[0] = ambient_linear.r;
			reflection_ubo.ambient[1] = ambient_linear.g;
			reflection_ubo.ambient[2] = ambient_linear.b;
			reflection_ubo.ambient[3] = 0;
		}

		int cell_size = reflection_atlas->size / reflection_atlas->subdiv;
		int x = (rpi->reflection_atlas_index % reflection_atlas->subdiv) * cell_size;
		int y = (rpi->reflection_atlas_index / reflection_atlas->subdiv) * cell_size;
		int width = cell_size;
		int height = cell_size;

		reflection_ubo.atlas_clamp[0] = float(x) / reflection_atlas->size;
		reflection_ubo.atlas_clamp[1] = float(y) / reflection_atlas->size;
		reflection_ubo.atlas_clamp[2] = float(width) / reflection_atlas->size;
		reflection_ubo.atlas_clamp[3] = float(height) / reflection_atlas->size;

		Transform proj = (p_camera_inverse_transform * rpi->transform).inverse();
		store_transform(proj, reflection_ubo.local_matrix);

		rpi->reflection_index = state.reflection_probe_count;
		copymem(&state.reflection_array_tmp[rpi->reflection_index * sizeof(ReflectionProbeDataUBO)], &reflection_ubo, sizeof(ReflectionProbeDataUBO));
		state.reflection_probe_count++;
	}

	if (state.reflection_probe_count) {

		glBindBuffer(GL_UNIFORM_BUFFER, state.reflection_array_ubo);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, state.reflection_probe_count * sizeof(ReflectionProbeDataUBO), state.reflection_array_tmp);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	glBindBufferBase(GL_UNIFORM_BUFFER, 6, state.reflection_array_ubo);
}

void RasterizerSceneGLES3::_copy_screen(bool p_invalidate_color, bool p_invalidate_depth) {

#ifndef GLES_OVER_GL
	if (p_invalidate_color) {

		GLenum attachments[2] = {
			GL_COLOR_ATTACHMENT0,
			GL_DEPTH_STENCIL_ATTACHMENT
		};

		glInvalidateFramebuffer(GL_FRAMEBUFFER, p_invalidate_depth ? 2 : 1, attachments);
	}
#endif

	glBindVertexArray(storage->resources.quadie_array);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindVertexArray(0);
}

void RasterizerSceneGLES3::_copy_to_front_buffer(Environment *env) {

	//copy to front buffer
	glBindFramebuffer(GL_FRAMEBUFFER, storage->frame.current_rt->fbo);

	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDepthFunc(GL_LEQUAL);
	glColorMask(1, 1, 1, 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->buffers.diffuse);

	storage->shaders.copy.set_conditional(CopyShaderGLES3::DISABLE_ALPHA, true);

	if (!env) {
		//no environment, simply convert from linear to srgb
		storage->shaders.copy.set_conditional(CopyShaderGLES3::LINEAR_TO_SRGB, true);
	} else {
		/* FIXME: Why are both statements equal? */
		storage->shaders.copy.set_conditional(CopyShaderGLES3::LINEAR_TO_SRGB, true);
	}

	storage->shaders.copy.bind();

	_copy_screen();

	//turn off everything used
	storage->shaders.copy.set_conditional(CopyShaderGLES3::LINEAR_TO_SRGB, false);
	storage->shaders.copy.set_conditional(CopyShaderGLES3::DISABLE_ALPHA, false);
}

void RasterizerSceneGLES3::_copy_texture_to_front_buffer(GLuint p_texture) {

	//copy to front buffer
	glBindFramebuffer(GL_FRAMEBUFFER, storage->frame.current_rt->fbo);

	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDepthFunc(GL_LEQUAL);
	glColorMask(1, 1, 1, 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, p_texture);

	glViewport(0, 0, storage->frame.current_rt->width * 0.5, storage->frame.current_rt->height * 0.5);

	storage->shaders.copy.set_conditional(CopyShaderGLES3::DISABLE_ALPHA, true);
	storage->shaders.copy.bind();

	_copy_screen();

	//turn off everything used
	storage->shaders.copy.set_conditional(CopyShaderGLES3::LINEAR_TO_SRGB, false);
	storage->shaders.copy.set_conditional(CopyShaderGLES3::DISABLE_ALPHA, false);
}

void RasterizerSceneGLES3::_fill_render_list(InstanceBase **p_cull_result, int p_cull_count, bool p_shadow) {

	current_geometry_index = 0;
	current_material_index = 0;
	state.used_sss = false;
	state.used_screen_texture = false;

	//fill list

	for (int i = 0; i < p_cull_count; i++) {

		InstanceBase *inst = p_cull_result[i];
		switch (inst->base_type) {

			case VS::INSTANCE_MESH: {

				RasterizerStorageGLES3::Mesh *mesh = storage->mesh_owner.getptr(inst->base);
				ERR_CONTINUE(!mesh);

				int ssize = mesh->surfaces.size();

				for (int i = 0; i < ssize; i++) {

					int mat_idx = inst->materials[i].is_valid() ? i : -1;
					RasterizerStorageGLES3::Surface *s = mesh->surfaces[i];
					_add_geometry(s, inst, NULL, mat_idx, p_shadow);
				}

				//mesh->last_pass=frame;

			} break;
			case VS::INSTANCE_MULTIMESH: {

				RasterizerStorageGLES3::MultiMesh *multi_mesh = storage->multimesh_owner.getptr(inst->base);
				ERR_CONTINUE(!multi_mesh);

				if (multi_mesh->size == 0 || multi_mesh->visible_instances == 0)
					continue;

				RasterizerStorageGLES3::Mesh *mesh = storage->mesh_owner.getptr(multi_mesh->mesh);
				if (!mesh)
					continue; //mesh not assigned

				int ssize = mesh->surfaces.size();

				for (int i = 0; i < ssize; i++) {

					RasterizerStorageGLES3::Surface *s = mesh->surfaces[i];
					_add_geometry(s, inst, multi_mesh, -1, p_shadow);
				}

			} break;
			case VS::INSTANCE_IMMEDIATE: {

				RasterizerStorageGLES3::Immediate *immediate = storage->immediate_owner.getptr(inst->base);
				ERR_CONTINUE(!immediate);

				_add_geometry(immediate, inst, NULL, -1, p_shadow);

			} break;
			case VS::INSTANCE_PARTICLES: {

				RasterizerStorageGLES3::Particles *particles = storage->particles_owner.getptr(inst->base);
				ERR_CONTINUE(!particles);

				for (int i = 0; i < particles->draw_passes.size(); i++) {

					RID pmesh = particles->draw_passes[i];
					if (!pmesh.is_valid())
						continue;
					RasterizerStorageGLES3::Mesh *mesh = storage->mesh_owner.get(pmesh);
					if (!mesh)
						continue; //mesh not assigned

					int ssize = mesh->surfaces.size();

					for (int j = 0; j < ssize; j++) {

						RasterizerStorageGLES3::Surface *s = mesh->surfaces[j];
						_add_geometry(s, inst, particles, -1, p_shadow);
					}
				}

			} break;
		}
	}
}

void RasterizerSceneGLES3::_blur_effect_buffer() {

	//blur diffuse into effect mipmaps using separatable convolution
	//storage->shaders.copy.set_conditional(CopyShaderGLES3::GAUSSIAN_HORIZONTAL,true);
	for (int i = 0; i < storage->frame.current_rt->effects.mip_maps[1].sizes.size(); i++) {

		int vp_w = storage->frame.current_rt->effects.mip_maps[1].sizes[i].width;
		int vp_h = storage->frame.current_rt->effects.mip_maps[1].sizes[i].height;
		glViewport(0, 0, vp_w, vp_h);
		//horizontal pass
		state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::GAUSSIAN_HORIZONTAL, true);
		state.effect_blur_shader.bind();
		state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::PIXEL_SIZE, Vector2(1.0 / vp_w, 1.0 / vp_h));
		state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::LOD, float(i));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->effects.mip_maps[0].color); //previous level, since mipmaps[0] starts one level bigger
		glBindFramebuffer(GL_FRAMEBUFFER, storage->frame.current_rt->effects.mip_maps[1].sizes[i].fbo);
		_copy_screen(true);
		state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::GAUSSIAN_HORIZONTAL, false);

		//vertical pass
		state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::GAUSSIAN_VERTICAL, true);
		state.effect_blur_shader.bind();
		state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::PIXEL_SIZE, Vector2(1.0 / vp_w, 1.0 / vp_h));
		state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::LOD, float(i));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->effects.mip_maps[1].color);
		glBindFramebuffer(GL_FRAMEBUFFER, storage->frame.current_rt->effects.mip_maps[0].sizes[i + 1].fbo); //next level, since mipmaps[0] starts one level bigger
		_copy_screen(true);
		state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::GAUSSIAN_VERTICAL, false);
	}
}

void RasterizerSceneGLES3::_render_mrts(Environment *env, const CameraMatrix &p_cam_projection) {

	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	if (env->ssao_enabled) {
		//copy diffuse to front buffer
		glBindFramebuffer(GL_READ_FRAMEBUFFER, storage->frame.current_rt->buffers.fbo);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, storage->frame.current_rt->fbo);
		glBlitFramebuffer(0, 0, storage->frame.current_rt->width, storage->frame.current_rt->height, 0, 0, storage->frame.current_rt->width, storage->frame.current_rt->height, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		//copy from depth, convert to linear
		GLint ss[2];
		ss[0] = storage->frame.current_rt->width;
		ss[1] = storage->frame.current_rt->height;

		for (int i = 0; i < storage->frame.current_rt->effects.ssao.depth_mipmap_fbos.size(); i++) {

			state.ssao_minify_shader.set_conditional(SsaoMinifyShaderGLES3::MINIFY_START, i == 0);
			state.ssao_minify_shader.bind();
			state.ssao_minify_shader.set_uniform(SsaoMinifyShaderGLES3::CAMERA_Z_FAR, p_cam_projection.get_z_far());
			state.ssao_minify_shader.set_uniform(SsaoMinifyShaderGLES3::CAMERA_Z_NEAR, p_cam_projection.get_z_near());
			state.ssao_minify_shader.set_uniform(SsaoMinifyShaderGLES3::SOURCE_MIPMAP, MAX(0, i - 1));
			glUniform2iv(state.ssao_minify_shader.get_uniform(SsaoMinifyShaderGLES3::FROM_SIZE), 1, ss);
			ss[0] >>= 1;
			ss[1] >>= 1;

			glActiveTexture(GL_TEXTURE0);
			if (i == 0) {
				glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->depth);
			} else {
				glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->effects.ssao.linear_depth);
			}

			glBindFramebuffer(GL_FRAMEBUFFER, storage->frame.current_rt->effects.ssao.depth_mipmap_fbos[i]); //copy to front first
			glViewport(0, 0, ss[0], ss[1]);

			_copy_screen(true);
		}
		ss[0] = storage->frame.current_rt->width;
		ss[1] = storage->frame.current_rt->height;

		glViewport(0, 0, ss[0], ss[1]);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_GREATER);
		// do SSAO!
		state.ssao_shader.set_conditional(SsaoShaderGLES3::ENABLE_RADIUS2, env->ssao_radius2 > 0.001);
		state.ssao_shader.bind();
		state.ssao_shader.set_uniform(SsaoShaderGLES3::CAMERA_Z_FAR, p_cam_projection.get_z_far());
		state.ssao_shader.set_uniform(SsaoShaderGLES3::CAMERA_Z_NEAR, p_cam_projection.get_z_near());
		glUniform2iv(state.ssao_shader.get_uniform(SsaoShaderGLES3::SCREEN_SIZE), 1, ss);
		float radius = env->ssao_radius;
		state.ssao_shader.set_uniform(SsaoShaderGLES3::RADIUS, radius);
		float intensity = env->ssao_intensity;
		state.ssao_shader.set_uniform(SsaoShaderGLES3::INTENSITY_DIV_R6, intensity / pow(radius, 6.0f));

		if (env->ssao_radius2 > 0.001) {

			float radius2 = env->ssao_radius2;
			state.ssao_shader.set_uniform(SsaoShaderGLES3::RADIUS2, radius2);
			float intensity2 = env->ssao_intensity2;
			state.ssao_shader.set_uniform(SsaoShaderGLES3::INTENSITY_DIV_R62, intensity2 / pow(radius2, 6.0f));
		}

		float proj_info[4] = {
			-2.0f / (ss[0] * p_cam_projection.matrix[0][0]),
			-2.0f / (ss[1] * p_cam_projection.matrix[1][1]),
			(1.0f - p_cam_projection.matrix[0][2]) / p_cam_projection.matrix[0][0],
			(1.0f + p_cam_projection.matrix[1][2]) / p_cam_projection.matrix[1][1]
		};

		glUniform4fv(state.ssao_shader.get_uniform(SsaoShaderGLES3::PROJ_INFO), 1, proj_info);
		float pixels_per_meter = float(p_cam_projection.get_pixels_per_meter(ss[0]));

		state.ssao_shader.set_uniform(SsaoShaderGLES3::PROJ_SCALE, pixels_per_meter);
		state.ssao_shader.set_uniform(SsaoShaderGLES3::BIAS, env->ssao_bias);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->depth);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->effects.ssao.linear_depth);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->buffers.effect);

		glBindFramebuffer(GL_FRAMEBUFFER, storage->frame.current_rt->effects.ssao.blur_fbo[0]); //copy to front first
		Color white(1, 1, 1, 1);
		glClearBufferfv(GL_COLOR, 0, white.components); // specular

		_copy_screen(true);

		//do the batm, i mean blur

		state.ssao_blur_shader.bind();

		if (env->ssao_filter) {
			for (int i = 0; i < 2; i++) {

				state.ssao_blur_shader.set_uniform(SsaoBlurShaderGLES3::CAMERA_Z_FAR, p_cam_projection.get_z_far());
				state.ssao_blur_shader.set_uniform(SsaoBlurShaderGLES3::CAMERA_Z_NEAR, p_cam_projection.get_z_near());
				GLint axis[2] = { i, 1 - i };
				glUniform2iv(state.ssao_blur_shader.get_uniform(SsaoBlurShaderGLES3::AXIS), 1, axis);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->effects.ssao.blur_red[i]);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->depth);
				glBindFramebuffer(GL_FRAMEBUFFER, storage->frame.current_rt->effects.ssao.blur_fbo[1 - i]);
				if (i == 0) {
					glClearBufferfv(GL_COLOR, 0, white.components); // specular
				}
				_copy_screen(true);
			}
		}

		glDisable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		// just copy diffuse while applying SSAO

		state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::SSAO_MERGE, true);
		state.effect_blur_shader.bind();
		state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::SSAO_COLOR, env->ssao_color);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->color); //previous level, since mipmaps[0] starts one level bigger
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->effects.ssao.blur_red[0]); //previous level, since mipmaps[0] starts one level bigger
		glBindFramebuffer(GL_FRAMEBUFFER, storage->frame.current_rt->effects.mip_maps[0].sizes[0].fbo); // copy to base level
		_copy_screen(true);
		state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::SSAO_MERGE, false);

	} else {

		//copy diffuse to effect buffer
		glBindFramebuffer(GL_READ_FRAMEBUFFER, storage->frame.current_rt->buffers.fbo);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, storage->frame.current_rt->effects.mip_maps[0].sizes[0].fbo);
		glBlitFramebuffer(0, 0, storage->frame.current_rt->width, storage->frame.current_rt->height, 0, 0, storage->frame.current_rt->width, storage->frame.current_rt->height, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}

	if (state.used_sss) { //sss enabled
		//copy diffuse while performing sss

		//copy normal and roughness to effect buffer
		glBindFramebuffer(GL_READ_FRAMEBUFFER, storage->frame.current_rt->buffers.fbo);
		glReadBuffer(GL_COLOR_ATTACHMENT3);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, storage->frame.current_rt->effects.ssao.blur_fbo[0]);
		glBlitFramebuffer(0, 0, storage->frame.current_rt->width, storage->frame.current_rt->height, 0, 0, storage->frame.current_rt->width, storage->frame.current_rt->height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

		state.sss_shader.set_conditional(SubsurfScatteringShaderGLES3::USE_11_SAMPLES, subsurface_scatter_quality == SSS_QUALITY_LOW);
		state.sss_shader.set_conditional(SubsurfScatteringShaderGLES3::USE_17_SAMPLES, subsurface_scatter_quality == SSS_QUALITY_MEDIUM);
		state.sss_shader.set_conditional(SubsurfScatteringShaderGLES3::USE_25_SAMPLES, subsurface_scatter_quality == SSS_QUALITY_HIGH);
		state.sss_shader.set_conditional(SubsurfScatteringShaderGLES3::ENABLE_FOLLOW_SURFACE, subsurface_scatter_follow_surface);
		state.sss_shader.bind();
		state.sss_shader.set_uniform(SubsurfScatteringShaderGLES3::MAX_RADIUS, subsurface_scatter_size);
		state.sss_shader.set_uniform(SubsurfScatteringShaderGLES3::FOVY, p_cam_projection.get_fov());
		state.sss_shader.set_uniform(SubsurfScatteringShaderGLES3::CAMERA_Z_NEAR, p_cam_projection.get_z_near());
		state.sss_shader.set_uniform(SubsurfScatteringShaderGLES3::CAMERA_Z_FAR, p_cam_projection.get_z_far());
		state.sss_shader.set_uniform(SubsurfScatteringShaderGLES3::DIR, Vector2(1, 0));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->effects.mip_maps[0].color);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //disable filter (fixes bugs on AMD)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->effects.ssao.blur_red[0]);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->depth);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

		glBindFramebuffer(GL_FRAMEBUFFER, storage->frame.current_rt->fbo); //copy to front first

		_copy_screen(true);

		glActiveTexture(GL_TEXTURE0);

		glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->color);
		state.sss_shader.set_uniform(SubsurfScatteringShaderGLES3::DIR, Vector2(0, 1));
		glBindFramebuffer(GL_FRAMEBUFFER, storage->frame.current_rt->effects.mip_maps[0].sizes[0].fbo); // copy to base level
		_copy_screen(true);

		glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->effects.mip_maps[0].color); //restore filter
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}

	if (env->ssr_enabled) {

		//copy normal and roughness to effect buffer
		glBindFramebuffer(GL_READ_FRAMEBUFFER, storage->frame.current_rt->buffers.fbo);
		glReadBuffer(GL_COLOR_ATTACHMENT2);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, storage->frame.current_rt->buffers.effect_fbo);
		glBlitFramebuffer(0, 0, storage->frame.current_rt->width, storage->frame.current_rt->height, 0, 0, storage->frame.current_rt->width, storage->frame.current_rt->height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		//blur diffuse into effect mipmaps using separatable convolution
		//storage->shaders.copy.set_conditional(CopyShaderGLES3::GAUSSIAN_HORIZONTAL,true);
		_blur_effect_buffer();

		//perform SSR

		state.ssr_shader.set_conditional(ScreenSpaceReflectionShaderGLES3::REFLECT_ROUGHNESS, env->ssr_roughness);

		state.ssr_shader.bind();

		int ssr_w = storage->frame.current_rt->effects.mip_maps[1].sizes[0].width;
		int ssr_h = storage->frame.current_rt->effects.mip_maps[1].sizes[0].height;

		state.ssr_shader.set_uniform(ScreenSpaceReflectionShaderGLES3::PIXEL_SIZE, Vector2(1.0 / (ssr_w * 0.5), 1.0 / (ssr_h * 0.5)));
		state.ssr_shader.set_uniform(ScreenSpaceReflectionShaderGLES3::CAMERA_Z_NEAR, p_cam_projection.get_z_near());
		state.ssr_shader.set_uniform(ScreenSpaceReflectionShaderGLES3::CAMERA_Z_FAR, p_cam_projection.get_z_far());
		state.ssr_shader.set_uniform(ScreenSpaceReflectionShaderGLES3::PROJECTION, p_cam_projection);
		state.ssr_shader.set_uniform(ScreenSpaceReflectionShaderGLES3::INVERSE_PROJECTION, p_cam_projection.inverse());
		state.ssr_shader.set_uniform(ScreenSpaceReflectionShaderGLES3::VIEWPORT_SIZE, Size2(ssr_w, ssr_h));
		//state.ssr_shader.set_uniform(ScreenSpaceReflectionShaderGLES3::FRAME_INDEX,int(render_pass));
		state.ssr_shader.set_uniform(ScreenSpaceReflectionShaderGLES3::FILTER_MIPMAP_LEVELS, float(storage->frame.current_rt->effects.mip_maps[0].sizes.size()));
		state.ssr_shader.set_uniform(ScreenSpaceReflectionShaderGLES3::NUM_STEPS, env->ssr_max_steps);
		state.ssr_shader.set_uniform(ScreenSpaceReflectionShaderGLES3::DEPTH_TOLERANCE, env->ssr_depth_tolerance);
		state.ssr_shader.set_uniform(ScreenSpaceReflectionShaderGLES3::DISTANCE_FADE, env->ssr_fade_out);
		state.ssr_shader.set_uniform(ScreenSpaceReflectionShaderGLES3::CURVE_FADE_IN, env->ssr_fade_in);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->effects.mip_maps[0].color);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->buffers.effect);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->depth);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

		glBindFramebuffer(GL_FRAMEBUFFER, storage->frame.current_rt->effects.mip_maps[1].sizes[0].fbo);
		glViewport(0, 0, ssr_w, ssr_h);

		_copy_screen(true);
		glViewport(0, 0, storage->frame.current_rt->width, storage->frame.current_rt->height);
	}

	glBindFramebuffer(GL_READ_FRAMEBUFFER, storage->frame.current_rt->buffers.fbo);
	glReadBuffer(GL_COLOR_ATTACHMENT1);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, storage->frame.current_rt->fbo);
	//glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glBlitFramebuffer(0, 0, storage->frame.current_rt->width, storage->frame.current_rt->height, 0, 0, storage->frame.current_rt->width, storage->frame.current_rt->height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glReadBuffer(GL_COLOR_ATTACHMENT0);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	//copy reflection over diffuse, resolving SSR if needed
	state.resolve_shader.set_conditional(ResolveShaderGLES3::USE_SSR, env->ssr_enabled);
	state.resolve_shader.bind();
	state.resolve_shader.set_uniform(ResolveShaderGLES3::PIXEL_SIZE, Vector2(1.0 / storage->frame.current_rt->width, 1.0 / storage->frame.current_rt->height));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->color);
	if (env->ssr_enabled) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->effects.mip_maps[1].color);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, storage->frame.current_rt->effects.mip_maps[0].sizes[0].fbo);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE); //use additive to accumulate one over the other

	_copy_screen(true);

	glDisable(GL_BLEND); //end additive

	if (state.used_screen_texture) {
		_blur_effect_buffer();
		//restored framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, storage->frame.current_rt->effects.mip_maps[0].sizes[0].fbo);
		glViewport(0, 0, storage->frame.current_rt->width, storage->frame.current_rt->height);
	}

	state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::SIMPLE_COPY, true);
	state.effect_blur_shader.bind();
	state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::LOD, float(0));

	{
		GLuint db = GL_COLOR_ATTACHMENT0;
		glDrawBuffers(1, &db);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, storage->frame.current_rt->buffers.fbo);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->effects.mip_maps[0].color);

	_copy_screen(true);

	state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::SIMPLE_COPY, false);
}

void RasterizerSceneGLES3::_post_process(Environment *env, const CameraMatrix &p_cam_projection) {

	//copy to front buffer

	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDepthFunc(GL_LEQUAL);
	glColorMask(1, 1, 1, 1);

	//turn off everything used

	//copy specular to front buffer
	//copy diffuse to effect buffer

	if (storage->frame.current_rt->buffers.active) {
		//transfer to effect buffer if using buffers, also resolve MSAA
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, storage->frame.current_rt->buffers.fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, storage->frame.current_rt->effects.mip_maps[0].sizes[0].fbo);
		glBlitFramebuffer(0, 0, storage->frame.current_rt->width, storage->frame.current_rt->height, 0, 0, storage->frame.current_rt->width, storage->frame.current_rt->height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}

	if (!env || storage->frame.current_rt->flags[RasterizerStorage::RENDER_TARGET_TRANSPARENT]) {
		//no environment or transparent render, simply return and convert to SRGB
		glBindFramebuffer(GL_FRAMEBUFFER, storage->frame.current_rt->fbo);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->effects.mip_maps[0].color);
		storage->shaders.copy.set_conditional(CopyShaderGLES3::LINEAR_TO_SRGB, true);
		storage->shaders.copy.set_conditional(CopyShaderGLES3::V_FLIP, storage->frame.current_rt->flags[RasterizerStorage::RENDER_TARGET_VFLIP]);
		storage->shaders.copy.set_conditional(CopyShaderGLES3::DISABLE_ALPHA, !storage->frame.current_rt->flags[RasterizerStorage::RENDER_TARGET_TRANSPARENT]);
		storage->shaders.copy.bind();

		_copy_screen(true);

		storage->shaders.copy.set_conditional(CopyShaderGLES3::LINEAR_TO_SRGB, false);
		storage->shaders.copy.set_conditional(CopyShaderGLES3::DISABLE_ALPHA, false); //compute luminance
		storage->shaders.copy.set_conditional(CopyShaderGLES3::V_FLIP, false);

		return;
	}

	//order of operation
	//1) DOF Blur (first blur, then copy to buffer applying the blur)
	//2) Motion Blur
	//3) Bloom
	//4) Tonemap
	//5) Adjustments

	GLuint composite_from = storage->frame.current_rt->effects.mip_maps[0].color;

	if (env->dof_blur_far_enabled) {

		//blur diffuse into effect mipmaps using separatable convolution
		//storage->shaders.copy.set_conditional(CopyShaderGLES3::GAUSSIAN_HORIZONTAL,true);

		int vp_h = storage->frame.current_rt->height;
		int vp_w = storage->frame.current_rt->width;

		state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::DOF_FAR_BLUR, true);
		state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::DOF_QUALITY_LOW, env->dof_blur_far_quality == VS::ENV_DOF_BLUR_QUALITY_LOW);
		state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::DOF_QUALITY_MEDIUM, env->dof_blur_far_quality == VS::ENV_DOF_BLUR_QUALITY_MEDIUM);
		state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::DOF_QUALITY_HIGH, env->dof_blur_far_quality == VS::ENV_DOF_BLUR_QUALITY_HIGH);

		state.effect_blur_shader.bind();
		int qsteps[3] = { 4, 10, 20 };

		float radius = (env->dof_blur_far_amount * env->dof_blur_far_amount) / qsteps[env->dof_blur_far_quality];

		state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::DOF_BEGIN, env->dof_blur_far_distance);
		state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::DOF_END, env->dof_blur_far_distance + env->dof_blur_far_transition);
		state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::DOF_DIR, Vector2(1, 0));
		state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::DOF_RADIUS, radius);
		state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::PIXEL_SIZE, Vector2(1.0 / vp_w, 1.0 / vp_h));
		state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::CAMERA_Z_NEAR, p_cam_projection.get_z_near());
		state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::CAMERA_Z_FAR, p_cam_projection.get_z_far());

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->depth);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, composite_from);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glBindFramebuffer(GL_FRAMEBUFFER, storage->frame.current_rt->fbo); //copy to front first

		_copy_screen(true);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->color);
		state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::DOF_DIR, Vector2(0, 1));
		glBindFramebuffer(GL_FRAMEBUFFER, storage->frame.current_rt->effects.mip_maps[0].sizes[0].fbo); // copy to base level
		_copy_screen();

		state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::DOF_FAR_BLUR, false);
		state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::DOF_QUALITY_LOW, false);
		state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::DOF_QUALITY_MEDIUM, false);
		state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::DOF_QUALITY_HIGH, false);

		composite_from = storage->frame.current_rt->effects.mip_maps[0].color;
	}

	if (env->dof_blur_near_enabled) {

		//blur diffuse into effect mipmaps using separatable convolution
		//storage->shaders.copy.set_conditional(CopyShaderGLES3::GAUSSIAN_HORIZONTAL,true);

		int vp_h = storage->frame.current_rt->height;
		int vp_w = storage->frame.current_rt->width;

		state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::DOF_NEAR_BLUR, true);
		state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::DOF_NEAR_FIRST_TAP, true);

		state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::DOF_QUALITY_LOW, env->dof_blur_near_quality == VS::ENV_DOF_BLUR_QUALITY_LOW);
		state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::DOF_QUALITY_MEDIUM, env->dof_blur_near_quality == VS::ENV_DOF_BLUR_QUALITY_MEDIUM);
		state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::DOF_QUALITY_HIGH, env->dof_blur_near_quality == VS::ENV_DOF_BLUR_QUALITY_HIGH);

		state.effect_blur_shader.bind();
		int qsteps[3] = { 4, 10, 20 };

		float radius = (env->dof_blur_near_amount * env->dof_blur_near_amount) / qsteps[env->dof_blur_near_quality];

		state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::DOF_BEGIN, env->dof_blur_near_distance);
		state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::DOF_END, env->dof_blur_near_distance - env->dof_blur_near_transition);
		state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::DOF_DIR, Vector2(1, 0));
		state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::DOF_RADIUS, radius);
		state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::PIXEL_SIZE, Vector2(1.0 / vp_w, 1.0 / vp_h));
		state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::CAMERA_Z_NEAR, p_cam_projection.get_z_near());
		state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::CAMERA_Z_FAR, p_cam_projection.get_z_far());

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->depth);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, composite_from);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glBindFramebuffer(GL_FRAMEBUFFER, storage->frame.current_rt->fbo); //copy to front first

		_copy_screen();
		//manually do the blend if this is the first operation resolving from the diffuse buffer
		state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::DOF_NEAR_BLUR_MERGE, composite_from == storage->frame.current_rt->buffers.diffuse);
		state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::DOF_NEAR_FIRST_TAP, false);
		state.effect_blur_shader.bind();

		state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::DOF_BEGIN, env->dof_blur_near_distance);
		state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::DOF_END, env->dof_blur_near_distance - env->dof_blur_near_transition);
		state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::DOF_DIR, Vector2(0, 1));
		state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::DOF_RADIUS, radius);
		state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::PIXEL_SIZE, Vector2(1.0 / vp_w, 1.0 / vp_h));
		state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::CAMERA_Z_NEAR, p_cam_projection.get_z_near());
		state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::CAMERA_Z_FAR, p_cam_projection.get_z_far());

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->color);

		glBindFramebuffer(GL_FRAMEBUFFER, storage->frame.current_rt->effects.mip_maps[0].sizes[0].fbo); // copy to base level

		if (composite_from != storage->frame.current_rt->buffers.diffuse) {

			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		} else {
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->buffers.diffuse);
		}

		_copy_screen(true);

		if (composite_from != storage->frame.current_rt->buffers.diffuse) {

			glDisable(GL_BLEND);
		}

		state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::DOF_NEAR_BLUR, false);
		state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::DOF_NEAR_FIRST_TAP, false);
		state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::DOF_NEAR_BLUR_MERGE, false);
		state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::DOF_QUALITY_LOW, false);
		state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::DOF_QUALITY_MEDIUM, false);
		state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::DOF_QUALITY_HIGH, false);

		composite_from = storage->frame.current_rt->effects.mip_maps[0].color;
	}

	if (env->dof_blur_near_enabled || env->dof_blur_far_enabled) {
		//these needed to disable filtering, reenamble
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->effects.mip_maps[0].color);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	if (env->auto_exposure) {

		//compute auto exposure
		//first step, copy from image to luminance buffer
		state.exposure_shader.set_conditional(ExposureShaderGLES3::EXPOSURE_BEGIN, true);
		state.exposure_shader.bind();
		int ss[2] = {
			storage->frame.current_rt->width,
			storage->frame.current_rt->height,
		};
		int ds[2] = {
			exposure_shrink_size,
			exposure_shrink_size,
		};

		glUniform2iv(state.exposure_shader.get_uniform(ExposureShaderGLES3::SOURCE_RENDER_SIZE), 1, ss);
		glUniform2iv(state.exposure_shader.get_uniform(ExposureShaderGLES3::TARGET_SIZE), 1, ds);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, composite_from);

		glBindFramebuffer(GL_FRAMEBUFFER, exposure_shrink[0].fbo);
		glViewport(0, 0, exposure_shrink_size, exposure_shrink_size);

		_copy_screen(true);

		//second step, shrink to 2x2 pixels
		state.exposure_shader.set_conditional(ExposureShaderGLES3::EXPOSURE_BEGIN, false);
		state.exposure_shader.bind();
		//shrink from second to previous to last level

		int s_size = exposure_shrink_size / 3;
		for (int i = 1; i < exposure_shrink.size() - 1; i++) {

			glBindFramebuffer(GL_FRAMEBUFFER, exposure_shrink[i].fbo);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, exposure_shrink[i - 1].color);

			_copy_screen();

			glViewport(0, 0, s_size, s_size);

			s_size /= 3;
		}
		//third step, shrink to 1x1 pixel taking in consideration the previous exposure
		state.exposure_shader.set_conditional(ExposureShaderGLES3::EXPOSURE_END, true);

		uint64_t tick = OS::get_singleton()->get_ticks_usec();
		uint64_t tick_diff = storage->frame.current_rt->last_exposure_tick == 0 ? 0 : tick - storage->frame.current_rt->last_exposure_tick;
		storage->frame.current_rt->last_exposure_tick = tick;

		if (tick_diff == 0 || tick_diff > 1000000) {
			state.exposure_shader.set_conditional(ExposureShaderGLES3::EXPOSURE_FORCE_SET, true);
		}

		state.exposure_shader.bind();

		glBindFramebuffer(GL_FRAMEBUFFER, exposure_shrink[exposure_shrink.size() - 1].fbo);
		glViewport(0, 0, 1, 1);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, exposure_shrink[exposure_shrink.size() - 2].color);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->exposure.color); //read from previous

		state.exposure_shader.set_uniform(ExposureShaderGLES3::EXPOSURE_ADJUST, env->auto_exposure_speed * (tick_diff / 1000000.0));
		state.exposure_shader.set_uniform(ExposureShaderGLES3::MAX_LUMINANCE, env->auto_exposure_max);
		state.exposure_shader.set_uniform(ExposureShaderGLES3::MIN_LUMINANCE, env->auto_exposure_min);

		_copy_screen(true);

		state.exposure_shader.set_conditional(ExposureShaderGLES3::EXPOSURE_FORCE_SET, false);
		state.exposure_shader.set_conditional(ExposureShaderGLES3::EXPOSURE_END, false);

		//last step, swap with the framebuffer exposure, so the right exposure is kept int he framebuffer
		SWAP(exposure_shrink[exposure_shrink.size() - 1].fbo, storage->frame.current_rt->exposure.fbo);
		SWAP(exposure_shrink[exposure_shrink.size() - 1].color, storage->frame.current_rt->exposure.color);

		glViewport(0, 0, storage->frame.current_rt->width, storage->frame.current_rt->height);
	}

	int max_glow_level = -1;
	int glow_mask = 0;

	if (env->glow_enabled) {

		for (int i = 0; i < VS::MAX_GLOW_LEVELS; i++) {
			if (env->glow_levels & (1 << i)) {

				if (i >= storage->frame.current_rt->effects.mip_maps[1].sizes.size()) {
					max_glow_level = storage->frame.current_rt->effects.mip_maps[1].sizes.size() - 1;
					glow_mask |= 1 << max_glow_level;

				} else {
					max_glow_level = i;
					glow_mask |= (1 << i);
				}
			}
		}

		//blur diffuse into effect mipmaps using separatable convolution
		//storage->shaders.copy.set_conditional(CopyShaderGLES3::GAUSSIAN_HORIZONTAL,true);

		for (int i = 0; i < (max_glow_level + 1); i++) {

			int vp_w = storage->frame.current_rt->effects.mip_maps[1].sizes[i].width;
			int vp_h = storage->frame.current_rt->effects.mip_maps[1].sizes[i].height;
			glViewport(0, 0, vp_w, vp_h);
			//horizontal pass
			if (i == 0) {
				state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::GLOW_FIRST_PASS, true);
				state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::GLOW_USE_AUTO_EXPOSURE, env->auto_exposure);
			}

			state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::GLOW_GAUSSIAN_HORIZONTAL, true);
			state.effect_blur_shader.bind();
			state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::PIXEL_SIZE, Vector2(1.0 / vp_w, 1.0 / vp_h));
			state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::LOD, float(i));
			state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::GLOW_STRENGTH, env->glow_strength);

			glActiveTexture(GL_TEXTURE0);
			if (i == 0) {
				glBindTexture(GL_TEXTURE_2D, composite_from);

				state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::EXPOSURE, env->tone_mapper_exposure);
				if (env->auto_exposure) {
					state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::AUTO_EXPOSURE_GREY, env->auto_exposure_grey);
				}

				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->exposure.color);

				state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::GLOW_BLOOM, env->glow_bloom);
				state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::GLOW_HDR_THRESHOLD, env->glow_hdr_bleed_threshold);
				state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::GLOW_HDR_SCALE, env->glow_hdr_bleed_scale);

			} else {
				glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->effects.mip_maps[0].color); //previous level, since mipmaps[0] starts one level bigger
			}
			glBindFramebuffer(GL_FRAMEBUFFER, storage->frame.current_rt->effects.mip_maps[1].sizes[i].fbo);
			_copy_screen(true);
			state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::GLOW_GAUSSIAN_HORIZONTAL, false);
			state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::GLOW_FIRST_PASS, false);
			state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::GLOW_USE_AUTO_EXPOSURE, false);

			//vertical pass
			state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::GLOW_GAUSSIAN_VERTICAL, true);
			state.effect_blur_shader.bind();
			state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::PIXEL_SIZE, Vector2(1.0 / vp_w, 1.0 / vp_h));
			state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::LOD, float(i));
			state.effect_blur_shader.set_uniform(EffectBlurShaderGLES3::GLOW_STRENGTH, env->glow_strength);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->effects.mip_maps[1].color);
			glBindFramebuffer(GL_FRAMEBUFFER, storage->frame.current_rt->effects.mip_maps[0].sizes[i + 1].fbo); //next level, since mipmaps[0] starts one level bigger
			_copy_screen();
			state.effect_blur_shader.set_conditional(EffectBlurShaderGLES3::GLOW_GAUSSIAN_VERTICAL, false);
		}

		glViewport(0, 0, storage->frame.current_rt->width, storage->frame.current_rt->height);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, storage->frame.current_rt->fbo);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, composite_from);

	state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_FILMIC_TONEMAPPER, env->tone_mapper == VS::ENV_TONE_MAPPER_FILMIC);
	state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_ACES_TONEMAPPER, env->tone_mapper == VS::ENV_TONE_MAPPER_ACES);
	state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_REINDHART_TONEMAPPER, env->tone_mapper == VS::ENV_TONE_MAPPER_REINHARDT);

	state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_AUTO_EXPOSURE, env->auto_exposure);
	state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_GLOW_FILTER_BICUBIC, env->glow_bicubic_upscale);

	if (max_glow_level >= 0) {

		for (int i = 0; i < (max_glow_level + 1); i++) {

			if (glow_mask & (1 << i)) {
				if (i == 0) {
					state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_GLOW_LEVEL1, true);
				}
				if (i == 1) {
					state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_GLOW_LEVEL2, true);
				}
				if (i == 2) {
					state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_GLOW_LEVEL3, true);
				}
				if (i == 3) {
					state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_GLOW_LEVEL4, true);
				}
				if (i == 4) {
					state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_GLOW_LEVEL5, true);
				}
				if (i == 5) {
					state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_GLOW_LEVEL6, true);
				}
				if (i == 6) {
					state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_GLOW_LEVEL7, true);
				}
			}
		}

		state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_GLOW_SCREEN, env->glow_blend_mode == VS::GLOW_BLEND_MODE_SCREEN);
		state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_GLOW_SOFTLIGHT, env->glow_blend_mode == VS::GLOW_BLEND_MODE_SOFTLIGHT);
		state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_GLOW_REPLACE, env->glow_blend_mode == VS::GLOW_BLEND_MODE_REPLACE);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->effects.mip_maps[0].color);
	}

	if (env->adjustments_enabled) {

		state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_BCS, true);
		RasterizerStorageGLES3::Texture *tex = storage->texture_owner.getornull(env->color_correction);
		if (tex) {
			state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_COLOR_CORRECTION, true);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(tex->target, tex->tex_id);
		}
	}

	state.tonemap_shader.set_conditional(TonemapShaderGLES3::V_FLIP, storage->frame.current_rt->flags[RasterizerStorage::RENDER_TARGET_VFLIP]);
	state.tonemap_shader.bind();

	state.tonemap_shader.set_uniform(TonemapShaderGLES3::EXPOSURE, env->tone_mapper_exposure);
	state.tonemap_shader.set_uniform(TonemapShaderGLES3::WHITE, env->tone_mapper_exposure_white);

	if (max_glow_level >= 0) {

		state.tonemap_shader.set_uniform(TonemapShaderGLES3::GLOW_INTENSITY, env->glow_intensity);
		int ss[2] = {
			storage->frame.current_rt->width,
			storage->frame.current_rt->height,
		};
		glUniform2iv(state.tonemap_shader.get_uniform(TonemapShaderGLES3::GLOW_TEXTURE_SIZE), 1, ss);
	}

	if (env->auto_exposure) {

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->exposure.color);
		state.tonemap_shader.set_uniform(TonemapShaderGLES3::AUTO_EXPOSURE_GREY, env->auto_exposure_grey);
	}

	if (env->adjustments_enabled) {

		state.tonemap_shader.set_uniform(TonemapShaderGLES3::BCS, Vector3(env->adjustments_brightness, env->adjustments_contrast, env->adjustments_saturation));
	}

	_copy_screen(true, true);

	//turn off everything used
	state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_AUTO_EXPOSURE, false);
	state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_FILMIC_TONEMAPPER, false);
	state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_ACES_TONEMAPPER, false);
	state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_REINDHART_TONEMAPPER, false);
	state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_GLOW_LEVEL1, false);
	state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_GLOW_LEVEL2, false);
	state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_GLOW_LEVEL3, false);
	state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_GLOW_LEVEL4, false);
	state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_GLOW_LEVEL5, false);
	state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_GLOW_LEVEL6, false);
	state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_GLOW_LEVEL7, false);
	state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_GLOW_REPLACE, false);
	state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_GLOW_SCREEN, false);
	state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_GLOW_SOFTLIGHT, false);
	state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_GLOW_FILTER_BICUBIC, false);
	state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_BCS, false);
	state.tonemap_shader.set_conditional(TonemapShaderGLES3::USE_COLOR_CORRECTION, false);
	state.tonemap_shader.set_conditional(TonemapShaderGLES3::V_FLIP, false);
}

void RasterizerSceneGLES3::render_scene(const Transform &p_cam_transform, const CameraMatrix &p_cam_projection, bool p_cam_ortogonal, InstanceBase **p_cull_result, int p_cull_count, RID *p_light_cull_result, int p_light_cull_count, RID *p_reflection_probe_cull_result, int p_reflection_probe_cull_count, RID p_environment, RID p_shadow_atlas, RID p_reflection_atlas, RID p_reflection_probe, int p_reflection_probe_pass) {

	//first of all, make a new render pass
	render_pass++;

	//fill up ubo

	storage->info.render.object_count += p_cull_count;

	Environment *env = environment_owner.getornull(p_environment);
	ShadowAtlas *shadow_atlas = shadow_atlas_owner.getornull(p_shadow_atlas);
	ReflectionAtlas *reflection_atlas = reflection_atlas_owner.getornull(p_reflection_atlas);

	if (shadow_atlas && shadow_atlas->size) {
		glActiveTexture(GL_TEXTURE0 + storage->config.max_texture_image_units - 5);
		glBindTexture(GL_TEXTURE_2D, shadow_atlas->depth);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
		state.ubo_data.shadow_atlas_pixel_size[0] = 1.0 / shadow_atlas->size;
		state.ubo_data.shadow_atlas_pixel_size[1] = 1.0 / shadow_atlas->size;
	}

	if (reflection_atlas && reflection_atlas->size) {
		glActiveTexture(GL_TEXTURE0 + storage->config.max_texture_image_units - 3);
		glBindTexture(GL_TEXTURE_2D, reflection_atlas->color);
	}

	if (p_reflection_probe.is_valid()) {
		state.ubo_data.reflection_multiplier = 0.0;
	} else {
		state.ubo_data.reflection_multiplier = 1.0;
	}

	state.ubo_data.subsurface_scatter_width = subsurface_scatter_size;

	state.ubo_data.z_offset = 0;
	state.ubo_data.z_slope_scale = 0;
	state.ubo_data.shadow_dual_paraboloid_render_side = 0;
	state.ubo_data.shadow_dual_paraboloid_render_zfar = 0;

	if (storage->frame.current_rt) {
		state.ubo_data.screen_pixel_size[0] = 1.0 / storage->frame.current_rt->width;
		state.ubo_data.screen_pixel_size[1] = 1.0 / storage->frame.current_rt->height;
	}

	_setup_environment(env, p_cam_projection, p_cam_transform);

	bool fb_cleared = false;

	glDepthFunc(GL_LEQUAL);

	state.used_contact_shadows = true;

	if (!storage->config.no_depth_prepass && storage->frame.current_rt && state.debug_draw != VS::VIEWPORT_DEBUG_DRAW_OVERDRAW) { //detect with state.used_contact_shadows too
		//pre z pass

		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_SCISSOR_TEST);
		glBindFramebuffer(GL_FRAMEBUFFER, storage->frame.current_rt->buffers.fbo);
		glDrawBuffers(0, NULL);

		glViewport(0, 0, storage->frame.current_rt->width, storage->frame.current_rt->height);

		glColorMask(0, 0, 0, 0);
		glClearDepth(1.0f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		render_list.clear();
		_fill_render_list(p_cull_result, p_cull_count, true);
		render_list.sort_by_depth(false);
		state.scene_shader.set_conditional(SceneShaderGLES3::RENDER_DEPTH, true);
		_render_list(render_list.elements, render_list.element_count, p_cam_transform, p_cam_projection, 0, false, false, true, false, false);
		state.scene_shader.set_conditional(SceneShaderGLES3::RENDER_DEPTH, false);

		glColorMask(1, 1, 1, 1);

		if (state.used_contact_shadows) {

			glBindFramebuffer(GL_READ_FRAMEBUFFER, storage->frame.current_rt->buffers.fbo);
			glReadBuffer(GL_COLOR_ATTACHMENT0);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, storage->frame.current_rt->fbo);
			glBlitFramebuffer(0, 0, storage->frame.current_rt->width, storage->frame.current_rt->height, 0, 0, storage->frame.current_rt->width, storage->frame.current_rt->height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			//bind depth for read
			glActiveTexture(GL_TEXTURE0 + storage->config.max_texture_image_units - 8);
			glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->depth);
		}

		fb_cleared = true;
		render_pass++;
		state.using_contact_shadows = true;
	} else {
		state.using_contact_shadows = false;
	}

	_setup_lights(p_light_cull_result, p_light_cull_count, p_cam_transform.affine_inverse(), p_cam_projection, p_shadow_atlas);
	_setup_reflections(p_reflection_probe_cull_result, p_reflection_probe_cull_count, p_cam_transform.affine_inverse(), p_cam_projection, p_reflection_atlas, env);

	render_list.clear();

	bool use_mrt = false;

	_fill_render_list(p_cull_result, p_cull_count, false);
	//

	glEnable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_SCISSOR_TEST);

	//rendering to a probe cubemap side
	ReflectionProbeInstance *probe = reflection_probe_instance_owner.getornull(p_reflection_probe);
	GLuint current_fbo;

	if (probe) {

		ReflectionAtlas *ref_atlas = reflection_atlas_owner.getptr(probe->atlas);
		ERR_FAIL_COND(!ref_atlas);

		int target_size = ref_atlas->size / ref_atlas->subdiv;

		int cubemap_index = reflection_cubemaps.size() - 1;

		for (int i = reflection_cubemaps.size() - 1; i >= 0; i--) {
			//find appropriate cubemap to render to
			if (reflection_cubemaps[i].size > target_size * 2)
				break;

			cubemap_index = i;
		}

		current_fbo = reflection_cubemaps[cubemap_index].fbo_id[p_reflection_probe_pass];
		use_mrt = false;
		state.scene_shader.set_conditional(SceneShaderGLES3::USE_MULTIPLE_RENDER_TARGETS, false);

		glViewport(0, 0, reflection_cubemaps[cubemap_index].size, reflection_cubemaps[cubemap_index].size);
		glBindFramebuffer(GL_FRAMEBUFFER, current_fbo);

	} else {

		use_mrt = env && (state.used_sss || env->ssao_enabled || env->ssr_enabled); //only enable MRT rendering if any of these is enabled
		//effects disabled and transparency also prevent using MRTs
		use_mrt = use_mrt && !storage->frame.current_rt->flags[RasterizerStorage::RENDER_TARGET_TRANSPARENT];
		use_mrt = use_mrt && !storage->frame.current_rt->flags[RasterizerStorage::RENDER_TARGET_NO_3D_EFFECTS];
		use_mrt = use_mrt && state.debug_draw != VS::VIEWPORT_DEBUG_DRAW_OVERDRAW;
		use_mrt = use_mrt && env && (env->bg_mode != VS::ENV_BG_KEEP && env->bg_mode != VS::ENV_BG_CANVAS);

		glViewport(0, 0, storage->frame.current_rt->width, storage->frame.current_rt->height);

		if (use_mrt) {

			current_fbo = storage->frame.current_rt->buffers.fbo;

			glBindFramebuffer(GL_FRAMEBUFFER, storage->frame.current_rt->buffers.fbo);
			state.scene_shader.set_conditional(SceneShaderGLES3::USE_MULTIPLE_RENDER_TARGETS, true);

			Vector<GLenum> draw_buffers;
			draw_buffers.push_back(GL_COLOR_ATTACHMENT0);
			draw_buffers.push_back(GL_COLOR_ATTACHMENT1);
			draw_buffers.push_back(GL_COLOR_ATTACHMENT2);
			if (state.used_sss) {
				draw_buffers.push_back(GL_COLOR_ATTACHMENT3);
			}
			glDrawBuffers(draw_buffers.size(), draw_buffers.ptr());

			Color black(0, 0, 0, 0);
			glClearBufferfv(GL_COLOR, 1, black.components); // specular
			glClearBufferfv(GL_COLOR, 2, black.components); // normal metal rough
			if (state.used_sss) {
				glClearBufferfv(GL_COLOR, 3, black.components); // normal metal rough
			}

		} else {

			if (storage->frame.current_rt->buffers.active) {
				current_fbo = storage->frame.current_rt->buffers.fbo;
			} else {
				current_fbo = storage->frame.current_rt->effects.mip_maps[0].sizes[0].fbo;
			}

			glBindFramebuffer(GL_FRAMEBUFFER, current_fbo);
			state.scene_shader.set_conditional(SceneShaderGLES3::USE_MULTIPLE_RENDER_TARGETS, false);

			Vector<GLenum> draw_buffers;
			draw_buffers.push_back(GL_COLOR_ATTACHMENT0);
			glDrawBuffers(draw_buffers.size(), draw_buffers.ptr());
		}
	}

	if (!fb_cleared) {
		glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0, 0);
	}

	Color clear_color(0, 0, 0, 0);

	RasterizerStorageGLES3::Sky *sky = NULL;
	GLuint env_radiance_tex = 0;

	if (state.debug_draw == VS::VIEWPORT_DEBUG_DRAW_OVERDRAW) {
		clear_color = Color(0, 0, 0, 0);
		storage->frame.clear_request = false;
	} else if (!probe && storage->frame.current_rt->flags[RasterizerStorage::RENDER_TARGET_TRANSPARENT]) {
		clear_color = Color(0, 0, 0, 0);
		storage->frame.clear_request = false;

	} else if (!env || env->bg_mode == VS::ENV_BG_CLEAR_COLOR) {

		if (storage->frame.clear_request) {

			clear_color = storage->frame.clear_request_color.to_linear();
			storage->frame.clear_request = false;
		}

	} else if (env->bg_mode == VS::ENV_BG_CANVAS) {

		clear_color = env->bg_color.to_linear();
		storage->frame.clear_request = false;
	} else if (env->bg_mode == VS::ENV_BG_COLOR) {

		clear_color = env->bg_color.to_linear();
		storage->frame.clear_request = false;
	} else if (env->bg_mode == VS::ENV_BG_SKY) {

		sky = storage->sky_owner.getornull(env->sky);

		if (sky) {
			env_radiance_tex = sky->radiance;
		}
		storage->frame.clear_request = false;

	} else {
		storage->frame.clear_request = false;
	}

	if (!env || env->bg_mode != VS::ENV_BG_KEEP) {
		glClearBufferfv(GL_COLOR, 0, clear_color.components); // specular
	}

	if (env && env->bg_mode == VS::ENV_BG_CANVAS) {
		//copy canvas to 3d buffer and convert it to linear

		glDisable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->color);

		storage->shaders.copy.set_conditional(CopyShaderGLES3::DISABLE_ALPHA, true);

		storage->shaders.copy.set_conditional(CopyShaderGLES3::SRGB_TO_LINEAR, true);

		storage->shaders.copy.bind();

		_copy_screen(true, true);

		//turn off everything used
		storage->shaders.copy.set_conditional(CopyShaderGLES3::SRGB_TO_LINEAR, false);
		storage->shaders.copy.set_conditional(CopyShaderGLES3::DISABLE_ALPHA, false);

		//restore
		glEnable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
	}

	state.texscreen_copied = false;

	glBlendEquation(GL_FUNC_ADD);

	if (storage->frame.current_rt && storage->frame.current_rt->flags[RasterizerStorage::RENDER_TARGET_TRANSPARENT]) {
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
	} else {
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_BLEND);
	}

	render_list.sort_by_key(false);

	if (state.directional_light_count == 0) {
		directional_light = NULL;
		_render_list(render_list.elements, render_list.element_count, p_cam_transform, p_cam_projection, env_radiance_tex, false, false, false, false, shadow_atlas != NULL);
	} else {
		for (int i = 0; i < state.directional_light_count; i++) {
			directional_light = directional_lights[i];
			if (i > 0) {
				glEnable(GL_BLEND);
			}
			_setup_directional_light(i, p_cam_transform.affine_inverse(), shadow_atlas != NULL);
			_render_list(render_list.elements, render_list.element_count, p_cam_transform, p_cam_projection, env_radiance_tex, false, false, false, i > 0, shadow_atlas != NULL);
		}
	}

	state.scene_shader.set_conditional(SceneShaderGLES3::USE_MULTIPLE_RENDER_TARGETS, false);

	if (use_mrt) {
		GLenum gldb = GL_COLOR_ATTACHMENT0;
		glDrawBuffers(1, &gldb);
	}

	if (env && env->bg_mode == VS::ENV_BG_SKY && (!storage->frame.current_rt || (!storage->frame.current_rt->flags[RasterizerStorage::RENDER_TARGET_TRANSPARENT] && state.debug_draw != VS::VIEWPORT_DEBUG_DRAW_OVERDRAW))) {

		/*
		if (use_mrt) {
			glBindFramebuffer(GL_FRAMEBUFFER,storage->frame.current_rt->buffers.fbo); //switch to alpha fbo for sky, only diffuse/ambient matters
		*/

		_draw_sky(sky, p_cam_projection, p_cam_transform, false, env->sky_scale, env->bg_energy);
	}

	//_render_list_forward(&alpha_render_list,camera_transform,camera_transform_inverse,camera_projection,false,fragment_lighting,true);
	//glColorMask(1,1,1,1);

	//state.scene_shader.set_conditional( SceneShaderGLES3::USE_FOG,false);

	if (use_mrt) {
		_render_mrts(env, p_cam_projection);
	} else {
		//FIXME: check that this is possible to use
		if (storage->frame.current_rt && storage->frame.current_rt->buffers.active && state.used_screen_texture) {
			glBindFramebuffer(GL_READ_FRAMEBUFFER, storage->frame.current_rt->buffers.fbo);
			glReadBuffer(GL_COLOR_ATTACHMENT0);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, storage->frame.current_rt->effects.mip_maps[0].sizes[0].fbo);
			glBlitFramebuffer(0, 0, storage->frame.current_rt->width, storage->frame.current_rt->height, 0, 0, storage->frame.current_rt->width, storage->frame.current_rt->height, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			_blur_effect_buffer();
			//restored framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, storage->frame.current_rt->buffers.fbo);
			glViewport(0, 0, storage->frame.current_rt->width, storage->frame.current_rt->height);
		}
	}

	if (storage->frame.current_rt && state.used_screen_texture && storage->frame.current_rt->buffers.active) {
		glActiveTexture(GL_TEXTURE0 + storage->config.max_texture_image_units - 7);
		glBindTexture(GL_TEXTURE_2D, storage->frame.current_rt->effects.mip_maps[0].color);
	}

	glEnable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_SCISSOR_TEST);

	render_list.sort_by_reverse_depth(true);

	if (state.directional_light_count == 0) {
		directional_light = NULL;
		_render_list(&render_list.elements[render_list.max_elements - render_list.alpha_element_count], render_list.alpha_element_count, p_cam_transform, p_cam_projection, env_radiance_tex, false, true, false, false, shadow_atlas != NULL);
	} else {
		for (int i = 0; i < state.directional_light_count; i++) {
			directional_light = directional_lights[i];
			_setup_directional_light(i, p_cam_transform.affine_inverse(), shadow_atlas != NULL);
			_render_list(&render_list.elements[render_list.max_elements - render_list.alpha_element_count], render_list.alpha_element_count, p_cam_transform, p_cam_projection, env_radiance_tex, false, true, false, i > 0, shadow_atlas != NULL);
		}
	}

	if (probe) {
		//rendering a probe, do no more!
		return;
	}

	_post_process(env, p_cam_projection);

	if (false && shadow_atlas) {

		//_copy_texture_to_front_buffer(shadow_atlas->depth);
		storage->canvas->canvas_begin();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, shadow_atlas->depth);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
		storage->canvas->draw_generic_textured_rect(Rect2(0, 0, storage->frame.current_rt->width / 2, storage->frame.current_rt->height / 2), Rect2(0, 0, 1, 1));
	}

	if (false && storage->frame.current_rt) {

		//_copy_texture_to_front_buffer(shadow_atlas->depth);
		storage->canvas->canvas_begin();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, exposure_shrink[4].color);
		//glBindTexture(GL_TEXTURE_2D,storage->frame.current_rt->exposure.color);
		storage->canvas->draw_generic_textured_rect(Rect2(0, 0, storage->frame.current_rt->width / 16, storage->frame.current_rt->height / 16), Rect2(0, 0, 1, 1));
	}

	if (false && reflection_atlas && storage->frame.current_rt) {

		//_copy_texture_to_front_buffer(shadow_atlas->depth);
		storage->canvas->canvas_begin();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, reflection_atlas->color);
		storage->canvas->draw_generic_textured_rect(Rect2(0, 0, storage->frame.current_rt->width / 2, storage->frame.current_rt->height / 2), Rect2(0, 0, 1, 1));
	}

	if (false && directional_shadow.fbo) {

		//_copy_texture_to_front_buffer(shadow_atlas->depth);
		storage->canvas->canvas_begin();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, directional_shadow.depth);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
		storage->canvas->draw_generic_textured_rect(Rect2(0, 0, storage->frame.current_rt->width / 2, storage->frame.current_rt->height / 2), Rect2(0, 0, 1, 1));
	}

	if (false && env_radiance_tex) {

		//_copy_texture_to_front_buffer(shadow_atlas->depth);
		storage->canvas->canvas_begin();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, env_radiance_tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		storage->canvas->draw_generic_textured_rect(Rect2(0, 0, storage->frame.current_rt->width / 2, storage->frame.current_rt->height / 2), Rect2(0, 0, 1, 1));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	//disable all stuff
}

void RasterizerSceneGLES3::render_shadow(RID p_light, RID p_shadow_atlas, int p_pass, InstanceBase **p_cull_result, int p_cull_count) {

	render_pass++;

	directional_light = NULL;

	LightInstance *light_instance = light_instance_owner.getornull(p_light);
	ERR_FAIL_COND(!light_instance);
	RasterizerStorageGLES3::Light *light = storage->light_owner.getornull(light_instance->light);
	ERR_FAIL_COND(!light);

	uint32_t x, y, width, height, vp_height;

	float dp_direction = 0.0;
	float zfar = 0;
	bool flip_facing = false;
	int custom_vp_size = 0;
	GLuint fbo;
	int current_cubemap = -1;
	float bias = 0;
	float normal_bias = 0;

	state.using_contact_shadows = false;

	CameraMatrix light_projection;
	Transform light_transform;

	if (light->type == VS::LIGHT_DIRECTIONAL) {
		//set pssm stuff
		if (light_instance->last_scene_shadow_pass != scene_pass) {
			//assign rect if unassigned
			light_instance->light_directional_index = directional_shadow.current_light;
			light_instance->last_scene_shadow_pass = scene_pass;
			directional_shadow.current_light++;

			if (directional_shadow.light_count == 1) {
				light_instance->directional_rect = Rect2(0, 0, directional_shadow.size, directional_shadow.size);
			} else if (directional_shadow.light_count == 2) {
				light_instance->directional_rect = Rect2(0, 0, directional_shadow.size, directional_shadow.size / 2);
				if (light_instance->light_directional_index == 1) {
					light_instance->directional_rect.position.x += light_instance->directional_rect.size.x;
				}
			} else { //3 and 4
				light_instance->directional_rect = Rect2(0, 0, directional_shadow.size / 2, directional_shadow.size / 2);
				if (light_instance->light_directional_index & 1) {
					light_instance->directional_rect.position.x += light_instance->directional_rect.size.x;
				}
				if (light_instance->light_directional_index / 2) {
					light_instance->directional_rect.position.y += light_instance->directional_rect.size.y;
				}
			}
		}

		light_projection = light_instance->shadow_transform[p_pass].camera;
		light_transform = light_instance->shadow_transform[p_pass].transform;

		x = light_instance->directional_rect.position.x;
		y = light_instance->directional_rect.position.y;
		width = light_instance->directional_rect.size.x;
		height = light_instance->directional_rect.size.y;

		if (light->directional_shadow_mode == VS::LIGHT_DIRECTIONAL_SHADOW_PARALLEL_4_SPLITS) {

			width /= 2;
			height /= 2;

			if (p_pass == 0) {

			} else if (p_pass == 1) {
				x += width;
			} else if (p_pass == 2) {
				y += height;
			} else if (p_pass == 3) {
				x += width;
				y += height;
			}

		} else if (light->directional_shadow_mode == VS::LIGHT_DIRECTIONAL_SHADOW_PARALLEL_2_SPLITS) {

			height /= 2;

			if (p_pass == 0) {

			} else {
				y += height;
			}
		}

		zfar = light->param[VS::LIGHT_PARAM_RANGE];
		bias = light->param[VS::LIGHT_PARAM_SHADOW_BIAS] * light_instance->shadow_transform[p_pass].bias_scale;
		normal_bias = light->param[VS::LIGHT_PARAM_SHADOW_NORMAL_BIAS] * light_instance->shadow_transform[p_pass].bias_scale;
		fbo = directional_shadow.fbo;
		vp_height = directional_shadow.size;

	} else {
		//set from shadow atlas

		ShadowAtlas *shadow_atlas = shadow_atlas_owner.getornull(p_shadow_atlas);
		ERR_FAIL_COND(!shadow_atlas);
		ERR_FAIL_COND(!shadow_atlas->shadow_owners.has(p_light));

		fbo = shadow_atlas->fbo;
		vp_height = shadow_atlas->size;

		uint32_t key = shadow_atlas->shadow_owners[p_light];

		uint32_t quadrant = (key >> ShadowAtlas::QUADRANT_SHIFT) & 0x3;
		uint32_t shadow = key & ShadowAtlas::SHADOW_INDEX_MASK;

		ERR_FAIL_INDEX(shadow, shadow_atlas->quadrants[quadrant].shadows.size());

		uint32_t quadrant_size = shadow_atlas->size >> 1;

		x = (quadrant & 1) * quadrant_size;
		y = (quadrant >> 1) * quadrant_size;

		uint32_t shadow_size = (quadrant_size / shadow_atlas->quadrants[quadrant].subdivision);
		x += (shadow % shadow_atlas->quadrants[quadrant].subdivision) * shadow_size;
		y += (shadow / shadow_atlas->quadrants[quadrant].subdivision) * shadow_size;

		width = shadow_size;
		height = shadow_size;

		if (light->type == VS::LIGHT_OMNI) {

			if (light->omni_shadow_mode == VS::LIGHT_OMNI_SHADOW_CUBE) {

				int cubemap_index = shadow_cubemaps.size() - 1;

				for (int i = shadow_cubemaps.size() - 1; i >= 0; i--) {
					//find appropriate cubemap to render to
					if (shadow_cubemaps[i].size > shadow_size * 2)
						break;

					cubemap_index = i;
				}

				fbo = shadow_cubemaps[cubemap_index].fbo_id[p_pass];
				light_projection = light_instance->shadow_transform[0].camera;
				light_transform = light_instance->shadow_transform[0].transform;
				custom_vp_size = shadow_cubemaps[cubemap_index].size;
				zfar = light->param[VS::LIGHT_PARAM_RANGE];

				current_cubemap = cubemap_index;

			} else {

				light_projection = light_instance->shadow_transform[0].camera;
				light_transform = light_instance->shadow_transform[0].transform;

				if (light->omni_shadow_detail == VS::LIGHT_OMNI_SHADOW_DETAIL_HORIZONTAL) {

					height /= 2;
					y += p_pass * height;
				} else {
					width /= 2;
					x += p_pass * width;
				}

				dp_direction = p_pass == 0 ? 1.0 : -1.0;
				flip_facing = (p_pass == 1);
				zfar = light->param[VS::LIGHT_PARAM_RANGE];
				bias = light->param[VS::LIGHT_PARAM_SHADOW_BIAS];

				state.scene_shader.set_conditional(SceneShaderGLES3::RENDER_DEPTH_DUAL_PARABOLOID, true);
			}

		} else if (light->type == VS::LIGHT_SPOT) {

			light_projection = light_instance->shadow_transform[0].camera;
			light_transform = light_instance->shadow_transform[0].transform;

			dp_direction = 1.0;
			flip_facing = false;
			zfar = light->param[VS::LIGHT_PARAM_RANGE];
			bias = light->param[VS::LIGHT_PARAM_SHADOW_BIAS];
			normal_bias = light->param[VS::LIGHT_PARAM_SHADOW_NORMAL_BIAS];
		}
	}

	//todo hacer que se redibuje cuando corresponde

	render_list.clear();
	_fill_render_list(p_cull_result, p_cull_count, true);

	render_list.sort_by_depth(false); //shadow is front to back for performance

	glDisable(GL_BLEND);
	glDisable(GL_DITHER);
	glEnable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glDepthMask(true);
	glColorMask(0, 0, 0, 0);

	if (custom_vp_size) {
		glViewport(0, 0, custom_vp_size, custom_vp_size);
		glScissor(0, 0, custom_vp_size, custom_vp_size);

	} else {
		glViewport(x, y, width, height);
		glScissor(x, y, width, height);
	}

	glEnable(GL_SCISSOR_TEST);
	glClearDepth(1.0f);
	glClear(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);

	state.ubo_data.z_offset = bias;
	state.ubo_data.z_slope_scale = normal_bias;
	state.ubo_data.shadow_dual_paraboloid_render_side = dp_direction;
	state.ubo_data.shadow_dual_paraboloid_render_zfar = zfar;

	_setup_environment(NULL, light_projection, light_transform);

	state.scene_shader.set_conditional(SceneShaderGLES3::RENDER_DEPTH, true);

	_render_list(render_list.elements, render_list.element_count, light_transform, light_projection, 0, flip_facing, false, true, false, false);

	state.scene_shader.set_conditional(SceneShaderGLES3::RENDER_DEPTH, false);
	state.scene_shader.set_conditional(SceneShaderGLES3::RENDER_DEPTH_DUAL_PARABOLOID, false);

	if (light->type == VS::LIGHT_OMNI && light->omni_shadow_mode == VS::LIGHT_OMNI_SHADOW_CUBE && p_pass == 5) {
		//convert the chosen cubemap to dual paraboloid!

		ShadowAtlas *shadow_atlas = shadow_atlas_owner.getornull(p_shadow_atlas);

		glBindFramebuffer(GL_FRAMEBUFFER, shadow_atlas->fbo);
		state.cube_to_dp_shader.bind();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, shadow_cubemaps[current_cubemap].cubemap);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_NONE);
		glDisable(GL_CULL_FACE);

		for (int i = 0; i < 2; i++) {

			state.cube_to_dp_shader.set_uniform(CubeToDpShaderGLES3::Z_FLIP, i == 1);
			state.cube_to_dp_shader.set_uniform(CubeToDpShaderGLES3::Z_NEAR, light_projection.get_z_near());
			state.cube_to_dp_shader.set_uniform(CubeToDpShaderGLES3::Z_FAR, light_projection.get_z_far());
			state.cube_to_dp_shader.set_uniform(CubeToDpShaderGLES3::BIAS, light->param[VS::LIGHT_PARAM_SHADOW_BIAS]);

			uint32_t local_width = width, local_height = height;
			uint32_t local_x = x, local_y = y;
			if (light->omni_shadow_detail == VS::LIGHT_OMNI_SHADOW_DETAIL_HORIZONTAL) {

				local_height /= 2;
				local_y += i * local_height;
			} else {
				local_width /= 2;
				local_x += i * local_width;
			}

			glViewport(local_x, local_y, local_width, local_height);
			glScissor(local_x, local_y, local_width, local_height);
			glEnable(GL_SCISSOR_TEST);
			glClearDepth(1.0f);
			glClear(GL_DEPTH_BUFFER_BIT);
			glDisable(GL_SCISSOR_TEST);
			//glDisable(GL_DEPTH_TEST);
			glDisable(GL_BLEND);

			_copy_screen();
		}
	}

	glColorMask(1, 1, 1, 1);
}

void RasterizerSceneGLES3::set_scene_pass(uint64_t p_pass) {
	scene_pass = p_pass;
}

bool RasterizerSceneGLES3::free(RID p_rid) {

	if (light_instance_owner.owns(p_rid)) {

		LightInstance *light_instance = light_instance_owner.getptr(p_rid);

		//remove from shadow atlases..
		for (Set<RID>::Element *E = light_instance->shadow_atlases.front(); E; E = E->next()) {
			ShadowAtlas *shadow_atlas = shadow_atlas_owner.get(E->get());
			ERR_CONTINUE(!shadow_atlas->shadow_owners.has(p_rid));
			uint32_t key = shadow_atlas->shadow_owners[p_rid];
			uint32_t q = (key >> ShadowAtlas::QUADRANT_SHIFT) & 0x3;
			uint32_t s = key & ShadowAtlas::SHADOW_INDEX_MASK;

			shadow_atlas->quadrants[q].shadows[s].owner = RID();
			shadow_atlas->shadow_owners.erase(p_rid);
		}

		light_instance_owner.free(p_rid);
		memdelete(light_instance);

	} else if (shadow_atlas_owner.owns(p_rid)) {

		ShadowAtlas *shadow_atlas = shadow_atlas_owner.get(p_rid);
		shadow_atlas_set_size(p_rid, 0);
		shadow_atlas_owner.free(p_rid);
		memdelete(shadow_atlas);
	} else if (reflection_atlas_owner.owns(p_rid)) {

		ReflectionAtlas *reflection_atlas = reflection_atlas_owner.get(p_rid);
		reflection_atlas_set_size(p_rid, 0);
		reflection_atlas_owner.free(p_rid);
		memdelete(reflection_atlas);
	} else if (reflection_probe_instance_owner.owns(p_rid)) {

		ReflectionProbeInstance *reflection_instance = reflection_probe_instance_owner.get(p_rid);

		reflection_probe_release_atlas_index(p_rid);
		reflection_probe_instance_owner.free(p_rid);
		memdelete(reflection_instance);

	} else {
		return false;
	}

	return true;
}

void RasterizerSceneGLES3::set_debug_draw_mode(VS::ViewportDebugDraw p_debug_draw) {

	state.debug_draw = p_debug_draw;
}

void RasterizerSceneGLES3::initialize() {

	render_pass = 0;

	state.scene_shader.init();

	{
		//default material and shader

		default_shader = storage->shader_create();
		storage->shader_set_code(default_shader, "shader_type spatial;\n");
		default_material = storage->material_create();
		storage->material_set_shader(default_material, default_shader);

		default_shader_twosided = storage->shader_create();
		default_material_twosided = storage->material_create();
		storage->shader_set_code(default_shader_twosided, "shader_type spatial; render_mode cull_disabled;\n");
		storage->material_set_shader(default_material_twosided, default_shader_twosided);
	}

	{
		//default material and shader

		default_overdraw_shader = storage->shader_create();
		storage->shader_set_code(default_overdraw_shader, "shader_type spatial;\nrender_mode blend_add,unshaded;\n void fragment() { ALBEDO=vec3(0.4,0.8,0.8); ALPHA=0.2; }");
		default_overdraw_material = storage->material_create();
		storage->material_set_shader(default_overdraw_material, default_overdraw_shader);
	}

	glGenBuffers(1, &state.scene_ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, state.scene_ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(State::SceneDataUBO), &state.scene_ubo, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glGenBuffers(1, &state.env_radiance_ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, state.env_radiance_ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(State::EnvironmentRadianceUBO), &state.env_radiance_ubo, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	render_list.max_elements = GLOBAL_DEF("rendering/limits/rendering/max_renderable_elements", (int)RenderList::DEFAULT_MAX_ELEMENTS);
	if (render_list.max_elements > 1000000)
		render_list.max_elements = 1000000;
	if (render_list.max_elements < 1024)
		render_list.max_elements = 1024;

	{
		//quad buffers

		glGenBuffers(1, &state.sky_verts);
		glBindBuffer(GL_ARRAY_BUFFER, state.sky_verts);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * 8, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind

		glGenVertexArrays(1, &state.sky_array);
		glBindVertexArray(state.sky_array);
		glBindBuffer(GL_ARRAY_BUFFER, state.sky_verts);
		glVertexAttribPointer(VS::ARRAY_VERTEX, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3) * 2, 0);
		glEnableVertexAttribArray(VS::ARRAY_VERTEX);
		glVertexAttribPointer(VS::ARRAY_TEX_UV, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3) * 2, ((uint8_t *)NULL) + sizeof(Vector3));
		glEnableVertexAttribArray(VS::ARRAY_TEX_UV);
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind
	}
	render_list.init();
	state.cube_to_dp_shader.init();

	shadow_atlas_realloc_tolerance_msec = 500;

	int max_shadow_cubemap_sampler_size = 512;

	int cube_size = max_shadow_cubemap_sampler_size;

	glActiveTexture(GL_TEXTURE0);

	while (cube_size >= 32) {

		ShadowCubeMap cube;
		cube.size = cube_size;

		glGenTextures(1, &cube.cubemap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cube.cubemap);
		//gen cubemap first
		for (int i = 0; i < 6; i++) {

			glTexImage2D(_cube_side_enum[i], 0, GL_DEPTH_COMPONENT24, cube.size, cube.size, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// Remove artifact on the edges of the shadowmap
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		//gen renderbuffers second, because it needs a complete cubemap
		for (int i = 0; i < 6; i++) {

			glGenFramebuffers(1, &cube.fbo_id[i]);
			glBindFramebuffer(GL_FRAMEBUFFER, cube.fbo_id[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _cube_side_enum[i], cube.cubemap, 0);

			GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			ERR_CONTINUE(status != GL_FRAMEBUFFER_COMPLETE);
		}

		shadow_cubemaps.push_back(cube);

		cube_size >>= 1;
	}

	{
		//directional light shadow
		directional_shadow.light_count = 0;
		directional_shadow.size = nearest_power_of_2(GLOBAL_GET("rendering/quality/directional_shadow/size"));
		glGenFramebuffers(1, &directional_shadow.fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, directional_shadow.fbo);
		glGenTextures(1, &directional_shadow.depth);
		glBindTexture(GL_TEXTURE_2D, directional_shadow.depth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, directional_shadow.size, directional_shadow.size, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, directional_shadow.depth, 0);
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			ERR_PRINT("Directional shadow framebuffer status invalid");
		}
	}

	{
		//spot and omni ubos

		int max_ubo_size;
		glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &max_ubo_size);
		const int ubo_light_size = 160;
		state.ubo_light_size = ubo_light_size;
		state.max_ubo_lights = MIN(RenderList::MAX_LIGHTS, max_ubo_size / ubo_light_size);
		print_line("GLES3: max ubo light: " + itos(state.max_ubo_lights));

		state.spot_array_tmp = (uint8_t *)memalloc(ubo_light_size * state.max_ubo_lights);
		state.omni_array_tmp = (uint8_t *)memalloc(ubo_light_size * state.max_ubo_lights);

		glGenBuffers(1, &state.spot_array_ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, state.spot_array_ubo);
		glBufferData(GL_UNIFORM_BUFFER, ubo_light_size * state.max_ubo_lights, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glGenBuffers(1, &state.omni_array_ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, state.omni_array_ubo);
		glBufferData(GL_UNIFORM_BUFFER, ubo_light_size * state.max_ubo_lights, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glGenBuffers(1, &state.directional_ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, state.directional_ubo);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(LightDataUBO), NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		state.max_forward_lights_per_object = 8;

		state.scene_shader.add_custom_define("#define MAX_LIGHT_DATA_STRUCTS " + itos(state.max_ubo_lights) + "\n");
		state.scene_shader.add_custom_define("#define MAX_FORWARD_LIGHTS " + itos(state.max_forward_lights_per_object) + "\n");

		state.max_ubo_reflections = MIN(RenderList::MAX_REFLECTIONS, max_ubo_size / sizeof(ReflectionProbeDataUBO));
		print_line("GLES3: max ubo reflections: " + itos(state.max_ubo_reflections) + ", ubo size: " + itos(sizeof(ReflectionProbeDataUBO)));

		state.reflection_array_tmp = (uint8_t *)memalloc(sizeof(ReflectionProbeDataUBO) * state.max_ubo_reflections);

		glGenBuffers(1, &state.reflection_array_ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, state.reflection_array_ubo);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(ReflectionProbeDataUBO) * state.max_ubo_reflections, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		state.scene_shader.add_custom_define("#define MAX_REFLECTION_DATA_STRUCTS " + itos(state.max_ubo_reflections) + "\n");

		state.max_skeleton_bones = MIN(2048, max_ubo_size / (12 * sizeof(float)));
		state.scene_shader.add_custom_define("#define MAX_SKELETON_BONES " + itos(state.max_skeleton_bones) + "\n");
	}

	shadow_filter_mode = SHADOW_FILTER_NEAREST;

	{ //reflection cubemaps
		int max_reflection_cubemap_sampler_size = 512;

		int cube_size = max_reflection_cubemap_sampler_size;

		glActiveTexture(GL_TEXTURE0);

		bool use_float = true;

		GLenum internal_format = use_float ? GL_RGBA16F : GL_RGB10_A2;
		GLenum format = GL_RGBA;
		GLenum type = use_float ? GL_HALF_FLOAT : GL_UNSIGNED_INT_2_10_10_10_REV;

		while (cube_size >= 32) {

			ReflectionCubeMap cube;
			cube.size = cube_size;

			glGenTextures(1, &cube.depth);
			glBindTexture(GL_TEXTURE_2D, cube.depth);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, cube.size, cube.size, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glGenTextures(1, &cube.cubemap);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cube.cubemap);
			//gen cubemap first
			for (int i = 0; i < 6; i++) {

				glTexImage2D(_cube_side_enum[i], 0, internal_format, cube.size, cube.size, 0, format, type, NULL);
			}

			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			// Remove artifact on the edges of the reflectionmap
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

			//gen renderbuffers second, because it needs a complete cubemap
			for (int i = 0; i < 6; i++) {

				glGenFramebuffers(1, &cube.fbo_id[i]);
				glBindFramebuffer(GL_FRAMEBUFFER, cube.fbo_id[i]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _cube_side_enum[i], cube.cubemap, 0);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, cube.depth, 0);

				GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
				ERR_CONTINUE(status != GL_FRAMEBUFFER_COMPLETE);
			}

			reflection_cubemaps.push_back(cube);

			cube_size >>= 1;
		}
	}

	{

		uint32_t immediate_buffer_size = GLOBAL_DEF("rendering/limits/buffers/immediate_buffer_size_kb", 2048);

		glGenBuffers(1, &state.immediate_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, state.immediate_buffer);
		glBufferData(GL_ARRAY_BUFFER, immediate_buffer_size * 1024, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenVertexArrays(1, &state.immediate_array);
	}

#ifdef GLES_OVER_GL
	//"desktop" opengl needs this.
	glEnable(GL_PROGRAM_POINT_SIZE);

#endif

	state.resolve_shader.init();
	state.ssr_shader.init();
	state.effect_blur_shader.init();
	state.sss_shader.init();
	state.ssao_minify_shader.init();
	state.ssao_shader.init();
	state.ssao_blur_shader.init();
	state.exposure_shader.init();
	state.tonemap_shader.init();

	{
		GLOBAL_DEF("rendering/quality/subsurface_scattering/quality", 1);
		ProjectSettings::get_singleton()->set_custom_property_info("rendering/quality/subsurface_scattering/quality", PropertyInfo(Variant::INT, "rendering/quality/subsurface_scattering/quality", PROPERTY_HINT_ENUM, "Low,Medium,High"));
		GLOBAL_DEF("rendering/quality/subsurface_scattering/scale", 1.0);
		ProjectSettings::get_singleton()->set_custom_property_info("rendering/quality/subsurface_scattering/scale", PropertyInfo(Variant::INT, "rendering/quality/subsurface_scattering/scale", PROPERTY_HINT_RANGE, "0.01,8,0.01"));
		GLOBAL_DEF("rendering/quality/subsurface_scattering/follow_surface", false);

		GLOBAL_DEF("rendering/quality/voxel_cone_tracing/high_quality", true);
	}

	exposure_shrink_size = 243;
	int max_exposure_shrink_size = exposure_shrink_size;

	while (max_exposure_shrink_size > 0) {

		RasterizerStorageGLES3::RenderTarget::Exposure e;

		glGenFramebuffers(1, &e.fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, e.fbo);

		glGenTextures(1, &e.color);
		glBindTexture(GL_TEXTURE_2D, e.color);
#ifdef IPHONE_ENABLED
		///@TODO ugly hack to get around iOS not supporting 32bit single channel floating point textures...
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, max_exposure_shrink_size, max_exposure_shrink_size, 0, GL_RED, GL_FLOAT, NULL);
#else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, max_exposure_shrink_size, max_exposure_shrink_size, 0, GL_RED, GL_FLOAT, NULL);
#endif
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, e.color, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		exposure_shrink.push_back(e);
		max_exposure_shrink_size /= 3;

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		ERR_CONTINUE(status != GL_FRAMEBUFFER_COMPLETE);
	}

	state.debug_draw = VS::VIEWPORT_DEBUG_DRAW_DISABLED;
}

void RasterizerSceneGLES3::iteration() {

	shadow_filter_mode = ShadowFilterMode(int(ProjectSettings::get_singleton()->get("rendering/quality/shadows/filter_mode")));
	subsurface_scatter_follow_surface = ProjectSettings::get_singleton()->get("rendering/quality/subsurface_scattering/follow_surface");
	subsurface_scatter_quality = SubSurfaceScatterQuality(int(ProjectSettings::get_singleton()->get("rendering/quality/subsurface_scattering/quality")));
	subsurface_scatter_size = ProjectSettings::get_singleton()->get("rendering/quality/subsurface_scattering/scale");

	state.scene_shader.set_conditional(SceneShaderGLES3::VCT_QUALITY_HIGH, ProjectSettings::get_singleton()->get("rendering/quality/voxel_cone_tracing/high_quality"));
}

void RasterizerSceneGLES3::finalize() {
}

RasterizerSceneGLES3::RasterizerSceneGLES3() {
}
