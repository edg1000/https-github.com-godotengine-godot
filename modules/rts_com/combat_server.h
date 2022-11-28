#ifndef COMBAT_SERVER_H
#define COMBAT_SERVER_H

#include <memory>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>

#include "core/hash_map.h"
#include "core/object.h"
#include "core/rid.h"
#include "core/vector.h"
#include "rcs_maincomp.h"

#define MAX_OBJECT_PER_CONTAINER 1024
// #define USE_THREAD_SAFE_API

class Sentrience;
class SentrienceInstancesWatcher;

class SentrienceInstancesWatcher : public Reference {
	GDCLASS(SentrienceInstancesWatcher, Reference);
private:
	List<RID_TYPE> rid_pool;
	std::recursive_mutex lock;
private:
	_FORCE_INLINE_ void add_rid(const RID_TYPE& rid){
		std::lock_guard<std::recursive_mutex> guard(lock);
		rid_pool.push_back(rid);
	}
	_FORCE_INLINE_ void remove_rid(const RID_TYPE& rid){
		std::lock_guard<std::recursive_mutex> guard(lock);
		rid_pool.erase(rid);
	}
	_FORCE_INLINE_ List<RID_TYPE> *get_rid_pool() { return &rid_pool; } 
	friend class Sentrience;
protected:
	static void _bind_methods();
public:
	SentrienceInstancesWatcher(){}
	~SentrienceInstancesWatcher(){}

	void flush_all();
	_FORCE_INLINE_ uint32_t size() const {
		return rid_pool.size();
	}
	_FORCE_INLINE_ Array get_rids() const {
		Array re;
		for (auto E = rid_pool.front(); E; E = E->next()){
			re.push_back(E->get());
		}
		return re;
	}
};

class Sentrience : public Object {
	GDCLASS(Sentrience, Object);

	bool active;
#ifdef USE_THREAD_SAFE_API
	uint64_t gc_interval_msec = 5;
	bool gc_close = false;
	std::thread *gc_thread;
	std::mutex gc_queue_mutex;
	std::recursive_mutex mutex_simulations;
	std::recursive_mutex mutex_recordings;
	std::recursive_mutex mutex_combatants;
	std::recursive_mutex mutex_squads;
	std::recursive_mutex mutex_teams;
	std::recursive_mutex mutex_radars;
	List<RID_TYPE> rid_deletion_queue;
	List<RID_TYPE> all_rids;
#else
	Ref<SentrienceInstancesWatcher> active_watcher;
	std::recursive_mutex watcher_mutex;
#endif

	VECTOR<RCSSimulation*> active_simulations;
	VECTOR<RCSRecording*> active_rec;
	// VECTOR<Ref<RCSUnilateralTeamsBind>> active_team_links;
// #define USE_SAFE_RID_COUNT

#ifdef USE_SAFE_RID_COUNT
	mutable RCS_Owner<RCSRecording> recording_owner;
	mutable RCS_Owner<RCSSimulation> simulation_owner;
	mutable RCS_Owner<RCSCombatant> combatant_owner;
	mutable RCS_Owner<RCSSquad> squad_owner;
	mutable RCS_Owner<RCSTeam> team_owner;
	mutable RCS_Owner<RCSRadar> radar_owner;
#else
	mutable RID_Owner<RCSRecording> recording_owner;
	mutable RID_Owner<RCSSimulation> simulation_owner;
	mutable RID_Owner<RCSCombatant> combatant_owner;
	mutable RID_Owner<RCSSquad> squad_owner;
	mutable RID_Owner<RCSTeam> team_owner;
	mutable RID_Owner<RCSRadar> radar_owner;
#endif
protected:
	static void _bind_methods();
	static Sentrience* singleton;

	static void log(const String& msg);
	static void log(const std::string& msg);
	static void log(const char *msg);
	
	String rid_sort(const RID_TYPE& target);

	void free_single_rid_internal(const RID_TYPE& target);
	void free_rid_internal();
	void gc_worker();
public:
	Sentrience();
	~Sentrience();

	// _FORCE_INLINE_ RID_Owner<RCSCombatant> *get_combatant_owner() { return &combatant_owner; }

	// friend class std::thread;

	static Sentrience* get_singleton() { return singleton; }
	virtual void poll(const float& delta);

	/* Core */
	_FORCE_INLINE_ void set_active(const bool& is_active) { active = is_active;}
	_FORCE_INLINE_ bool get_state() const { return active; }
#ifdef USE_THREAD_SAFE_API
	_FORCE_INLINE_ uint32_t get_instances_count() const { return all_rids.size(); }
#else
	_FORCE_INLINE_ uint32_t get_instances_count() const {
		if (active_watcher.is_null()) return 0;
		return active_watcher->size();
	}
#endif
	_FORCE_INLINE_ uint64_t get_memory_usage() const { return RCSMemoryAllocation::tracker_ptr->currently_allocated(); }
	_FORCE_INLINE_ String get_memory_usage_humanized() const { return String::humanize_size(get_memory_usage()); }
	virtual void free_rid(const RID_TYPE& target);
	void free_all_instances();
	void flush_instances_pool();
#ifndef USE_THREAD_SAFE_API
	Ref<SentrienceInstancesWatcher> watcher_create();
	void watcher_remove();
	void watcher_flush(Ref<SentrienceInstancesWatcher> watcher);
#else
	_FORCE_INLINE_ uint8_t watcher_create() { return 0; }
	_FORCE_INLINE_ void watcher_remove() {}
	void watcher_flush(uint32_t& watcher) {}
#endif

	/* Recording API */
	virtual RID_TYPE recording_create();
	virtual bool recording_assert(const RID_TYPE& r_rec);
	virtual bool recording_start(const RID_TYPE& r_rec);
	virtual bool recording_end(const RID_TYPE& r_rec);
	virtual bool recording_running(const RID_TYPE& r_rec);
	virtual void recording_purge(const RID_TYPE& r_rec);

	/* Simulation API */
	virtual RID_TYPE simulation_create();
	virtual bool simulation_assert(const RID_TYPE& r_simul);
	virtual void simulation_set_active(const RID_TYPE& r_simul, const bool& p_active);
	virtual bool simulation_is_active(const RID_TYPE& r_simul);
	virtual Array simulation_get_all_engagements(const RID_TYPE& r_simul);
	virtual void simulation_bind_recording(const RID_TYPE& r_simul, const RID_TYPE& r_rec);
	virtual void simulation_unbind_recording(const RID_TYPE& r_simul);
	virtual uint32_t simulation_count_combatant(const RID_TYPE& r_simul);
	virtual uint32_t simulation_count_squad(const RID_TYPE& r_simul);
	virtual uint32_t simulation_count_team(const RID_TYPE& r_simul);
	virtual uint32_t simulation_count_radar(const RID_TYPE& r_simul);
	virtual uint32_t simulation_count_engagement(const RID_TYPE& r_simul);
	virtual uint32_t simulation_count_all_instances(const RID_TYPE& r_simul);

	/* Combatant API */
	virtual RID_TYPE combatant_create();
	virtual bool combatant_assert(const RID_TYPE& r_com);
	virtual void combatant_set_simulation(const RID_TYPE& r_com, const RID_TYPE& r_simul);
	virtual RID_TYPE combatant_get_simulation(const RID_TYPE& r_com);
	virtual bool combatant_is_squad(const RID_TYPE& r_com, const RID_TYPE& r_squad);
	virtual bool combatant_is_team(const RID_TYPE& r_com, const RID_TYPE& r_team);
	virtual Array combatant_get_involving_engagements(const RID_TYPE& r_com);
	virtual void combatant_set_local_transform(const RID_TYPE& r_com, const Transform& trans);
	virtual Transform combatant_get_space_transform(const RID_TYPE& r_com);
	virtual Transform combatant_get_local_transform(const RID_TYPE& r_com);
	virtual Transform combatant_get_combined_transform(const RID_TYPE& r_com);
	virtual void combatant_set_space_transform(const RID_TYPE& r_com, const Transform& trans);
	virtual void combatant_set_stand(const RID_TYPE& r_com, const uint32_t& stand);
	virtual uint32_t combatant_get_stand(const RID_TYPE& r_com);
	virtual uint32_t combatant_get_status(const RID_TYPE& r_com);
	virtual void combatant_set_iid(const RID_TYPE& r_com, const uint64_t& iid);
	virtual uint64_t combatant_get_iid(const RID_TYPE& r_com);
	virtual void combatant_set_detection_meter(const RID_TYPE& r_com, const double& dmeter);
	virtual double combatant_get_detection_meter(const RID_TYPE& r_com);
	virtual bool combatant_engagable(const RID_TYPE& from, const RID_TYPE& to);
	virtual void combatant_bind_chip(const RID_TYPE& r_com, const Ref<RCSChip>& chip, const bool& auto_unbind);
	virtual void combatant_unbind_chip(const RID_TYPE& r_com);
	virtual void combatant_set_profile(const RID_TYPE& r_com, const Ref<RCSCombatantProfile>& profile);
	virtual Ref<RCSCombatantProfile> combatant_get_profile(const RID_TYPE& r_com);

	/* Squad API */
	virtual RID_TYPE squad_create();
	virtual bool squad_assert(const RID_TYPE& r_squad);
	virtual void squad_set_simulation(const RID_TYPE& r_squad, const RID_TYPE& r_simul);
	virtual RID_TYPE squad_get_simulation(const RID_TYPE& r_squad);
	virtual bool squad_is_team(const RID_TYPE& r_squad, const RID_TYPE& r_team);
	virtual Array squad_get_involving_engagements(const RID_TYPE& r_squad);
	virtual void squad_add_combatant(const RID_TYPE& r_squad, const RID_TYPE& r_com);
	virtual void squad_remove_combatant(const RID_TYPE& r_squad, const RID_TYPE& r_com);
	virtual bool squad_has_combatant(const RID_TYPE& r_squad, const RID_TYPE& r_com);
	virtual bool squad_engagable(const RID_TYPE& from, const RID_TYPE& to);
	virtual uint32_t squad_count_combatant(const RID_TYPE& r_squad);
	virtual void squad_bind_chip(const RID_TYPE& r_com, const Ref<RCSChip>& chip, const bool& auto_unbind);
	virtual void squad_unbind_chip(const RID_TYPE& r_com);

	/* Team API */
	virtual RID_TYPE team_create();
	virtual bool team_assert(const RID_TYPE& r_team);
	virtual void team_set_simulation(const RID_TYPE& r_team, const RID_TYPE& r_simul);
	virtual RID_TYPE team_get_simulation(const RID_TYPE& r_team);
	virtual void team_add_squad(const RID_TYPE& r_team, const RID_TYPE& r_squad);
	virtual void team_remove_squad(const RID_TYPE& r_team, const RID_TYPE& r_squad);
	virtual Array team_get_involving_engagements(const RID_TYPE& r_team);
	virtual bool team_has_squad(const RID_TYPE& r_team, const RID_TYPE& r_squad);
	virtual bool team_engagable(const RID_TYPE& from, const RID_TYPE& to);
	virtual Ref<RCSUnilateralTeamsBind> team_create_link(const RID_TYPE& from, const RID_TYPE& to);
	virtual void team_create_link_bilateral(const RID_TYPE& from, const RID_TYPE& to);
	virtual Ref<RCSUnilateralTeamsBind> team_get_link(const RID_TYPE& from, const RID_TYPE& to);
	virtual bool team_has_link(const RID_TYPE& from, const RID_TYPE& to);
	virtual bool team_unlink(const RID_TYPE& from, const RID_TYPE& to);
	virtual bool team_unlink_bilateral(const RID_TYPE& from, const RID_TYPE& to);
	virtual void team_purge_links_multilateral(const RID_TYPE& from);
	virtual uint32_t team_count_squad(const RID_TYPE& r_team);
	virtual void team_bind_chip(const RID_TYPE& r_team, const Ref<RCSChip>& chip, const bool& auto_unbind);
	virtual void team_unbind_chip(const RID_TYPE& r_team);

	/* Radar API */
	virtual RID_TYPE radar_create();
	virtual bool radar_assert(const RID_TYPE& r_rad);
	virtual void radar_set_simulation(const RID_TYPE& r_rad, const RID_TYPE& r_simul);
	virtual RID_TYPE radar_get_simulation(const RID_TYPE& r_rad);
	virtual void radar_set_profile(const RID_TYPE& r_rad, const Ref<RCSRadarProfile>& profile);
	virtual Ref<RCSRadarProfile> radar_get_profile(const RID_TYPE& r_rad);
	virtual void radar_request_recheck_on(const RID_TYPE& r_rad, const RID_TYPE& r_com);
	virtual Array radar_get_detected(const RID_TYPE& r_rad);
	virtual Array radar_get_locked(const RID_TYPE& r_rad);
};

#endif
