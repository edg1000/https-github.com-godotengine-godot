/*************************************************************************/
/*  csg_shape.h                                                          */
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

#ifndef CSG_SHAPE_H
#define CSG_SHAPE_H

#define CSGJS_HEADER_ONLY

#include "csg.h"
#include "scene/3d/visual_instance_3d.h"
#include "scene/resources/concave_polygon_shape_3d.h"
#include "thirdparty/misc/mikktspace.h"

class CSGShape3D : public GeometryInstance3D {
	GDCLASS(CSGShape3D, GeometryInstance3D);

public:
	enum Operation {
		OPERATION_UNION,
		OPERATION_INTERSECTION,
		OPERATION_SUBTRACTION,

	};

private:
	Operation operation;
	CSGShape3D *parent;

	CSGBrush *brush;

	AABB node_aabb;

	bool dirty;
	float snap;

	bool use_collision;
	uint32_t collision_layer;
	uint32_t collision_mask;
	Ref<ConcavePolygonShape3D> root_collision_shape;
	RID root_collision_instance;

	bool calculate_tangents;

	Ref<ArrayMesh> root_mesh;

	struct Vector3Hasher {
		_ALWAYS_INLINE_ uint32_t hash(const Vector3 &p_vec3) const {
			uint32_t h = hash_djb2_one_float(p_vec3.x);
			h = hash_djb2_one_float(p_vec3.y, h);
			h = hash_djb2_one_float(p_vec3.z, h);
			return h;
		}
	};

	struct ShapeUpdateSurface {
		Vector<Vector3> vertices;
		Vector<Vector3> normals;
		Vector<Vector2> uvs;
		Vector<float> tans;
		Ref<Material> material;
		int last_added;

		Vector3 *verticesw;
		Vector3 *normalsw;
		Vector2 *uvsw;
		float *tansw;
	};

	//mikktspace callbacks
	static int mikktGetNumFaces(const SMikkTSpaceContext *pContext);
	static int mikktGetNumVerticesOfFace(const SMikkTSpaceContext *pContext, const int iFace);
	static void mikktGetPosition(const SMikkTSpaceContext *pContext, float fvPosOut[], const int iFace, const int iVert);
	static void mikktGetNormal(const SMikkTSpaceContext *pContext, float fvNormOut[], const int iFace, const int iVert);
	static void mikktGetTexCoord(const SMikkTSpaceContext *pContext, float fvTexcOut[], const int iFace, const int iVert);
	static void mikktSetTSpaceDefault(const SMikkTSpaceContext *pContext, const float fvTangent[], const float fvBiTangent[], const float fMagS, const float fMagT,
			const tbool bIsOrientationPreserving, const int iFace, const int iVert);

	void _update_shape();

protected:
	void _notification(int p_what);
	virtual CSGBrush *_build_brush() = 0;
	void _make_dirty();

	static void _bind_methods();

	friend class CSGCombiner3D;
	CSGBrush *_get_brush();

	virtual void _validate_property(PropertyInfo &property) const override;

public:
	Array get_meshes() const;

	void set_operation(Operation p_operation);
	Operation get_operation() const;

	virtual Vector<Vector3> get_brush_faces();

	virtual AABB get_aabb() const override;
	virtual Vector<Face3> get_faces(uint32_t p_usage_flags) const override;

	void set_use_collision(bool p_enable);
	bool is_using_collision() const;

	void set_collision_layer(uint32_t p_layer);
	uint32_t get_collision_layer() const;

	void set_collision_mask(uint32_t p_mask);
	uint32_t get_collision_mask() const;

	void set_collision_layer_bit(int p_bit, bool p_value);
	bool get_collision_layer_bit(int p_bit) const;

	void set_collision_mask_bit(int p_bit, bool p_value);
	bool get_collision_mask_bit(int p_bit) const;

	void set_snap(float p_snap);
	float get_snap() const;

	void set_calculate_tangents(bool p_calculate_tangents);
	bool is_calculating_tangents() const;

	bool is_root_shape() const;
	CSGShape3D();
	~CSGShape3D();
};

VARIANT_ENUM_CAST(CSGShape3D::Operation)

class CSGCombiner3D : public CSGShape3D {
	GDCLASS(CSGCombiner3D, CSGShape3D);

private:
	virtual CSGBrush *_build_brush() override;

public:
	CSGCombiner3D();
};

class CSGPrimitive3D : public CSGShape3D {
	GDCLASS(CSGPrimitive3D, CSGShape3D);

private:
	bool invert_faces;

protected:
	CSGBrush *_create_brush_from_arrays(const Vector<Vector3> &p_vertices, const Vector<Vector2> &p_uv, const Vector<bool> &p_smooth, const Vector<Ref<Material>> &p_materials);
	static void _bind_methods();

public:
	void set_invert_faces(bool p_invert);
	bool is_inverting_faces();

	CSGPrimitive3D();
};

class CSGMesh3D : public CSGPrimitive3D {
	GDCLASS(CSGMesh3D, CSGPrimitive3D);

	virtual CSGBrush *_build_brush() override;

	Ref<Mesh> mesh;
	Ref<Material> material;

	void _mesh_changed();

protected:
	static void _bind_methods();

public:
	void set_mesh(const Ref<Mesh> &p_mesh);
	Ref<Mesh> get_mesh();

	void set_material(const Ref<Material> &p_material);
	Ref<Material> get_material() const;
};

class CSGSphere3D : public CSGPrimitive3D {
	GDCLASS(CSGSphere3D, CSGPrimitive3D);
	virtual CSGBrush *_build_brush() override;

	Ref<Material> material;
	bool smooth_faces;
	float radius;
	int radial_segments;
	int rings;

protected:
	static void _bind_methods();

public:
	void set_radius(const float p_radius);
	float get_radius() const;

	void set_radial_segments(const int p_radial_segments);
	int get_radial_segments() const;

	void set_rings(const int p_rings);
	int get_rings() const;

	void set_material(const Ref<Material> &p_material);
	Ref<Material> get_material() const;

	void set_smooth_faces(bool p_smooth_faces);
	bool get_smooth_faces() const;

	CSGSphere3D();
};

class CSGBox3D : public CSGPrimitive3D {
	GDCLASS(CSGBox3D, CSGPrimitive3D);
	virtual CSGBrush *_build_brush() override;

	Ref<Material> material;
	Vector3 size = Vector3(2, 2, 2);

protected:
	static void _bind_methods();

public:
	void set_size(const Vector3 &p_size);
	Vector3 get_size() const;

	void set_material(const Ref<Material> &p_material);
	Ref<Material> get_material() const;

	CSGBox3D() {}
};

class CSGCylinder3D : public CSGPrimitive3D {
	GDCLASS(CSGCylinder3D, CSGPrimitive3D);
	virtual CSGBrush *_build_brush() override;

	Ref<Material> material;
	float radius;
	float height;
	int sides;
	bool cone;
	bool smooth_faces;

protected:
	static void _bind_methods();

public:
	void set_radius(const float p_radius);
	float get_radius() const;

	void set_height(const float p_height);
	float get_height() const;

	void set_sides(const int p_sides);
	int get_sides() const;

	void set_cone(const bool p_cone);
	bool is_cone() const;

	void set_smooth_faces(bool p_smooth_faces);
	bool get_smooth_faces() const;

	void set_material(const Ref<Material> &p_material);
	Ref<Material> get_material() const;

	CSGCylinder3D();
};

class CSGTorus3D : public CSGPrimitive3D {
	GDCLASS(CSGTorus3D, CSGPrimitive3D);
	virtual CSGBrush *_build_brush() override;

	Ref<Material> material;
	float inner_radius;
	float outer_radius;
	int sides;
	int ring_sides;
	bool smooth_faces;

protected:
	static void _bind_methods();

public:
	void set_inner_radius(const float p_inner_radius);
	float get_inner_radius() const;

	void set_outer_radius(const float p_outer_radius);
	float get_outer_radius() const;

	void set_sides(const int p_sides);
	int get_sides() const;

	void set_ring_sides(const int p_ring_sides);
	int get_ring_sides() const;

	void set_smooth_faces(bool p_smooth_faces);
	bool get_smooth_faces() const;

	void set_material(const Ref<Material> &p_material);
	Ref<Material> get_material() const;

	CSGTorus3D();
};

class CSGPolygon3D : public CSGPrimitive3D {
	GDCLASS(CSGPolygon3D, CSGPrimitive3D);

public:
	enum Mode {
		MODE_DEPTH,
		MODE_SPIN,
		MODE_PATH
	};

	enum PathRotation {
		PATH_ROTATION_POLYGON,
		PATH_ROTATION_PATH,
		PATH_ROTATION_PATH_FOLLOW,
	};

private:
	virtual CSGBrush *_build_brush() override;

	Vector<Vector2> polygon;
	Ref<Material> material;

	Mode mode;

	float depth;

	float spin_degrees;
	int spin_sides;

	NodePath path_node;
	float path_interval;
	PathRotation path_rotation;
	bool path_local;

	Node *path_cache;

	bool smooth_faces;
	bool path_continuous_u;
	bool path_joined;

	bool _is_editable_3d_polygon() const;
	bool _has_editable_3d_polygon_no_depth() const;

	void _path_changed();
	void _path_exited();

protected:
	static void _bind_methods();
	virtual void _validate_property(PropertyInfo &property) const override;
	void _notification(int p_what);

public:
	void set_polygon(const Vector<Vector2> &p_polygon);
	Vector<Vector2> get_polygon() const;

	void set_mode(Mode p_mode);
	Mode get_mode() const;

	void set_depth(float p_depth);
	float get_depth() const;

	void set_spin_degrees(float p_spin_degrees);
	float get_spin_degrees() const;

	void set_spin_sides(int p_spin_sides);
	int get_spin_sides() const;

	void set_path_node(const NodePath &p_path);
	NodePath get_path_node() const;

	void set_path_interval(float p_interval);
	float get_path_interval() const;

	void set_path_rotation(PathRotation p_rotation);
	PathRotation get_path_rotation() const;

	void set_path_local(bool p_enable);
	bool is_path_local() const;

	void set_path_continuous_u(bool p_enable);
	bool is_path_continuous_u() const;

	void set_path_joined(bool p_enable);
	bool is_path_joined() const;

	void set_smooth_faces(bool p_smooth_faces);
	bool get_smooth_faces() const;

	void set_material(const Ref<Material> &p_material);
	Ref<Material> get_material() const;

	CSGPolygon3D();
};

VARIANT_ENUM_CAST(CSGPolygon3D::Mode)
VARIANT_ENUM_CAST(CSGPolygon3D::PathRotation)

#endif // CSG_SHAPE_H
