/*************************************************************************/
/*  os.cpp                                                               */
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

#include "os.h"

#include "core/input/input.h"
#include "core/os/dir_access.h"
#include "core/os/file_access.h"
#include "core/os/midi_driver.h"
#include "core/project_settings.h"
#include "core/version_generated.gen.h"
#include "servers/audio_server.h"

#include <stdarg.h>

<<<<<<< HEAD
OS *OS::singleton = nullptr;
=======
OS *OS::singleton = NULL;
>>>>>>> audio-bus-effect-fixed
uint64_t OS::target_ticks = 0;

OS *OS::get_singleton() {
	return singleton;
}

uint32_t OS::get_ticks_msec() const {
	return get_ticks_usec() / 1000;
}

String OS::get_iso_date_time(bool local) const {
	OS::Date date = get_date(local);
	OS::Time time = get_time(local);

	String timezone;
	if (!local) {
		TimeZoneInfo zone = get_time_zone_info();
		if (zone.bias >= 0) {
			timezone = "+";
		}
		timezone = timezone + itos(zone.bias / 60).pad_zeros(2) + itos(zone.bias % 60).pad_zeros(2);
	} else {
		timezone = "Z";
	}

	return itos(date.year).pad_zeros(2) +
		   "-" +
		   itos(date.month).pad_zeros(2) +
		   "-" +
		   itos(date.day).pad_zeros(2) +
		   "T" +
		   itos(time.hour).pad_zeros(2) +
		   ":" +
		   itos(time.min).pad_zeros(2) +
		   ":" +
		   itos(time.sec).pad_zeros(2) +
		   timezone;
}

uint64_t OS::get_splash_tick_msec() const {
	return _msec_splash;
}

double OS::get_unix_time() const {
	return 0;
}

void OS::debug_break() {
	// something
}

void OS::_set_logger(CompositeLogger *p_logger) {
	if (_logger) {
		memdelete(_logger);
	}
	_logger = p_logger;
}

void OS::add_logger(Logger *p_logger) {
	if (!_logger) {
		Vector<Logger *> loggers;
		loggers.push_back(p_logger);
		_logger = memnew(CompositeLogger(loggers));
	} else {
		_logger->add_logger(p_logger);
	}
}

void OS::print_error(const char *p_function, const char *p_file, int p_line, const char *p_code, const char *p_rationale, Logger::ErrorType p_type) {
	_logger->log_error(p_function, p_file, p_line, p_code, p_rationale, p_type);
}

void OS::print(const char *p_format, ...) {
	va_list argp;
	va_start(argp, p_format);

	_logger->logv(p_format, argp, false);

	va_end(argp);
}

void OS::printerr(const char *p_format, ...) {
	va_list argp;
	va_start(argp, p_format);

	_logger->logv(p_format, argp, true);

	va_end(argp);
}

void OS::set_low_processor_usage_mode(bool p_enabled) {
	low_processor_usage_mode = p_enabled;
}

bool OS::is_in_low_processor_usage_mode() const {
	return low_processor_usage_mode;
}

void OS::set_low_processor_usage_mode_sleep_usec(int p_usec) {
	low_processor_usage_mode_sleep_usec = p_usec;
}

int OS::get_low_processor_usage_mode_sleep_usec() const {
	return low_processor_usage_mode_sleep_usec;
}

String OS::get_executable_path() const {
	return _execpath;
}

int OS::get_process_id() const {
	return -1;
}

void OS::vibrate_handheld(int p_duration_ms) {
	WARN_PRINT("vibrate_handheld() only works with Android and iOS");
}

bool OS::is_stdout_verbose() const {
	return _verbose_stdout;
}

bool OS::is_stdout_debug_enabled() const {
	return _debug_stdout;
}
<<<<<<< HEAD
=======

void OS::dump_memory_to_file(const char *p_file) {
>>>>>>> audio-bus-effect-fixed

void OS::dump_memory_to_file(const char *p_file) {
	//Memory::dump_static_mem_to_file(p_file);
}

static FileAccess *_OSPRF = nullptr;

static void _OS_printres(Object *p_obj) {
	Resource *res = Object::cast_to<Resource>(p_obj);
	if (!res) {
		return;
	}

	String str = itos(res->get_instance_id()) + String(res->get_class()) + ":" + String(res->get_name()) + " - " + res->get_path();
	if (_OSPRF) {
		_OSPRF->store_line(str);
	} else {
		print_line(str);
<<<<<<< HEAD
	}
=======
}

bool OS::has_virtual_keyboard() const {

	return false;
}

void OS::show_virtual_keyboard(const String &p_existing_text, const Rect2 &p_screen_rect, bool p_multiline, int p_max_input_length, int p_cursor_start, int p_cursor_end) {
}

void OS::hide_virtual_keyboard() {
}

int OS::get_virtual_keyboard_height() const {
	return 0;
}

void OS::set_cursor_shape(CursorShape p_shape) {
}

OS::CursorShape OS::get_cursor_shape() const {
	return CURSOR_ARROW;
}

void OS::set_custom_mouse_cursor(const RES &p_cursor, CursorShape p_shape, const Vector2 &p_hotspot) {
>>>>>>> audio-bus-effect-fixed
}

void OS::print_all_resources(String p_to_file) {
	ERR_FAIL_COND(p_to_file != "" && _OSPRF);
	if (p_to_file != "") {
		Error err;
		_OSPRF = FileAccess::open(p_to_file, FileAccess::WRITE, &err);
		if (err != OK) {
			_OSPRF = nullptr;
			ERR_FAIL_MSG("Can't print all resources to file: " + String(p_to_file) + ".");
		}
	}

	ObjectDB::debug_objects(_OS_printres);

	if (p_to_file != "") {
		if (_OSPRF) {
			memdelete(_OSPRF);
		}
		_OSPRF = nullptr;
	}
}

void OS::print_resources_in_use(bool p_short) {
	ResourceCache::dump(nullptr, p_short);
}

void OS::dump_resources_to_file(const char *p_file) {
	ResourceCache::dump(p_file);
}

void OS::set_no_window_mode(bool p_enable) {
	_no_window = p_enable;
}

bool OS::is_no_window_mode_enabled() const {
	return _no_window;
}

int OS::get_exit_code() const {
	return _exit_code;
}

void OS::set_exit_code(int p_code) {
	_exit_code = p_code;
}

String OS::get_locale() const {
	return "en";
}

// Helper function to ensure that a dir name/path will be valid on the OS
String OS::get_safe_dir_name(const String &p_dir_name, bool p_allow_dir_separator) const {
	Vector<String> invalid_chars = String(": * ? \" < > |").split(" ");
	if (p_allow_dir_separator) {
		// Dir separators are allowed, but disallow ".." to avoid going up the filesystem
		invalid_chars.push_back("..");
	} else {
		invalid_chars.push_back("/");
	}

	String safe_dir_name = p_dir_name.replace("\\", "/").strip_edges();
	for (int i = 0; i < invalid_chars.size(); i++) {
		safe_dir_name = safe_dir_name.replace(invalid_chars[i], "-");
	}
	return safe_dir_name;
}

// Path to data, config, cache, etc. OS-specific folders

// Get properly capitalized engine name for system paths
String OS::get_godot_dir_name() const {
	// Default to lowercase, so only override when different case is needed
	return String(VERSION_SHORT_NAME).to_lower();
}

// OS equivalent of XDG_DATA_HOME
String OS::get_data_path() const {
	return ".";
}

// OS equivalent of XDG_CONFIG_HOME
String OS::get_config_path() const {
	return ".";
}

// OS equivalent of XDG_CACHE_HOME
String OS::get_cache_path() const {
	return ".";
}

// Path to macOS .app bundle resources
String OS::get_bundle_resource_dir() const {
	return ".";
}

// OS specific path for user://
String OS::get_user_data_dir() const {
	return ".";
}

// Absolute path to res://
String OS::get_resource_dir() const {
	return ProjectSettings::get_singleton()->get_resource_path();
}

// Access system-specific dirs like Documents, Downloads, etc.
String OS::get_system_dir(SystemDir p_dir) const {
	return ".";
}

Error OS::shell_open(String p_uri) {
	return ERR_UNAVAILABLE;
}

// implement these with the canvas?

uint64_t OS::get_static_memory_usage() const {
	return Memory::get_mem_usage();
}

uint64_t OS::get_static_memory_peak_usage() const {
	return Memory::get_mem_max_usage();
}

Error OS::set_cwd(const String &p_cwd) {
	return ERR_CANT_OPEN;
}

uint64_t OS::get_free_static_memory() const {
	return Memory::get_mem_available();
}

void OS::yield() {
}

void OS::ensure_user_data_dir() {
	String dd = get_user_data_dir();
	DirAccess *da = DirAccess::open(dd);
	if (da) {
		memdelete(da);
		return;
	}

	da = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
	Error err = da->make_dir_recursive(dd);
	ERR_FAIL_COND_MSG(err != OK, "Error attempting to create data dir: " + dd + ".");

	memdelete(da);
}

String OS::get_model_name() const {
	return "GenericDevice";
}

void OS::set_cmdline(const char *p_execpath, const List<String> &p_args) {
	_execpath = p_execpath;
	_cmdline = p_args;
}

String OS::get_unique_id() const {
	ERR_FAIL_V("");
}

int OS::get_processor_count() const {
	return 1;
}

bool OS::can_use_threads() const {
#ifdef NO_THREADS
	return false;
#else
	return true;
#endif
}

<<<<<<< HEAD
=======
OS::MouseMode OS::get_mouse_mode() const {

	return MOUSE_MODE_VISIBLE;
}

OS::LatinKeyboardVariant OS::get_latin_keyboard_variant() const {

	return LATIN_KEYBOARD_QWERTY;
}

int OS::keyboard_get_layout_count() const {
	return 0;
}

int OS::keyboard_get_current_layout() const {
	return -1;
}

void OS::keyboard_set_current_layout(int p_index) {}

String OS::keyboard_get_layout_language(int p_index) const {
	return "";
}

String OS::keyboard_get_layout_name(int p_index) const {
	return "";
}

bool OS::is_joy_known(int p_device) {
	return true;
}

String OS::get_joy_guid(int p_device) const {
	return "Default Joypad";
}

void OS::set_context(int p_context) {
}

OS::SwitchVSyncCallbackInThread OS::switch_vsync_function = NULL;

void OS::set_use_vsync(bool p_enable) {
	_use_vsync = p_enable;
	if (switch_vsync_function) { //if a function was set, use function
		switch_vsync_function(p_enable);
	} else { //otherwise just call here
		_set_use_vsync(p_enable);
	}
}

bool OS::is_vsync_enabled() const {

	return _use_vsync;
}

void OS::set_vsync_via_compositor(bool p_enable) {
	_vsync_via_compositor = p_enable;
}

bool OS::is_vsync_via_compositor_enabled() const {
	return _vsync_via_compositor;
}

OS::PowerState OS::get_power_state() {
	return POWERSTATE_UNKNOWN;
}
int OS::get_power_seconds_left() {
	return -1;
}
int OS::get_power_percent_left() {
	return -1;
}

>>>>>>> audio-bus-effect-fixed
void OS::set_has_server_feature_callback(HasServerFeatureCallback p_callback) {
	has_server_feature_callback = p_callback;
}

bool OS::has_feature(const String &p_feature) {
	if (p_feature == get_name()) {
		return true;
	}
#ifdef DEBUG_ENABLED
	if (p_feature == "debug") {
		return true;
	}
#else
	if (p_feature == "release")
		return true;
#endif
#ifdef TOOLS_ENABLED
	if (p_feature == "editor") {
		return true;
	}
#else
	if (p_feature == "standalone")
		return true;
#endif

	if (sizeof(void *) == 8 && p_feature == "64") {
		return true;
	}
	if (sizeof(void *) == 4 && p_feature == "32") {
		return true;
	}
#if defined(__x86_64) || defined(__x86_64__) || defined(__amd64__)
	if (p_feature == "x86_64") {
		return true;
	}
#elif (defined(__i386) || defined(__i386__))
	if (p_feature == "x86") {
		return true;
	}
#elif defined(__aarch64__)
	if (p_feature == "arm64") {
		return true;
	}
#elif defined(__arm__)
#if defined(__ARM_ARCH_7A__)
	if (p_feature == "armv7a" || p_feature == "armv7") {
		return true;
	}
#endif
#if defined(__ARM_ARCH_7S__)
	if (p_feature == "armv7s" || p_feature == "armv7") {
		return true;
	}
#endif
	if (p_feature == "arm") {
		return true;
	}
#endif

	if (_check_internal_feature_support(p_feature)) {
		return true;
	}

	if (has_server_feature_callback && has_server_feature_callback(p_feature)) {
		return true;
	}

	if (ProjectSettings::get_singleton()->has_custom_feature(p_feature)) {
		return true;
	}

	return false;
}

void OS::set_restart_on_exit(bool p_restart, const List<String> &p_restart_arguments) {
	restart_on_exit = p_restart;
	restart_commandline = p_restart_arguments;
}

bool OS::is_restart_on_exit_set() const {
	return restart_on_exit;
}

List<String> OS::get_restart_on_exit_arguments() const {
	return restart_commandline;
}

PackedStringArray OS::get_connected_midi_inputs() {
	if (MIDIDriver::get_singleton()) {
		return MIDIDriver::get_singleton()->get_connected_inputs();
	}

<<<<<<< HEAD
	PackedStringArray list;
=======
	PoolStringArray list;
>>>>>>> audio-bus-effect-fixed
	ERR_FAIL_V_MSG(list, vformat("MIDI input isn't supported on %s.", OS::get_singleton()->get_name()));
}

void OS::open_midi_inputs() {
<<<<<<< HEAD
=======

>>>>>>> audio-bus-effect-fixed
	if (MIDIDriver::get_singleton()) {
		MIDIDriver::get_singleton()->open();
	} else {
		ERR_PRINT(vformat("MIDI input isn't supported on %s.", OS::get_singleton()->get_name()));
	}
}

void OS::close_midi_inputs() {
<<<<<<< HEAD
=======

>>>>>>> audio-bus-effect-fixed
	if (MIDIDriver::get_singleton()) {
		MIDIDriver::get_singleton()->close();
	} else {
		ERR_PRINT(vformat("MIDI input isn't supported on %s.", OS::get_singleton()->get_name()));
	}
}

void OS::add_frame_delay(bool p_can_draw) {
	const uint32_t frame_delay = Engine::get_singleton()->get_frame_delay();
	if (frame_delay) {
		// Add fixed frame delay to decrease CPU/GPU usage. This doesn't take
		// the actual frame time into account.
		// Due to the high fluctuation of the actual sleep duration, it's not recommended
		// to use this as a FPS limiter.
		delay_usec(frame_delay * 1000);
	}

	// Add a dynamic frame delay to decrease CPU/GPU usage. This takes the
	// previous frame time into account for a smoother result.
	uint64_t dynamic_delay = 0;
	if (is_in_low_processor_usage_mode() || !p_can_draw) {
		dynamic_delay = get_low_processor_usage_mode_sleep_usec();
	}
	const int target_fps = Engine::get_singleton()->get_target_fps();
	if (target_fps > 0 && !Engine::get_singleton()->is_editor_hint()) {
		// Override the low processor usage mode sleep delay if the target FPS is lower.
		dynamic_delay = MAX(dynamic_delay, (uint64_t)(1000000 / target_fps));
	}

	if (dynamic_delay > 0) {
		target_ticks += dynamic_delay;
		uint64_t current_ticks = get_ticks_usec();

		if (current_ticks < target_ticks) {
			delay_usec(target_ticks - current_ticks);
		}

		current_ticks = get_ticks_usec();
		target_ticks = MIN(MAX(target_ticks, current_ticks - dynamic_delay), current_ticks + dynamic_delay);
	}
}

OS::OS() {
	void *volatile stack_bottom;

	singleton = this;
<<<<<<< HEAD
=======
	_keep_screen_on = true; // set default value to true, because this had been true before godot 2.0.
	low_processor_usage_mode = false;
	low_processor_usage_mode_sleep_usec = 10000;
	_verbose_stdout = false;
	_debug_stdout = false;
	_no_window = false;
	_exit_code = 0;
	_orientation = SCREEN_LANDSCAPE;

	_render_thread_mode = RENDER_THREAD_SAFE;

	_allow_hidpi = false;
	_allow_layered = false;
	_stack_bottom = (void *)(&stack_bottom);

	_logger = NULL;
>>>>>>> audio-bus-effect-fixed

	_stack_bottom = (void *)(&stack_bottom);

	Vector<Logger *> loggers;
	loggers.push_back(memnew(StdLogger));
	_set_logger(memnew(CompositeLogger(loggers)));
}

OS::~OS() {
	memdelete(_logger);
	singleton = nullptr;
}
