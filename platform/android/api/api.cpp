#include "api.h"
#include "core/engine.h"
#include "core/os/os.h"
#include "permission_manager.h"

static PermissionManager *permission_manager = NULL;

void register_android_api() {

	ClassDB::register_virtual_class<PermissionManager>();
	permission_manager = memnew(PermissionManager);
	Engine::get_singleton()->add_singleton(Engine::Singleton("PermissionManager", permission_manager));
}

void unregister_android_api() {

	memdelete(permission_manager);
}

void PermissionManager::_bind_methods() {

	ClassDB::bind_method(D_METHOD("request_permission", "permission_index"), &PermissionManager::request_permission);
	ClassDB::bind_method(D_METHOD("request_permissions", "permission_list"), &PermissionManager::request_permissions);
	ClassDB::bind_method(D_METHOD("check_permission", "permission_index"), &PermissionManager::check_permission);

	BIND_ENUM_CONSTANT(ACCESS_CHECKIN_PROPERTIES);
	BIND_ENUM_CONSTANT(ACCESS_COARSE_LOCATION);
	BIND_ENUM_CONSTANT(ACCESS_FINE_LOCATION);
	BIND_ENUM_CONSTANT(ACCESS_LOCATION_EXTRA_COMMANDS);
	BIND_ENUM_CONSTANT(ACCESS_MOCK_LOCATION);
	BIND_ENUM_CONSTANT(ACCESS_NETWORK_STATE);
	BIND_ENUM_CONSTANT(ACCESS_SURFACE_FLINGER);
	BIND_ENUM_CONSTANT(ACCESS_WIFI_STATE);
	BIND_ENUM_CONSTANT(ACCOUNT_MANAGER);
	BIND_ENUM_CONSTANT(ADD_VOICEMAIL);
	BIND_ENUM_CONSTANT(AUTHENTICATE_ACCOUNTS);
	BIND_ENUM_CONSTANT(BATTERY_STATS);
	BIND_ENUM_CONSTANT(BIND_ACCESSIBILITY_SERVICE);
	BIND_ENUM_CONSTANT(BIND_APPWIDGET);
	BIND_ENUM_CONSTANT(BIND_DEVICE_ADMIN);
	BIND_ENUM_CONSTANT(BIND_INPUT_METHOD);
	BIND_ENUM_CONSTANT(BIND_NFC_SERVICE);
	BIND_ENUM_CONSTANT(BIND_NOTIFICATION_LISTENER_SERVICE);
	BIND_ENUM_CONSTANT(BIND_PRINT_SERVICE);
	BIND_ENUM_CONSTANT(BIND_REMOTEVIEWS);
	BIND_ENUM_CONSTANT(BIND_TEXT_SERVICE);
	BIND_ENUM_CONSTANT(BIND_VPN_SERVICE);
	BIND_ENUM_CONSTANT(BIND_WALLPAPER);
	BIND_ENUM_CONSTANT(BLUETOOTH);
	BIND_ENUM_CONSTANT(BLUETOOTH_ADMIN);
	BIND_ENUM_CONSTANT(BLUETOOTH_PRIVILEGED);
	BIND_ENUM_CONSTANT(BRICK);
	BIND_ENUM_CONSTANT(BROADCAST_PACKAGE_REMOVED);
	BIND_ENUM_CONSTANT(BROADCAST_SMS);
	BIND_ENUM_CONSTANT(BROADCAST_STICKY);
	BIND_ENUM_CONSTANT(BROADCAST_WAP_PUSH);
	BIND_ENUM_CONSTANT(CALL_PHONE);
	BIND_ENUM_CONSTANT(CALL_PRIVILEGED);
	BIND_ENUM_CONSTANT(CAMERA);
	BIND_ENUM_CONSTANT(CAPTURE_AUDIO_OUTPUT);
	BIND_ENUM_CONSTANT(CAPTURE_SECURE_VIDEO_OUTPUT);
	BIND_ENUM_CONSTANT(CAPTURE_VIDEO_OUTPUT);
	BIND_ENUM_CONSTANT(CHANGE_COMPONENT_ENABLED_STATE);
	BIND_ENUM_CONSTANT(CHANGE_CONFIGURATION);
	BIND_ENUM_CONSTANT(CHANGE_NETWORK_STATE);
	BIND_ENUM_CONSTANT(CHANGE_WIFI_MULTICAST_STATE);
	BIND_ENUM_CONSTANT(CHANGE_WIFI_STATE);
	BIND_ENUM_CONSTANT(CLEAR_APP_CACHE);
	BIND_ENUM_CONSTANT(CLEAR_APP_USER_DATA);
	BIND_ENUM_CONSTANT(CONTROL_LOCATION_UPDATES);
	BIND_ENUM_CONSTANT(DELETE_CACHE_FILES);
	BIND_ENUM_CONSTANT(DELETE_PACKAGES);
	BIND_ENUM_CONSTANT(DEVICE_POWER);
	BIND_ENUM_CONSTANT(DIAGNOSTIC);
	BIND_ENUM_CONSTANT(DISABLE_KEYGUARD);
	BIND_ENUM_CONSTANT(DUMP);
	BIND_ENUM_CONSTANT(EXPAND_STATUS_BAR);
	BIND_ENUM_CONSTANT(FACTORY_TEST);
	BIND_ENUM_CONSTANT(FLASHLIGHT);
	BIND_ENUM_CONSTANT(FORCE_BACK);
	BIND_ENUM_CONSTANT(GET_ACCOUNTS);
	BIND_ENUM_CONSTANT(GET_PACKAGE_SIZE);
	BIND_ENUM_CONSTANT(GET_TASKS);
	BIND_ENUM_CONSTANT(GET_TOP_ACTIVITY_INFO);
	BIND_ENUM_CONSTANT(GLOBAL_SEARCH);
	BIND_ENUM_CONSTANT(HARDWARE_TEST);
	BIND_ENUM_CONSTANT(INJECT_EVENTS);
	BIND_ENUM_CONSTANT(INSTALL_LOCATION_PROVIDER);
	BIND_ENUM_CONSTANT(INSTALL_PACKAGES);
	BIND_ENUM_CONSTANT(INSTALL_SHORTCUT);
	BIND_ENUM_CONSTANT(INTERNAL_SYSTEM_WINDOW);
	BIND_ENUM_CONSTANT(INTERNET);
	BIND_ENUM_CONSTANT(KILL_BACKGROUND_PROCESSES);
	BIND_ENUM_CONSTANT(LOCATION_HARDWARE);
	BIND_ENUM_CONSTANT(MANAGE_ACCOUNTS);
	BIND_ENUM_CONSTANT(MANAGE_APP_TOKENS);
	BIND_ENUM_CONSTANT(MANAGE_DOCUMENTS);
	BIND_ENUM_CONSTANT(MASTER_CLEAR);
	BIND_ENUM_CONSTANT(MEDIA_CONTENT_CONTROL);
	BIND_ENUM_CONSTANT(MODIFY_AUDIO_SETTINGS);
	BIND_ENUM_CONSTANT(MODIFY_PHONE_STATE);
	BIND_ENUM_CONSTANT(MOUNT_FORMAT_FILESYSTEMS);
	BIND_ENUM_CONSTANT(MOUNT_UNMOUNT_FILESYSTEMS);
	BIND_ENUM_CONSTANT(NFC);
	BIND_ENUM_CONSTANT(PERSISTENT_ACTIVITY);
	BIND_ENUM_CONSTANT(PROCESS_OUTGOING_CALLS);
	BIND_ENUM_CONSTANT(READ_CALENDAR);
	BIND_ENUM_CONSTANT(READ_CALL_LOG);
	BIND_ENUM_CONSTANT(READ_CONTACTS);
	BIND_ENUM_CONSTANT(READ_EXTERNAL_STORAGE);
	BIND_ENUM_CONSTANT(READ_FRAME_BUFFER);
	BIND_ENUM_CONSTANT(READ_HISTORY_BOOKMARKS);
	BIND_ENUM_CONSTANT(READ_INPUT_STATE);
	BIND_ENUM_CONSTANT(READ_LOGS);
	BIND_ENUM_CONSTANT(READ_PHONE_STATE);
	BIND_ENUM_CONSTANT(READ_PROFILE);
	BIND_ENUM_CONSTANT(READ_SMS);
	BIND_ENUM_CONSTANT(READ_SOCIAL_STREAM);
	BIND_ENUM_CONSTANT(READ_SYNC_SETTINGS);
	BIND_ENUM_CONSTANT(READ_SYNC_STATS);
	BIND_ENUM_CONSTANT(READ_USER_DICTIONARY);
	BIND_ENUM_CONSTANT(REBOOT);
	BIND_ENUM_CONSTANT(RECEIVE_BOOT_COMPLETED);
	BIND_ENUM_CONSTANT(RECEIVE_MMS);
	BIND_ENUM_CONSTANT(RECEIVE_SMS);
	BIND_ENUM_CONSTANT(RECEIVE_WAP_PUSH);
	BIND_ENUM_CONSTANT(RECORD_AUDIO);
	BIND_ENUM_CONSTANT(REORDER_TASKS);
	BIND_ENUM_CONSTANT(RESTART_PACKAGES);
	BIND_ENUM_CONSTANT(SEND_RESPOND_VIA_MESSAGE);
	BIND_ENUM_CONSTANT(SEND_SMS);
	BIND_ENUM_CONSTANT(SET_ACTIVITY_WATCHER);
	BIND_ENUM_CONSTANT(SET_ALARM);
	BIND_ENUM_CONSTANT(SET_ALWAYS_FINISH);
	BIND_ENUM_CONSTANT(SET_ANIMATION_SCALE);
	BIND_ENUM_CONSTANT(SET_DEBUG_APP);
	BIND_ENUM_CONSTANT(SET_ORIENTATION);
	BIND_ENUM_CONSTANT(SET_POINTER_SPEED);
	BIND_ENUM_CONSTANT(SET_PREFERRED_APPLICATIONS);
	BIND_ENUM_CONSTANT(SET_PROCESS_LIMIT);
	BIND_ENUM_CONSTANT(SET_TIME);
	BIND_ENUM_CONSTANT(SET_TIME_ZONE);
	BIND_ENUM_CONSTANT(SET_WALLPAPER);
	BIND_ENUM_CONSTANT(SET_WALLPAPER_HINTS);
	BIND_ENUM_CONSTANT(SIGNAL_PERSISTENT_PROCESSES);
	BIND_ENUM_CONSTANT(STATUS_BAR);
	BIND_ENUM_CONSTANT(SUBSCRIBED_FEEDS_READ);
	BIND_ENUM_CONSTANT(SUBSCRIBED_FEEDS_WRITE);
	BIND_ENUM_CONSTANT(SYSTEM_ALERT_WINDOW);
	BIND_ENUM_CONSTANT(TRANSMIT_IR);
	BIND_ENUM_CONSTANT(UNINSTALL_SHORTCUT);
	BIND_ENUM_CONSTANT(UPDATE_DEVICE_STATS);
	BIND_ENUM_CONSTANT(USE_CREDENTIALS);
	BIND_ENUM_CONSTANT(USE_SIP);
	BIND_ENUM_CONSTANT(VIBRATE);
	BIND_ENUM_CONSTANT(WAKE_LOCK);
	BIND_ENUM_CONSTANT(WRITE_APN_SETTINGS);
	BIND_ENUM_CONSTANT(WRITE_CALENDAR);
	BIND_ENUM_CONSTANT(WRITE_CALL_LOG);
	BIND_ENUM_CONSTANT(WRITE_CONTACTS);
	BIND_ENUM_CONSTANT(WRITE_EXTERNAL_STORAGE);
	BIND_ENUM_CONSTANT(WRITE_GSERVICES);
	BIND_ENUM_CONSTANT(WRITE_HISTORY_BOOKMARKS);
	BIND_ENUM_CONSTANT(WRITE_PROFILE);
	BIND_ENUM_CONSTANT(WRITE_SECURE_SETTINGS);
	BIND_ENUM_CONSTANT(WRITE_SETTINGS);
	BIND_ENUM_CONSTANT(WRITE_SMS);
	BIND_ENUM_CONSTANT(WRITE_SOCIAL_STREAM);
	BIND_ENUM_CONSTANT(WRITE_SYNC_SETTINGS);
	BIND_ENUM_CONSTANT(WRITE_USER_DICTIONARY);
}

bool PermissionManager::request_permission(int p_index) {
	return OS::get_singleton()->request_permission_with_index(p_index);
}

bool PermissionManager::request_permissions(const PoolIntArray &p_permissions) {
	return OS::get_singleton()->request_permissions(p_permissions);
}

bool PermissionManager::check_permission(int p_index) {
	return OS::get_singleton()->check_permission(p_index);
}

PermissionManager *PermissionManager::singleton = NULL;

PermissionManager::PermissionManager() {
	ERR_FAIL_COND_MSG(singleton != NULL, "PermissionManager singleton already exist.");
	singleton = this;
}
