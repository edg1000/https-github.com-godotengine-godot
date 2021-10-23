/*************************************************************************/
/*  visual_server_scene.h                                                */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
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

#ifndef VISUALSERVERSCENE_H
#define VISUALSERVERSCENE_H

#include "servers/visual/rasterizer.h"

#include "core/math/bvh.h"
#include "core/math/geometry.h"
#include "core/math/octree.h"
#include "core/os/semaphore.h"
#include "core/os/thread.h"
#include "core/safe_refcount.h"
#include "core/self_list.h"
#include "portals/portal_renderer.h"
#include "servers/arvr/arvr_interface.h"

class VisualServerScene {
public:
	enum {

		MAX_INSTANCE_CULL = 65536,
		MAX_LIGHTS_CULLED = 4096,
		MAX_REFLECTION_PROBES_CULLED = 4096,
		MAX_ROOM_CULL = 32,
		MAX_EXTERIOR_PORTALS = 128,
	};

	enum ShadowMapUpdate {
		SHADOW_MAP_UPDATE_POINT_AND_DIRECTIONAL, // Update both point and directional light shadows for this frame (default).
		SHADOW_MAP_UPDATE_POINT, // Update point light shadows only for this frame.
		SHADOW_MAP_UPDATE_DIRECTIONAL, // Update directional light shadows only for this frame.
	};

	uint64_t render_pass;
	static VisualServerScene *singleton;

	/* CAMERA API */

	struct Camera : public RID_Data {
		enum Type {
			PERSPECTIVE,
			ORTHOGONAL,
			FRUSTUM
		};
		Type type;
		float fov;
		float znear, zfar;
		float size;
		Vector2 offset;
		uint32_t visible_layers;
		bool vaspect;
		RID env;

		Transform transform;
		int32_t previous_room_id_hint;

		Camera() {
			visible_layers = 0xFFFFFFFF;
			fov = 70;
			type = PERSPECTIVE;
			znear = 0.05;
			zfar = 100;
			size = 1.0;
			offset = Vector2();
			vaspect = false;
			previous_room_id_hint = -1;
		}
	};

	mutable RID_Owner<Camera> camera_owner;

	virtual RID camera_create();
	virtual void camera_set_perspective(RID p_camera, float p_fovy_degrees, float p_z_near, float p_z_far);
	virtual void camera_set_orthogonal(RID p_camera, float p_size, float p_z_near, float p_z_far);
	virtual void camera_set_frustum(RID p_camera, float p_size, Vector2 p_offset, float p_z_near, float p_z_far);
	virtual void camera_set_transform(RID p_camera, const Transform &p_transform);
	virtual void camera_set_cull_mask(RID p_camera, uint32_t p_layers);
	virtual void camera_set_environment(RID p_camera, RID p_env);
	virtual void camera_set_use_vertical_aspect(RID p_camera, bool p_enable);

	/* SCENARIO API */

	struct Instance;

	// common interface for all spatial partitioning schemes
	// this is a bit excessive boilerplatewise but can be removed if we decide to stick with one method

	// note this is actually the BVH id +1, so that visual server can test against zero
	// for validity to maintain compatibility with octree (where 0 indicates invalid)
	typedef uint32_t SpatialPartitionID;

	class SpatialPartitioningScene {
	public:
		virtual SpatialPartitionID create(Instance *p_userdata, const AABB &p_aabb = AABB(), int p_subindex = 0, bool p_pairable = false, uint32_t p_pairable_type = 0, uint32_t pairable_mask = 1) = 0;
		virtual void erase(SpatialPartitionID p_handle) = 0;
		virtual void move(SpatialPartitionID p_handle, const AABB &p_aabb) = 0;
		virtual void activate(SpatialPartitionID p_handle, const AABB &p_aabb) {}
		virtual void deactivate(SpatialPartitionID p_handle) {}
		virtual void force_collision_check(SpatialPartitionID p_handle) {}
		virtual void update() {}
		virtual void update_collisions() {}
		virtual void set_pairable(SpatialPartitionID p_handle, bool p_pairable, uint32_t p_pairable_type, uint32_t p_pairable_mask) = 0;
		virtual int cull_convex(const Vector<Plane> &p_convex, Instance **p_result_array, int p_result_max, uint32_t p_mask = 0xFFFFFFFF) = 0;
		virtual int cull_aabb(const AABB &p_aabb, Instance **p_result_array, int p_result_max, int *p_subindex_array = nullptr, uint32_t p_mask = 0xFFFFFFFF) = 0;
		virtual int cull_segment(const Vector3 &p_from, const Vector3 &p_to, Instance **p_result_array, int p_result_max, int *p_subindex_array = nullptr, uint32_t p_mask = 0xFFFFFFFF) = 0;

		typedef void *(*PairCallback)(void *, uint32_t, Instance *, int, uint32_t, Instance *, int);
		typedef void (*UnpairCallback)(void *, uint32_t, Instance *, int, uint32_t, Instance *, int, void *);

		virtual void set_pair_callback(PairCallback p_callback, void *p_userdata) = 0;
		virtual void set_unpair_callback(UnpairCallback p_callback, void *p_userdata) = 0;

		// bvh specific
		virtual void params_set_node_expansion(real_t p_value) {}
		virtual void params_set_pairing_expansion(real_t p_value) {}

		// octree specific
		virtual void set_balance(float p_balance) {}

		virtual ~SpatialPartitioningScene() {}
	};

	class SpatialPartitioningScene_Octree : public SpatialPartitioningScene {
		Octree_CL<Instance, true> _octree;

	public:
		SpatialPartitionID create(Instance *p_userdata, const AABB &p_aabb = AABB(), int p_subindex = 0, bool p_pairable = false, uint32_t p_pairable_type = 0, uint32_t pairable_mask = 1);
		void erase(SpatialPartitionID p_handle);
		void move(SpatialPartitionID p_handle, const AABB &p_aabb);
		void set_pairable(SpatialPartitionID p_handle, bool p_pairable, uint32_t p_pairable_type, uint32_t p_pairable_mask);
		int cull_convex(const Vector<Plane> &p_convex, Instance **p_result_array, int p_result_max, uint32_t p_mask = 0xFFFFFFFF);
		int cull_aabb(const AABB &p_aabb, Instance **p_result_array, int p_result_max, int *p_subindex_array = nullptr, uint32_t p_mask = 0xFFFFFFFF);
		int cull_segment(const Vector3 &p_from, const Vector3 &p_to, Instance **p_result_array, int p_result_max, int *p_subindex_array = nullptr, uint32_t p_mask = 0xFFFFFFFF);
		void set_pair_callback(PairCallback p_callback, void *p_userdata);
		void set_unpair_callback(UnpairCallback p_callback, void *p_userdata);
		void set_balance(float p_balance);
	};

	class SpatialPartitioningScene_BVH : public SpatialPartitioningScene {
		// Note that SpatialPartitionIDs are +1 based when stored in visual server, to enable 0 to indicate invalid ID.
		BVH_Manager<Instance, true, 256> _bvh;

	public:
		SpatialPartitioningScene_BVH();
		SpatialPartitionID create(Instance *p_userdata, const AABB &p_aabb = AABB(), int p_subindex = 0, bool p_pairable = false, uint32_t p_pairable_type = 0, uint32_t p_pairable_mask = 1);
		void erase(SpatialPartitionID p_handle);
		void move(SpatialPartitionID p_handle, const AABB &p_aabb);
		void activate(SpatialPartitionID p_handle, const AABB &p_aabb);
		void deactivate(SpatialPartitionID p_handle);
		void force_collision_check(SpatialPartitionID p_handle);
		void update();
		void update_collisions();
		void set_pairable(SpatialPartitionID p_handle, bool p_pairable, uint32_t p_pairable_type, uint32_t p_pairable_mask);
		int cull_convex(const Vector<Plane> &p_convex, Instance **p_result_array, int p_result_max, uint32_t p_mask = 0xFFFFFFFF);
		int cull_aabb(const AABB &p_aabb, Instance **p_result_array, int p_result_max, int *p_subindex_array = nullptr, uint32_t p_mask = 0xFFFFFFFF);
		int cull_segment(const Vector3 &p_from, const Vector3 &p_to, Instance **p_result_array, int p_result_max, int *p_subindex_array = nullptr, uint32_t p_mask = 0xFFFFFFFF);
		void set_pair_callback(PairCallback p_callback, void *p_userdata);
		void set_unpair_callback(UnpairCallback p_callback, void *p_userdata);

		void params_set_node_expansion(real_t p_value) { _bvh.params_set_node_expansion(p_value); }
		void params_set_pairing_expansion(real_t p_value) { _bvh.params_set_pairing_expansion(p_value); }
	};

	struct Scenario : RID_Data {
		VS::ScenarioDebugMode debug;
		RID self;

		SpatialPartitioningScene *sps;
		PortalRenderer _portal_renderer;

		List<Instance *> directional_lights;
		RID environment;
		RID fallback_environment;
		RID reflection_probe_shadow_atlas;
		RID reflection_atlas;

		SelfList<Instance>::List instances;

		Scenario();
		~Scenario() { memdelete(sps); }
	};

	mutable RID_Owner<Scenario> scenario_owner;

	static void *_instance_pair(void *p_self, SpatialPartitionID, Instance *p_A, int, SpatialPartitionID, Instance *p_B, int);
	static void _instance_unpair(void *p_self, SpatialPartitionID, Instance *p_A, int, SpatialPartitionID, Instance *p_B, int, void *);

	virtual RID scenario_create();

	virtual void scenario_set_debug(RID p_scenario, VS::ScenarioDebugMode p_debug_mode);
	virtual void scenario_set_environment(RID p_scenario, RID p_environment);
	virtual void scenario_set_fallback_environment(RID p_scenario, RID p_environment);
	virtual void scenario_set_reflection_atlas_size(RID p_scenario, int p_size, int p_subdiv);

	/* INSTANCING API */

	struct InstanceBaseData {
		virtual ~InstanceBaseData() {}
	};

	struct Instance : RasterizerScene::InstanceBase {
		RID self;
		//scenario stuff
		SpatialPartitionID spatial_partition_id;

		// rooms & portals
		OcclusionHandle occlusion_handle; // handle of instance in occlusion system (or 0)
		VisualServer::InstancePortalMode portal_mode;

		Scenario *scenario;
		SelfList<Instance> scenario_item;

		//aabb stuff
		bool update_aabb;
		bool update_materials;

		SelfList<Instance> update_item;

		AABB aabb;
		AABB transformed_aabb;
		AABB *custom_aabb; // <Zylann> would using aabb directly with a bool be better?
		float extra_margin;
		uint32_t object_id;

		float lod_begin;
		float lod_end;
		float lod_begin_hysteresis;
		float lod_end_hysteresis;
		RID lod_instance;

		uint64_t last_render_pass;
		uint64_t last_frame_pass;

		uint64_t version; // changes to this, and changes to base increase version

		InstanceBaseData *base_data;

		virtual void base_removed() {
			singleton->instance_set_base(self, RID());
		}

		virtual void base_changed(bool p_aabb, bool p_materials) {
			singleton->_instance_queue_update(this, p_aabb, p_materials);
		}

		Instance() :
				scenario_item(this),
				update_item(this) {
			spatial_partition_id = 0;
			scenario = nullptr;

			update_aabb = false;
			update_materials = false;

			extra_margin = 0;

			object_id = 0;
			visible = true;

			occlusion_handle = 0;
			portal_mode = VisualServer::InstancePortalMode::INSTANCE_PORTAL_MODE_STATIC;

			lod_begin = 0;
			lod_end = 0;
			lod_begin_hysteresis = 0;
			lod_end_hysteresis = 0;

			last_render_pass = 0;
			last_frame_pass = 0;
			version = 1;
			base_data = nullptr;

			custom_aabb = nullptr;
		}

		~Instance() {
			if (base_data) {
				memdelete(base_data);
			}
			if (custom_aabb) {
				memdelete(custom_aabb);
			}
		}
	};

	SelfList<Instance>::List _instance_update_list;
	void _instance_queue_update(Instance *p_instance, bool p_update_aabb, bool p_update_materials = false);

	struct InstanceGeometryData : public InstanceBaseData {
		List<Instance *> lighting;
		bool lighting_dirty;
		bool can_cast_shadows;
		bool material_is_animated;

		List<Instance *> reflection_probes;
		bool reflection_dirty;

		List<Instance *> gi_probes;
		bool gi_probes_dirty;

		List<Instance *> lightmap_captures;

		InstanceGeometryData() {
			lighting_dirty = false;
			reflection_dirty = true;
			can_cast_shadows = true;
			material_is_animated = true;
			gi_probes_dirty = true;
		}
	};

	struct InstanceReflectionProbeData : public InstanceBaseData {
		Instance *owner;

		struct PairInfo {
			List<Instance *>::Element *L; //reflection iterator in geometry
			Instance *geometry;
		};
		List<PairInfo> geometries;

		RID instance;
		bool reflection_dirty;
		SelfList<InstanceReflectionProbeData> update_list;

		int render_step;
		int32_t previous_room_id_hint;

		InstanceReflectionProbeData() :
				update_list(this) {
			reflection_dirty = true;
			render_step = -1;
			previous_room_id_hint = -1;
		}
	};

	SelfList<InstanceReflectionProbeData>::List reflection_probe_render_list;

	struct InstanceLightData : public InstanceBaseData {
		struct PairInfo {
			List<Instance *>::Element *L; //light iterator in geometry
			Instance *geometry;
		};

		RID instance;
		uint64_t last_version;
		List<Instance *>::Element *D; // directional light in scenario

		bool shadow_dirty;

		List<PairInfo> geometries;

		Instance *baked_light;
		int32_t previous_room_id_hint;

		InstanceLightData() {
			shadow_dirty = true;
			D = nullptr;
			last_version = 0;
			baked_light = nullptr;
			previous_room_id_hint = -1;
		}
	};

	struct InstanceGIProbeData : public InstanceBaseData {
		Instance *owner;

		struct PairInfo {
			List<Instance *>::Element *L; //gi probe iterator in geometry
			Instance *geometry;
		};

		List<PairInfo> geometries;

		Set<Instance *> lights;

		struct LightCache {
			VS::LightType type;
			Transform transform;
			Color color;
			float energy;
			float radius;
			float attenuation;
			float spot_angle;
			float spot_attenuation;
			bool visible;

			bool operator==(const LightCache &p_cache) {
				return (type == p_cache.type &&
						transform == p_cache.transform &&
						color == p_cache.color &&
						energy == p_cache.energy &&
						radius == p_cache.radius &&
						attenuation == p_cache.attenuation &&
						spot_angle == p_cache.spot_angle &&
						spot_attenuation == p_cache.spot_attenuation &&
						visible == p_cache.visible);
			}

			bool operator!=(const LightCache &p_cache) {
				return !operator==(p_cache);
			}

			LightCache() {
				type = VS::LIGHT_DIRECTIONAL;
				energy = 1.0;
				radius = 1.0;
				attenuation = 1.0;
				spot_angle = 1.0;
				spot_attenuation = 1.0;
				visible = true;
			}
		};

		struct LocalData {
			uint16_t pos[3];
			uint16_t energy[3]; //using 0..1024 for float range 0..1. integer is needed for deterministic add/remove of lights
		};

		struct CompBlockS3TC {
			uint32_t offset; //offset in mipmap
			uint32_t source_count; //sources
			uint32_t sources[16]; //id for each source
			uint8_t alpha[8]; //alpha block is pre-computed
		};

		struct Dynamic {
			Map<RID, LightCache> light_cache;
			Map<RID, LightCache> light_cache_changes;
			PoolVector<int> light_data;
			PoolVector<LocalData> local_data;
			Vector<Vector<uint32_t>> level_cell_lists;
			RID probe_data;
			bool enabled;
			int bake_dynamic_range;
			RasterizerStorage::GIProbeCompression compression;

			Vector<PoolVector<uint8_t>> mipmaps_3d;
			Vector<PoolVector<CompBlockS3TC>> mipmaps_s3tc; //for s3tc

			int updating_stage;
			float propagate;

			int grid_size[3];

			Transform light_to_cell_xform;

		} dynamic;

		RID probe_instance;

		bool invalid;
		uint32_t base_version;

		SelfList<InstanceGIProbeData> update_element;

		InstanceGIProbeData() :
				update_element(this) {
			invalid = true;
			base_version = 0;
			dynamic.updating_stage = GI_UPDATE_STAGE_CHECK;
		}
	};

	SelfList<InstanceGIProbeData>::List gi_probe_update_list;

	struct InstanceLightmapCaptureData : public InstanceBaseData {
		struct PairInfo {
			List<Instance *>::Element *L; //iterator in geometry
			Instance *geometry;
		};
		List<PairInfo> geometries;

		Set<Instance *> users;

		InstanceLightmapCaptureData() {
		}
	};

	int instance_cull_count;
	Instance *instance_cull_result[MAX_INSTANCE_CULL];
	Instance *instance_shadow_cull_result[MAX_INSTANCE_CULL]; //used for generating shadowmaps
	Instance *light_cull_result[MAX_LIGHTS_CULLED];
	RID light_instance_cull_result[MAX_LIGHTS_CULLED];
	int light_cull_count;
	int directional_light_count;
	RID reflection_probe_instance_cull_result[MAX_REFLECTION_PROBES_CULLED];
	int reflection_probe_cull_count;
	ShadowMapUpdate shadow_map_update = ShadowMapUpdate::SHADOW_MAP_UPDATE_POINT_AND_DIRECTIONAL;

	RID_Owner<Instance> instance_owner;

	virtual RID instance_create();

	virtual void instance_set_base(RID p_instance, RID p_base);
	virtual void instance_set_scenario(RID p_instance, RID p_scenario);
	virtual void instance_set_layer_mask(RID p_instance, uint32_t p_mask);
	virtual void instance_set_transform(RID p_instance, const Transform &p_transform);
	virtual void instance_attach_object_instance_id(RID p_instance, ObjectID p_id);
	virtual void instance_set_blend_shape_weight(RID p_instance, int p_shape, float p_weight);
	virtual void instance_set_surface_material(RID p_instance, int p_surface, RID p_material);
	virtual void instance_set_visible(RID p_instance, bool p_visible);
	virtual void instance_set_use_lightmap(RID p_instance, RID p_lightmap_instance, RID p_lightmap, int p_lightmap_slice, const Rect2 &p_lightmap_uv_rect);

	virtual void instance_set_custom_aabb(RID p_instance, AABB p_aabb);

	virtual void instance_attach_skeleton(RID p_instance, RID p_skeleton);
	virtual void instance_set_exterior(RID p_instance, bool p_enabled);

	virtual void instance_set_extra_visibility_margin(RID p_instance, real_t p_margin);

	// Portals
	virtual void instance_set_portal_mode(RID p_instance, VisualServer::InstancePortalMode p_mode);
	bool _instance_get_transformed_aabb(RID p_instance, AABB &r_aabb);
	bool _instance_get_transformed_aabb_for_occlusion(VSInstance *p_instance, AABB &r_aabb) const {
		r_aabb = ((Instance *)p_instance)->transformed_aabb;
		return ((Instance *)p_instance)->portal_mode != VisualServer::INSTANCE_PORTAL_MODE_GLOBAL;
	}
	void *_instance_get_from_rid(RID p_instance);
	bool _instance_cull_check(VSInstance *p_instance, uint32_t p_cull_mask) const {
		uint32_t pairable_type = 1 << ((Instance *)p_instance)->base_type;
		return pairable_type & p_cull_mask;
	}
	ObjectID _instance_get_object_ID(VSInstance *p_instance) const {
		if (p_instance) {
			return ((Instance *)p_instance)->object_id;
		}
		return 0;
	}

private:
	void _instance_create_occlusion_rep(Instance *p_instance);
	void _instance_destroy_occlusion_rep(Instance *p_instance);

public:
	struct Ghost : RID_Data {
		// all interations with actual ghosts are indirect, as the ghost is part of the scenario
		Scenario *scenario = nullptr;
		uint32_t object_id = 0;
		RGhostHandle rghost_handle = 0; // handle in occlusion system (or 0)
		AABB aabb;
		virtual ~Ghost() {
			if (scenario) {
				if (rghost_handle) {
					scenario->_portal_renderer.rghost_destroy(rghost_handle);
					rghost_handle = 0;
				}
				scenario = nullptr;
			}
		}
	};
	RID_Owner<Ghost> ghost_owner;

	virtual RID ghost_create();
	virtual void ghost_set_scenario(RID p_ghost, RID p_scenario, ObjectID p_id, const AABB &p_aabb);
	virtual void ghost_update(RID p_ghost, const AABB &p_aabb);

private:
	void _ghost_create_occlusion_rep(Ghost *p_ghost);
	void _ghost_destroy_occlusion_rep(Ghost *p_ghost);

public:
	struct Portal : RID_Data {
		// all interations with actual portals are indirect, as the portal is part of the scenario
		uint32_t scenario_portal_id = 0;
		Scenario *scenario = nullptr;
		virtual ~Portal() {
			if (scenario) {
				scenario->_portal_renderer.portal_destroy(scenario_portal_id);
				scenario = nullptr;
				scenario_portal_id = 0;
			}
		}
	};
	RID_Owner<Portal> portal_owner;

	virtual RID portal_create();
	virtual void portal_set_scenario(RID p_portal, RID p_scenario);
	virtual void portal_set_geometry(RID p_portal, const Vector<Vector3> &p_points, real_t p_margin);
	virtual void portal_link(RID p_portal, RID p_room_from, RID p_room_to, bool p_two_way);
	virtual void portal_set_active(RID p_portal, bool p_active);

	// RoomGroups
	struct RoomGroup : RID_Data {
		// all interations with actual roomgroups are indirect, as the roomgroup is part of the scenario
		uint32_t scenario_roomgroup_id = 0;
		Scenario *scenario = nullptr;
		virtual ~RoomGroup() {
			if (scenario) {
				scenario->_portal_renderer.roomgroup_destroy(scenario_roomgroup_id);
				scenario = nullptr;
				scenario_roomgroup_id = 0;
			}
		}
	};
	RID_Owner<RoomGroup> roomgroup_owner;

	virtual RID roomgroup_create();
	virtual void roomgroup_prepare(RID p_roomgroup, ObjectID p_roomgroup_object_id);
	virtual void roomgroup_set_scenario(RID p_roomgroup, RID p_scenario);
	virtual void roomgroup_add_room(RID p_roomgroup, RID p_room);

	// Occluders
	struct Occluder : RID_Data {
		uint32_t scenario_occluder_id = 0;
		Scenario *scenario = nullptr;
		virtual ~Occluder() {
			if (scenario) {
				scenario->_portal_renderer.occluder_destroy(scenario_occluder_id);
				scenario = nullptr;
				scenario_occluder_id = 0;
			}
		}
	};
	RID_Owner<Occluder> occluder_owner;

	virtual RID occluder_create();
	virtual void occluder_set_scenario(RID p_occluder, RID p_scenario, VisualServer::OccluderType p_type);
	virtual void occluder_spheres_update(RID p_occluder, const Vector<Plane> &p_spheres);
	virtual void occluder_set_transform(RID p_occluder, const Transform &p_xform);
	virtual void occluder_set_active(RID p_occluder, bool p_active);
	virtual void set_use_occlusion_culling(bool p_enable);

	// Rooms
	struct Room : RID_Data {
		// all interations with actual rooms are indirect, as the room is part of the scenario
		uint32_t scenario_room_id = 0;
		Scenario *scenario = nullptr;
		virtual ~Room() {
			if (scenario) {
				scenario->_portal_renderer.room_destroy(scenario_room_id);
				scenario = nullptr;
				scenario_room_id = 0;
			}
		}
	};
	RID_Owner<Room> room_owner;

	virtual RID room_create();
	virtual void room_set_scenario(RID p_room, RID p_scenario);
	virtual void room_add_instance(RID p_room, RID p_instance, const AABB &p_aabb, const Vector<Vector3> &p_object_pts);
	virtual void room_add_ghost(RID p_room, ObjectID p_object_id, const AABB &p_aabb);
	virtual void room_set_bound(RID p_room, ObjectID p_room_object_id, const Vector<Plane> &p_convex, const AABB &p_aabb, const Vector<Vector3> &p_verts);
	virtual void room_prepare(RID p_room, int32_t p_priority);
	virtual void rooms_and_portals_clear(RID p_scenario);
	virtual void rooms_unload(RID p_scenario, String p_reason);
	virtual void rooms_finalize(RID p_scenario, bool p_generate_pvs, bool p_cull_using_pvs, bool p_use_secondary_pvs, bool p_use_signals, String p_pvs_filename, bool p_use_simple_pvs, bool p_log_pvs_generation);
	virtual void rooms_override_camera(RID p_scenario, bool p_override, const Vector3 &p_point, const Vector<Plane> *p_convex);
	virtual void rooms_set_active(RID p_scenario, bool p_active);
	virtual void rooms_set_params(RID p_scenario, int p_portal_depth_limit);
	virtual void rooms_set_debug_feature(RID p_scenario, VisualServer::RoomsDebugFeature p_feature, bool p_active);
	virtual void rooms_update_gameplay_monitor(RID p_scenario, const Vector<Vector3> &p_camera_positions);

	// don't use this in a game
	virtual bool rooms_is_loaded(RID p_scenario) const;

	virtual void callbacks_register(VisualServerCallbacks *p_callbacks);
	VisualServerCallbacks *get_callbacks() const { return _visual_server_callbacks; }

	// don't use these in a game!
	virtual Vector<ObjectID> instances_cull_aabb(const AABB &p_aabb, RID p_scenario = RID()) const;
	virtual Vector<ObjectID> instances_cull_ray(const Vector3 &p_from, const Vector3 &p_to, RID p_scenario = RID()) const;
	virtual Vector<ObjectID> instances_cull_convex(const Vector<Plane> &p_convex, RID p_scenario = RID()) const;

	// internal (uses portals when available)
	int _cull_convex_from_point(Scenario *p_scenario, const Vector3 &p_point, const Vector<Plane> &p_convex, Instance **p_result_array, int p_result_max, int32_t &r_previous_room_id_hint, uint32_t p_mask = 0xFFFFFFFF);
	void _rooms_instance_update(Instance *p_instance, const AABB &p_aabb);

	virtual void instance_geometry_set_flag(RID p_instance, VS::InstanceFlags p_flags, bool p_enabled);
	virtual void instance_geometry_set_cast_shadows_setting(RID p_instance, VS::ShadowCastingSetting p_shadow_casting_setting);
	virtual void instance_geometry_set_material_override(RID p_instance, RID p_material);

	virtual void instance_geometry_set_draw_range(RID p_instance, float p_min, float p_max, float p_min_margin, float p_max_margin);
	virtual void instance_geometry_set_as_instance_lod(RID p_instance, RID p_as_lod_of_instance);

	_FORCE_INLINE_ void _update_instance(Instance *p_instance);
	_FORCE_INLINE_ void _update_instance_aabb(Instance *p_instance);
	_FORCE_INLINE_ void _update_dirty_instance(Instance *p_instance);
	_FORCE_INLINE_ void _update_instance_lightmap_captures(Instance *p_instance);

	_FORCE_INLINE_ bool _light_instance_update_shadow(Instance *p_instance, const Transform p_cam_transform, const CameraMatrix &p_cam_projection, bool p_cam_orthogonal, RID p_shadow_atlas, Scenario *p_scenario);

	void _prepare_scene(const Transform p_cam_transform, const CameraMatrix &p_cam_projection, bool p_cam_orthogonal, RID p_force_environment, uint32_t p_visible_layers, RID p_scenario, RID p_shadow_atlas, RID p_reflection_probe, int32_t &r_previous_room_id_hint);
	void _render_scene(const Transform p_cam_transform, const CameraMatrix &p_cam_projection, const int p_eye, bool p_cam_orthogonal, RID p_force_environment, RID p_scenario, RID p_shadow_atlas, RID p_reflection_probe, int p_reflection_probe_pass);
	void render_empty_scene(RID p_scenario, RID p_shadow_atlas);

	void render_camera(RID p_camera, RID p_scenario, Size2 p_viewport_size, RID p_shadow_atlas);
	void render_camera(Ref<ARVRInterface> &p_interface, ARVRInterface::Eyes p_eye, RID p_camera, RID p_scenario, Size2 p_viewport_size, RID p_shadow_atlas);
	void update_dirty_instances();

	//probes
	struct GIProbeDataHeader {
		uint32_t version;
		uint32_t cell_subdiv;
		uint32_t width;
		uint32_t height;
		uint32_t depth;
		uint32_t cell_count;
		uint32_t leaf_cell_count;
	};

	struct GIProbeDataCell {
		uint32_t children[8];
		uint32_t albedo;
		uint32_t emission;
		uint32_t normal;
		uint32_t level_alpha;
	};

	enum {
		GI_UPDATE_STAGE_CHECK,
		GI_UPDATE_STAGE_LIGHTING,
		GI_UPDATE_STAGE_UPLOADING,
	};

	void _gi_probe_bake_thread();
	static void _gi_probe_bake_threads(void *);

	bool probe_bake_thread_exit;
	Thread probe_bake_thread;
	Semaphore probe_bake_sem;
	Mutex probe_bake_mutex;
	List<Instance *> probe_bake_list;

	bool _render_reflection_probe_step(Instance *p_instance, int p_step);
	void _gi_probe_fill_local_data(int p_idx, int p_level, int p_x, int p_y, int p_z, const GIProbeDataCell *p_cell, const GIProbeDataHeader *p_header, InstanceGIProbeData::LocalData *p_local_data, Vector<uint32_t> *prev_cell);

	_FORCE_INLINE_ uint32_t _gi_bake_find_cell(const GIProbeDataCell *cells, int x, int y, int z, int p_cell_subdiv);
	void _bake_gi_downscale_light(int p_idx, int p_level, const GIProbeDataCell *p_cells, const GIProbeDataHeader *p_header, InstanceGIProbeData::LocalData *p_local_data, float p_propagate);
	void _bake_gi_probe_light(const GIProbeDataHeader *header, const GIProbeDataCell *cells, InstanceGIProbeData::LocalData *local_data, const uint32_t *leaves, int p_leaf_count, const InstanceGIProbeData::LightCache &light_cache, int p_sign);
	void _bake_gi_probe(Instance *p_gi_probe);
	bool _check_gi_probe(Instance *p_gi_probe);
	void _setup_gi_probe(Instance *p_instance);

	void render_probes();

	bool free(RID p_rid);

private:
	bool _use_bvh;
	VisualServerCallbacks *_visual_server_callbacks;

public:
	VisualServerScene();
	virtual ~VisualServerScene();
};

#endif // VISUALSERVERSCENE_H
