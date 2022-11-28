#include "register_types.h"
#include "core/class_db.h"
#include "core/engine.h"
#include "combat_server.h"

static Sentrience* RTSCombatServerPtr = nullptr;

void register_rts_com_types(){
#ifndef USE_THREAD_SAFE_API
	ClassDB::register_class<SentrienceInstancesWatcher>();
#endif
	ClassDB::register_class<Sentrience>();
	ClassDB::register_class<RCSChip>();
	ClassDB::register_class<RCSEngagement>();
	ClassDB::register_class<RCSRadarProfile>();
	ClassDB::register_class<RCSCombatantProfile>();
	ClassDB::register_class<RCSUnilateralTeamsBind>();
	RTSCombatServerPtr = memnew(Sentrience);
	Engine::get_singleton()->add_singleton(Engine::Singleton("Sentrience", Sentrience::get_singleton()));
}
void unregister_rts_com_types(){
	memdelete(RTSCombatServerPtr);
}
