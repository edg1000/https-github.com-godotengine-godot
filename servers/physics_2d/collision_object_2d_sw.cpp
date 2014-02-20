/*************************************************************************/
/*  collision_object_2d_sw.cpp                                           */
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
#include "collision_object_2d_sw.h"
#include "space_2d_sw.h"

void CollisionObject2DSW::add_shape(Shape2DSW *p_shape,const Matrix32& p_transform) {

	Shape s;
	s.shape=p_shape;
	s.xform=p_transform;
	s.xform_inv=s.xform.affine_inverse();
	s.bpid=0; //needs update
	s.trigger=false;
	shapes.push_back(s);
	p_shape->add_owner(this);
	_update_shapes();
	_shapes_changed();

}

void CollisionObject2DSW::set_shape(int p_index,Shape2DSW *p_shape){

	ERR_FAIL_INDEX(p_index,shapes.size());
	shapes[p_index].shape->remove_owner(this);
	shapes[p_index].shape=p_shape;

	p_shape->add_owner(this);
	_update_shapes();
	_shapes_changed();

}
void CollisionObject2DSW::set_shape_transform(int p_index,const Matrix32& p_transform){

	ERR_FAIL_INDEX(p_index,shapes.size());

	shapes[p_index].xform=p_transform;
	shapes[p_index].xform_inv=p_transform.affine_inverse();
	_update_shapes();
	_shapes_changed();
}

void CollisionObject2DSW::remove_shape(Shape2DSW *p_shape) {

	//remove a shape, all the times it appears
	for(int i=0;i<shapes.size();i++) {

		if (shapes[i].shape==p_shape) {
			remove_shape(i);
			i--;
		}
	}
}

void CollisionObject2DSW::remove_shape(int p_index){

	//remove anything from shape to be erased to end, so subindices don't change
	ERR_FAIL_INDEX(p_index,shapes.size());
	for(int i=p_index;i<shapes.size();i++) {

		if (shapes[i].bpid==0)
			continue;
		//should never get here with a null owner
		space->get_broadphase()->remove(shapes[i].bpid);
		shapes[i].bpid=0;
	}
	shapes[p_index].shape->remove_owner(this);
	shapes.remove(p_index);

	_shapes_changed();

}

void CollisionObject2DSW::_set_static(bool p_static) {
	if (_static==p_static)
		return;
	_static=p_static;

	if (!space)
		return;
	for(int i=0;i<get_shape_count();i++) {
		Shape &s=shapes[i];
		if (s.bpid>0) {
			space->get_broadphase()->set_static(s.bpid,_static);
		}
	}

}

void CollisionObject2DSW::_unregister_shapes() {

	for(int i=0;i<shapes.size();i++) {

		Shape &s=shapes[i];
		if (s.bpid>0) {
			space->get_broadphase()->remove(s.bpid);
			s.bpid=0;
		}
	}

}

void CollisionObject2DSW::_update_shapes() {

	if (!space)
		return;


	for(int i=0;i<shapes.size();i++) {

		Shape &s=shapes[i];
		if (s.bpid==0) {
			s.bpid=space->get_broadphase()->create(this,i);
			space->get_broadphase()->set_static(s.bpid,_static);
		}

		//not quite correct, should compute the next matrix..
		Rect2 shape_aabb=s.shape->get_aabb();
		Matrix32 xform = transform * s.xform;
		shape_aabb=xform.xform(shape_aabb);
		s.aabb_cache=shape_aabb;
		s.aabb_cache=s.aabb_cache.grow( (s.aabb_cache.size.x + s.aabb_cache.size.y)*0.5*0.05 );


		space->get_broadphase()->move(s.bpid,s.aabb_cache);
	}

}

void CollisionObject2DSW::_update_shapes_with_motion(const Vector2& p_motion) {


	if (!space)
		return;

	for(int i=0;i<shapes.size();i++) {

		Shape &s=shapes[i];
		if (s.bpid==0) {
			s.bpid=space->get_broadphase()->create(this,i);
			space->get_broadphase()->set_static(s.bpid,_static);
		}

		//not quite correct, should compute the next matrix..
		Rect2 shape_aabb=s.shape->get_aabb();
		Matrix32 xform = transform * s.xform;
		shape_aabb=xform.xform(shape_aabb);
		shape_aabb=shape_aabb.merge(Rect2( shape_aabb.pos+p_motion,shape_aabb.size)); //use motion
		s.aabb_cache=shape_aabb;

		space->get_broadphase()->move(s.bpid,shape_aabb);
	}


}

void CollisionObject2DSW::_set_space(Space2DSW *p_space) {

	if (space) {

		space->remove_object(this);

		for(int i=0;i<shapes.size();i++) {

			Shape &s=shapes[i];
			if (s.bpid) {
				space->get_broadphase()->remove(s.bpid);
				s.bpid=0;
			}
		}

	}

	space=p_space;

	if (space) {

		space->add_object(this);
		_update_shapes();
	}

}

void CollisionObject2DSW::_shape_changed() {

	_update_shapes();
	_shapes_changed();
}

CollisionObject2DSW::CollisionObject2DSW(Type p_type) {

	_static=true;
	type=p_type;
	space=NULL;
	instance_id=0;
}
