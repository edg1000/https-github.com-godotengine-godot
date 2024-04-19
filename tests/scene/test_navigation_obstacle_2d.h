/**************************************************************************/
/*  test_navigation_obstacle_2d.h                                         */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef TEST_NAVIGATION_OBSTACLE_2D_H
#define TEST_NAVIGATION_OBSTACLE_2D_H

#include "scene/2d/navigation_obstacle_2d.h"
#include "scene/main/window.h"

#include "tests/test_macros.h"

namespace TestNavigationObstacle2D {

TEST_SUITE("[Navigation]") {
	TEST_CASE("[SceneTree][NavigationObstacle2D] New obstacle should have valid RID") {
		NavigationObstacle2D *obstacle_node = memnew(NavigationObstacle2D);
		CHECK(obstacle_node->get_rid().is_valid());
		memdelete(obstacle_node);
	}

	TEST_CASE("[SceneTree][NavigationObstacle2D] New obstacle should attach to default avoidance space") {
		Node2D *node_2d = memnew(Node2D);
		SceneTree::get_singleton()->get_root()->add_child(node_2d);

		NavigationObstacle2D *obstacle_node = memnew(NavigationObstacle2D);
		// obstacle should not be attached to any avoidance space when outside of tree
		CHECK_FALSE(obstacle_node->get_avoidance_space().is_valid());

		SUBCASE("Obstacle should attach to default avoidance space when it enters the tree") {
			node_2d->add_child(obstacle_node);
			CHECK(obstacle_node->get_avoidance_space().is_valid());
			CHECK(obstacle_node->get_avoidance_space() == node_2d->get_world_2d()->get_avoidance_space());
		}

		memdelete(obstacle_node);
		memdelete(node_2d);
	}
}

} //namespace TestNavigationObstacle2D

#endif // TEST_NAVIGATION_OBSTACLE_2D_H
