/*************************************************************************/
/*  broad_phase_3d_sw.h                                                  */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
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

#ifndef BROAD_PHASE_SW_H
#define BROAD_PHASE_SW_H

#include "core/math/aabb.h"
#include "core/math/math_funcs.h"

class CollisionObject3DSW;

class BroadPhase3DSW {
public:
	typedef BroadPhase3DSW *(*CreateFunction)();

	static CreateFunction create_func;

	typedef uint32_t ID;

	typedef void *(*PairCallback)(CollisionObject3DSW *A, int p_subindex_A, CollisionObject3DSW *B, int p_subindex_B, void *p_userdata);
	typedef void (*UnpairCallback)(CollisionObject3DSW *A, int p_subindex_A, CollisionObject3DSW *B, int p_subindex_B, void *p_data, void *p_userdata);

	// 0 is an invalid ID
	virtual ID create(CollisionObject3DSW *p_object_, int p_subindex = 0) = 0;
	virtual void move(ID p_id, const AABB &p_aabb) = 0;
	virtual void set_static(ID p_id, bool p_static) = 0;
	virtual void remove(ID p_id) = 0;

	virtual CollisionObject3DSW *get_object(ID p_id) const = 0;
	virtual bool is_static(ID p_id) const = 0;
	virtual int get_subindex(ID p_id) const = 0;

	virtual int cull_point(const Vector3 &p_point, CollisionObject3DSW **p_results, int p_max_results, int *p_result_indices = nullptr) = 0;
	virtual int cull_segment(const Vector3 &p_from, const Vector3 &p_to, CollisionObject3DSW **p_results, int p_max_results, int *p_result_indices = nullptr) = 0;
	virtual int cull_aabb(const AABB &p_aabb, CollisionObject3DSW **p_results, int p_max_results, int *p_result_indices = nullptr) = 0;

	virtual void set_pair_callback(PairCallback p_pair_callback, void *p_userdata) = 0;
	virtual void set_unpair_callback(UnpairCallback p_unpair_callback, void *p_userdata) = 0;

	virtual void update() = 0;

	virtual ~BroadPhase3DSW();
};

#endif // BROAD_PHASE__SW_H
