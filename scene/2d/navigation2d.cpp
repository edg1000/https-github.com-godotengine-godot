#include "navigation2d.h"

#define USE_ENTRY_POINT

void Navigation2D::_navpoly_link(int p_id) {

	ERR_FAIL_COND(!navpoly_map.has(p_id));
	NavMesh &nm=navpoly_map[p_id];
	ERR_FAIL_COND(nm.linked);

	print_line("LINK");

	DVector<Vector2> vertices=nm.navpoly->get_vertices();
	int len = vertices.size();
	if (len==0)
		return;

	DVector<Vector2>::Read r=vertices.read();

	for(int i=0;i<nm.navpoly->get_polygon_count();i++) {

		//build

		List<Polygon>::Element *P=nm.polygons.push_back(Polygon());
		Polygon &p=P->get();
		p.owner=&nm;

		Vector<int> poly = nm.navpoly->get_polygon(i);
		int plen=poly.size();
		const int *indices=poly.ptr();
		bool valid=true;
		p.edges.resize(plen);

		Vector2 center;

		for(int j=0;j<plen;j++) {

			int idx = indices[j];
			if (idx<0 || idx>=len) {
				valid=false;
				break;
			}

			Polygon::Edge e;
			Vector2 ep=nm.xform.xform(r[idx]);
			center+=ep;
			e.point=_get_point(ep);
			p.edges[j]=e;
		}

		if (!valid) {
			nm.polygons.pop_back();
			ERR_CONTINUE(!valid);
			continue;
		}

		p.center=center/plen;

		//connect

		for(int j=0;j<plen;j++) {

			int next = (j+1)%plen;
			EdgeKey ek(p.edges[j].point,p.edges[next].point);

			Map<EdgeKey,Connection>::Element *C=connections.find(ek);
			if (!C) {

				Connection c;
				c.A=&p;
				c.A_edge=j;
				c.B=NULL;
				c.B_edge=-1;
				connections[ek]=c;
			} else {

				if (C->get().B!=NULL) {
					print_line(String()+_get_vertex(ek.a)+" -> "+_get_vertex(ek.b));
				}
				ERR_CONTINUE(C->get().B!=NULL); //wut

				C->get().B=&p;
				C->get().B_edge=j;
				C->get().A->edges[C->get().A_edge].C=&p;
				C->get().A->edges[C->get().A_edge].C_edge=j;;
				p.edges[j].C=C->get().A;
				p.edges[j].C_edge=C->get().A_edge;
				//connection successful.
			}
		}
	}

	nm.linked=true;

}


void Navigation2D::_navpoly_unlink(int p_id) {

	ERR_FAIL_COND(!navpoly_map.has(p_id));
	NavMesh &nm=navpoly_map[p_id];
	ERR_FAIL_COND(!nm.linked);

	print_line("UNLINK");

	for (List<Polygon>::Element *E=nm.polygons.front();E;E=E->next()) {


		Polygon &p=E->get();

		int ec = p.edges.size();
		Polygon::Edge *edges=p.edges.ptr();

		for(int i=0;i<ec;i++) {
			int next = (i+1)%ec;

			EdgeKey ek(edges[i].point,edges[next].point);
			Map<EdgeKey,Connection>::Element *C=connections.find(ek);
			ERR_CONTINUE(!C);
			if (C->get().B) {
				//disconnect

				C->get().B->edges[C->get().B_edge].C=NULL;
				C->get().B->edges[C->get().B_edge].C_edge=-1;
				C->get().A->edges[C->get().A_edge].C=NULL;
				C->get().A->edges[C->get().A_edge].C_edge=-1;

				if (C->get().A==&E->get()) {

					C->get().A=C->get().B;
					C->get().A_edge=C->get().B_edge;
				}
				C->get().B=NULL;
				C->get().B_edge=-1;

			} else {
				connections.erase(C);
				//erase
			}
		}
	}

	nm.polygons.clear();

	nm.linked=false;


}


int Navigation2D::navpoly_create(const Ref<NavigationPolygon>& p_mesh, const Matrix32& p_xform, Object *p_owner) {

	int id = last_id++;
	NavMesh nm;
	nm.linked=false;
	nm.navpoly=p_mesh;
	nm.xform=p_xform;
	nm.owner=p_owner;
	navpoly_map[id]=nm;

	_navpoly_link(id);

	return id;
}

void Navigation2D::navpoly_set_transform(int p_id, const Matrix32& p_xform){

	ERR_FAIL_COND(!navpoly_map.has(p_id));
	NavMesh &nm=navpoly_map[p_id];
	if (nm.xform==p_xform)
		return; //bleh
	_navpoly_unlink(p_id);
	nm.xform=p_xform;
	_navpoly_link(p_id);



}
void Navigation2D::navpoly_remove(int p_id){

	ERR_FAIL_COND(!navpoly_map.has(p_id));
	_navpoly_unlink(p_id);
	navpoly_map.erase(p_id);

}
#if 0
void Navigation2D::_clip_path(Vector<Vector2>& path, Polygon *from_poly, const Vector2& p_to_point, Polygon* p_to_poly) {

	Vector2 from = path[path.size()-1];

	if (from.distance_to(p_to_point)<CMP_EPSILON)
		return;
	Plane cut_plane;
	cut_plane.normal = (from-p_to_point).cross(up);
	if (cut_plane.normal==Vector2())
		return;
	cut_plane.normal.normalize();
	cut_plane.d = cut_plane.normal.dot(from);


	while(from_poly!=p_to_poly) {

		int pe = from_poly->prev_edge;
		Vector2 a = _get_vertex(from_poly->edges[pe].point);
		Vector2 b = _get_vertex(from_poly->edges[(pe+1)%from_poly->edges.size()].point);

		from_poly=from_poly->edges[pe].C;
		ERR_FAIL_COND(!from_poly);

		if (a.distance_to(b)>CMP_EPSILON) {

			Vector2 inters;
			if (cut_plane.intersects_segment(a,b,&inters)) {
				if (inters.distance_to(p_to_point)>CMP_EPSILON && inters.distance_to(path[path.size()-1])>CMP_EPSILON) {
					path.push_back(inters);
				}
			}
		}
	}
}
#endif

Vector<Vector2> Navigation2D::get_simple_path(const Vector2& p_start, const Vector2& p_end, bool p_optimize) {


	Polygon *begin_poly=NULL;
	Polygon *end_poly=NULL;
	Vector2 begin_point;
	Vector2 end_point;
	float begin_d=1e20;
	float end_d=1e20;

	//look for point inside triangle

	for (Map<int,NavMesh>::Element*E=navpoly_map.front();E;E=E->next()) {

		if (!E->get().linked)
			continue;
		for(List<Polygon>::Element *F=E->get().polygons.front();F;F=F->next()) {


			Polygon &p=F->get();
			if (begin_d || end_d) {
				for(int i=2;i<p.edges.size();i++) {

					if (begin_d>0) {

						if (Geometry::is_point_in_triangle(p_start,_get_vertex(p.edges[0].point),_get_vertex(p.edges[i-1].point),_get_vertex(p.edges[i].point))) {

							begin_poly=&p;
							begin_point=p_start;
							begin_d=0;
							if (end_d==0)
								break;

						}
					}

					if (end_d>0) {

						if (Geometry::is_point_in_triangle(p_end,_get_vertex(p.edges[0].point),_get_vertex(p.edges[i-1].point),_get_vertex(p.edges[i].point))) {

							end_poly=&p;
							end_point=p_end;
							end_d=0;
							if (begin_d==0)
								break;
						}
					}

				}
			}

			p.prev_edge=-1;
		}
	}

	//start or end not inside triangle.. look for closest segment :|
	if (begin_d || end_d) {
		for (Map<int,NavMesh>::Element*E=navpoly_map.front();E;E=E->next()) {

			if (!E->get().linked)
				continue;
			for(List<Polygon>::Element *F=E->get().polygons.front();F;F=F->next()) {

				Polygon &p=F->get();
				int es = p.edges.size();
				for(int i=0;i<es;i++) {

					Vector2 edge[2]={
						_get_vertex(p.edges[i].point),
						_get_vertex(p.edges[(i+1)%es].point)
					};


					if (begin_d>0) {
						Vector2 spoint=Geometry::get_closest_point_to_segment_2d(p_start,edge);
						float d = spoint.distance_to(p_start);
						if (d<begin_d) {
							begin_poly=&p;
							begin_point=spoint;
							begin_d=d;
						}
					}

					if (end_d>0) {
						Vector2 spoint=Geometry::get_closest_point_to_segment_2d(p_end,edge);
						float d = spoint.distance_to(p_end);
						if (d<end_d) {
							end_poly=&p;
							end_point=spoint;
							end_d=d;
						}
					}
				}
			}
		}
	}

	if (!begin_poly || !end_poly) {

		//print_line("No Path Path");
		return Vector<Vector2>(); //no path
	}

	if (begin_poly==end_poly) {

		Vector<Vector2> path;
		path.resize(2);
		path[0]=begin_point;
		path[1]=end_point;
		//print_line("Direct Path");
		return path;
	}


	bool found_route=false;

	List<Polygon*> open_list;

	begin_poly->entry=p_start;

	for(int i=0;i<begin_poly->edges.size();i++) {

		if (begin_poly->edges[i].C) {

			begin_poly->edges[i].C->prev_edge=begin_poly->edges[i].C_edge;
#ifdef USE_ENTRY_POINT
			Vector2 edge[2]={
				_get_vertex(begin_poly->edges[i].point),
				_get_vertex(begin_poly->edges[(i+1)%begin_poly->edges.size()].point)
			};

			Vector2 entry = Geometry::get_closest_point_to_segment_2d(begin_poly->entry,edge);
			begin_poly->edges[i].C->distance = begin_poly->entry.distance_to(entry);
			begin_poly->edges[i].C->entry=entry;
#else
			begin_poly->edges[i].C->distance=begin_poly->center.distance_to(begin_poly->edges[i].C->center);
#endif
			open_list.push_back(begin_poly->edges[i].C);

			if (begin_poly->edges[i].C==end_poly) {
				found_route=true;
			}
		}
	}


	while(!found_route) {

		if (open_list.size()==0) {
		//	print_line("NOU OPEN LIST");
			break;
		}
		//check open list

		List<Polygon*>::Element *least_cost_poly=NULL;
		float least_cost=1e30;

		//this could be faster (cache previous results)
		for (List<Polygon*>::Element *E=open_list.front();E;E=E->next()) {

			Polygon *p=E->get();


			float cost=p->distance;
			cost+=p->center.distance_to(end_point);

			if (cost<least_cost) {

				least_cost_poly=E;
				least_cost=cost;
			}
		}


		Polygon *p=least_cost_poly->get();
		//open the neighbours for search
		int es = p->edges.size();

		for(int i=0;i<es;i++) {


			Polygon::Edge &e=p->edges[i];

			if (!e.C)
				continue;

#ifdef USE_ENTRY_POINT
			Vector2 edge[2]={
				_get_vertex(p->edges[i].point),
				_get_vertex(p->edges[(i+1)%es].point)
			};

			Vector2 edge_entry = Geometry::get_closest_point_to_segment_2d(p->entry,edge);
			float distance = p->entry.distance_to(edge_entry) + p->distance;

#else

			float distance = p->center.distance_to(e.C->center) + p->distance;

#endif


			if (e.C->prev_edge!=-1) {
				//oh this was visited already, can we win the cost?

				if (e.C->distance>distance) {

					e.C->prev_edge=e.C_edge;
					e.C->distance=distance;
#ifdef USE_ENTRY_POINT
					e.C->entry=edge_entry;
#endif
				}
			} else {
				//add to open neighbours

				e.C->prev_edge=e.C_edge;
				e.C->distance=distance;
#ifdef USE_ENTRY_POINT
				e.C->entry=edge_entry;
#endif

				open_list.push_back(e.C);

				if (e.C==end_poly) {
					//oh my reached end! stop algorithm
					found_route=true;
					break;

				}

			}
		}

		if (found_route)
			break;

		open_list.erase(least_cost_poly);
	}

	if (found_route) {

		Vector<Vector2> path;

		if (p_optimize) {
			//string pulling

			Polygon *apex_poly=end_poly;
			Vector2 apex_point=end_point;
			Vector2 portal_left=apex_point;
			Vector2 portal_right=apex_point;
			Polygon *left_poly=end_poly;
			Polygon *right_poly=end_poly;
			Polygon *p=end_poly;
			path.push_back(end_point);

			while(p) {

				Vector2 left;
				Vector2 right;

//#define CLOCK_TANGENT(m_a,m_b,m_c) ( ((m_a)-(m_c)).cross((m_a)-(m_b)) )
#define CLOCK_TANGENT(m_a,m_b,m_c) ((((m_a).x - (m_c).x) * ((m_b).y - (m_c).y) - ((m_b).x - (m_c).x) * ((m_a).y - (m_c).y)))

				if (p==begin_poly) {
					left=begin_point;
					right=begin_point;
				} else {
					int prev = p->prev_edge;
					int prev_n = (p->prev_edge+1)%p->edges.size();
					left = _get_vertex(p->edges[prev].point);
					right = _get_vertex(p->edges[prev_n].point);

					if (CLOCK_TANGENT(apex_point,left,(left+right)*0.5) < 0){
						SWAP(left,right);
					}
				}

				bool skip=false;


				if (CLOCK_TANGENT(apex_point,portal_left,left) >= 0){
					//process
					if (portal_left==apex_point || CLOCK_TANGENT(apex_point,left,portal_right) > 0) {
						left_poly=p;
						portal_left=left;
					} else {

						//_clip_path(path,apex_poly,portal_right,right_poly);

						apex_point=portal_right;
						p=right_poly;
						left_poly=p;
						apex_poly=p;
						portal_left=apex_point;
						portal_right=apex_point;
						path.push_back(apex_point);
						skip=true;
					}
				}

				if (!skip && CLOCK_TANGENT(apex_point,portal_right,right) <= 0){
					//process
					if (portal_right==apex_point || CLOCK_TANGENT(apex_point,right,portal_left) < 0) {
						right_poly=p;
						portal_right=right;
					} else {

						//_clip_path(path,apex_poly,portal_left,left_poly);

						apex_point=portal_left;
						p=left_poly;
						right_poly=p;
						apex_poly=p;
						portal_right=apex_point;
						portal_left=apex_point;
						path.push_back(apex_point);
					}
				}

				if (p!=begin_poly)
					p=p->edges[p->prev_edge].C;
				else
					p=NULL;

			}

			if (path[path.size()-1]!=begin_point)
				path.push_back(begin_point);

			path.invert();




		} else {
			//midpoints
			Polygon *p=end_poly;

			path.push_back(end_point);
			while(true) {
				int prev = p->prev_edge;
				int prev_n = (p->prev_edge+1)%p->edges.size();
				Vector2 point = (_get_vertex(p->edges[prev].point) + _get_vertex(p->edges[prev_n].point))*0.5;
				path.push_back(point);
				p = p->edges[prev].C;
				if (p==begin_poly)
					break;
			}

			path.push_back(begin_point);


			path.invert();;
		}

		return path;
	}


	return Vector<Vector2>();

}


Vector2 Navigation2D::get_closest_point(const Vector2& p_point) {

	Vector2 closest_point=Vector2();
	float closest_point_d=1e20;

	for (Map<int,NavMesh>::Element*E=navpoly_map.front();E;E=E->next()) {

		if (!E->get().linked)
			continue;
		for(List<Polygon>::Element *F=E->get().polygons.front();F;F=F->next()) {

			Polygon &p=F->get();
			for(int i=2;i<p.edges.size();i++) {

				if (Geometry::is_point_in_triangle(p_point,_get_vertex(p.edges[0].point),_get_vertex(p.edges[i-1].point),_get_vertex(p.edges[i].point))) {

					return p_point; //inside triangle, nothing else to discuss
				}

			}
		}
	}

	for (Map<int,NavMesh>::Element*E=navpoly_map.front();E;E=E->next()) {

		if (!E->get().linked)
			continue;
		for(List<Polygon>::Element *F=E->get().polygons.front();F;F=F->next()) {

			Polygon &p=F->get();
			int es = p.edges.size();
			for(int i=0;i<es;i++) {

				Vector2 edge[2]={
					_get_vertex(p.edges[i].point),
					_get_vertex(p.edges[(i+1)%es].point)
				};


				Vector2 spoint=Geometry::get_closest_point_to_segment_2d(p_point,edge);
				float d = spoint.distance_squared_to(p_point);
				if (d<closest_point_d) {

					closest_point=spoint;
					closest_point_d=d;
				}
			}
		}
	}

	return closest_point;

}


void Navigation2D::_bind_methods() {

	ObjectTypeDB::bind_method(_MD("navpoly_create","mesh:NavigationPolygon","xform","owner"),&Navigation2D::navpoly_create,DEFVAL(Variant()));
	ObjectTypeDB::bind_method(_MD("navpoly_set_transform","id","xform"),&Navigation2D::navpoly_set_transform);
	ObjectTypeDB::bind_method(_MD("navpoly_remove","id"),&Navigation2D::navpoly_remove);

	ObjectTypeDB::bind_method(_MD("get_simple_path","start","end","optimize"),&Navigation2D::get_simple_path,DEFVAL(true));
	ObjectTypeDB::bind_method(_MD("get_closest_point","to_point"),&Navigation2D::get_closest_point);

}

Navigation2D::Navigation2D() {

	ERR_FAIL_COND( sizeof(Point)!=8 );
	cell_size=1; // one pixel
	last_id=1;

}
