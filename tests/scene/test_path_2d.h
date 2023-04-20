/**************************************************************************/
/*  test_path_2d.h                                                        */
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

#ifndef TEST_PATH_2D_H
#define TEST_PATH_2D_H

#include "scene/2d/path_2d.h"

#include "tests/test_macros.h"

namespace TestPath2D {

TEST_CASE("[SceneTree][Path2D] Initialization") {
	SUBCASE("Path should be empty right after initialization") {
		Path2D *test_path = memnew(Path2D);
		CHECK(test_path->get_curve() == nullptr);
		memdelete(test_path);
	}
}

TEST_CASE("[SceneTree][Path2D] Curve setter and getter") {
	Path2D *test_path = memnew(Path2D);
	const Ref<Curve2D> &curve = memnew(Curve2D);

	SUBCASE("Curve passed to the class should remain the same") {
		test_path->set_curve(curve);
		CHECK(test_path->get_curve() == curve);
	}
	SUBCASE("Curve passed many times to the class should remain the same") {
		test_path->set_curve(curve);
		test_path->set_curve(curve);
		test_path->set_curve(curve);
		CHECK(test_path->get_curve() == curve);
	}
	SUBCASE("Curve rewrite testing") {
		const Ref<Curve2D> &curve1 = memnew(Curve2D);
		const Ref<Curve2D> &curve2 = memnew(Curve2D);

		test_path->set_curve(curve1);
		test_path->set_curve(curve2);
		CHECK_MESSAGE(test_path->get_curve() != curve1,
				"After rewrite, second curve should be in class");
		CHECK_MESSAGE(test_path->get_curve() == curve2,
				"After rewrite, second curve should be in class");
	}

	SUBCASE("Assign same curve to two paths") {
		Path2D *path2 = memnew(Path2D);

		test_path->set_curve(curve);
		path2->set_curve(curve);
		CHECK_MESSAGE(test_path->get_curve() == path2->get_curve(),
				"Both paths have the same curve.");
		memdelete(path2);
	}

	SUBCASE("Swapping curves between two paths") {
		Path2D *path2 = memnew(Path2D);
		const Ref<Curve2D> &curve1 = memnew(Curve2D);
		const Ref<Curve2D> &curve2 = memnew(Curve2D);

		test_path->set_curve(curve1);
		path2->set_curve(curve2);
		CHECK(test_path->get_curve() == curve1);
		CHECK(path2->get_curve() == curve2);

		// Do the swap
		Ref<Curve2D> temp = test_path->get_curve();
		test_path->set_curve(path2->get_curve());
		path2->set_curve(temp);

		CHECK(test_path->get_curve() == curve2);
		CHECK(path2->get_curve() == curve1);
		memdelete(path2);
	}

	memdelete(test_path);
}
	
TEST_CASE("[SceneTree][Path2D] _edit_get_rect check") {
	SUBCASE("Invalid curve") {
		Path2D *test_path = memnew(Path2D);
		Rect2 test_rect_1 = test_path->_edit_get_rect();
		Rect2 test_rect_2 = Rect2(0, 0, 0, 0);
        	CHECK(test_rect_1 == test_rect_2);
		memdelete(test_path);
    }
	SUBCASE("Valid curve with points") {
		Path2D *test_path = memnew(Path2D);
		Ref <Curve2D> *test_curve = memnew(Curve2D);
		Vector2 p0 = Vector2(40, 30);
		test_curve->add_point(p0);
		test_path->set_curve(test_curve);
		Rect2 test_rect_1 = test_path->_edit_get_rect();
		CHECK(test_rect_1 != Rect2(0, 0, 0, 0));
		memdelete(test_path);
    }
}

TEST_CASE("[SceneTree][Path2D] _edit_use_rect check") {
	SUBCASE("Invalid curve") {
		Path2D *test_path = memnew(Path2D);
        	CHECK(test_path->_edit_use_rect() == false);
		memdelete(test_path);
    }
	SUBCASE("Valid curve with points") {
		Path2D *test_path = memnew(Path2D);
		Ref <Curve2D> *test_curve = memnew(Curve2D);
		test_curve->set_point_count(1);
		test_path->set_curve(test_curve);
		CHECK(test_path->_edit_use_rect() == true);
		memdelete(test_path);
    }
}

TEST_CASE("[SceneTree][Path2D] _edit_is_selected_on_click check") {
	SUBCASE("Null curve") {
		Path2D *test_path = memnew(Path2D);
		Vector2 p0 = Vector2(0, 0);
		CHECK(test_path->_edit_is_selected_on_click(p0,0.0) == false);
		memdelete(test_path);
	}
	SUBCASE("Curve with points - Curve selected") {
		Path2D *test_path = memnew(Path2D);
		Ref <Curve2D> *test_curve = memnew(Curve2D);
		Vector2 p0 = Vector2(0, 0);
		test_curve->add_point(p0);
		test_path->set_curve(test_curve);
		CHECK(test_path->_edit_is_selected_on_click(p0, 1.0) == true);
		memdelete(test_path);
	}
	SUBCASE("Curve with points - Curve not selected") {
		Path2D *test_path = memnew(Path2D);
		Ref <Curve2D> *test_curve = memnew(Curve2D);
		Vector2 p0 = Vector2(0, 0);
		test_curve->add_point(p0);
		test_path->set_curve(test_curve);
		Vector2 outside_point = Vector2(50,50);
		CHECK(test_path->_edit_is_selected_on_click(outside_point, 1.0) == false);
		memdelete(test_path);
	}
}

} // namespace TestPath2D

#endif // TEST_PATH_2D_H
