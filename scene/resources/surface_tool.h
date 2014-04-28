/*************************************************************************/
/*  surface_tool.h                                                       */
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
#ifndef SURFACE_TOOL_H
#define SURFACE_TOOL_H

#include "scene/resources/mesh.h"



class SurfaceTool : public Reference {

	OBJ_TYPE(SurfaceTool, Reference );
public:
	struct Vertex {

			Vector3 vertex;
			Color color;
			Vector3 normal; // normal, binormal, tangent
			Vector3 binormal;
			Vector3 tangent;
			Vector2 uv;
			Vector2 uv2;
			Vector<int> bones;
			Vector<real_t> weights;

			Vertex() {  }
	};


private:
	bool compare(const Vertex& p_a,const Vertex& p_b) const;

	bool begun;
	bool first;
	Mesh::PrimitiveType primitive;
	int format;
	Ref<Material> material;
	//arrays
	List< Vertex > vertex_array;
	List< int > index_array;

	//memory
	Color last_color;
	Vector3 last_normal;
	Vector2 last_uv;
	Vector2 last_uv2;
	Vector<int> last_bones;
	Vector<real_t> last_weights;
	Plane last_tangent;

	void _create_list(const Ref<Mesh>& p_existing, int p_surface, List<Vertex> *r_vertex, List<int> *r_index,int &lformat);

protected:

	static void _bind_methods();

public:

	void begin(Mesh::PrimitiveType p_primitive);

	void add_vertex( const Vector3& p_vertex);
	void add_color( Color p_color );
	void add_normal( const Vector3& p_normal);
	void add_tangent( const Plane& p_tangent );
	void add_uv( const Vector2& p_uv);
	void add_uv2( const Vector2& p_uv);
	void add_bones( const Vector<int>& p_indices);
	void add_weights( const Vector<real_t>& p_weights);

	void add_index( int p_index);

	void index();
	void deindex();
	void generate_flat_normals();
	void generate_smooth_normals();
	void generate_tangents();

	void add_to_format(int p_flags) { format|=p_flags; }

	void set_material(const Ref<Material>& p_material);

	void clear();

	List< Vertex > &get_vertex_array() { return vertex_array; }

	void create_from(const Ref<Mesh>& p_existing, int p_surface);
	void append_from(const Ref<Mesh>& p_existing, int p_surface,const Transform& p_xform);
	Ref<Mesh> commit(const Ref<Mesh>& p_existing=Ref<Mesh>());

	SurfaceTool();
};


#endif
