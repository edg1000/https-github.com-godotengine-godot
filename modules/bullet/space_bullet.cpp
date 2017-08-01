/*************************************************************************/
/*  space_bullet.cpp                                                     */
/*  Author: AndreaCatania                                                */
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

#include "space_bullet.h"
#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "body_bullet.h"
#include "btBulletDynamicsCommon.h"
#include "bullet_physics_server.h"
#include "bullet_types_converter.h"
#include "bullet_utilities.h"
#include "constraint_bullet.h"
#include "godot_collision_configuration.h"
#include "godot_collision_dispatcher.h"
#include "servers/physics_server.h"
#include "ustring.h"
#include <assert.h>

// test only
//#include "scene/3d/immediate_geometry.h"

bool GodotFilterCallback::test_collision_filters(uint32_t body0_collision_layer, uint32_t body0_collision_mask, uint32_t body1_collision_layer, uint32_t body1_collision_mask) {
	return body0_collision_layer & body1_collision_mask || body1_collision_layer & body0_collision_mask;
}

bool GodotFilterCallback::needBroadphaseCollision(btBroadphaseProxy *proxy0, btBroadphaseProxy *proxy1) const {
	// Even if you think I'm crazy, I'm not.
	// This gBody0 == gBody1 is required because the kinematic actor use the ghost, and in this case I don't want collide with it
	void *gBody0 = static_cast<btCollisionObject *>(proxy0->m_clientObject)->getUserPointer();
	void *gBody1 = static_cast<btCollisionObject *>(proxy1->m_clientObject)->getUserPointer();
	if (gBody0 == gBody1) {
		return false;
	}

	return GodotFilterCallback::test_collision_filters(proxy0->m_collisionFilterGroup, proxy0->m_collisionFilterMask, proxy1->m_collisionFilterGroup, proxy1->m_collisionFilterMask);
}

/// It performs an additional check allow exclusions.
struct GodotClosestRayResultCallback : public btCollisionWorld::ClosestRayResultCallback {
	const Set<RID> *m_exclude;
	bool m_pickRay;

public:
	GodotClosestRayResultCallback(const btVector3 &rayFromWorld, const btVector3 &rayToWorld, const Set<RID> *p_exclude)
		: btCollisionWorld::ClosestRayResultCallback(rayFromWorld, rayToWorld), m_exclude(p_exclude), m_pickRay(false) {}

	virtual bool needsCollision(btBroadphaseProxy *proxy0) const {
		const bool needs = GodotFilterCallback::test_collision_filters(m_collisionFilterGroup, m_collisionFilterMask, proxy0->m_collisionFilterGroup, proxy0->m_collisionFilterMask);
		if (needs) {
			btCollisionObject *btObj = static_cast<btCollisionObject *>(proxy0->m_clientObject);
			CollisionObjectBullet *gObj = static_cast<CollisionObjectBullet *>(btObj->getUserPointer());
			if (m_pickRay && gObj->is_ray_pickable()) {
				return true;
			} else if (m_exclude->has(gObj->get_self())) {
				return false;
			}
			return true;
		} else {
			return false;
		}
	}
};

// store all colliding object
struct GodotAllConvexResultCallback : public btCollisionWorld::ConvexResultCallback {
public:
	PhysicsDirectSpaceState::ShapeResult *m_results;
	int m_resultMax;
	const Set<RID> *m_exclude;
	int count;

	GodotAllConvexResultCallback(PhysicsDirectSpaceState::ShapeResult *p_results, int p_resultMax, const Set<RID> *p_exclude)
		: m_results(p_results), m_exclude(p_exclude), m_resultMax(p_resultMax), count(0) {}

	virtual bool needsCollision(btBroadphaseProxy *proxy0) const {
		const bool needs = GodotFilterCallback::test_collision_filters(m_collisionFilterGroup, m_collisionFilterMask, proxy0->m_collisionFilterGroup, proxy0->m_collisionFilterMask);
		if (needs) {
			btCollisionObject *btObj = static_cast<btCollisionObject *>(proxy0->m_clientObject);
			CollisionObjectBullet *gObj = static_cast<CollisionObjectBullet *>(btObj->getUserPointer());
			if (m_exclude->has(gObj->get_self())) {
				return false;
			}
			return true;
		} else {
			return false;
		}
	}

	virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult &convexResult, bool normalInWorldSpace) {
		CollisionObjectBullet *gObj = static_cast<CollisionObjectBullet *>(convexResult.m_hitCollisionObject->getUserPointer());

		PhysicsDirectSpaceState::ShapeResult &result = m_results[count];

		result.shape = -1;
		result.rid = gObj->get_self();
		result.collider_id = gObj->get_instance_id();
		result.collider = 0 == result.collider_id ? NULL : ObjectDB::get_instance(result.collider_id);

		++count;
		return count < m_resultMax;
	}
};

struct GodotClosestConvexResultCallback : public btCollisionWorld::ClosestConvexResultCallback {
public:
	const BodyBullet *m_self_object;
	const bool m_ignore_areas;

	GodotClosestConvexResultCallback(const btVector3 &convexFromWorld, const btVector3 &convexToWorld, const BodyBullet *p_self_object, bool p_ignore_areas)
		: btCollisionWorld::ClosestConvexResultCallback(convexFromWorld, convexToWorld), m_self_object(p_self_object), m_ignore_areas(p_ignore_areas) {}

	virtual bool needsCollision(btBroadphaseProxy *proxy0) const {
		const bool needs = GodotFilterCallback::test_collision_filters(m_collisionFilterGroup, m_collisionFilterMask, proxy0->m_collisionFilterGroup, proxy0->m_collisionFilterMask);
		if (needs) {
			btCollisionObject *btObj = static_cast<btCollisionObject *>(proxy0->m_clientObject);
			CollisionObjectBullet *gObj = static_cast<CollisionObjectBullet *>(btObj->getUserPointer());
			if (gObj == m_self_object) {
				return false;
			} else {
				if (m_ignore_areas && gObj->getType() == CollisionObjectBullet::TYPE_AREA) {
					return false;
				} else if (m_self_object->has_collision_exception(gObj)) {
					return false;
				}
			}
			return true;
		} else {
			return false;
		}
	}
};

struct GodotRecoverAndClosestContactResultCallback : public btCollisionWorld::ContactResultCallback {
public:
	btVector3 m_pointNormalWorld;
	btVector3 m_pointWorld;
	btScalar m_penetration_distance;
	int m_other_compound_shape_index;
	const btCollisionObject *m_pointCollisionObject;

	const BodyBullet *m_self_object;
	bool m_ignore_areas;

	btScalar m_most_penetrated_distance;
	btVector3 m_recover_penetration;

	GodotRecoverAndClosestContactResultCallback()
		: m_pointCollisionObject(NULL), m_penetration_distance(0), m_other_compound_shape_index(0), m_self_object(NULL), m_ignore_areas(true), m_most_penetrated_distance(9999999999), m_recover_penetration(0, 0, 0) {}

	GodotRecoverAndClosestContactResultCallback(const BodyBullet *p_self_object, bool p_ignore_areas)
		: m_pointCollisionObject(NULL), m_penetration_distance(0), m_other_compound_shape_index(0), m_self_object(p_self_object), m_ignore_areas(p_ignore_areas), m_most_penetrated_distance(9999999999), m_recover_penetration(0, 0, 0) {}

	void reset() {
		m_pointCollisionObject = NULL;
		m_most_penetrated_distance = 9999999999;
		m_recover_penetration.setZero();
	}

	bool hasHit() {
		return m_pointCollisionObject;
	}

	virtual bool needsCollision(btBroadphaseProxy *proxy0) const {
		const bool needs = GodotFilterCallback::test_collision_filters(m_collisionFilterGroup, m_collisionFilterMask, proxy0->m_collisionFilterGroup, proxy0->m_collisionFilterMask);
		if (needs) {
			btCollisionObject *btObj = static_cast<btCollisionObject *>(proxy0->m_clientObject);
			CollisionObjectBullet *gObj = static_cast<CollisionObjectBullet *>(btObj->getUserPointer());
			if (gObj == m_self_object) {
				return false;
			} else {
				if (m_ignore_areas && gObj->getType() == CollisionObjectBullet::TYPE_AREA) {
					return false;
				} else if (m_self_object->has_collision_exception(gObj)) {
					return false;
				}
			}
			return true;
		} else {
			return false;
		}
	}

	virtual btScalar addSingleResult(btManifoldPoint &cp, const btCollisionObjectWrapper *colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper *colObj1Wrap, int partId1, int index1) {

		if (cp.getDistance() < -MAX_PENETRATION_DEPTH) {
			if (m_most_penetrated_distance > cp.getDistance()) {
				m_most_penetrated_distance = cp.getDistance();

				// take other object
				btScalar sign(1);
				if (m_self_object == colObj0Wrap->getCollisionObject()->getUserPointer()) {
					m_pointCollisionObject = colObj1Wrap->getCollisionObject();
					m_other_compound_shape_index = cp.m_index1;
				} else {
					m_pointCollisionObject = colObj0Wrap->getCollisionObject();
					sign = -1;
					m_other_compound_shape_index = cp.m_index0;
				}

				m_pointNormalWorld = cp.m_normalWorldOnB * sign;
				m_pointWorld = cp.getPositionWorldOnB();
				m_penetration_distance = cp.getDistance();

				m_recover_penetration -= cp.m_normalWorldOnB * sign * (cp.getDistance() + MAX_PENETRATION_DEPTH);
			}
		}
		return 1;
	}
};

BulletPhysicsDirectSpaceState::BulletPhysicsDirectSpaceState(SpaceBullet *p_space)
	: PhysicsDirectSpaceState(), space(p_space) {}

int BulletPhysicsDirectSpaceState::intersect_point(const Vector3 &p_point, ShapeResult *r_results, int p_result_max, const Set<RID> &p_exclude, uint32_t p_collision_layer, uint32_t p_object_type_mask) {
	return 0;
}

bool BulletPhysicsDirectSpaceState::intersect_ray(const Vector3 &p_from, const Vector3 &p_to, RayResult &r_result, const Set<RID> &p_exclude, uint32_t p_collision_layer, uint32_t p_object_type_mask, bool p_pick_ray) {

	btVector3 btVec_from;
	btVector3 btVec_to;

	G_TO_B(p_from, btVec_from);
	G_TO_B(p_to, btVec_to);

	// setup query
	GodotClosestRayResultCallback btResult(btVec_from, btVec_to, &p_exclude);
	btResult.m_collisionFilterGroup = p_collision_layer;
	btResult.m_collisionFilterMask = p_object_type_mask;
	btResult.m_pickRay = p_pick_ray;

	space->dynamicsWorld->rayTest(btVec_from, btVec_to, btResult);
	if (btResult.hasHit()) {
		B_TO_G(btResult.m_hitPointWorld, r_result.position);
		B_TO_G(btResult.m_hitNormalWorld.normalize(), r_result.normal);
		CollisionObjectBullet *gObj = static_cast<CollisionObjectBullet *>(btResult.m_collisionObject->getUserPointer());
		if (gObj) {
			r_result.shape = -1;
			r_result.rid = gObj->get_self();
			r_result.collider_id = gObj->get_instance_id();
			r_result.collider = 0 == r_result.collider_id ? NULL : ObjectDB::get_instance(r_result.collider_id);
		} else {
			WARN_PRINTS("The raycast performed has hit a collision object that is not part of Godot scene, please check it.");
		}
		return true;
	} else {
		return false;
	}
}

int BulletPhysicsDirectSpaceState::intersect_shape(const RID &p_shape, const Transform &p_xform, float p_margin, ShapeResult *p_results, int p_result_max, const Set<RID> &p_exclude, uint32_t p_collision_layer, uint32_t p_object_type_mask) {
	if (p_result_max <= 0)
		return 0;

	ShapeBullet *shape = space->get_physics_server()->get_shape_owner()->get(p_shape);

	btConvexShape *btConvex = dynamic_cast<btConvexShape *>(shape->create_bt_shape());
	if (!btConvex) {
		bulletdelete(btConvex);
		ERR_PRINTS("The shape is not a convex shape, then is not supported: shape type: " + itos(shape->get_type()));
		return 0;
	}

	btVector3 scale;
	G_TO_B(p_xform.basis.get_scale(), scale);
	btConvex->setLocalScaling(scale);

	btTransform bt_xform;
	G_TO_B(p_xform, bt_xform);

	// Setup query
	GodotAllConvexResultCallback btResult(p_results, p_result_max, &p_exclude);
	btResult.m_collisionFilterGroup = p_collision_layer;
	btResult.m_collisionFilterMask = p_object_type_mask;
	space->dynamicsWorld->convexSweepTest(btConvex, bt_xform, bt_xform, btResult);

	bulletdelete(btConvex);

	// The results is already populated by GodotAllConvexResultCallback
	return btResult.count;
}

bool BulletPhysicsDirectSpaceState::cast_motion(const RID &p_shape, const Transform &p_xform, const Vector3 &p_motion, float p_margin, float &p_closest_safe, float &p_closest_unsafe, const Set<RID> &p_exclude, uint32_t p_collision_layer, uint32_t p_object_type_mask, ShapeRestInfo *r_info) {
	WARN_PRINT("This function must be implemented")
	return false;
}

bool BulletPhysicsDirectSpaceState::collide_shape(RID p_shape, const Transform &p_shape_xform, float p_margin, Vector3 *r_results, int p_result_max, int &r_result_count, const Set<RID> &p_exclude, uint32_t p_collision_layer, uint32_t p_object_type_mask) {
	WARN_PRINT("This function must be implemented")
	return false;
}

bool BulletPhysicsDirectSpaceState::rest_info(RID p_shape, const Transform &p_shape_xform, float p_margin, ShapeRestInfo *r_info, const Set<RID> &p_exclude, uint32_t p_collision_layer, uint32_t p_object_type_mask) {
	WARN_PRINT("This function must be implemented")
	return false;
}

Vector3 BulletPhysicsDirectSpaceState::get_closest_point_to_object_volume(RID p_object, const Vector3 p_point) const {
	WARN_PRINT("This function must be implemented")
	return Vector3();
}

SpaceBullet::SpaceBullet()
	: broadphase(NULL), dispatcher(NULL), solver(NULL), collisionConfiguration(NULL), dynamicsWorld(NULL), ghostPairCallback(NULL), godotFilterCallback(NULL), gravityDirection(0, -1, 0), gravityMagnitude(10), contactDebugCount(0) {
	create_empty_world();
	direct_access = memnew(BulletPhysicsDirectSpaceState(this));
}

SpaceBullet::~SpaceBullet() {
	memdelete(direct_access);
	destroy_world();
}

void SpaceBullet::flush_queries() {
	const btCollisionObjectArray &colObjArray = dynamicsWorld->getCollisionObjectArray();
	for (int i = colObjArray.size() - 1; 0 <= i; --i) {
		btCollisionObject *obj = colObjArray[i];
		if (static_cast<int>(QUERY_TYPE_EXE) == obj->getUserIndex()) {
			// Only not ghost
			static_cast<CollisionObjectBullet *>(obj->getUserPointer())->dispatch_callbacks();
		}
	}
}

void SpaceBullet::set_param(PhysicsServer::AreaParameter p_param, const Variant &p_value) {
	assert(dynamicsWorld);

	switch (p_param) {
		case PhysicsServer::AREA_PARAM_GRAVITY:
			gravityMagnitude = p_value;
			update_gravity();
			break;
		case PhysicsServer::AREA_PARAM_GRAVITY_VECTOR:
			gravityDirection = p_value;
			update_gravity();
			break;
		case PhysicsServer::AREA_PARAM_LINEAR_DAMP:
		case PhysicsServer::AREA_PARAM_ANGULAR_DAMP:
			break; // No damp
		case PhysicsServer::AREA_PARAM_PRIORITY:
			// Priority is always 0, the lower
			break;
		case PhysicsServer::AREA_PARAM_GRAVITY_IS_POINT:
		case PhysicsServer::AREA_PARAM_GRAVITY_DISTANCE_SCALE:
		case PhysicsServer::AREA_PARAM_GRAVITY_POINT_ATTENUATION:
			break;
		default:
			WARN_PRINTS("This set parameter (" + itos(p_param) + ") is ignored, the SpaceBullet doesn't support it.");
			break;
	}
}

Variant SpaceBullet::get_param(PhysicsServer::AreaParameter p_param) {
	switch (p_param) {
		case PhysicsServer::AREA_PARAM_GRAVITY:
			return gravityMagnitude;
		case PhysicsServer::AREA_PARAM_GRAVITY_VECTOR:
			return gravityDirection;
		case PhysicsServer::AREA_PARAM_LINEAR_DAMP:
		case PhysicsServer::AREA_PARAM_ANGULAR_DAMP:
			return 0; // No damp
		case PhysicsServer::AREA_PARAM_PRIORITY:
			return 0; // Priority is always 0, the lower
		case PhysicsServer::AREA_PARAM_GRAVITY_IS_POINT:
			return false;
		case PhysicsServer::AREA_PARAM_GRAVITY_DISTANCE_SCALE:
			return 0;
		case PhysicsServer::AREA_PARAM_GRAVITY_POINT_ATTENUATION:
			return 0;
		default:
			WARN_PRINTS("This get parameter (" + itos(p_param) + ") is ignored, the SpaceBullet doesn't support it.");
			return Variant();
	}
}

void SpaceBullet::set_param(PhysicsServer::SpaceParameter p_param, real_t p_value) {
	switch (p_param) {
		case PhysicsServer::SPACE_PARAM_CONTACT_RECYCLE_RADIUS:
		case PhysicsServer::SPACE_PARAM_CONTACT_MAX_SEPARATION:
		case PhysicsServer::SPACE_PARAM_BODY_MAX_ALLOWED_PENETRATION:
		case PhysicsServer::SPACE_PARAM_BODY_LINEAR_VELOCITY_SLEEP_THRESHOLD:
		case PhysicsServer::SPACE_PARAM_BODY_ANGULAR_VELOCITY_SLEEP_THRESHOLD:
		case PhysicsServer::SPACE_PARAM_BODY_TIME_TO_SLEEP:
		case PhysicsServer::SPACE_PARAM_BODY_ANGULAR_VELOCITY_DAMP_RATIO:
		case PhysicsServer::SPACE_PARAM_CONSTRAINT_DEFAULT_BIAS:
		default:
			WARN_PRINTS("This set parameter (" + itos(p_param) + ") is ignored, the SpaceBullet doesn't support it.");
			break;
	}
}

real_t SpaceBullet::get_param(PhysicsServer::SpaceParameter p_param) {
	switch (p_param) {
		case PhysicsServer::SPACE_PARAM_CONTACT_RECYCLE_RADIUS:
		case PhysicsServer::SPACE_PARAM_CONTACT_MAX_SEPARATION:
		case PhysicsServer::SPACE_PARAM_BODY_MAX_ALLOWED_PENETRATION:
		case PhysicsServer::SPACE_PARAM_BODY_LINEAR_VELOCITY_SLEEP_THRESHOLD:
		case PhysicsServer::SPACE_PARAM_BODY_ANGULAR_VELOCITY_SLEEP_THRESHOLD:
		case PhysicsServer::SPACE_PARAM_BODY_TIME_TO_SLEEP:
		case PhysicsServer::SPACE_PARAM_BODY_ANGULAR_VELOCITY_DAMP_RATIO:
		case PhysicsServer::SPACE_PARAM_CONSTRAINT_DEFAULT_BIAS:
		default:
			WARN_PRINTS("The SpaceBullet  doesn't support this get parameter (" + itos(p_param) + "), 0 is returned.");
			return 0.f;
	}
}

void SpaceBullet::add_area(AreaBullet *p_area) {
	areas.push_back(p_area);
	dynamicsWorld->addCollisionObject(p_area->get_bt_ghost(), p_area->get_collision_layer(), p_area->get_collision_mask());
}

void SpaceBullet::remove_area(AreaBullet *p_area) {
	areas.erase(p_area);
	dynamicsWorld->removeCollisionObject(p_area->get_bt_ghost());
}

void SpaceBullet::reload_collision_filters(AreaBullet *p_area) {
	// This is necessary to change collision filter
	dynamicsWorld->removeCollisionObject(p_area->get_bt_ghost());
	dynamicsWorld->addCollisionObject(p_area->get_bt_ghost(), p_area->get_collision_layer(), p_area->get_collision_mask());
}

void SpaceBullet::add_body(BodyBullet *p_body) {
	if (p_body->is_static()) {
		dynamicsWorld->addCollisionObject(p_body->get_bt_body(), p_body->get_collision_layer(), p_body->get_collision_mask());
	} else {
		dynamicsWorld->addRigidBody(p_body->get_bt_body(), p_body->get_collision_layer(), p_body->get_collision_mask());
	}

	add_ghost(p_body);
}

void SpaceBullet::remove_body(BodyBullet *p_body) {
	if (p_body->is_static()) {
		dynamicsWorld->removeCollisionObject(p_body->get_bt_body());
	} else {
		dynamicsWorld->removeRigidBody(p_body->get_bt_body());
	}

	remove_ghost(p_body);
}

void SpaceBullet::reload_collision_filters(BodyBullet *p_body) {
	// This is necessary to change collision filter
	remove_body(p_body);
	add_body(p_body);
}

void SpaceBullet::add_ghost(BodyBullet *p_ghost) {
	if (p_ghost->get_kinematic_utilities()) {
		dynamicsWorld->addCollisionObject(p_ghost->get_kinematic_utilities()->m_ghostObject, p_ghost->get_collision_layer(), p_ghost->get_collision_mask());
	}
}

void SpaceBullet::remove_ghost(BodyBullet *p_ghost) {
	if (p_ghost->get_kinematic_utilities()) {
		dynamicsWorld->removeCollisionObject(p_ghost->get_kinematic_utilities()->m_ghostObject);
	}
}

void SpaceBullet::add_constraint(ConstraintBullet *p_constraint, bool disableCollisionsBetweenLinkedBodies) {
	p_constraint->set_space(this);
	dynamicsWorld->addConstraint(p_constraint->get_bt_constraint(), disableCollisionsBetweenLinkedBodies);
}

void SpaceBullet::remove_constraint(ConstraintBullet *p_constraint) {
	dynamicsWorld->removeConstraint(p_constraint->get_bt_constraint());
}

int SpaceBullet::get_num_collision_objects() const {
	return dynamicsWorld->getNumCollisionObjects();
}

void SpaceBullet::remove_all_collision_objects() {
	for (int i = dynamicsWorld->getNumCollisionObjects() - 1; 0 <= i; --i) {
		btCollisionObject *btObj = dynamicsWorld->getCollisionObjectArray()[i];
		CollisionObjectBullet *colObj = static_cast<CollisionObjectBullet *>(btObj->getUserPointer());
		colObj->set_space(NULL);
	}
}

void onBulletTickCallback(btDynamicsWorld *p_dynamicsWorld, btScalar timeStep) {

	// Notify all Collision objects the collision checker is started
	const btCollisionObjectArray &colObjArray = p_dynamicsWorld->getCollisionObjectArray();
	for (int i = colObjArray.size() - 1; 0 <= i; --i) {
		CollisionObjectBullet *colObj = static_cast<CollisionObjectBullet *>(colObjArray[i]->getUserPointer());
		assert(NULL != colObj);
		colObj->on_collision_checker_start();
	}

	SpaceBullet *sb = static_cast<SpaceBullet *>(p_dynamicsWorld->getWorldUserInfo());
	sb->check_ghost_overlaps();
	sb->check_body_collision();
}

BulletPhysicsDirectSpaceState *SpaceBullet::get_direct_state() {
	return direct_access;
}

void SpaceBullet::create_empty_world() {
	assert(NULL == broadphase);
	assert(NULL == dispatcher);
	assert(NULL == solver);
	assert(NULL == collisionConfiguration);
	assert(NULL == dynamicsWorld);
	assert(NULL == ghostPairCallback);
	assert(NULL == godotFilterCallback);

	collisionConfiguration = bulletnew(GodotCollisionConfiguration);
	dispatcher = bulletnew(GodotCollisionDispatcher(collisionConfiguration));
	broadphase = bulletnew(btDbvtBroadphase);
	solver = bulletnew(btSequentialImpulseConstraintSolver);
	dynamicsWorld = bulletnew(btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration));
	ghostPairCallback = bulletnew(btGhostPairCallback);
	godotFilterCallback = bulletnew(GodotFilterCallback);

	dynamicsWorld->setWorldUserInfo(this);

	// Setup ghost check
	dynamicsWorld->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(ghostPairCallback);
	const bool isPreTick = false;
	dynamicsWorld->setInternalTickCallback(onBulletTickCallback, this, isPreTick);
	dynamicsWorld->getPairCache()->setOverlapFilterCallback(godotFilterCallback);

	update_gravity();
}

void SpaceBullet::destroy_world() {
	assert(NULL != broadphase);
	assert(NULL != dispatcher);
	assert(NULL != solver);
	assert(NULL != collisionConfiguration);
	assert(NULL != dynamicsWorld);
	assert(NULL != ghostPairCallback);
	assert(NULL != godotFilterCallback);

	/// The world element (Collision Objects, Constraints, Shapes) are cleared by godot

	dynamicsWorld->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(NULL);

	bulletdelete(ghostPairCallback);
	bulletdelete(dynamicsWorld);
	bulletdelete(solver);
	bulletdelete(broadphase);
	bulletdelete(dispatcher);
	bulletdelete(collisionConfiguration);
	bulletdelete(godotFilterCallback);
}

void SpaceBullet::check_ghost_overlaps() {
	// Define all pointers variables only one time.
	AreaBullet *area;
	btGhostObject *ghost;
	CollisionObjectBullet *otherObject;
	int x(-1), i(-1), indexOverlap(-1);

	// For each areas
	for (x = areas.size() - 1; 0 <= x; --x) {
		area = areas[x];

		// Check all overlapping objects
		ghost = area->get_bt_ghost();

		// Reset all states
		for (i = area->overlappingObjects.size() - 1; 0 <= i; --i) {
			AreaBullet::OverlappingObjectData &otherObj = area->overlappingObjects[i];
			// This check prevent the overwrite of ENTER state
			// if this function is called more times before dispatchCallbacks
			if (otherObj.state != AreaBullet::OVERLAP_STATE_ENTER) {
				otherObj.state = AreaBullet::OVERLAP_STATE_DIRTY;
			}
		}

		const btAlignedObjectArray<btCollisionObject *> ghostOverlaps = ghost->getOverlappingPairs();

		for (i = ghostOverlaps.size() - 1; 0 <= i; --i) {
			otherObject = static_cast<CollisionObjectBullet *>(ghostOverlaps[i]->getUserPointer());
			indexOverlap = area->find_overlapping_object(otherObject);
			if (-1 == indexOverlap) {
				// Not found
				area->add_overlap(otherObject);
			} else {
				// Found
				area->put_overlap_as_inside(indexOverlap);
			}
		}

		for (i = area->overlappingObjects.size() - 1; 0 <= i; --i) {
			// If the overlap has DIRTY state it means that it's no more overlapping
			if (area->overlappingObjects[i].state == AreaBullet::OVERLAP_STATE_DIRTY) {
				area->put_overlap_as_exit(i);
			}
		}
	}
}

void SpaceBullet::check_body_collision() {
#ifdef DEBUG_ENABLED
	reset_debug_contact_count();
#endif

	const int numManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
	for (int i = 0; i < numManifolds; ++i) {
		btPersistentManifold *contactManifold = dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
		const btCollisionObject *obA = contactManifold->getBody0();
		const btCollisionObject *obB = contactManifold->getBody1();

		if (btCollisionObject::CO_RIGID_BODY != obA->getInternalType() || btCollisionObject::CO_RIGID_BODY != obB->getInternalType()) {
			// This checks is required to be sure the ghost object is skipped
			// The ghost object "getUserPointer" return the BodyBullet owner so this check is required
			continue;
		}

		// Asserts all Godot objects are assigned
		assert(NULL != obA->getUserPointer());
		assert(NULL != obB->getUserPointer());

		// I know this static cast is a bit risky. But I'm checking its type just after it.
		// This allow me to avoid a lot of other cast and checks
		BodyBullet *bodyA = static_cast<BodyBullet *>(obA->getUserPointer());
		BodyBullet *bodyB = static_cast<BodyBullet *>(obB->getUserPointer());

		if (CollisionObjectBullet::TYPE_BODY == bodyA->getType() && CollisionObjectBullet::TYPE_BODY == bodyB->getType()) {
			if (!bodyA->can_add_collision() && !bodyB->can_add_collision()) {
				continue;
			}

			const int numContacts = contactManifold->getNumContacts();
#define REPORT_ALL_CONTACTS 0
#if REPORT_ALL_CONTACTS
			for (int j = 0; j < numContacts; j++) {
				btManifoldPoint &pt = contactManifold->getContactPoint(j);
#else
			// Since I don't need report all contacts for these objects, I'll report only the first
			if (numContacts) {
				btManifoldPoint &pt = contactManifold->getContactPoint(0);
#endif
				Vector3 collisionWorldPosition;
				Vector3 collisionLocalPosition;
				Vector3 normalOnB;
				B_TO_G(pt.m_normalWorldOnB, normalOnB);

				if (bodyA->can_add_collision()) {
					B_TO_G(pt.getPositionWorldOnB(), collisionWorldPosition);
					B_TO_G(pt.m_localPointA, collisionLocalPosition);
					bodyA->add_collision_object(bodyB, collisionWorldPosition, collisionLocalPosition, normalOnB, pt.m_index1, pt.m_index0);
				}
				if (bodyB->can_add_collision()) {
					B_TO_G(pt.getPositionWorldOnA(), collisionWorldPosition);
					B_TO_G(pt.m_localPointB, collisionLocalPosition);
					bodyB->add_collision_object(bodyA, collisionWorldPosition, collisionLocalPosition, normalOnB * -1, pt.m_index0, pt.m_index1);
				}

#ifdef DEBUG_ENABLED
				if (is_debugging_contacts()) {
					add_debug_contact(collisionWorldPosition);
				}
#endif
			}
		}
	}
}

void SpaceBullet::update_gravity() {
	btVector3 btGravity;
	G_TO_B(gravityDirection * gravityMagnitude, btGravity);
	dynamicsWorld->setGravity(btGravity);
}

class HackBtCompoundShape : public btCompoundShape {
public:
	void get_local_half_extents(btVector3 &local_half_extents) {
		// This function is required since localAabb data are protected
		local_half_extents = btScalar(0.5) * (m_localAabbMax - m_localAabbMin);
	}
};

/// IMPORTANT: Please don't turn it ON this is not managed correctly!!
/// I'm leaving this here just for future tests.
/// Debug motion and normal vector drawing
#define debug_test_motion 0
#if debug_test_motion
static ImmediateGeometry *motionVec(NULL);
static ImmediateGeometry *normalLine(NULL);
static Ref<SpatialMaterial> red_mat;
static Ref<SpatialMaterial> blue_mat;
#endif

#define IGNORE_AREAS_TRUE true
bool SpaceBullet::test_body_motion(BodyBullet *p_body, const Transform &p_from, const Vector3 &p_motion, real_t p_margin, PhysicsServer::MotionResult *r_result) {

#if debug_test_motion
	/// Yes I know this is not good, but I've used it as fast debugging.
	/// I'm leaving it here just for speedup the other eventual debugs
	if (!normalLine) {
		motionVec = memnew(ImmediateGeometry);
		normalLine = memnew(ImmediateGeometry);
		SceneTree::get_singleton()->get_current_scene()->add_child(motionVec);
		SceneTree::get_singleton()->get_current_scene()->add_child(normalLine);

		red_mat = Ref<SpatialMaterial>(memnew(SpatialMaterial));
		red_mat->set_flag(SpatialMaterial::FLAG_UNSHADED, true);
		red_mat->set_line_width(20.0);
		red_mat->set_feature(SpatialMaterial::FEATURE_TRANSPARENT, true);
		red_mat->set_flag(SpatialMaterial::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
		red_mat->set_flag(SpatialMaterial::FLAG_SRGB_VERTEX_COLOR, true);
		red_mat->set_albedo(Color(1, 0, 0, 1));
		motionVec->set_material_override(red_mat);

		blue_mat = Ref<SpatialMaterial>(memnew(SpatialMaterial));
		blue_mat->set_flag(SpatialMaterial::FLAG_UNSHADED, true);
		blue_mat->set_line_width(20.0);
		blue_mat->set_feature(SpatialMaterial::FEATURE_TRANSPARENT, true);
		blue_mat->set_flag(SpatialMaterial::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
		blue_mat->set_flag(SpatialMaterial::FLAG_SRGB_VERTEX_COLOR, true);
		blue_mat->set_albedo(Color(0, 0, 1, 1));
		normalLine->set_material_override(blue_mat);
	}
#endif

	///// Release all generated manifolds
	//{
	//    if(p_body->get_kinematic_utilities()){
	//        for(int i= p_body->get_kinematic_utilities()->m_generatedManifold.size()-1; 0<=i; --i){
	//            dispatcher->releaseManifold( p_body->get_kinematic_utilities()->m_generatedManifold[i] );
	//        }
	//        p_body->get_kinematic_utilities()->m_generatedManifold.clear();
	//    }
	//}

	btVector3 recover_initial_position;
	recover_initial_position.setZero();

/// I'm performing the unstack at the end of movement so I'm sure the player is unstacked even after the movement.
///  I've removed the initial unstack because this is useful just for the first tick since after the first
///  the real unstack is performed at the end of process.
/// However I'm leaving here the old code.
///  Note: It has a bug when two shapes touches something simultaneously the body is moved too much away (I'm not fixing it for the reason written above).
#define INITIAL_UNSTACK 0
#if !INITIAL_UNSTACK
	btTransform body_safe_position;
	G_TO_B(p_from, body_safe_position);
//btTransform body_unsafe_positino;
//G_TO_B(p_from, body_unsafe_positino);
#else
	btTransform body_safe_position;
	btTransform body_unsafe_positino;
	{ /// Phase one - multi shapes depenetration using margin
		G_TO_B(p_from, body_safe_position);
		G_TO_B(p_from, body_unsafe_positino);

		// MAX_PENETRATION_DEPTH Is useful have the ghost a bit penetrated so I can detect the floor easily
		recover_from_penetration(p_body, body_safe_position, MAX_PENETRATION_DEPTH, /* p_depenetration_speed */ 1, recover_initial_position);

		/// Not required if I put p_depenetration_speed = 1
		//for(int t = 0; t<4; ++t){
		//    if(!recover_from_penetration(p_body, body_safe_position, MAX_PENETRATION_DEPTH, /* p_depenetration_speed */0.2, recover_initial_position)){
		//       break;
		//    }
		//}

		// Add recover position to "From" and "To" transforms
		body_safe_position.getOrigin() += recover_initial_position;
	}
#endif

	int shape_most_recovered(-1);
	btVector3 recovered_motion;
	G_TO_B(p_motion, recovered_motion);
	const int shape_count(p_body->get_shape_count());

	{ /// phase two - sweep test, from a secure position without margin

#if debug_test_motion
		Vector3 sup_line;
		B_TO_G(body_safe_position.getOrigin(), sup_line);
		motionVec->clear();
		motionVec->begin(Mesh::PRIMITIVE_LINES, NULL);
		motionVec->add_vertex(sup_line);
		motionVec->add_vertex(sup_line + p_motion * 10);
		motionVec->end();
#endif

		for (int shIndex = 0; shIndex < shape_count; ++shIndex) {
			if (p_body->is_shape_disabled(shIndex)) {
				continue;
			}

			btConvexShape *convex_shape_test(dynamic_cast<btConvexShape *>(p_body->get_bt_shape(shIndex)));
			if (!convex_shape_test) {
				// Skip no convex shape
				continue;
			}

			btTransform shape_xform_from;
			G_TO_B(p_body->get_shape_transform(shIndex), shape_xform_from);
			//btTransform shape_xform_to(shape_xform_from);

			// Add local shape transform
			shape_xform_from.getOrigin() += body_safe_position.getOrigin();
			shape_xform_from.getBasis() *= body_safe_position.getBasis();

			btTransform shape_xform_to(shape_xform_from);
			//shape_xform_to.getOrigin() += body_unsafe_positino.getOrigin();
			//shape_xform_to.getBasis() *= body_unsafe_positino.getBasis();
			shape_xform_to.getOrigin() += recovered_motion;

			GodotClosestConvexResultCallback btResult(shape_xform_from.getOrigin(), shape_xform_to.getOrigin(), p_body, IGNORE_AREAS_TRUE);
			btResult.m_collisionFilterGroup = p_body->get_collision_layer();
			btResult.m_collisionFilterMask = p_body->get_collision_mask();

			dynamicsWorld->convexSweepTest(convex_shape_test, shape_xform_from, shape_xform_to, btResult);

			if (btResult.hasHit()) {
				//recovered_motion *= btResult.m_closestHitFraction;
				/// Since for each sweep test I fix the motion of new shapes in base the recover result,
				/// if another shape will hit something it means that has a deepest recovering respect the previous shape
				shape_most_recovered = shIndex;
			}
		}
	}

	bool hasHit = false;

	{ /// Phase three - contact test with margin

		btGhostObject *ghost = p_body->get_kinematic_utilities()->m_ghostObject;

		GodotRecoverAndClosestContactResultCallback result_callabck;

		if (false && 0 <= shape_most_recovered) {
			result_callabck.m_self_object = p_body;
			result_callabck.m_ignore_areas = IGNORE_AREAS_TRUE;
			result_callabck.m_collisionFilterGroup = p_body->get_collision_layer();
			result_callabck.m_collisionFilterMask = p_body->get_collision_mask();

			const BodyBullet::KinematicShape &kin(p_body->get_kinematic_utilities()->m_shapes[shape_most_recovered]);
			ghost->setCollisionShape(kin.shape);
			ghost->setWorldTransform(body_safe_position);

			ghost->getWorldTransform().getOrigin() += recovered_motion;
			ghost->getWorldTransform().getOrigin() += kin.transform.getOrigin();
			ghost->getWorldTransform().getBasis() *= kin.transform.getBasis();

			dynamicsWorld->contactTest(ghost, result_callabck);

			recovered_motion += result_callabck.m_recover_penetration; // Required to avoid all kind of penetration

		} else {
			// The sweep result does not return a penetrated shape, so I've to check all shapes
			// Then return the most penetrated shape

			GodotRecoverAndClosestContactResultCallback iter_result_callabck(p_body, IGNORE_AREAS_TRUE);
			iter_result_callabck.m_collisionFilterGroup = p_body->get_collision_layer();
			iter_result_callabck.m_collisionFilterMask = p_body->get_collision_mask();

			btScalar max_penetration(99999999999);
			for (int i = 0; i < shape_count; ++i) {

				const BodyBullet::KinematicShape &kin(p_body->get_kinematic_utilities()->m_shapes[i]);
				if (!kin.is_active()) {
					continue;
				}

				// reset callback each function
				iter_result_callabck.reset();

				ghost->setCollisionShape(kin.shape);
				ghost->setWorldTransform(body_safe_position);
				ghost->getWorldTransform().getOrigin() += recovered_motion;
				ghost->getWorldTransform().getOrigin() += kin.transform.getOrigin();
				ghost->getWorldTransform().getBasis() *= kin.transform.getBasis();

				dynamicsWorld->contactTest(ghost, iter_result_callabck);

				if (iter_result_callabck.hasHit()) {
					if (max_penetration > iter_result_callabck.m_penetration_distance) {
						max_penetration = iter_result_callabck.m_penetration_distance;
						shape_most_recovered = i;
						// This is more penetrated
						result_callabck.m_pointCollisionObject = iter_result_callabck.m_pointCollisionObject;
						result_callabck.m_pointNormalWorld = iter_result_callabck.m_pointNormalWorld;
						result_callabck.m_pointWorld = iter_result_callabck.m_pointWorld;
						result_callabck.m_penetration_distance = iter_result_callabck.m_penetration_distance;
						result_callabck.m_other_compound_shape_index = iter_result_callabck.m_other_compound_shape_index;

						recovered_motion += iter_result_callabck.m_recover_penetration; // Required to avoid all kind of penetration
					}
				}
			}
		}

		hasHit = result_callabck.hasHit();

		if (r_result) {

			B_TO_G(recovered_motion + recover_initial_position, r_result->motion);

			if (hasHit) {

				if (btCollisionObject::CO_RIGID_BODY != result_callabck.m_pointCollisionObject->getInternalType()) {
					ERR_PRINT("The collision is not against a rigid body. Please check what's going on.");
					goto EndExecution;
				}
				const btRigidBody *btRigid = static_cast<const btRigidBody *>(result_callabck.m_pointCollisionObject);
				CollisionObjectBullet *collisionObject = static_cast<CollisionObjectBullet *>(btRigid->getUserPointer());

				r_result->remainder = p_motion - r_result->motion; // is the remaining movements
				B_TO_G(result_callabck.m_pointWorld, r_result->collision_point);
				B_TO_G(result_callabck.m_pointNormalWorld, r_result->collision_normal);
				B_TO_G(btRigid->getVelocityInLocalPoint(result_callabck.m_pointWorld - btRigid->getWorldTransform().getOrigin()), r_result->collider_velocity); // It calculates velocity at point and assign it using special function Bullet_to_Godot
				r_result->collider = collisionObject->get_self();
				r_result->collider_id = collisionObject->get_instance_id();
				r_result->collider_shape = result_callabck.m_other_compound_shape_index;
				r_result->collision_local_shape = shape_most_recovered;

//{ /// Add manifold point to manage collisions
//    btPersistentManifold* manifold = dynamicsWorld->getDispatcher()->getNewManifold(p_body->getBtBody(), btRigid);
//    btManifoldPoint manifoldPoint(result_callabck.m_pointWorld, result_callabck.m_pointWorld, result_callabck.m_pointNormalWorld, result_callabck.m_penetration_distance);
//    manifoldPoint.m_index0 = r_result->collision_local_shape;
//    manifoldPoint.m_index1 = r_result->collider_shape;
//    manifold->addManifoldPoint(manifoldPoint);
//    p_body->get_kinematic_utilities()->m_generatedManifold.push_back(manifold);
//}

#if debug_test_motion
				Vector3 sup_line2;
				B_TO_G(recovered_motion, sup_line2);
				//Vector3 sup_pos;
				//B_TO_G( pt.getPositionWorldOnB(), sup_pos);
				normalLine->clear();
				normalLine->begin(Mesh::PRIMITIVE_LINES, NULL);
				normalLine->add_vertex(r_result->collision_point);
				normalLine->add_vertex(r_result->collision_point + r_result->collision_normal * 10);
				normalLine->end();
#endif

			} else {
				r_result->remainder = Vector3();
			}
		}
	}

EndExecution:
	p_body->get_kinematic_utilities()->resetDefShape();
	return hasHit;
}

///  Note: It has a bug when two shapes touches something simultaneously the body is moved too much away
/// (I'm not fixing it because I don't use it).
bool SpaceBullet::recover_from_penetration(BodyBullet *p_body, const btTransform &p_from, btScalar p_maxPenetrationDepth, btScalar p_depenetration_speed, btVector3 &out_recover_position) {

	bool penetration = false;
	btPairCachingGhostObject *ghost = p_body->get_kinematic_utilities()->m_ghostObject;

	for (int kinIndex = p_body->get_kinematic_utilities()->m_shapes.size() - 1; 0 <= kinIndex; --kinIndex) {
		const BodyBullet::KinematicShape &kin_shape(p_body->get_kinematic_utilities()->m_shapes[kinIndex]);
		if (!kin_shape.is_active()) {
			continue;
		}

		btConvexShape *convexShape = kin_shape.shape;
		btTransform shape_xform(kin_shape.transform);

		// from local to world
		shape_xform.getOrigin() += p_from.getOrigin();
		shape_xform.getBasis() *= p_from.getBasis();

		// Apply last recovery to avoid doubling the recovering
		shape_xform.getOrigin() += out_recover_position;

		ghost->setCollisionShape(convexShape);
		ghost->setWorldTransform(shape_xform);

		btVector3 minAabb, maxAabb;
		convexShape->getAabb(shape_xform, minAabb, maxAabb);
		dynamicsWorld->getBroadphase()->setAabb(ghost->getBroadphaseHandle(),
				minAabb,
				maxAabb,
				dynamicsWorld->getDispatcher());

		dynamicsWorld->getDispatcher()->dispatchAllCollisionPairs(ghost->getOverlappingPairCache(), dynamicsWorld->getDispatchInfo(), dynamicsWorld->getDispatcher());

		for (int i = 0; i < ghost->getOverlappingPairCache()->getNumOverlappingPairs(); ++i) {
			p_body->get_kinematic_utilities()->m_manifoldArray.resize(0);

			btBroadphasePair *collisionPair = &ghost->getOverlappingPairCache()->getOverlappingPairArray()[i];

			btCollisionObject *obj0 = static_cast<btCollisionObject *>(collisionPair->m_pProxy0->m_clientObject);
			btCollisionObject *obj1 = static_cast<btCollisionObject *>(collisionPair->m_pProxy1->m_clientObject);

			if ((obj0 && !obj0->hasContactResponse()) || (obj1 && !obj1->hasContactResponse()))
				continue;

			// This is not required since the dispatched does all the job
			//if (!needsCollision(obj0, obj1))
			//    continue;

			if (collisionPair->m_algorithm)
				collisionPair->m_algorithm->getAllContactManifolds(p_body->get_kinematic_utilities()->m_manifoldArray);

			for (int j = 0; j < p_body->get_kinematic_utilities()->m_manifoldArray.size(); ++j) {

				btPersistentManifold *manifold = p_body->get_kinematic_utilities()->m_manifoldArray[j];
				btScalar directionSign = manifold->getBody0() == ghost ? btScalar(-1.0) : btScalar(1.0);
				for (int p = 0; p < manifold->getNumContacts(); ++p) {
					const btManifoldPoint &pt = manifold->getContactPoint(p);

					btScalar dist = pt.getDistance();
					if (dist < -p_maxPenetrationDepth) {
						penetration = true;
						out_recover_position += pt.m_normalWorldOnB * directionSign * (dist + p_maxPenetrationDepth) * p_depenetration_speed;
						//print_line("penetrate distance: " + rtos(dist));
					}
					//else {
					//    print_line("touching distance: " + rtos(dist));
					//}
				}
			}
		}
	}

	p_body->get_kinematic_utilities()->resetDefShape();
	return penetration;
}
