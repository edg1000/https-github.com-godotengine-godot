/*************************************************************************/
/*  decal_atlas_storage.h                                                */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
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

#ifndef DECAL_ATLAS_STORAGE_H
#define DECAL_ATLAS_STORAGE_H

#include "servers/rendering_server.h"

class RendererDecalAtlasStorage {
public:
	virtual ~RendererDecalAtlasStorage(){};

	virtual RID decal_allocate() = 0;
	virtual void decal_initialize(RID p_rid) = 0;
	virtual void decal_free(RID p_rid) = 0;

	virtual void decal_set_extents(RID p_decal, const Vector3 &p_extents) = 0;
	virtual void decal_set_texture(RID p_decal, RS::DecalTexture p_type, RID p_texture) = 0;
	virtual void decal_set_emission_energy(RID p_decal, float p_energy) = 0;
	virtual void decal_set_albedo_mix(RID p_decal, float p_mix) = 0;
	virtual void decal_set_modulate(RID p_decal, const Color &p_modulate) = 0;
	virtual void decal_set_cull_mask(RID p_decal, uint32_t p_layers) = 0;
	virtual void decal_set_distance_fade(RID p_decal, bool p_enabled, float p_begin, float p_length) = 0;
	virtual void decal_set_fade(RID p_decal, float p_above, float p_below) = 0;
	virtual void decal_set_normal_fade(RID p_decal, float p_fade) = 0;

	virtual AABB decal_get_aabb(RID p_decal) const = 0;

	virtual void texture_add_to_decal_atlas(RID p_texture, bool p_panorama_to_dp = false) = 0;
	virtual void texture_remove_from_decal_atlas(RID p_texture, bool p_panorama_to_dp = false) = 0;
};

#endif // !DECAL_ATLAS_STORAGE_H
