/*************************************************************************/
/*  cone_twist_joint_bullet.cpp                                          */
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

#include "cone_twist_joint_bullet.h"
#include "BulletDynamics/ConstraintSolver/btConeTwistConstraint.h"
#include "body_bullet.h"
#include "bullet_types_converter.h"
#include "bullet_utilities.h"

ConeTwistJointBullet::ConeTwistJointBullet(BodyBullet *rbA, BodyBullet *rbB, const Transform &rbAFrame, const Transform &rbBFrame)
	: JointBullet() {
	btTransform btFrameA;
	G_TO_B(rbAFrame, btFrameA);
	if (rbB) {
		btTransform btFrameB;
		G_TO_B(rbBFrame, btFrameB);
		coneConstraint = bulletnew(btConeTwistConstraint(*rbA->get_bt_body(), *rbB->get_bt_body(), btFrameA, btFrameB));
	} else {
		coneConstraint = bulletnew(btConeTwistConstraint(*rbA->get_bt_body(), btFrameA));
	}
	setup(coneConstraint);
}

void ConeTwistJointBullet::set_angular_only(bool angularOnly) {
	coneConstraint->setAngularOnly(angularOnly);
}

void ConeTwistJointBullet::set_limit(real_t _swingSpan1, real_t _swingSpan2, real_t _twistSpan, real_t _softness, real_t _biasFactor, real_t _relaxationFactor) {
	coneConstraint->setLimit(_swingSpan1, _swingSpan2, _twistSpan, _softness, _biasFactor, _relaxationFactor);
}

int ConeTwistJointBullet::get_solve_twist_limit() {
	return coneConstraint->getSolveTwistLimit();
}

int ConeTwistJointBullet::get_solve_swing_limit() {
	return coneConstraint->getSolveSwingLimit();
}

real_t ConeTwistJointBullet::get_twist_limit_sign() {
	return coneConstraint->getTwistLimitSign();
}

void ConeTwistJointBullet::set_param(PhysicsServer::ConeTwistJointParam p_param, real_t p_value) {
	switch (p_param) {
		case PhysicsServer::CONE_TWIST_JOINT_SWING_SPAN:
			coneConstraint->setLimit(5, p_value);
			coneConstraint->setLimit(4, p_value);
			break;
		case PhysicsServer::CONE_TWIST_JOINT_TWIST_SPAN:
			coneConstraint->setLimit(3, p_value);
			break;
		case PhysicsServer::CONE_TWIST_JOINT_BIAS:
			coneConstraint->setLimit(coneConstraint->getSwingSpan1(), coneConstraint->getSwingSpan2(), coneConstraint->getTwistSpan(), coneConstraint->getLimitSoftness(), p_value, coneConstraint->getRelaxationFactor());
			break;
		case PhysicsServer::CONE_TWIST_JOINT_SOFTNESS:
			coneConstraint->setLimit(coneConstraint->getSwingSpan1(), coneConstraint->getSwingSpan2(), coneConstraint->getTwistSpan(), p_value, coneConstraint->getBiasFactor(), coneConstraint->getRelaxationFactor());
			break;
		case PhysicsServer::CONE_TWIST_JOINT_RELAXATION:
			coneConstraint->setLimit(coneConstraint->getSwingSpan1(), coneConstraint->getSwingSpan2(), coneConstraint->getTwistSpan(), coneConstraint->getLimitSoftness(), coneConstraint->getBiasFactor(), p_value);
			break;
		default:
			WARN_PRINT("This parameter is not supported by Bullet engine");
	}
}

real_t ConeTwistJointBullet::get_param(PhysicsServer::ConeTwistJointParam p_param) const {
	switch (p_param) {
		case PhysicsServer::CONE_TWIST_JOINT_SWING_SPAN:
			return coneConstraint->getSwingSpan1();
		case PhysicsServer::CONE_TWIST_JOINT_TWIST_SPAN:
			return coneConstraint->getTwistSpan();
		case PhysicsServer::CONE_TWIST_JOINT_BIAS:
			return coneConstraint->getBiasFactor();
		case PhysicsServer::CONE_TWIST_JOINT_SOFTNESS:
			return coneConstraint->getLimitSoftness();
		case PhysicsServer::CONE_TWIST_JOINT_RELAXATION:
			return coneConstraint->getRelaxationFactor();
		default:
			WARN_PRINT("This parameter is not supported by Bullet engine");
			return 0;
	}
}
