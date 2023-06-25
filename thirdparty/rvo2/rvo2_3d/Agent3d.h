/*
 * Agent3d.h
 * RVO2-3D Library
 *
 * SPDX-FileCopyrightText: 2008 University of North Carolina at Chapel Hill
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please send all bug reports to <geom@cs.unc.edu>.
 *
 * The authors may be contacted via:
 *
 * Jur van den Berg, Stephen J. Guy, Jamie Snape, Ming C. Lin, Dinesh Manocha
 * Dept. of Computer Science
 * 201 S. Columbia St.
 * Frederick P. Brooks, Jr. Computer Science Bldg.
 * Chapel Hill, N.C. 27599-3175
 * United States of America
 *
 * <https://gamma.cs.unc.edu/RVO2/>
 */

#ifndef RVO3D_AGENT_H_
#define RVO3D_AGENT_H_

/**
 * @file  Agent3d.h
 * @brief Contains the Agent3D class.
 */

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include "Plane.h"
#include "Vector3.h"

namespace RVO3D {
class RVOSimulator3D;

/**
 * @brief Defines an agent in the simulation.
 */
class Agent3D {
 public:
  /**
   * @brief     Constructs an agent instance.
   * @param[in] sim The simulator instance.
   */
  explicit Agent3D();

  /**
   * @brief Destroys this agent instance.
   */
  ~Agent3D();

  /**
   * @brief Computes the neighbors of this agent.
   */
  void computeNeighbors(RVOSimulator3D *sim_);

  /**
   * @brief Computes the new velocity of this agent.
   */
  void computeNewVelocity(RVOSimulator3D *sim_);

  /**
   * @brief     Inserts an agent neighbor into the set of neighbors of this
   *            agent.
   * @param[in] agent   A pointer to the agent to be inserted.
   * @param[in] rangeSq The squared range around this agent.
   */
  void insertAgentNeighbor(const Agent3D *agent,
                           float &rangeSq); /* NOLINT(runtime/references) */

  /**
   * @brief Updates the three-dimensional position and three-dimensional
   *        velocity of this agent.
   */
  void update(RVOSimulator3D *sim_);;

  /* Not implemented. */
  Agent3D(const Agent3D &other);

  /* Not implemented. */
  Agent3D &operator=(const Agent3D &other);

  Vector3 newVelocity_;
  Vector3 position_;
  Vector3 prefVelocity_;
  Vector3 velocity_;
  RVOSimulator3D *sim_;
  std::size_t id_;
  std::size_t maxNeighbors_;
  float maxSpeed_;
  float neighborDist_;
  float radius_;
  float timeHorizon_;
  float timeHorizonObst_;
  std::vector<std::pair<float, const Agent3D *> > agentNeighbors_;
  std::vector<Plane> orcaPlanes_;
	float height_ = 1.0;
	uint32_t avoidance_layers_ = 1;
	uint32_t avoidance_mask_ = 1;
	float avoidance_priority_ = 1.0;

  friend class KdTree3D;
  friend class RVOSimulator3D;
};
} /* namespace RVO3D */

#endif /* RVO3D_AGENT_H_ */
