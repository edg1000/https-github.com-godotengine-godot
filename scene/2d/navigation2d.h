#ifndef NAVIGATION_2D_H
#define NAVIGATION_2D_H

#include "scene/2d/node_2d.h"
#include "scene/2d/navigation_polygon.h"

class Navigation2D : public Node2D {

	OBJ_TYPE( Navigation2D, Node2D);


	union Point {

		struct {
			int64_t x:32;
			int64_t y:32;
		};

		uint64_t key;
		bool operator<(const Point& p_key) const { return key < p_key.key; }
	};


	struct EdgeKey {

		Point a;
		Point b;

		bool operator<(const EdgeKey& p_key) const {
			return (a.key==p_key.a.key)?(b.key<p_key.b.key):(a.key<p_key.a.key);
		};

		EdgeKey(const Point& p_a=Point(),const Point& p_b=Point()) {
			a=p_a;
			b=p_b;
			if (a.key > b.key) {
				SWAP(a,b);
			}
		}
	};


	struct NavMesh;


	struct Polygon {

		struct Edge {
			Point point;
			Polygon *C; //connection
			int C_edge;
			Edge() { C=NULL; C_edge=-1; }
		};

		Vector<Edge> edges;

		Vector2 center;
		Vector2 entry;

		float distance;
		int prev_edge;

		NavMesh *owner;
	};


	struct Connection {

		Polygon *A;
		int A_edge;
		Polygon *B;
		int B_edge;
		Connection() { A=NULL; B=NULL; A_edge=-1; B_edge=-1;}
	};

	Map<EdgeKey,Connection> connections;


	struct NavMesh {

		Object *owner;
		Matrix32 xform;
		bool linked;
		Ref<NavigationPolygon> navpoly;
		List<Polygon> polygons;

	};



	_FORCE_INLINE_ Point _get_point(const Vector2& p_pos) const {

		int x = int(Math::floor(p_pos.x/cell_size));
		int y = int(Math::floor(p_pos.y/cell_size));

		Point p;
		p.key=0;
		p.x=x;
		p.y=y;
		return p;

	}

	_FORCE_INLINE_ Vector2 _get_vertex(const Point& p_point) const {

		return Vector2(p_point.x,p_point.y)*cell_size;
	}



	void _navpoly_link(int p_id);
	void _navpoly_unlink(int p_id);

	float cell_size;
	Map<int,NavMesh> navpoly_map;
	int last_id;
#if 0
	void _clip_path(Vector<Vector2>& path,Polygon *from_poly, const Vector2& p_to_point, Polygon* p_to_poly);
#endif
protected:

	static void _bind_methods();

public:

	//API should be as dynamic as possible
	int navpoly_create(const Ref<NavigationPolygon>& p_mesh,const Matrix32& p_xform,Object* p_owner=NULL);
	void navpoly_set_transform(int p_id, const Matrix32& p_xform);
	void navpoly_remove(int p_id);

	Vector<Vector2> get_simple_path(const Vector2& p_start, const Vector2& p_end,bool p_optimize=true);
	Vector2 get_closest_point(const Vector2& p_point);

	Navigation2D();
};


#endif // Navigation2D2D_H
