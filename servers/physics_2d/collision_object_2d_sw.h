/*************************************************************************/
/*  collision_object_2d_sw.h                                             */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                 */
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
#ifndef COLLISION_OBJECT_2D_SW_H
#define COLLISION_OBJECT_2D_SW_H

#include "shape_2d_sw.h"
#include "servers/physics_2d_server.h"
#include "self_list.h"
#include "broad_phase_2d_sw.h"

class Space2DSW;

class CollisionObject2DSW : public ShapeOwner2DSW {
public:
	enum Type {
		TYPE_AREA,
		TYPE_BODY
	};
private:

	Type type;
	RID self;
	ObjectID instance_id;

	struct Shape {

		Matrix32 xform;
		Matrix32 xform_inv;
		BroadPhase2DSW::ID bpid;
		Rect2 aabb_cache; //for rayqueries
		Shape2DSW *shape;
		Vector2 kinematic_advance;
		float kinematic_retreat;
		bool trigger;
		Shape() { trigger=false; kinematic_retreat=0; }
	};

	Vector<Shape> shapes;
	Space2DSW *space;
	Matrix32 transform;
	Matrix32 inv_transform;
	uint32_t user_mask;
	bool _static;

	void _update_shapes();

protected:


	void _update_shapes_with_motion(const Vector2& p_motion);
	void _unregister_shapes();

	_FORCE_INLINE_ void _set_transform(const Matrix32& p_transform, bool p_update_shapes=true) { transform=p_transform; if (p_update_shapes) {_update_shapes();} }
	_FORCE_INLINE_ void _set_inv_transform(const Matrix32& p_transform) { inv_transform=p_transform; }
	void _set_static(bool p_static);

	virtual void _shapes_changed()=0;
	void _set_space(Space2DSW *space);

	CollisionObject2DSW(Type p_type);
public:

	_FORCE_INLINE_ void set_self(const RID& p_self) { self=p_self; }
	_FORCE_INLINE_ RID get_self() const { return self; }

	_FORCE_INLINE_ void set_instance_id(const ObjectID& p_instance_id) { instance_id=p_instance_id; }
	_FORCE_INLINE_ ObjectID get_instance_id() const { return instance_id; }

	void _shape_changed();

	_FORCE_INLINE_ Type get_type() const { return type; }
	void add_shape(Shape2DSW *p_shape,const Matrix32& p_transform=Matrix32());
	void set_shape(int p_index,Shape2DSW *p_shape);
	void set_shape_transform(int p_index,const Matrix32& p_transform);
	_FORCE_INLINE_ int get_shape_count() const { return shapes.size(); }
	_FORCE_INLINE_ Shape2DSW *get_shape(int p_index) const { return shapes[p_index].shape; }
	_FORCE_INLINE_ const Matrix32& get_shape_transform(int p_index) const { return shapes[p_index].xform; }
	_FORCE_INLINE_ const Matrix32& get_shape_inv_transform(int p_index) const { return shapes[p_index].xform_inv; }
	_FORCE_INLINE_ const Rect2& get_shape_aabb(int p_index) const { return shapes[p_index].aabb_cache; }

	_FORCE_INLINE_ void set_shape_kinematic_advance(int p_index,const Vector2& p_advance) { shapes[p_index].kinematic_advance=p_advance; }
	_FORCE_INLINE_ Vector2 get_shape_kinematic_advance(int p_index) const { return shapes[p_index].kinematic_advance; }

	_FORCE_INLINE_ void set_shape_kinematic_retreat(int p_index,float p_retreat) { shapes[p_index].kinematic_retreat=p_retreat; }
	_FORCE_INLINE_ float get_shape_kinematic_retreat(int p_index) const { return shapes[p_index].kinematic_retreat; }

	_FORCE_INLINE_ Matrix32 get_transform() const { return transform; }
	_FORCE_INLINE_ Matrix32 get_inv_transform() const { return inv_transform; }
	_FORCE_INLINE_ Space2DSW* get_space() const { return space; }

	_FORCE_INLINE_ void set_shape_as_trigger(int p_idx,bool p_enable) { shapes[p_idx].trigger=p_enable; }
	_FORCE_INLINE_ bool is_shape_set_as_trigger(int p_idx) const { return shapes[p_idx].trigger; }


	void set_user_mask(uint32_t p_mask) {user_mask=p_mask;}
	_FORCE_INLINE_ uint32_t get_user_mask() const { return user_mask; }

	void remove_shape(Shape2DSW *p_shape);
	void remove_shape(int p_index);

	virtual void set_space(Space2DSW *p_space)=0;

	_FORCE_INLINE_ bool is_static() const { return _static;  }

	virtual ~CollisionObject2DSW() {}

};

#endif // COLLISION_OBJECT_2D_SW_H
