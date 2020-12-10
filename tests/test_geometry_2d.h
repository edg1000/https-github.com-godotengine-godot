/*************************************************************************/
/*  test_geometry_2d.h                                                   */
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

#ifndef TEST_GEOMETRY_2D_H
#define TEST_GEOMETRY_2D_H

#include "core/math/geometry_2d.h"
#include "core/templates/vector.h"

#include "thirdparty/doctest/doctest.h"

namespace TestGeometry2D {

TEST_CASE("[Geometry2D] Point in circle") {
	CHECK(Geometry2D::is_point_in_circle(Vector2(0, 0), Vector2(0, 0), 1.0));

	CHECK(Geometry2D::is_point_in_circle(Vector2(0, 0), Vector2(11.99, 0), 12));
	CHECK(Geometry2D::is_point_in_circle(Vector2(-11.99, 0), Vector2(0, 0), 12));

	CHECK_FALSE(Geometry2D::is_point_in_circle(Vector2(0, 0), Vector2(12.01, 0), 12));
	CHECK_FALSE(Geometry2D::is_point_in_circle(Vector2(-12.01, 0), Vector2(0, 0), 12));

	CHECK(Geometry2D::is_point_in_circle(Vector2(7, -42), Vector2(4, -40), 3.7));
	CHECK_FALSE(Geometry2D::is_point_in_circle(Vector2(7, -42), Vector2(4, -40), 3.5));

	// This tests points on the edge of the circle. They are treated as beeing inside the circle.
	// In `is_point_in_triangle` and `is_point_in_polygon` they are treated as being outside, so in order the make
	// the behaviour consistent this may change in the future (see issue #44717 and PR #44274).
	CHECK(Geometry2D::is_point_in_circle(Vector2(1.0, 0.0), Vector2(0, 0), 1.0));
	CHECK(Geometry2D::is_point_in_circle(Vector2(0.0, -1.0), Vector2(0, 0), 1.0));
}

TEST_CASE("[Geometry2D] Point in triangle") {
	CHECK(Geometry2D::is_point_in_triangle(Vector2(0, 0), Vector2(-1, 1), Vector2(0, -1), Vector2(1, 1)));
	CHECK_FALSE(Geometry2D::is_point_in_triangle(Vector2(-1.01, 1.0), Vector2(-1, 1), Vector2(0, -1), Vector2(1, 1)));

	CHECK(Geometry2D::is_point_in_triangle(Vector2(3, 2.5), Vector2(1, 4), Vector2(3, 2), Vector2(5, 4)));
	CHECK(Geometry2D::is_point_in_triangle(Vector2(-3, -2.5), Vector2(-1, -4), Vector2(-3, -2), Vector2(-5, -4)));
	CHECK_FALSE(Geometry2D::is_point_in_triangle(Vector2(0, 0), Vector2(1, 4), Vector2(3, 2), Vector2(5, 4)));

	// This tests points on the edge of the triangle. They are treated as beeing outside the triangle.
	// In `is_point_in_circle` they are treated as being inside, so in order the make
	// the behaviour consistent this may change in the future (see issue #44717 and PR #44274).
	CHECK_FALSE(Geometry2D::is_point_in_triangle(Vector2(1, 1), Vector2(-1, 1), Vector2(0, -1), Vector2(1, 1)));
	CHECK_FALSE(Geometry2D::is_point_in_triangle(Vector2(0, 1), Vector2(-1, 1), Vector2(0, -1), Vector2(1, 1)));
}

TEST_CASE("[Geometry2D] Point in polygon") {
	Vector<Vector2> p;
	CHECK_FALSE(Geometry2D::is_point_in_polygon(Vector2(0, 0), p));

	p.push_back(Vector2(-88, 120));
	p.push_back(Vector2(-74, -38));
	p.push_back(Vector2(135, -145));
	p.push_back(Vector2(425, 70));
	p.push_back(Vector2(68, 112));
	p.push_back(Vector2(-120, 370));
	p.push_back(Vector2(-323, -145));
	CHECK_FALSE(Geometry2D::is_point_in_polygon(Vector2(-350, 0), p));
	CHECK_FALSE(Geometry2D::is_point_in_polygon(Vector2(-110, 60), p));
	CHECK_FALSE(Geometry2D::is_point_in_polygon(Vector2(412, 96), p));
	CHECK_FALSE(Geometry2D::is_point_in_polygon(Vector2(83, 130), p));
	CHECK_FALSE(Geometry2D::is_point_in_polygon(Vector2(-320, -153), p));

	CHECK(Geometry2D::is_point_in_polygon(Vector2(0, 0), p));
	CHECK(Geometry2D::is_point_in_polygon(Vector2(-230, 0), p));
	CHECK(Geometry2D::is_point_in_polygon(Vector2(130, -110), p));
	CHECK(Geometry2D::is_point_in_polygon(Vector2(370, 55), p));
	CHECK(Geometry2D::is_point_in_polygon(Vector2(-160, 190), p));

	// This tests points on the edge of the polygon. They are treated as beeing outside the polygon.
	// In `is_point_in_circle` they are treated as being inside, so in order the make
	// the behaviour consistent this may change in the future (see issue #44717 and PR #44274).
	CHECK_FALSE(Geometry2D::is_point_in_polygon(Vector2(68, 112), p));
	CHECK_FALSE(Geometry2D::is_point_in_polygon(Vector2(-88, 120), p));
}

TEST_CASE("[Geometry2D] Polygon clockwise") {
	Vector<Vector2> p;
	CHECK_FALSE(Geometry2D::is_polygon_clockwise(p));

	p.push_back(Vector2(5, -5));
	p.push_back(Vector2(-1, -5));
	p.push_back(Vector2(-5, -1));
	p.push_back(Vector2(-1, 3));
	p.push_back(Vector2(1, 5));
	CHECK(Geometry2D::is_polygon_clockwise(p));

	p.invert();
	CHECK_FALSE(Geometry2D::is_polygon_clockwise(p));
}

TEST_CASE("[Geometry2D] Line intersection") {
	Vector2 r;
	CHECK(Geometry2D::line_intersects_line(Vector2(2, 0), Vector2(0, 1), Vector2(0, 2), Vector2(1, 0), r));
	CHECK(r.is_equal_approx(Vector2(2, 2)));

	CHECK(Geometry2D::line_intersects_line(Vector2(-1, 1), Vector2(1, -1), Vector2(4, 1), Vector2(-1, -1), r));
	CHECK(r.is_equal_approx(Vector2(1.5, -1.5)));

	CHECK(Geometry2D::line_intersects_line(Vector2(-1, 0), Vector2(-1, -1), Vector2(1, 0), Vector2(1, -1), r));
	CHECK(r.is_equal_approx(Vector2(0, 1)));

	CHECK_FALSE_MESSAGE(
			Geometry2D::line_intersects_line(Vector2(-1, 1), Vector2(1, -1), Vector2(0, 1), Vector2(1, -1), r),
			"Parallel lines should not intersect.");
}

TEST_CASE("[Geometry2D] Segment intersection.") {
	Vector2 r;

	CHECK(Geometry2D::segment_intersects_segment(Vector2(-1, 1), Vector2(1, -1), Vector2(1, 1), Vector2(-1, -1), &r));
	CHECK(r.is_equal_approx(Vector2(0, 0)));

	CHECK_FALSE(Geometry2D::segment_intersects_segment(Vector2(-1, 1), Vector2(1, -1), Vector2(1, 1), Vector2(0.1, 0.1), &r));
	CHECK_FALSE_MESSAGE(
			Geometry2D::segment_intersects_segment(Vector2(-1, 1), Vector2(1, -1), Vector2(0, 1), Vector2(1, -1), &r),
			"Parallel segments should not intersect.");
}

TEST_CASE("[Geometry2D] Closest point to segment") {
	Vector2 s[] = { Vector2(-4, -4), Vector2(4, 4) };
	CHECK(Geometry2D::get_closest_point_to_segment(Vector2(4.1, 4.1), s).is_equal_approx(Vector2(4, 4)));
	CHECK(Geometry2D::get_closest_point_to_segment(Vector2(-4.1, -4.1), s).is_equal_approx(Vector2(-4, -4)));
	CHECK(Geometry2D::get_closest_point_to_segment(Vector2(-1, 1), s).is_equal_approx(Vector2(0, 0)));
}

TEST_CASE("[Geometry2D] Closest point to uncapped segment") {
	Vector2 s[] = { Vector2(-4, -4), Vector2(4, 4) };
	CHECK(Geometry2D::get_closest_point_to_segment_uncapped(Vector2(-1, 1), s).is_equal_approx(Vector2(0, 0)));
	CHECK(Geometry2D::get_closest_point_to_segment_uncapped(Vector2(-4, -6), s).is_equal_approx(Vector2(-5, -5)));
	CHECK(Geometry2D::get_closest_point_to_segment_uncapped(Vector2(4, 6), s).is_equal_approx(Vector2(5, 5)));
}

TEST_CASE("[Geometry2D] Closest points between segments") {
	Vector2 c1, c2;
	Geometry2D::get_closest_points_between_segments(Vector2(2, 2), Vector2(3, 3), Vector2(4, 4), Vector2(4, 5), c1, c2);
	CHECK(c1.is_equal_approx(Vector2(3, 3)));
	CHECK(c2.is_equal_approx(Vector2(4, 4)));

	Geometry2D::get_closest_points_between_segments(Vector2(0, 1), Vector2(-2, -1), Vector2(0, 0), Vector2(2, -2), c1, c2);
	CHECK(c1.is_equal_approx(Vector2(-0.5, 0.5)));
	CHECK(c2.is_equal_approx(Vector2(0, 0)));

	Geometry2D::get_closest_points_between_segments(Vector2(-1, 1), Vector2(1, -1), Vector2(1, 1), Vector2(-1, -1), c1, c2);
	CHECK(c1.is_equal_approx(Vector2(0, 0)));
	CHECK(c2.is_equal_approx(Vector2(0, 0)));
}

TEST_CASE("[Geometry2D] Make atlas") {
	Vector<Point2i> result;
	Size2i size;

	Vector<Size2i> r;
	r.push_back(Size2i(2, 2));
	Geometry2D::make_atlas(r, result, size);
	CHECK(size == Size2i(2, 2));
	CHECK(result.size() == r.size());

	r.clear();
	result.clear();
	r.push_back(Size2i(1, 2));
	r.push_back(Size2i(3, 4));
	r.push_back(Size2i(5, 6));
	r.push_back(Size2i(7, 8));
	Geometry2D::make_atlas(r, result, size);
	CHECK(result.size() == r.size());
}

TEST_CASE("[Geometry2D] Polygon intersection") {
	Vector<Point2> a;
	Vector<Point2> b;
	Vector<Vector<Point2>> r;

	a.push_back(Point2(30, 60));
	a.push_back(Point2(70, 5));
	a.push_back(Point2(200, 40));
	a.push_back(Point2(80, 200));

	SUBCASE("[Geometry2D] Both polygons are empty") {
		r = Geometry2D::intersect_polygons(Vector<Point2>(), Vector<Point2>());
		CHECK_MESSAGE(r.is_empty(), "Both polygons are empty. The intersection should also be empty.");
	}

	SUBCASE("[Geometry2D] One polygon is empty") {
		r = Geometry2D::intersect_polygons(a, b);
		REQUIRE_MESSAGE(r.is_empty(), "One polygon is empty. The intersection should also be empty.");
	}

	SUBCASE("[Geometry2D] Basic intersection") {
		b.push_back(Point2(200, 300));
		b.push_back(Point2(90, 200));
		b.push_back(Point2(50, 100));
		b.push_back(Point2(200, 90));
		r = Geometry2D::intersect_polygons(a, b);
		REQUIRE_MESSAGE(r.size() == 1, "The polygons should intersect each other with 1 resulting intersection polygon.");
		REQUIRE_MESSAGE(r[0].size() == 3, "The resulting intersection polygon should have 3 vertices.");
		CHECK(r[0][0].is_equal_approx(Point2(86.52174, 191.30436)));
		CHECK(r[0][1].is_equal_approx(Point2(50, 100)));
		CHECK(r[0][2].is_equal_approx(Point2(160.52632, 92.63157)));
	}

	SUBCASE("[Geometry2D] Intersection with one polygon beeing completly inside the other polygon") {
		b.push_back(Point2(80, 100));
		b.push_back(Point2(50, 50));
		b.push_back(Point2(150, 50));
		r = Geometry2D::intersect_polygons(a, b);
		REQUIRE_MESSAGE(r.size() == 1, "The polygons should intersect each other with 1 resulting intersection polygon.");
		REQUIRE_MESSAGE(r[0].size() == 3, "The resulting intersection polygon should have 3 vertices.");
		CHECK(r[0][0].is_equal_approx(b[0]));
		CHECK(r[0][1].is_equal_approx(b[1]));
		CHECK(r[0][2].is_equal_approx(b[2]));
	}

	SUBCASE("[Geometry2D] No intersection with 2 non-empty polygons") {
		b.push_back(Point2(150, 150));
		b.push_back(Point2(250, 100));
		b.push_back(Point2(300, 200));
		r = Geometry2D::intersect_polygons(a, b);
		REQUIRE_MESSAGE(r.is_empty(), "The polygons should not intersect each other.");
	}

	SUBCASE("[Geometry2D] Intersection with 2 resulting polygons") {
		a.clear();
		a.push_back(Point2(70, 5));
		a.push_back(Point2(140, 7));
		a.push_back(Point2(100, 52));
		a.push_back(Point2(170, 50));
		a.push_back(Point2(60, 125));
		b.push_back(Point2(70, 105));
		b.push_back(Point2(115, 55));
		b.push_back(Point2(90, 15));
		b.push_back(Point2(160, 50));
		r = Geometry2D::intersect_polygons(a, b);
		REQUIRE_MESSAGE(r.size() == 2, "The polygons should intersect each other with 2 resulting intersection polygons.");
		REQUIRE_MESSAGE(r[0].size() == 4, "The resulting intersection polygon should have 4 vertices.");
		CHECK(r[0][0].is_equal_approx(Point2(70, 105)));
		CHECK(r[0][1].is_equal_approx(Point2(115, 55)));
		CHECK(r[0][2].is_equal_approx(Point2(112.894737, 51.63158)));
		CHECK(r[0][3].is_equal_approx(Point2(159.509537, 50.299728)));

		REQUIRE_MESSAGE(r[1].size() == 3, "The intersection polygon should have 3 vertices.");
		CHECK(r[1][0].is_equal_approx(Point2(119.692307, 29.846149)));
		CHECK(r[1][1].is_equal_approx(Point2(107.706421, 43.33028)));
		CHECK(r[1][2].is_equal_approx(Point2(90, 15)));
	}
}

TEST_CASE("[Geometry2D] Merge polygons") {
	Vector<Point2> a;
	Vector<Point2> b;
	Vector<Vector<Point2>> r;

	a.push_back(Point2(225, 180));
	a.push_back(Point2(160, 230));
	a.push_back(Point2(20, 212));
	a.push_back(Point2(50, 115));

	SUBCASE("[Geometry2D] Both polygons are empty") {
		r = Geometry2D::merge_polygons(Vector<Point2>(), Vector<Point2>());
		REQUIRE_MESSAGE(r.is_empty(), "Both polygons are empty. The union should also be empty.");
	}

	SUBCASE("[Geometry2D] One polygon is empty") {
		r = Geometry2D::merge_polygons(a, b);
		REQUIRE_MESSAGE(r.size() == 1, "One polygon is non-empty. There should be 1 resulting merged polygon.");
		REQUIRE_MESSAGE(r[0].size() == 4, "The resulting merged polygon should have 4 vertices.");
		CHECK(r[0][0].is_equal_approx(a[0]));
		CHECK(r[0][1].is_equal_approx(a[1]));
		CHECK(r[0][2].is_equal_approx(a[2]));
		CHECK(r[0][3].is_equal_approx(a[3]));
	}

	SUBCASE("[Geometry2D] Basic merge with 2 polygons") {
		b.push_back(Point2(180, 190));
		b.push_back(Point2(60, 140));
		b.push_back(Point2(160, 80));
		r = Geometry2D::merge_polygons(a, b);
		REQUIRE_MESSAGE(r.size() == 1, "The merged polygons should result in 1 polygon.");
		REQUIRE_MESSAGE(r[0].size() == 7, "The resulting merged polygon should have 7 vertices.");
		CHECK(r[0][0].is_equal_approx(Point2(174.791077, 161.350967)));
		CHECK(r[0][1].is_equal_approx(Point2(225, 180)));
		CHECK(r[0][2].is_equal_approx(Point2(160, 230)));
		CHECK(r[0][3].is_equal_approx(Point2(20, 212)));
		CHECK(r[0][4].is_equal_approx(Point2(50, 115)));
		CHECK(r[0][5].is_equal_approx(Point2(81.911758, 126.852943)));
		CHECK(r[0][6].is_equal_approx(Point2(160, 80)));
	}

	SUBCASE("[Geometry2D] Merge with 2 resulting merged polygons (outline and hole)") {
		b.push_back(Point2(180, 190));
		b.push_back(Point2(140, 125));
		b.push_back(Point2(60, 140));
		b.push_back(Point2(160, 80));
		r = Geometry2D::merge_polygons(a, b);
		REQUIRE_MESSAGE(r.size() == 2, "The merged polygons should result in 2 polygons.");

		REQUIRE_MESSAGE(!Geometry2D::is_polygon_clockwise(r[0]), "The merged polygon (outline) should be counter-clockwise.");
		REQUIRE_MESSAGE(r[0].size() == 7, "The resulting merged polygon (outline) should have 7 vertices.");
		CHECK(r[0][0].is_equal_approx(Point2(174.791077, 161.350967)));
		CHECK(r[0][1].is_equal_approx(Point2(225, 180)));
		CHECK(r[0][2].is_equal_approx(Point2(160, 230)));
		CHECK(r[0][3].is_equal_approx(Point2(20, 212)));
		CHECK(r[0][4].is_equal_approx(Point2(50, 115)));
		CHECK(r[0][5].is_equal_approx(Point2(81.911758, 126.852943)));
		CHECK(r[0][6].is_equal_approx(Point2(160, 80)));

		REQUIRE_MESSAGE(Geometry2D::is_polygon_clockwise(r[1]), "The resulting merged polygon (hole) should be clockwise.");
		REQUIRE_MESSAGE(r[1].size() == 3, "The resulting merged polygon (hole) should have 3 vertices.");
		CHECK(r[1][0].is_equal_approx(Point2(98.083069, 132.859421)));
		CHECK(r[1][1].is_equal_approx(Point2(158.689453, 155.370377)));
		CHECK(r[1][2].is_equal_approx(Point2(140, 125)));
	}
}

TEST_CASE("[Geometry2D] Clip polygons") {
	Vector<Point2> a;
	Vector<Point2> b;
	Vector<Vector<Point2>> r;

	a.push_back(Point2(225, 180));
	a.push_back(Point2(160, 230));
	a.push_back(Point2(20, 212));
	a.push_back(Point2(50, 115));

	SUBCASE("[Geometry2D] Both polygons are empty") {
		r = Geometry2D::clip_polygons(Vector<Point2>(), Vector<Point2>());
		CHECK_MESSAGE(r.is_empty(), "Both polygons are empty. The clip should also be empty.");
	}

	SUBCASE("[Geometry2D] Basic clip with one result polygon") {
		b.push_back(Point2(250, 170));
		b.push_back(Point2(175, 270));
		b.push_back(Point2(120, 260));
		b.push_back(Point2(25, 80));
		r = Geometry2D::clip_polygons(a, b);
		REQUIRE_MESSAGE(r.size() == 1, "The clipped polygons should result in 1 polygon.");
		REQUIRE_MESSAGE(r[0].size() == 3, "The resulting clipped polygon should have 3 vertices.");
		CHECK(r[0][0].is_equal_approx(Point2(100.102173, 222.298843)));
		CHECK(r[0][1].is_equal_approx(Point2(20, 212)));
		CHECK(r[0][2].is_equal_approx(Point2(47.588089, 122.798492)));
	}

	SUBCASE("[Geometry2D] Polygon b completely overlaps polygon a") {
		b.push_back(Point2(250, 170));
		b.push_back(Point2(175, 270));
		b.push_back(Point2(10, 210));
		b.push_back(Point2(55, 80));
		r = Geometry2D::clip_polygons(a, b);
		CHECK_MESSAGE(r.is_empty(), "Polygon 'b' completely overlaps polygon 'a'. This should result in no clipped polygons.");
	}

	SUBCASE("[Geometry2D] Polygon a completely overlaps polygon b") {
		b.push_back(Point2(150, 200));
		b.push_back(Point2(65, 190));
		b.push_back(Point2(80, 140));
		r = Geometry2D::clip_polygons(a, b);
		REQUIRE_MESSAGE(r.size() == 2, "Polygon 'a' completely overlaps polygon 'b'. This should result in 2 clipped polygons.");
		REQUIRE_MESSAGE(r[0].size() == 4, "The resulting clipped polygon should have 4 vertices.");
		REQUIRE_MESSAGE(!Geometry2D::is_polygon_clockwise(r[0]), "The resulting clipped polygon (outline) should be counter-clockwise.");
		CHECK(r[0][0].is_equal_approx(a[0]));
		CHECK(r[0][1].is_equal_approx(a[1]));
		CHECK(r[0][2].is_equal_approx(a[2]));
		CHECK(r[0][3].is_equal_approx(a[3]));
		REQUIRE_MESSAGE(r[1].size() == 3, "The resulting clipped polygon should have 3 vertices.");
		REQUIRE_MESSAGE(Geometry2D::is_polygon_clockwise(r[1]), "The resulting clipped polygon (hole) should be clockwise.");
		CHECK(r[1][0].is_equal_approx(b[1]));
		CHECK(r[1][1].is_equal_approx(b[0]));
		CHECK(r[1][2].is_equal_approx(b[2]));
	}
}

TEST_CASE("[Geometry2D] Exclude polygons") {
	Vector<Point2> a;
	Vector<Point2> b;
	Vector<Vector<Point2>> r;

	a.push_back(Point2(225, 180));
	a.push_back(Point2(160, 230));
	a.push_back(Point2(20, 212));
	a.push_back(Point2(50, 115));

	SUBCASE("[Geometry2D] Both polygons are empty") {
		r = Geometry2D::exclude_polygons(Vector<Point2>(), Vector<Point2>());
		CHECK_MESSAGE(r.is_empty(), "Both polygons are empty. The excluded polygon should also be empty.");
	}

	SUBCASE("[Geometry2D] One polygon is empty") {
		r = Geometry2D::exclude_polygons(a, b);
		REQUIRE_MESSAGE(r.size() == 1, "One polygon is non-empty. There should be 1 resulting excluded polygon.");
		REQUIRE_MESSAGE(r[0].size() == 4, "The resulting excluded polygon should have 4 vertices.");
		CHECK(r[0][0].is_equal_approx(a[0]));
		CHECK(r[0][1].is_equal_approx(a[1]));
		CHECK(r[0][2].is_equal_approx(a[2]));
		CHECK(r[0][3].is_equal_approx(a[3]));
	}

	SUBCASE("[Geometry2D] Exclude with 2 resulting polygons (outline and hole)") {
		b.push_back(Point2(140, 160));
		b.push_back(Point2(150, 220));
		b.push_back(Point2(40, 200));
		b.push_back(Point2(60, 140));
		r = Geometry2D::exclude_polygons(a, b);
		REQUIRE_MESSAGE(r.size() == 2, "There should be 2 resulting excluded polygons (outline and hole).");
		REQUIRE_MESSAGE(r[0].size() == 4, "The resulting excluded polygon should have 4 vertices.");
		REQUIRE_MESSAGE(!Geometry2D::is_polygon_clockwise(r[0]), "The resulting excluded polygon (outline) should be counter-clockwise.");
		CHECK(r[0][0].is_equal_approx(a[0]));
		CHECK(r[0][1].is_equal_approx(a[1]));
		CHECK(r[0][2].is_equal_approx(a[2]));
		CHECK(r[0][3].is_equal_approx(a[3]));
		REQUIRE_MESSAGE(r[1].size() == 4, "The resulting excluded polygon should have 4 vertices.");
		REQUIRE_MESSAGE(Geometry2D::is_polygon_clockwise(r[1]), "The resulting excluded polygon (hole) should be clockwise.");
		CHECK(r[1][0].is_equal_approx(Point2(40, 200)));
		CHECK(r[1][1].is_equal_approx(Point2(150, 220)));
		CHECK(r[1][2].is_equal_approx(Point2(140, 160)));
		CHECK(r[1][3].is_equal_approx(Point2(60, 140)));
	}
}

TEST_CASE("[Geometry2D] Intersect polyline with polygon") {
	Vector<Vector2> l;
	Vector<Vector2> p;
	Vector<Vector<Point2>> r;

	l.push_back(Vector2(100, 90));
	l.push_back(Vector2(120, 250));

	p.push_back(Vector2(225, 180));
	p.push_back(Vector2(160, 230));
	p.push_back(Vector2(20, 212));
	p.push_back(Vector2(50, 115));

	SUBCASE("[Geometry2D] Both line and polygon are empty") {
		r = Geometry2D::intersect_polyline_with_polygon(Vector<Vector2>(), Vector<Vector2>());
		CHECK_MESSAGE(r.is_empty(), "Both line and polygon are empty. The intersection line should also be empty.");
	}

	SUBCASE("[Geometry2D] Line is non-empty and polygon is empty") {
		r = Geometry2D::intersect_polyline_with_polygon(l, Vector<Vector2>());
		CHECK_MESSAGE(r.is_empty(), "The polygon is empty while the line is non-empty. The intersection line should be empty.");
	}

	SUBCASE("[Geometry2D] Basic intersection with 1 resulting intersection line") {
		r = Geometry2D::intersect_polyline_with_polygon(l, p);
		REQUIRE_MESSAGE(r.size() == 1, "There should be 1 resulting intersection line.");
		REQUIRE_MESSAGE(r[0].size() == 2, "The resulting intersection line should have 2 vertices.");
		CHECK(r[0][0].is_equal_approx(Vector2(105.711609, 135.692886)));
		CHECK(r[0][1].is_equal_approx(Vector2(116.805809, 224.446457)));
	}

	SUBCASE("[Geometry2D] Complex intersection with 2 resulting intersection lines") {
		l.clear();
		l.push_back(Vector2(100, 90));
		l.push_back(Vector2(190, 255));
		l.push_back(Vector2(135, 260));
		l.push_back(Vector2(57, 200));
		l.push_back(Vector2(50, 170));
		l.push_back(Vector2(15, 155));
		r = Geometry2D::intersect_polyline_with_polygon(l, p);
		REQUIRE_MESSAGE(r.size() == 2, "There should be 2 resulting intersection lines.");
		REQUIRE_MESSAGE(r[0].size() == 2, "The resulting intersection line should have 2 vertices.");
		CHECK(r[0][0].is_equal_approx(Vector2(129.804565, 144.641693)));
		CHECK(r[0][1].is_equal_approx(Vector2(171.527084, 221.132996)));
		REQUIRE_MESSAGE(r[1].size() == 4, "The resulting intersection line should have 4 vertices.");
		CHECK(r[1][0].is_equal_approx(Vector2(83.15609, 220.120087)));
		CHECK(r[1][1].is_equal_approx(Vector2(57, 200)));
		CHECK(r[1][2].is_equal_approx(Vector2(50, 170)));
		CHECK(r[1][3].is_equal_approx(Vector2(34.980492, 163.563065)));
	}
}

TEST_CASE("[Geometry2D] Clip polyline with polygon") {
	Vector<Vector2> l;
	Vector<Vector2> p;
	Vector<Vector<Point2>> r;

	l.push_back(Vector2(70, 140));
	l.push_back(Vector2(160, 320));

	p.push_back(Vector2(225, 180));
	p.push_back(Vector2(160, 230));
	p.push_back(Vector2(20, 212));
	p.push_back(Vector2(50, 115));

	SUBCASE("[Geometry2D] Both line and polygon are empty") {
		r = Geometry2D::clip_polyline_with_polygon(Vector<Vector2>(), Vector<Vector2>());
		CHECK_MESSAGE(r.is_empty(), "Both line and polygon are empty. The clipped line should also be empty.");
	}

	SUBCASE("[Geometry2D] Polygon is empty and line is non-empty") {
		r = Geometry2D::clip_polyline_with_polygon(l, Vector<Vector2>());
		REQUIRE_MESSAGE(r.size() == 1, "There should be 1 resulting clipped line.");
		REQUIRE_MESSAGE(r[0].size() == 2, "The resulting clipped line should have 2 vertices.");
		CHECK(r[0][0].is_equal_approx(l[0]));
		CHECK(r[0][1].is_equal_approx(l[1]));
	}

	SUBCASE("[Geometry2D] Basic clip with 1 resulting clipped line") {
		r = Geometry2D::clip_polyline_with_polygon(l, p);
		REQUIRE_MESSAGE(r.size() == 1, "There should be 1 resulting clipped line.");
		REQUIRE_MESSAGE(r[0].size() == 2, "The resulting clipped line should have 2 vertices.");
		CHECK(r[0][0].is_equal_approx(Vector2(111.908401, 223.816803)));
		CHECK(r[0][1].is_equal_approx(Vector2(160, 320)));
	}

	SUBCASE("[Geometry2D] Complex clip with 2 resulting clipped lines") {
		l.clear();
		l.push_back(Vector2(55, 70));
		l.push_back(Vector2(50, 190));
		l.push_back(Vector2(120, 165));
		l.push_back(Vector2(122, 250));
		l.push_back(Vector2(160, 320));
		r = Geometry2D::clip_polyline_with_polygon(l, p);
		REQUIRE_MESSAGE(r.size() == 2, "There should be 2 resulting clipped lines.");
		REQUIRE_MESSAGE(r[0].size() == 3, "The resulting clipped line should have 3 vertices.");
		CHECK(r[0][0].is_equal_approx(Vector2(160, 320)));
		CHECK(r[0][1].is_equal_approx(Vector2(122, 250)));
		CHECK(r[0][2].is_equal_approx(Vector2(121.412682, 225.038757)));
		REQUIRE_MESSAGE(r[1].size() == 2, "The resulting clipped line should have 2 vertices.");
		CHECK(r[1][0].is_equal_approx(Vector2(53.07737, 116.143021)));
		CHECK(r[1][1].is_equal_approx(Vector2(55, 70)));
	}
}
} // namespace TestGeometry2D

#endif // TEST_GEOMETRY_2D_H
