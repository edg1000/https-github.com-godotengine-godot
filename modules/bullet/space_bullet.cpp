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
#include "BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "BulletCollision/NarrowPhaseCollision/btGjkEpaPenetrationDepthSolver.h"
#include "BulletCollision/NarrowPhaseCollision/btGjkPairDetector.h"
#include "BulletCollision/NarrowPhaseCollision/btPointCollector.h"
#include "BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h"
#include "BulletSoftBody/btSoftRigidDynamicsWorld.h"
#include "btBulletDynamicsCommon.h"
#include "bullet_physics_server.h"
#include "bullet_types_converter.h"
#include "bullet_utilities.h"
#include "constraint_bullet.h"
#include "godot_collision_dispatcher.h"
#include "rigid_body_bullet.h"
#include "servers/physics_server.h"
#include "soft_body_bullet.h"
#include "ustring.h"
#include <assert.h>

// test only
//#include "scene/3d/immediate_geometry.h"

bool GodotFilterCallback::test_collision_filters(uint32_t body0_collision_layer, uint32_t body0_collision_mask, uint32_t body1_collision_layer, uint32_t body1_collision_mask) {
	return body0_collision_layer & body1_collision_mask || body1_collision_layer & body0_collision_mask;
}

bool GodotFilterCallback::needBroadphaseCollision(btBroadphaseProxy *proxy0, btBroadphaseProxy *proxy1) const {
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
	int count;
	const Set<RID> *m_exclude;

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

		result.shape = convexResult.m_localShapeInfo->m_shapePart;
		result.rid = gObj->get_self();
		result.collider_id = gObj->get_instance_id();
		result.collider = 0 == result.collider_id ? NULL : ObjectDB::get_instance(result.collider_id);

		++count;
		return count < m_resultMax;
	}
};

struct GodotKinClosestConvexResultCallback : public btCollisionWorld::ClosestConvexResultCallback {
public:
	const RigidBodyBullet *m_self_object;
	const bool m_ignore_areas;

	GodotKinClosestConvexResultCallback(const btVector3 &convexFromWorld, const btVector3 &convexToWorld, const RigidBodyBullet *p_self_object, bool p_ignore_areas)
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

struct GodotClosestConvexResultCallback : public btCollisionWorld::ClosestConvexResultCallback {
public:
	const Set<RID> *m_exclude;
	int m_shapePart;

	GodotClosestConvexResultCallback(const btVector3 &convexFromWorld, const btVector3 &convexToWorld, const Set<RID> *p_exclude)
		: btCollisionWorld::ClosestConvexResultCallback(convexFromWorld, convexToWorld), m_exclude(p_exclude) {}

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
		btScalar res = btCollisionWorld::ClosestConvexResultCallback::addSingleResult(convexResult, normalInWorldSpace);
		m_shapePart = convexResult.m_localShapeInfo->m_shapePart;
		return res;
	}
};

struct GodotAllContactResultCallback : public btCollisionWorld::ContactResultCallback {
public:
	const btCollisionObject *m_self_object;
	PhysicsDirectSpaceState::ShapeResult *m_results;
	int m_resultMax;
	int m_count;
	const Set<RID> *m_exclude;

	GodotAllContactResultCallback(btCollisionObject *p_self_object, PhysicsDirectSpaceState::ShapeResult *p_results, int p_resultMax, const Set<RID> *p_exclude)
		: m_self_object(p_self_object), m_results(p_results), m_exclude(p_exclude), m_resultMax(p_resultMax), m_count(0) {}

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

	virtual btScalar addSingleResult(btManifoldPoint &cp, const btCollisionObjectWrapper *colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper *colObj1Wrap, int partId1, int index1) {

		if (cp.getDistance() <= 0) {

			PhysicsDirectSpaceState::ShapeResult &result = m_results[m_count];
			// Penetrated

			CollisionObjectBullet *colObj;
			if (m_self_object == colObj0Wrap->getCollisionObject()) {
				colObj = static_cast<CollisionObjectBullet *>(colObj1Wrap->getCollisionObject()->getUserPointer());
				result.shape = cp.m_index1;
			} else {
				colObj = static_cast<CollisionObjectBullet *>(colObj0Wrap->getCollisionObject()->getUserPointer());
				result.shape = cp.m_index0;
			}

			if (colObj)
				result.collider_id = colObj->get_instance_id();
			else
				result.collider_id = 0;
			result.collider = 0 == result.collider_id ? NULL : ObjectDB::get_instance(result.collider_id);
			result.rid = colObj->get_self();
			++m_count;
		}

		return m_count < m_resultMax;
	}
};

/// Returns the list of contacts pairs in this order: Local contact, other body contact
struct GodotContactPairContactResultCallback : public btCollisionWorld::ContactResultCallback {
public:
	const btCollisionObject *m_self_object;
	Vector3 *m_results;
	int m_resultMax;
	int m_count;
	const Set<RID> *m_exclude;

	GodotContactPairContactResultCallback(btCollisionObject *p_self_object, Vector3 *p_results, int p_resultMax, const Set<RID> *p_exclude)
		: m_self_object(p_self_object), m_results(p_results), m_exclude(p_exclude), m_resultMax(p_resultMax), m_count(0) {}

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

	virtual btScalar addSingleResult(btManifoldPoint &cp, const btCollisionObjectWrapper *colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper *colObj1Wrap, int partId1, int index1) {

		if (m_self_object == colObj0Wrap->getCollisionObject()) {
			B_TO_G(cp.m_localPointA, m_results[m_count * 2 + 0]); // Local contact
			B_TO_G(cp.m_localPointB, m_results[m_count * 2 + 1]);
		} else {
			B_TO_G(cp.m_localPointB, m_results[m_count * 2 + 0]); // Local contact
			B_TO_G(cp.m_localPointA, m_results[m_count * 2 + 1]);
		}

		++m_count;

		return m_count < m_resultMax;
	}
};

struct GodotRestInfoContactResultCallback : public btCollisionWorld::ContactResultCallback {
public:
	const btCollisionObject *m_self_object;
	PhysicsDirectSpaceState::ShapeRestInfo *m_result;
	bool m_collided;
	real_t m_min_distance;
	const btCollisionObject *m_rest_info_collision_object;
	btVector3 m_rest_info_bt_point;
	const Set<RID> *m_exclude;

	GodotRestInfoContactResultCallback(btCollisionObject *p_self_object, PhysicsDirectSpaceState::ShapeRestInfo *p_result, const Set<RID> *p_exclude)
		: m_self_object(p_self_object), m_result(p_result), m_exclude(p_exclude), m_collided(false), m_min_distance(0) {}

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

	virtual btScalar addSingleResult(btManifoldPoint &cp, const btCollisionObjectWrapper *colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper *colObj1Wrap, int partId1, int index1) {

		if (cp.getDistance() <= m_min_distance) {
			m_min_distance = cp.getDistance();

			CollisionObjectBullet *colObj;
			if (m_self_object == colObj0Wrap->getCollisionObject()) {
				colObj = static_cast<CollisionObjectBullet *>(colObj1Wrap->getCollisionObject()->getUserPointer());
				m_result->shape = cp.m_index1;
				B_TO_G(cp.getPositionWorldOnB(), m_result->point);
				m_rest_info_bt_point = cp.getPositionWorldOnB();
				m_rest_info_collision_object = colObj1Wrap->getCollisionObject();
			} else {
				colObj = static_cast<CollisionObjectBullet *>(colObj0Wrap->getCollisionObject()->getUserPointer());
				m_result->shape = cp.m_index0;
				B_TO_G(cp.m_normalWorldOnB * -1, m_result->normal);
				m_rest_info_bt_point = cp.getPositionWorldOnA();
				m_rest_info_collision_object = colObj0Wrap->getCollisionObject();
			}

			if (colObj)
				m_result->collider_id = colObj->get_instance_id();
			else
				m_result->collider_id = 0;
			m_result->rid = colObj->get_self();

			m_collided = true;
		}

		return cp.getDistance();
	}
};

struct GodotRecoverAndClosestContactResultCallback : public btCollisionWorld::ContactResultCallback {
public:
	btVector3 m_pointNormalWorld;
	btVector3 m_pointWorld;
	btScalar m_penetration_distance;
	int m_other_compound_shape_index;
	const btCollisionObject *m_pointCollisionObject;

	const RigidBodyBullet *m_self_object;
	bool m_ignore_areas;

	btScalar m_most_penetrated_distance;
	btVector3 m_recover_penetration;

	GodotRecoverAndClosestContactResultCallback()
		: m_pointCollisionObject(NULL), m_penetration_distance(0), m_other_compound_shape_index(0), m_self_object(NULL), m_ignore_areas(true), m_most_penetrated_distance(1e20), m_recover_penetration(0, 0, 0) {}

	GodotRecoverAndClosestContactResultCallback(const RigidBodyBullet *p_self_object, bool p_ignore_areas)
		: m_pointCollisionObject(NULL), m_penetration_distance(0), m_other_compound_shape_index(0), m_self_object(p_self_object), m_ignore_areas(p_ignore_areas), m_most_penetrated_distance(9999999999), m_recover_penetration(0, 0, 0) {}

	void reset() {
		m_pointCollisionObject = NULL;
		m_most_penetrated_distance = 1e20;
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

	if (p_result_max <= 0)
		return 0;

	btVector3 bt_point;
	G_TO_B(p_point, bt_point);

	btSphereShape sphere_point(0.f);
	btCollisionObject collision_object_point;
	collision_object_point.setCollisionShape(&sphere_point);
	collision_object_point.setWorldTransform(btTransform(btQuaternion::getIdentity(), bt_point));

	// Setup query
	GodotAllContactResultCallback btResult(&collision_object_point, r_results, p_result_max, &p_exclude);
	btResult.m_collisionFilterGroup = p_collision_layer;
	btResult.m_collisionFilterMask = p_object_type_mask;
	space->dynamicsWorld->contactTest(&collision_object_point, btResult);

	// The results is already populated by GodotAllConvexResultCallback
	return btResult.m_count;
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
			r_result.shape = 0;
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

	btVector3 scale_with_margin;
	G_TO_B(p_xform.basis.get_scale(), scale_with_margin);
	btConvex->setLocalScaling(scale_with_margin);

	btTransform bt_xform;
	G_TO_B(p_xform, bt_xform);

	btCollisionObject collision_object;
	collision_object.setCollisionShape(btConvex);
	collision_object.setWorldTransform(bt_xform);

	GodotAllContactResultCallback btQuery(&collision_object, p_results, p_result_max, &p_exclude);
	btQuery.m_collisionFilterGroup = p_collision_layer;
	btQuery.m_collisionFilterMask = p_object_type_mask;
	btQuery.m_closestDistanceThreshold = p_margin;
	space->dynamicsWorld->contactTest(&collision_object, btQuery);

	bulletdelete(btConvex);

	return btQuery.m_count;
}

bool BulletPhysicsDirectSpaceState::cast_motion(const RID &p_shape, const Transform &p_xform, const Vector3 &p_motion, float p_margin, float &p_closest_safe, float &p_closest_unsafe, const Set<RID> &p_exclude, uint32_t p_collision_layer, uint32_t p_object_type_mask, ShapeRestInfo *r_info) {
	ShapeBullet *shape = space->get_physics_server()->get_shape_owner()->get(p_shape);

	btConvexShape *bt_convex_shape = dynamic_cast<btConvexShape *>(shape->create_bt_shape());
	if (!bt_convex_shape) {
		bulletdelete(bt_convex_shape);
		ERR_PRINTS("The shape is not a convex shape, then is not supported: shape type: " + itos(shape->get_type()));
		return 0;
	}

	btVector3 bt_motion;
	G_TO_B(p_motion, bt_motion);

	btVector3 scale_with_margin;
	G_TO_B(p_xform.basis.get_scale() + Vector3(p_margin, p_margin, p_margin), scale_with_margin);
	bt_convex_shape->setLocalScaling(scale_with_margin);

	btTransform bt_xform_from;
	G_TO_B(p_xform, bt_xform_from);

	btTransform bt_xform_to(bt_xform_from);
	bt_xform_to.getOrigin() += bt_motion;

	GodotClosestConvexResultCallback btResult(bt_xform_from.getOrigin(), bt_xform_to.getOrigin(), &p_exclude);
	btResult.m_collisionFilterGroup = p_collision_layer;
	btResult.m_collisionFilterMask = p_object_type_mask;

	space->dynamicsWorld->convexSweepTest(bt_convex_shape, bt_xform_from, bt_xform_to, btResult);

	if (btResult.hasHit()) {
		if (btCollisionObject::CO_RIGID_BODY == btResult.m_hitCollisionObject->getInternalType()) {
			B_TO_G(static_cast<const btRigidBody *>(btResult.m_hitCollisionObject)->getVelocityInLocalPoint(btResult.m_hitPointWorld), r_info->linear_velocity);
		}
		CollisionObjectBullet *collision_object = static_cast<CollisionObjectBullet *>(btResult.m_hitCollisionObject->getUserPointer());
		p_closest_safe = p_closest_unsafe = btResult.m_closestHitFraction;
		B_TO_G(btResult.m_hitPointWorld, r_info->point);
		B_TO_G(btResult.m_hitNormalWorld, r_info->normal);
		r_info->rid = collision_object->get_self();
		r_info->collider_id = collision_object->get_instance_id();
		r_info->shape = btResult.m_shapePart;
	}

	bulletdelete(bt_convex_shape);
	return btResult.hasHit();
}

/// Returns the list of contacts pairs in this order: Local contact, other body contact
bool BulletPhysicsDirectSpaceState::collide_shape(RID p_shape, const Transform &p_shape_xform, float p_margin, Vector3 *r_results, int p_result_max, int &r_result_count, const Set<RID> &p_exclude, uint32_t p_collision_layer, uint32_t p_object_type_mask) {
	if (p_result_max <= 0)
		return 0;

	ShapeBullet *shape = space->get_physics_server()->get_shape_owner()->get(p_shape);

	btConvexShape *btConvex = dynamic_cast<btConvexShape *>(shape->create_bt_shape());
	if (!btConvex) {
		bulletdelete(btConvex);
		ERR_PRINTS("The shape is not a convex shape, then is not supported: shape type: " + itos(shape->get_type()));
		return 0;
	}

	btVector3 scale_with_margin;
	G_TO_B(p_shape_xform.basis.get_scale(), scale_with_margin);
	btConvex->setLocalScaling(scale_with_margin);

	btTransform bt_xform;
	G_TO_B(p_shape_xform, bt_xform);

	btCollisionObject collision_object;
	collision_object.setCollisionShape(btConvex);
	collision_object.setWorldTransform(bt_xform);

	GodotContactPairContactResultCallback btQuery(&collision_object, r_results, p_result_max, &p_exclude);
	btQuery.m_collisionFilterGroup = p_collision_layer;
	btQuery.m_collisionFilterMask = p_object_type_mask;
	btQuery.m_closestDistanceThreshold = p_margin;
	space->dynamicsWorld->contactTest(&collision_object, btQuery);

	r_result_count = btQuery.m_count;
	bulletdelete(btConvex);

	return btQuery.m_count;
}

bool BulletPhysicsDirectSpaceState::rest_info(RID p_shape, const Transform &p_shape_xform, float p_margin, ShapeRestInfo *r_info, const Set<RID> &p_exclude, uint32_t p_collision_layer, uint32_t p_object_type_mask) {

	ShapeBullet *shape = space->get_physics_server()->get_shape_owner()->get(p_shape);

	btConvexShape *btConvex = dynamic_cast<btConvexShape *>(shape->create_bt_shape());
	if (!btConvex) {
		bulletdelete(btConvex);
		ERR_PRINTS("The shape is not a convex shape, then is not supported: shape type: " + itos(shape->get_type()));
		return 0;
	}

	btVector3 scale_with_margin;
	G_TO_B(p_shape_xform.basis.get_scale() + Vector3(p_margin, p_margin, p_margin), scale_with_margin);
	btConvex->setLocalScaling(scale_with_margin);

	btTransform bt_xform;
	G_TO_B(p_shape_xform, bt_xform);

	btCollisionObject collision_object;
	collision_object.setCollisionShape(btConvex);
	collision_object.setWorldTransform(bt_xform);

	GodotRestInfoContactResultCallback btQuery(&collision_object, r_info, &p_exclude);
	btQuery.m_collisionFilterGroup = p_collision_layer;
	btQuery.m_collisionFilterMask = p_object_type_mask;
	btQuery.m_closestDistanceThreshold = p_margin;
	space->dynamicsWorld->contactTest(&collision_object, btQuery);

	bulletdelete(btConvex);

	if (btQuery.m_collided) {
		if (btCollisionObject::CO_RIGID_BODY == btQuery.m_rest_info_collision_object->getInternalType()) {
			B_TO_G(static_cast<const btRigidBody *>(btQuery.m_rest_info_collision_object)->getVelocityInLocalPoint(btQuery.m_rest_info_bt_point), r_info->linear_velocity);
		}
		B_TO_G(btQuery.m_rest_info_bt_point, r_info->point);
	}

	return btQuery.m_collided;
}

Vector3 BulletPhysicsDirectSpaceState::get_closest_point_to_object_volume(RID p_object, const Vector3 p_point) const {

	RigidCollisionObjectBullet *rigid_object = space->get_physics_server()->get_rigid_collisin_object(p_object);
	ERR_FAIL_COND_V(!rigid_object, Vector3());

	btVector3 out_closest_point(0, 0, 0);
	btScalar out_distance = 1e20;

	btVector3 bt_point;
	G_TO_B(p_point, bt_point);

	btGjkEpaPenetrationDepthSolver gjk_epa_pen_solver;
	btVoronoiSimplexSolver gjk_simplex_solver;
	gjk_simplex_solver.setEqualVertexThreshold(0.);

	btSphereShape point_shape(0.);

	btCollisionShape *shape;
	btConvexShape *convex_shape;
	btTransform child_transform;
	btTransform body_transform(rigid_object->get_bt_collision_object()->getWorldTransform());

	btGjkPairDetector::ClosestPointInput input;
	input.m_transformA.getBasis().setIdentity();
	input.m_transformA.setOrigin(bt_point);

	bool shapes_found = false;

	btCompoundShape *compound = rigid_object->get_compound_shape();
	for (int i = compound->getNumChildShapes() - 1; 0 <= i; --i) {
		shape = compound->getChildShape(i);
		if (shape->isConvex()) {
			child_transform = compound->getChildTransform(i);
			convex_shape = static_cast<btConvexShape *>(shape);

			input.m_transformB = body_transform * child_transform;

			btPointCollector result;
			btGjkPairDetector gjk_pair_detector(&point_shape, convex_shape, &gjk_simplex_solver, &gjk_epa_pen_solver);
			gjk_pair_detector.getClosestPoints(input, result, 0);

			if (out_distance > result.m_distance) {
				out_distance = result.m_distance;
				out_closest_point = result.m_pointInWorld;
			}
		}
		shapes_found = true;
	}

	if (shapes_found) {

		Vector3 out;
		B_TO_G(out_closest_point, out);
		return out;
	} else {

		// no shapes found, use distance to origin.
		return rigid_object->get_transform().get_origin();
	}
}

SpaceBullet::SpaceBullet(bool p_create_soft_world)
	: broadphase(NULL),
	  dispatcher(NULL),
	  solver(NULL),
	  collisionConfiguration(NULL),
	  dynamicsWorld(NULL),
	  soft_body_world_info(NULL),
	  ghostPairCallback(NULL),
	  godotFilterCallback(NULL),
	  gravityDirection(0, -1, 0),
	  gravityMagnitude(10),
	  contactDebugCount(0) {

	create_empty_world(p_create_soft_world);
	direct_access = memnew(BulletPhysicsDirectSpaceState(this));
}

SpaceBullet::~SpaceBullet() {
	memdelete(direct_access);
	destroy_world();
}

void SpaceBullet::flush_queries() {
	const btCollisionObjectArray &colObjArray = dynamicsWorld->getCollisionObjectArray();
	for (int i = colObjArray.size() - 1; 0 <= i; --i) {
		static_cast<CollisionObjectBullet *>(colObjArray[i]->getUserPointer())->dispatch_callbacks();
	}
}

void SpaceBullet::step(real_t p_delta_time) {
	dynamicsWorld->stepSimulation(p_delta_time);
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

void SpaceBullet::add_rigid_body(RigidBodyBullet *p_body) {
	if (p_body->is_static()) {
		dynamicsWorld->addCollisionObject(p_body->get_bt_rigid_body(), p_body->get_collision_layer(), p_body->get_collision_mask());
	} else {
		dynamicsWorld->addRigidBody(p_body->get_bt_rigid_body(), p_body->get_collision_layer(), p_body->get_collision_mask());
	}
}

void SpaceBullet::remove_rigid_body(RigidBodyBullet *p_body) {
	if (p_body->is_static()) {
		dynamicsWorld->removeCollisionObject(p_body->get_bt_rigid_body());
	} else {
		dynamicsWorld->removeRigidBody(p_body->get_bt_rigid_body());
	}
}

void SpaceBullet::reload_collision_filters(RigidBodyBullet *p_body) {
	// This is necessary to change collision filter
	remove_rigid_body(p_body);
	add_rigid_body(p_body);
}

void SpaceBullet::add_soft_body(SoftBodyBullet *p_body) {
	if (is_using_soft_world()) {
		if (p_body->get_bt_soft_body()) {
			static_cast<btSoftRigidDynamicsWorld *>(dynamicsWorld)->addSoftBody(p_body->get_bt_soft_body(), p_body->get_collision_layer(), p_body->get_collision_mask());
		}
	} else {
		ERR_PRINT("This soft body can't be added to non soft world");
	}
}

void SpaceBullet::remove_soft_body(SoftBodyBullet *p_body) {
	if (is_using_soft_world()) {
		if (p_body->get_bt_soft_body()) {
			static_cast<btSoftRigidDynamicsWorld *>(dynamicsWorld)->removeSoftBody(p_body->get_bt_soft_body());
		}
	}
}

void SpaceBullet::reload_collision_filters(SoftBodyBullet *p_body) {
	// This is necessary to change collision filter
	remove_soft_body(p_body);
	add_soft_body(p_body);
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

void onBulletPreTickCallback(btDynamicsWorld *p_dynamicsWorld, btScalar timeStep) {
	static_cast<SpaceBullet *>(p_dynamicsWorld->getWorldUserInfo())->flush_queries();
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

void SpaceBullet::create_empty_world(bool p_create_soft_world) {
	assert(NULL == broadphase);
	assert(NULL == dispatcher);
	assert(NULL == solver);
	assert(NULL == collisionConfiguration);
	assert(NULL == dynamicsWorld);
	assert(NULL == ghostPairCallback);
	assert(NULL == godotFilterCallback);

	collisionConfiguration = p_create_soft_world ? bulletnew(btSoftBodyRigidBodyCollisionConfiguration) : bulletnew(btDefaultCollisionConfiguration);
	dispatcher = bulletnew(GodotCollisionDispatcher(collisionConfiguration));
	broadphase = bulletnew(btDbvtBroadphase);
	solver = bulletnew(btSequentialImpulseConstraintSolver);
	if (p_create_soft_world) {
		dynamicsWorld = bulletnew(btSoftRigidDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration));
		soft_body_world_info = bulletnew(btSoftBodyWorldInfo);
	} else {
		dynamicsWorld = bulletnew(btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration));
	}
	ghostPairCallback = bulletnew(btGhostPairCallback);
	godotFilterCallback = bulletnew(GodotFilterCallback);

	dynamicsWorld->setWorldUserInfo(this);

	dynamicsWorld->setInternalTickCallback(onBulletPreTickCallback, this, true);
	dynamicsWorld->setInternalTickCallback(onBulletTickCallback, this, false);
	dynamicsWorld->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(ghostPairCallback); // Setup ghost check
	dynamicsWorld->getPairCache()->setOverlapFilterCallback(godotFilterCallback);

	if (soft_body_world_info) {
		soft_body_world_info->m_broadphase = broadphase;
		soft_body_world_info->m_dispatcher = dispatcher;
		soft_body_world_info->m_sparsesdf.Initialize();
	}

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

	/// The world elements (like: Collision Objects, Constraints, Shapes) are managed by godot

	dynamicsWorld->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(NULL);
	dynamicsWorld->getPairCache()->setOverlapFilterCallback(NULL);

	bulletdelete(ghostPairCallback);
	bulletdelete(godotFilterCallback);
	bulletdelete(dynamicsWorld);
	bulletdelete(solver);
	bulletdelete(broadphase);
	bulletdelete(dispatcher);
	bulletdelete(collisionConfiguration);
	bulletdelete(soft_body_world_info);
}

void SpaceBullet::check_ghost_overlaps() {

	/// Algorith support variables
	btGjkEpaPenetrationDepthSolver gjk_epa_pen_solver;
	btVoronoiSimplexSolver gjk_simplex_solver;
	gjk_simplex_solver.setEqualVertexThreshold(0.f);
	btConvexShape *other_body_shape;
	btConvexShape *area_shape;
	btGjkPairDetector::ClosestPointInput gjk_input;
	AreaBullet *area;
	RigidCollisionObjectBullet *otherObject;
	int x(-1), i(-1), y(-1), z(-1), indexOverlap(-1);

	/// For each areas
	for (x = areas.size() - 1; 0 <= x; --x) {
		area = areas[x];

		if (!area->is_monitoring())
			continue;

		/// 1. Reset all states
		for (i = area->overlappingObjects.size() - 1; 0 <= i; --i) {
			AreaBullet::OverlappingObjectData &otherObj = area->overlappingObjects[i];
			// This check prevent the overwrite of ENTER state
			// if this function is called more times before dispatchCallbacks
			if (otherObj.state != AreaBullet::OVERLAP_STATE_ENTER) {
				otherObj.state = AreaBullet::OVERLAP_STATE_DIRTY;
			}
		}

		/// 2. Check all overlapping objects using GJK

		const btAlignedObjectArray<btCollisionObject *> ghostOverlaps = area->get_bt_ghost()->getOverlappingPairs();

		// For each overlapping
		for (i = ghostOverlaps.size() - 1; 0 <= i; --i) {

			if (!(ghostOverlaps[i]->getUserIndex() == CollisionObjectBullet::TYPE_RIGID_BODY || ghostOverlaps[i]->getUserIndex() == CollisionObjectBullet::TYPE_AREA))
				continue;

			otherObject = static_cast<RigidCollisionObjectBullet *>(ghostOverlaps[i]->getUserPointer());

			bool hasOverlap = false;

			// For each area shape
			for (y = area->get_compound_shape()->getNumChildShapes() - 1; 0 <= y; --y) {
				if (!area->get_compound_shape()->getChildShape(y)->isConvex())
					continue;

				gjk_input.m_transformA = area->get_transform__bullet() * area->get_compound_shape()->getChildTransform(y);
				area_shape = static_cast<btConvexShape *>(area->get_compound_shape()->getChildShape(y));

				// For each other object shape
				for (z = otherObject->get_compound_shape()->getNumChildShapes() - 1; 0 <= z; --z) {

					if (!otherObject->get_compound_shape()->getChildShape(z)->isConvex())
						continue;

					other_body_shape = static_cast<btConvexShape *>(otherObject->get_compound_shape()->getChildShape(z));
					gjk_input.m_transformB = otherObject->get_transform__bullet() * otherObject->get_compound_shape()->getChildTransform(z);

					btPointCollector result;
					btGjkPairDetector gjk_pair_detector(area_shape, other_body_shape, &gjk_simplex_solver, &gjk_epa_pen_solver);
					gjk_pair_detector.getClosestPoints(gjk_input, result, 0);

					if (0 >= result.m_distance) {
						hasOverlap = true;
						goto collision_found;
					}
				} // ~For each other object shape
			} // ~For each area shape

		collision_found:
			if (!hasOverlap)
				continue;

			indexOverlap = area->find_overlapping_object(otherObject);
			if (-1 == indexOverlap) {
				// Not found
				area->add_overlap(otherObject);
			} else {
				// Found
				area->put_overlap_as_inside(indexOverlap);
			}
		}

		/// 3. Remove not overlapping
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
		RigidBodyBullet *bodyA = static_cast<RigidBodyBullet *>(obA->getUserPointer());
		RigidBodyBullet *bodyB = static_cast<RigidBodyBullet *>(obB->getUserPointer());

		if (CollisionObjectBullet::TYPE_RIGID_BODY == bodyA->getType() && CollisionObjectBullet::TYPE_RIGID_BODY == bodyB->getType()) {
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
	if (soft_body_world_info) {
		soft_body_world_info->m_gravity = btGravity;
	}
}

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
bool SpaceBullet::test_body_motion(RigidBodyBullet *p_body, const Transform &p_from, const Vector3 &p_motion, real_t p_margin, PhysicsServer::MotionResult *r_result) {

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

			GodotKinClosestConvexResultCallback btResult(shape_xform_from.getOrigin(), shape_xform_to.getOrigin(), p_body, IGNORE_AREAS_TRUE);
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

			const RigidBodyBullet::KinematicShape &kin(p_body->get_kinematic_utilities()->m_shapes[shape_most_recovered]);
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

				const RigidBodyBullet::KinematicShape &kin(p_body->get_kinematic_utilities()->m_shapes[i]);
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
bool SpaceBullet::recover_from_penetration(RigidBodyBullet *p_body, const btTransform &p_from, btScalar p_maxPenetrationDepth, btScalar p_depenetration_speed, btVector3 &out_recover_position) {

	bool penetration = false;
	btPairCachingGhostObject *ghost = p_body->get_kinematic_utilities()->m_ghostObject;

	for (int kinIndex = p_body->get_kinematic_utilities()->m_shapes.size() - 1; 0 <= kinIndex; --kinIndex) {
		const RigidBodyBullet::KinematicShape &kin_shape(p_body->get_kinematic_utilities()->m_shapes[kinIndex]);
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
