/**************************************************************************/
/*  display_server_wayland.cpp                                            */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "display_server_wayland.h"

#ifdef WAYLAND_ENABLED

#ifdef VULKAN_ENABLED
#include "servers/rendering/renderer_rd/renderer_compositor_rd.h"
#endif

#ifdef GLES3_ENABLED
#include "drivers/gles3/rasterizer_gles3.h"
#endif

String DisplayServerWayland::_get_app_id_from_context(Context context) {
	String app_id;

	switch (context) {
		case CONTEXT_EDITOR: {
			app_id = "org.godotengine.Editor";
		} break;

		case CONTEXT_PROJECTMAN: {
			app_id = "org.godotengine.ProjectManager";
		} break;

		case CONTEXT_ENGINE:
		default: {
			String config_name = GLOBAL_GET("application/config/name");
			if (config_name.length() != 0) {
				app_id = config_name;
			} else {
				app_id = "org.godotengine.Godot";
			}
		}
	}

	return app_id;
}

void DisplayServerWayland::_send_window_event(WindowEvent p_event) {
	WindowData &wd = main_window;

	if (wd.window_event_callback.is_valid()) {
		Variant var_event = Variant(p_event);
		Variant *arg = &var_event;

		Variant ret;
		Callable::CallError ce;

		wd.window_event_callback.callp((const Variant **)&arg, 1, ret, ce);
	}
}

void DisplayServerWayland::dispatch_input_events(const Ref<InputEvent> &p_event) {
	((DisplayServerWayland *)(get_singleton()))->_dispatch_input_event(p_event);
}

void DisplayServerWayland::_dispatch_input_event(const Ref<InputEvent> &p_event) {
	Variant ev = p_event;
	Variant *evp = &ev;
	Variant ret;
	Callable::CallError ce;

	Callable callable = main_window.input_event_callback;
	if (callable.is_valid()) {
		callable.callp((const Variant **)&evp, 1, ret, ce);
	}
}

void DisplayServerWayland::_resize_window(Size2i size) {
	WindowData &wd = main_window;

	wd.rect.size = size;

#ifdef VULKAN_ENABLED
	if (wd.visible && context_vulkan) {
		context_vulkan->window_resize(MAIN_WINDOW_ID, wd.rect.size.width, wd.rect.size.height);
	}
#endif

#ifdef GLES3_ENABLED
	if (wd.visible && egl_manager) {
		wl_egl_window_resize(wd.wl_egl_window, wd.rect.size.width, wd.rect.size.height, 0, 0);
	}
#endif

	if (wd.rect_changed_callback.is_valid()) {
		Variant var_rect = Variant(wd.rect);
		Variant *arg = &var_rect;

		Variant ret;
		Callable::CallError ce;

		wd.rect_changed_callback.callp((const Variant **)&arg, 1, ret, ce);
	}
}

// Interface mthods

bool DisplayServerWayland::has_feature(Feature p_feature) const {
	switch (p_feature) {
		case FEATURE_MOUSE:
		case FEATURE_CLIPBOARD:
		case FEATURE_CURSOR_SHAPE:
		case FEATURE_WINDOW_TRANSPARENCY:
		case FEATURE_SWAP_BUFFERS:
		case FEATURE_KEEP_SCREEN_ON:
		case FEATURE_CLIPBOARD_PRIMARY: {
			return true;
		} break;

		default: {
			return false;
		}
	}
}

String DisplayServerWayland::get_name() const {
	return "Wayland";
}

#ifdef SPEECHD_ENABLED

bool DisplayServerWayland::tts_is_speaking() const {
	ERR_FAIL_COND_V(!tts, false);
	return tts->is_speaking();
}

bool DisplayServerWayland::tts_is_paused() const {
	ERR_FAIL_COND_V(!tts, false);
	return tts->is_paused();
}

TypedArray<Dictionary> DisplayServerWayland::tts_get_voices() const {
	ERR_FAIL_COND_V(!tts, TypedArray<Dictionary>());
	return tts->get_voices();
}

void DisplayServerWayland::tts_speak(const String &p_text, const String &p_voice, int p_volume, float p_pitch, float p_rate, int p_utterance_id, bool p_interrupt) {
	ERR_FAIL_COND(!tts);
	tts->speak(p_text, p_voice, p_volume, p_pitch, p_rate, p_utterance_id, p_interrupt);
}

void DisplayServerWayland::tts_pause() {
	ERR_FAIL_COND(!tts);
	tts->pause();
}

void DisplayServerWayland::tts_resume() {
	ERR_FAIL_COND(!tts);
	tts->resume();
}

void DisplayServerWayland::tts_stop() {
	ERR_FAIL_COND(!tts);
	tts->stop();
}

#endif

#ifdef DBUS_ENABLED

bool DisplayServerWayland::is_dark_mode_supported() const {
	return portal_desktop->is_supported();
}

bool DisplayServerWayland::is_dark_mode() const {
	switch (portal_desktop->get_appearance_color_scheme()) {
		case 1:
			// Prefers dark theme.
			return true;
		case 2:
			// Prefers light theme.
			return false;
		default:
			// Preference unknown.
			return false;
	}
}

#endif

void DisplayServerWayland::mouse_set_mode(MouseMode p_mode) {
	if (p_mode == wls.mouse_mode) {
		return;
	}

	MutexLock mutex_lock(wayland_thread.mutex);

	bool show_cursor = (p_mode == MOUSE_MODE_VISIBLE || p_mode == MOUSE_MODE_CONFINED);
	bool previously_shown = (wls.mouse_mode == MOUSE_MODE_VISIBLE || wls.mouse_mode == MOUSE_MODE_CONFINED);

	// If the cursor is being shown while it's focusing the window we must send a
	// mouse enter event.
	if (wls.current_seat && wls.current_seat->window_pointed && show_cursor && !previously_shown) {
		_send_window_event(WINDOW_EVENT_MOUSE_ENTER);
	}

	wls.mouse_mode = p_mode;

	WaylandThread::_wayland_state_update_cursor(wls);
}

DisplayServerWayland::MouseMode DisplayServerWayland::mouse_get_mode() const {
	return wls.mouse_mode;
}

void DisplayServerWayland::warp_mouse(const Point2i &p_to) {
	// NOTE: This is hacked together as for some reason the pointer constraints
	// protocol doesn't implement pointer warping (not even in the window). This
	// isn't efficient *at all* and perhaps there could be better behaviours in
	// the pointer capturing logic in general, but this will do for now.
	MutexLock mutex_lock(wayland_thread.mutex);

	if (wls.current_seat) {
		MouseMode old_mouse_mode = wls.mouse_mode;

		mouse_set_mode(MOUSE_MODE_CAPTURED);

		// Override the hint set by MOUSE_MODE_CAPTURED with the requested one.
		zwp_locked_pointer_v1_set_cursor_position_hint(wls.current_seat->wp_locked_pointer, wl_fixed_from_int(p_to.x), wl_fixed_from_int(p_to.y));

		struct wl_surface *wl_surface = wayland_thread.window_get_wl_surface(MAIN_WINDOW_ID);

		// Committing the surface is required to set the hint instantly.
		wl_surface_commit(wl_surface);

		mouse_set_mode(old_mouse_mode);
	}
}

Point2i DisplayServerWayland::mouse_get_position() const {
	MutexLock mutex_lock(wayland_thread.mutex);

	if (wls.current_seat) {
		return wls.current_seat->pointer_data.position;
	}

	return Point2i();
}

BitField<MouseButtonMask> DisplayServerWayland::mouse_get_button_state() const {
	MutexLock mutex_lock(wayland_thread.mutex);

	if (!wls.current_seat) {
		return BitField<MouseButtonMask>();
	}

	BitField<MouseButtonMask> mouse_button_mask;

	// Are we sure this is the only way? This seems sus.
	mouse_button_mask.set_flag(MouseButtonMask((int64_t)wls.current_seat->pointer_data.pressed_button_mask));
	mouse_button_mask.set_flag(MouseButtonMask((int64_t)wls.current_seat->tablet_tool_data.pressed_button_mask));

	return mouse_button_mask;
}

// NOTE: According to the Wayland specification, this method will only do
// anything if the user has interacted with the application by sending a
// "recent enough" input event.
// TODO: Add this limitation to the documentation.
void DisplayServerWayland::clipboard_set(const String &p_text) {
#if 0
// TODO: Port to WaylandThread semantics.
	MutexLock mutex_lock(wayland_thread.mutex);

	if (!wls.current_seat) {
		return;
	}

	WaylandThread::SeatState &ss = *wls.current_seat;

	if (!wls.current_seat->wl_data_source_selection && wls.globals.wl_data_device_manager) {
		ss.wl_data_source_selection = wl_data_device_manager_create_data_source(wls.globals.wl_data_device_manager);
		wl_data_source_add_listener(ss.wl_data_source_selection, &wl_data_source_listener, wls.current_seat);
		wl_data_source_offer(ss.wl_data_source_selection, "text/plain");
	}

	// TODO: Implement a good way of getting the latest serial from the user.
	wl_data_device_set_selection(ss.wl_data_device, ss.wl_data_source_selection, MAX(ss.pointer_data.button_serial, ss.last_key_pressed_serial));

	// Wait for the message to get to the server before continuing, otherwise the
	// clipboard update might come with a delay.
	wl_display_roundtrip(wls.wl_display);

	ss.selection_data = p_text.to_utf8_buffer();
#endif
}

String DisplayServerWayland::clipboard_get() const {
	MutexLock mutex_lock(wayland_thread.mutex);

	if (!wls.current_seat) {
		return String();
	}

	return WaylandThread::_wl_data_offer_read(wls.wl_display, wls.current_seat->wl_data_offer_selection);
}

void DisplayServerWayland::clipboard_set_primary(const String &p_text) {
#if 0
// TODO: Port to WaylandThread semantics.
	MutexLock mutex_lock(wayland_thread.mutex);

	if (!wls.current_seat) {
		return;
	}

	WaylandThread::SeatState &ss = *wls.current_seat;

	if (!wls.current_seat->wp_primary_selection_source && wls.globals.wp_primary_selection_device_manager) {
		ss.wp_primary_selection_source = zwp_primary_selection_device_manager_v1_create_source(wls.globals.wp_primary_selection_device_manager);
		zwp_primary_selection_source_v1_add_listener(ss.wp_primary_selection_source, &wp_primary_selection_source_listener, wls.current_seat);
		zwp_primary_selection_source_v1_offer(ss.wp_primary_selection_source, "text/plain");
	}

	// TODO: Implement a good way of getting the latest serial from the user.
	zwp_primary_selection_device_v1_set_selection(ss.wp_primary_selection_device, ss.wp_primary_selection_source, MAX(ss.pointer_data.button_serial, ss.last_key_pressed_serial));

	// Wait for the message to get to the server before continuing, otherwise the
	// clipboard update might come with a delay.
	wl_display_roundtrip(wls.wl_display);

	ss.primary_data = p_text.to_utf8_buffer();
#endif
}

String DisplayServerWayland::clipboard_get_primary() const {
	MutexLock mutex_lock(wayland_thread.mutex);

	if (!wls.current_seat) {
		return String();
	}

	return WaylandThread::_wp_primary_selection_offer_read(wls.wl_display, wls.current_seat->wp_primary_selection_offer);
}

int DisplayServerWayland::get_screen_count() const {
	MutexLock mutex_lock(wayland_thread.mutex);
	return wayland_thread.get_screen_count();
}

int DisplayServerWayland::get_primary_screen() const {
	// AFAIK Wayland doesn't allow knowing (nor we care) about which screen is
	// primary.
	return 0;
}

Point2i DisplayServerWayland::screen_get_position(int p_screen) const {
	MutexLock mutex_lock(wayland_thread.mutex);

	if (p_screen == SCREEN_OF_MAIN_WINDOW) {
		p_screen = window_get_current_screen();
	}

	return wayland_thread.screen_get_data(p_screen).position;
}

Size2i DisplayServerWayland::screen_get_size(int p_screen) const {
	MutexLock mutex_lock(wayland_thread.mutex);

	if (p_screen == SCREEN_OF_MAIN_WINDOW) {
		p_screen = window_get_current_screen();
	}

	return wayland_thread.screen_get_data(p_screen).size;
}

Rect2i DisplayServerWayland::screen_get_usable_rect(int p_screen) const {
	// Unsupported on wayland.
	return Rect2i(Point2i(), screen_get_size(p_screen));
}

int DisplayServerWayland::screen_get_dpi(int p_screen) const {
	MutexLock mutex_lock(wayland_thread.mutex);

	if (p_screen == SCREEN_OF_MAIN_WINDOW) {
		p_screen = window_get_current_screen();
	}

	const WaylandThread::ScreenData &data = wayland_thread.screen_get_data(p_screen);

	int width_mm = data.physical_size.width;
	int height_mm = data.physical_size.height;

	double xdpi = (width_mm ? data.size.width / (double)width_mm * 25.4 : 0);
	double ydpi = (height_mm ? data.size.height / (double)height_mm * 25.4 : 0);

	if (xdpi || ydpi) {
		return (xdpi + ydpi) / (xdpi && ydpi ? 2 : 1);
	}

	// Could not get DPI.
	return 96;
}

float DisplayServerWayland::screen_get_refresh_rate(int p_screen) const {
	MutexLock mutex_lock(wayland_thread.mutex);

	if (p_screen == SCREEN_OF_MAIN_WINDOW) {
		p_screen = window_get_current_screen();
	}

	return wayland_thread.screen_get_data(p_screen).refresh_rate;
}

void DisplayServerWayland::screen_set_keep_on(bool p_enable) {
	MutexLock mutex_lock(wayland_thread.mutex);

	if (screen_is_kept_on() == p_enable) {
		return;
	}

#ifdef DBUS_ENABLED
	if (screensaver) {
		if (p_enable) {
			screensaver->inhibit();
		} else {
			screensaver->uninhibit();
		}

		screensaver_inhibited = p_enable;
	}
#endif
}

bool DisplayServerWayland::screen_is_kept_on() const {
#ifdef DBUS_ENABLED
	return wayland_thread.window_get_idle_inhibition(MAIN_WINDOW_ID) || screensaver_inhibited;
#endif

	return wayland_thread.window_get_idle_inhibition(MAIN_WINDOW_ID);
}

Vector<DisplayServer::WindowID> DisplayServerWayland::get_window_list() const {
	MutexLock mutex_lock(wayland_thread.mutex);

	Vector<int> ret;
	ret.push_back(MAIN_WINDOW_ID);

	return ret;
}

void DisplayServerWayland::_show_window() {
	MutexLock mutex_lock(wayland_thread.mutex);

	WindowData &wd = main_window;

	if (!wd.visible) {
		DEBUG_LOG_WAYLAND("Showing window.");

		// Showing this window will reset its mode with whatever the compositor
		// reports. We'll save the mode beforehand so that we can reapply it later.
		// TODO: Fix/Port/Move/Whatever to `WaylandThread` APIs.
		WindowMode setup_mode = wd.mode;

		wayland_thread.window_create(MAIN_WINDOW_ID, wd.rect.size.width, wd.rect.size.height);
		wayland_thread.window_set_min_size(MAIN_WINDOW_ID, wd.min_size);
		wayland_thread.window_set_max_size(MAIN_WINDOW_ID, wd.max_size);
		wayland_thread.window_set_app_id(MAIN_WINDOW_ID, _get_app_id_from_context(context));
		wayland_thread.window_set_borderless(MAIN_WINDOW_ID, window_get_flag(WINDOW_FLAG_BORDERLESS));

		// NOTE: The XDG shell protocol is built in a way that causes the window to
		// be immediately shown as soon as a valid buffer is assigned to it. Hence,
		// the only acceptable way of implementing window showing is to move the
		// graphics context window creation logic here.
#ifdef VULKAN_ENABLED
		if (context_vulkan) {
			struct wl_surface *wl_surface = wayland_thread.window_get_wl_surface(wd.id);
			Error err = context_vulkan->window_create(MAIN_WINDOW_ID, wd.vsync_mode, wls.wl_display, wl_surface, wd.rect.size.width, wd.rect.size.height);
			ERR_FAIL_COND_MSG(err == ERR_CANT_CREATE, "Can't show a Vulkan window.");
		}
#endif

#ifdef GLES3_ENABLED
		if (egl_manager) {
			struct wl_surface *wl_surface = wayland_thread.window_get_wl_surface(wd.id);
			wd.wl_egl_window = wl_egl_window_create(wl_surface, wd.rect.size.width, wd.rect.size.height);

			Error err = egl_manager->window_create(MAIN_WINDOW_ID, wls.wl_display, wd.wl_egl_window, wd.rect.size.width, wd.rect.size.height);
			ERR_FAIL_COND_MSG(err == ERR_CANT_CREATE, "Can't show a GLES3 window.");

			window_set_vsync_mode(wd.vsync_mode, MAIN_WINDOW_ID);
		}
#endif
		// NOTE: The public window-handling methods might depend on this flag being
		// set. Ensure to not make any of these calls before this assignment.
		wd.visible = true;

		// Actually try to apply the window's mode now that it's visible.
		window_set_mode(setup_mode);

		wayland_thread.window_set_title(MAIN_WINDOW_ID, wd.title);
	}
}

DisplayServer::WindowID DisplayServerWayland::get_window_at_screen_position(const Point2i &p_position) const {
	// Standard Wayland APIs don't support this.
	return MAIN_WINDOW_ID;
}

void DisplayServerWayland::window_attach_instance_id(ObjectID p_instance, WindowID p_window_id) {
	MutexLock mutex_lock(wayland_thread.mutex);

	main_window.instance_id = p_instance;
}

ObjectID DisplayServerWayland::window_get_attached_instance_id(WindowID p_window_id) const {
	MutexLock mutex_lock(wayland_thread.mutex);

	return main_window.instance_id;
}

void DisplayServerWayland::window_set_title(const String &p_title, DisplayServer::WindowID p_window_id) {
	MutexLock mutex_lock(wayland_thread.mutex);

	WindowData &wd = main_window;

	wd.title = p_title;

	wayland_thread.window_set_title(MAIN_WINDOW_ID, wd.title);
}

void DisplayServerWayland::window_set_mouse_passthrough(const Vector<Vector2> &p_region, DisplayServer::WindowID p_window_id) {
	// TODO
	DEBUG_LOG_WAYLAND(vformat("wayland stub window_set_mouse_passthrough region %s", p_region));
}

void DisplayServerWayland::window_set_rect_changed_callback(const Callable &p_callable, DisplayServer::WindowID p_window_id) {
	MutexLock mutex_lock(wayland_thread.mutex);

	main_window.rect_changed_callback = p_callable;
}

void DisplayServerWayland::window_set_window_event_callback(const Callable &p_callable, DisplayServer::WindowID p_window_id) {
	MutexLock mutex_lock(wayland_thread.mutex);

	main_window.window_event_callback = p_callable;
}

void DisplayServerWayland::window_set_input_event_callback(const Callable &p_callable, DisplayServer::WindowID p_window_id) {
	MutexLock mutex_lock(wayland_thread.mutex);

	main_window.input_event_callback = p_callable;
}

void DisplayServerWayland::window_set_input_text_callback(const Callable &p_callable, WindowID p_window_id) {
	MutexLock mutex_lock(wayland_thread.mutex);

	main_window.input_text_callback = p_callable;
}

void DisplayServerWayland::window_set_drop_files_callback(const Callable &p_callable, DisplayServer::WindowID p_window_id) {
	MutexLock mutex_lock(wayland_thread.mutex);

	main_window.drop_files_callback = p_callable;
}

int DisplayServerWayland::window_get_current_screen(DisplayServer::WindowID p_window_id) const {
	// Standard Wayland APIs don't support getting the screen of a window.
	return 0;
}

void DisplayServerWayland::window_set_current_screen(int p_screen, DisplayServer::WindowID p_window_id) {
	// Standard Wayland APIs don't support setting the screen of a window.
}

Point2i DisplayServerWayland::window_get_position(DisplayServer::WindowID p_window_id) const {
	MutexLock mutex_lock(wayland_thread.mutex);

	// We can't know the position of toplevels with the standard protocol.
	return Point2i();
}

Point2i DisplayServerWayland::window_get_position_with_decorations(DisplayServer::WindowID p_window_id) const {
	MutexLock mutex_lock(wayland_thread.mutex);

	// We can't know the position of toplevels with the standard protocol, nor can
	// we get information about the decorations, at least with SSDs.
	return Point2i();
}

void DisplayServerWayland::window_set_position(const Point2i &p_position, DisplayServer::WindowID p_window_id) {
	// Setting the position of a non borderless window is not supported.
}

void DisplayServerWayland::window_set_max_size(const Size2i p_size, DisplayServer::WindowID p_window_id) {
	MutexLock mutex_lock(wayland_thread.mutex);

	DEBUG_LOG_WAYLAND(vformat("window max size set to %s", p_size));

	if (p_size.x < 0 || p_size.y < 0) {
		ERR_FAIL_MSG("Maximum window size can't be negative!");
	}

	WindowData &wd = main_window;

	// FIXME: Is `p_size.x < wd.min_size.x || p_size.y < wd.min_size.y` == `p_size < wd.min_size`?
	if ((p_size != Size2i()) && ((p_size.x < wd.min_size.x) || (p_size.y < wd.min_size.y))) {
		ERR_PRINT("Maximum window size can't be smaller than minimum window size!");
		return;
	}

	wd.max_size = p_size;

	wayland_thread.window_set_max_size(MAIN_WINDOW_ID, p_size);
}

Size2i DisplayServerWayland::window_get_max_size(DisplayServer::WindowID p_window_id) const {
	MutexLock mutex_lock(wayland_thread.mutex);

	return main_window.max_size;
}

void DisplayServerWayland::gl_window_make_current(DisplayServer::WindowID p_window_id_id) {
#ifdef GLES3_ENABLED
	if (egl_manager) {
		egl_manager->window_make_current(MAIN_WINDOW_ID);
	}
#endif
}

void DisplayServerWayland::window_set_transient(WindowID p_window_id, WindowID p_parent) {
	// Currently unsupported.
}

void DisplayServerWayland::window_set_min_size(const Size2i p_size, DisplayServer::WindowID p_window_id) {
	MutexLock mutex_lock(wayland_thread.mutex);

	DEBUG_LOG_WAYLAND(vformat("window minsize set to %s", p_size));

	WindowData &wd = main_window;

	if (p_size.x < 0 || p_size.y < 0) {
		ERR_FAIL_MSG("Minimum window size can't be negative!");
	}

	// FIXME: Is `p_size.x > wd.max_size.x || p_size.y > wd.max_size.y` == `p_size > wd.max_size`?
	if ((p_size != Size2i()) && (wd.max_size != Size2i()) && ((p_size.x > wd.max_size.x) || (p_size.y > wd.max_size.y))) {
		ERR_PRINT("Minimum window size can't be larger than maximum window size!");
		return;
	}

	wd.min_size = p_size;

	wayland_thread.window_set_min_size(MAIN_WINDOW_ID, p_size);
}

Size2i DisplayServerWayland::window_get_min_size(DisplayServer::WindowID p_window_id) const {
	MutexLock mutex_lock(wayland_thread.mutex);

	return main_window.min_size;
}

void DisplayServerWayland::window_set_size(const Size2i p_size, DisplayServer::WindowID p_window_id) {
	MutexLock mutex_lock(wayland_thread.mutex);

	wayland_thread.window_resize(MAIN_WINDOW_ID, p_size);
	_resize_window(p_size);
}

Size2i DisplayServerWayland::window_get_size(DisplayServer::WindowID p_window_id) const {
	MutexLock mutex_lock(wayland_thread.mutex);

	return main_window.rect.size;
}

Size2i DisplayServerWayland::window_get_size_with_decorations(DisplayServer::WindowID p_window_id) const {
	MutexLock mutex_lock(wayland_thread.mutex);

	// I don't think there's a way of actually knowing the size of the window
	// decoration in Wayland, at least in the case of SSDs, nor that it would be
	// that useful in this case. We'll just return the main window's size.
	return main_window.rect.size;
}

void DisplayServerWayland::window_set_mode(WindowMode p_mode, DisplayServer::WindowID p_window_id) {
	// TODO: Port to `WaylandThread` APIs.

#if 0
	MutexLock mutex_lock(wayland_thread.mutex);

	WindowData &wd = main_window;

	if (!wd.visible || wd.mode == p_mode) {
		return;
	}

	// Don't waste time with hidden windows and whatnot. Behave like it worked.
#ifdef LIBDECOR_ENABLED
	if ((!wd.wl_surface || !wd.xdg_toplevel) && !wd.libdecor_frame) {
#else
	if (!wd.wl_surface || !wd.xdg_toplevel) {
#endif // LIBDECOR_ENABLED
		wd.mode = p_mode;
		return;
	}

	// Return back to a windowed state so that we can apply what the user asked.
	switch (wd.mode) {
		case WINDOW_MODE_WINDOWED: {
			// Do nothing.
		} break;

		case WINDOW_MODE_MINIMIZED: {
			// We can't do much according to the xdg_shell protocol. I have no idea
			// whether this implies that we should return or who knows what. For now
			// we'll do nothing.
			// TODO: Test this properly.
		} break;

		case WINDOW_MODE_MAXIMIZED: {
			// Try to unmaximize. This isn't garaunteed to work actually, so we'll have
			// to check whether something changed.
			if (wd.xdg_toplevel) {
				xdg_toplevel_unset_maximized(wd.xdg_toplevel);
			}

#ifdef LIBDECOR_ENABLED
			if (wd.libdecor_frame) {
				libdecor_frame_unset_maximized(wd.libdecor_frame);
			}
#endif
		} break;

		case WINDOW_MODE_FULLSCREEN:
		case WINDOW_MODE_EXCLUSIVE_FULLSCREEN: {
			// Same thing as above, unset fullscreen and check later if it worked.
			if (wd.xdg_toplevel) {
				xdg_toplevel_unset_fullscreen(wd.xdg_toplevel);
			}

#ifdef LIBDECOR_ENABLED
			if (wd.libdecor_frame) {
				libdecor_frame_unset_fullscreen(wd.libdecor_frame);
			}
#endif
		} break;
	}

	// Wait for a configure event and hope that something changed.
	wl_display_roundtrip(wls.wl_display);

	if (wd.mode != WINDOW_MODE_WINDOWED) {
		// The compositor refused our "normalization" request. It'd be useless or
		// unpredictable to attempt setting a new state. We're done.
		return;
	}

	// Ask the compositor to set the state indicated by the new mode.
	switch (p_mode) {
		case WINDOW_MODE_WINDOWED: {
			// Do nothing. We're already windowed.
		} break;

		case WINDOW_MODE_MINIMIZED: {
			if (wd.xdg_toplevel) {
				if (!wd.can_minimize) {
					// We can't minimize, ignore.
					break;
				}

				xdg_toplevel_set_minimized(wd.xdg_toplevel);
			}

#ifdef LIBDECOR_ENABLED
			if (wd.libdecor_frame) {
				if (!libdecor_frame_has_capability(wd.libdecor_frame, LIBDECOR_ACTION_MINIMIZE)) {
					// We can't minimize, ignore.
					break;
				}

				libdecor_frame_set_minimized(wd.libdecor_frame);
			}
#endif
			// We have no way to actually detect this state, so we'll have to report it
			// manually to the engine (hoping that it worked). In the worst case it'll
			// get reset by the next configure event.
			wd.mode = WINDOW_MODE_MINIMIZED;
		} break;

		case WINDOW_MODE_MAXIMIZED: {
			if (wd.xdg_toplevel) {
				if (!wd.can_maximize) {
					// We can't maximize, ignore.
					break;
				}

				xdg_toplevel_set_maximized(wd.xdg_toplevel);
			}

#ifdef LIBDECOR_ENABLED
			if (wd.libdecor_frame) {
				// NOTE: libdecor doesn't seem to have a maximize capability query?
				// The fact that there's a fullscreen one makes me suspicious.
				libdecor_frame_set_maximized(wd.libdecor_frame);
			}
#endif
		} break;

		case WINDOW_MODE_FULLSCREEN:
		case WINDOW_MODE_EXCLUSIVE_FULLSCREEN: {
			if (wd.xdg_toplevel) {
				if (!wd.can_fullscreen) {
					// We can't fullscreen, ignore.
					break;
				}

				xdg_toplevel_set_fullscreen(wd.xdg_toplevel, nullptr);
			}

#ifdef LIBDECOR_ENABLED
			if (wd.libdecor_frame) {
				if (!libdecor_frame_has_capability(wd.libdecor_frame, LIBDECOR_ACTION_FULLSCREEN)) {
					// We can't fullscreen, ignore.
					break;
				}

				libdecor_frame_set_fullscreen(wd.libdecor_frame, nullptr);
			}
#endif
		} break;

		default: {
		} break;
	}
#endif // 0
}

DisplayServer::WindowMode DisplayServerWayland::window_get_mode(DisplayServer::WindowID p_window_id) const {
	MutexLock mutex_lock(wayland_thread.mutex);

	return main_window.mode;
}

bool DisplayServerWayland::window_is_maximize_allowed(DisplayServer::WindowID p_window_id) const {
	MutexLock mutex_lock(wayland_thread.mutex);

	return wayland_thread.window_can_set_mode(p_window_id, WINDOW_MODE_MAXIMIZED);
}

void DisplayServerWayland::window_set_flag(WindowFlags p_flag, bool p_enabled, DisplayServer::WindowID p_window_id) {
	MutexLock mutex_lock(wayland_thread.mutex);

	WindowData &wd = main_window;

	DEBUG_LOG_WAYLAND(vformat("Window set flag %d", p_flag));

	switch (p_flag) {
		case WINDOW_FLAG_BORDERLESS: {
			wayland_thread.window_set_borderless(MAIN_WINDOW_ID, p_enabled);
		} break;

		default: {
		}
	}

	if (p_enabled) {
		wd.flags |= 1 << p_flag;
	} else {
		wd.flags &= ~(1 << p_flag);
	}
}

bool DisplayServerWayland::window_get_flag(WindowFlags p_flag, DisplayServer::WindowID p_window_id) const {
	MutexLock mutex_lock(wayland_thread.mutex);

	return main_window.flags & (1 << p_flag);
}

void DisplayServerWayland::window_request_attention(DisplayServer::WindowID p_window_id) {
	MutexLock mutex_lock(wayland_thread.mutex);

	DEBUG_LOG_WAYLAND("Requested attention.");

	wayland_thread.window_request_attention(MAIN_WINDOW_ID);
}

void DisplayServerWayland::window_move_to_foreground(DisplayServer::WindowID p_window_id) {
	// Standard Wayland APIs don't support this.
}

bool DisplayServerWayland::window_can_draw(DisplayServer::WindowID p_window_id) const {
	// TODO: Implement this. For now a simple return true will work though.
	return true;
}

bool DisplayServerWayland::can_any_window_draw() const {
	// TODO: Implement this. For now a simple return true will work though.
	return true;
}

void DisplayServerWayland::window_set_ime_active(const bool p_active, DisplayServer::WindowID p_window_id) {
	// TODO
	DEBUG_LOG_WAYLAND(vformat("wayland stub window_set_ime_active active %s", p_active ? "true" : "false"));
}

void DisplayServerWayland::window_set_ime_position(const Point2i &p_pos, DisplayServer::WindowID p_window_id) {
	// TODO
	DEBUG_LOG_WAYLAND(vformat("wayland stub window_set_ime_position pos %s window %d", p_pos, p_window_id));
}

// NOTE: While Wayland is supposed to be tear-free, wayland-protocols version
// 1.30 added a protocol for allowing async flips which is supposed to be
// handled by drivers such as Vulkan. We can then just ask to disable v-sync and
// hope for the best. See: https://gitlab.freedesktop.org/wayland/wayland-protocols/-/commit/6394f0b4f3be151076f10a845a2fb131eeb56706
void DisplayServerWayland::window_set_vsync_mode(DisplayServer::VSyncMode p_vsync_mode, DisplayServer::WindowID p_window_id) {
	MutexLock mutex_lock(wayland_thread.mutex);

#ifdef VULKAN_ENABLED
	if (context_vulkan) {
		context_vulkan->set_vsync_mode(p_window_id, p_vsync_mode);
	}
#endif // VULKAN_ENABLED

#ifdef GLES3_ENABLED
	if (egl_manager) {
		egl_manager->set_use_vsync(p_vsync_mode != DisplayServer::VSYNC_DISABLED);
	}
#endif // GLES3_ENABLED
}

DisplayServer::VSyncMode DisplayServerWayland::window_get_vsync_mode(DisplayServer::WindowID p_window_id) const {
#ifdef VULKAN_ENABLED
	if (context_vulkan) {
		return context_vulkan->get_vsync_mode(p_window_id);
	}
#endif // VULKAN_ENABLED

#ifdef GLES3_ENABLED
	if (egl_manager) {
		return egl_manager->is_using_vsync() ? DisplayServer::VSYNC_ENABLED : DisplayServer::VSYNC_DISABLED;
	}
#endif // GLES3_ENABLED
	return DisplayServer::VSYNC_ENABLED;
}

void DisplayServerWayland::cursor_set_shape(CursorShape p_shape) {
	ERR_FAIL_INDEX(p_shape, CURSOR_MAX);

	MutexLock mutex_lock(wayland_thread.mutex);

	if (p_shape == wls.cursor_shape) {
		return;
	}

	wls.cursor_shape = p_shape;

	WaylandThread::_wayland_state_update_cursor(wls);
}

DisplayServerWayland::CursorShape DisplayServerWayland::cursor_get_shape() const {
	MutexLock mutex_lock(wayland_thread.mutex);

	return wls.cursor_shape;
}

void DisplayServerWayland::cursor_set_custom_image(const Ref<Resource> &p_cursor, CursorShape p_shape, const Vector2 &p_hotspot) {
	MutexLock mutex_lock(wayland_thread.mutex);

	if (p_cursor.is_valid()) {
		HashMap<CursorShape, WaylandThread::CustomWaylandCursor>::Iterator cursor_c = wls.custom_cursors.find(p_shape);

		if (cursor_c) {
			if (cursor_c->value.cursor_rid == p_cursor->get_rid() && cursor_c->value.hotspot == p_hotspot) {
				cursor_set_shape(p_shape);
				return;
			}

			wls.custom_cursors.erase(p_shape);
		}

		Ref<Texture2D> texture = p_cursor;
		ERR_FAIL_COND(!texture.is_valid());
		Ref<AtlasTexture> atlas_texture = p_cursor;
		Size2i texture_size;
		Rect2i atlas_rect;

		ERR_FAIL_COND(!texture.is_valid());

		if (atlas_texture.is_valid()) {
			texture = atlas_texture->get_atlas();

			atlas_rect.size.width = texture->get_width();
			atlas_rect.size.height = texture->get_height();
			atlas_rect.position.x = atlas_texture->get_region().position.x;
			atlas_rect.position.y = atlas_texture->get_region().position.y;

			texture_size.width = atlas_texture->get_region().size.x;
			texture_size.height = atlas_texture->get_region().size.y;
		} else {
			texture_size.width = texture->get_width();
			texture_size.height = texture->get_height();
		}

		ERR_FAIL_COND(p_hotspot.x < 0 || p_hotspot.y < 0);
		// NOTE: The Wayland protocol says nothing about cursor size limits, yet if
		// the texture is larger than 256x256 it won't show at least on sway.
		ERR_FAIL_COND(texture_size.width > 256 || texture_size.height > 256);
		ERR_FAIL_COND(p_hotspot.x > texture_size.width || p_hotspot.y > texture_size.height);
		ERR_FAIL_COND(texture_size.height == 0 || texture_size.width == 0);

		Ref<Image> image = texture->get_image();
		ERR_FAIL_COND(!image.is_valid());

		if (image->is_compressed()) {
			image = image->duplicate(true);
			Error err = image->decompress();
			ERR_FAIL_COND_MSG(err != OK, "Couldn't decompress VRAM-compressed custom mouse cursor image. Switch to a lossless compression mode in the Import dock.");
		}

		// NOTE: The stride is the width of the image in bytes.
		unsigned int texture_stride = texture_size.width * 4;
		unsigned int data_size = texture_stride * texture_size.height;

		// We need a shared memory object file descriptor in order to create a
		// wl_buffer through wl_shm.
		int fd = WaylandThread::_allocate_shm_file(data_size);
		ERR_FAIL_COND(fd == -1);

		WaylandThread::CustomWaylandCursor &cursor = wls.custom_cursors[p_shape];
		cursor.cursor_rid = p_cursor->get_rid();
		cursor.hotspot = p_hotspot;

		if (cursor.buffer_data) {
			// Clean up the old buffer data.
			munmap(cursor.buffer_data, cursor.buffer_data_size);
		}

		cursor.buffer_data = (uint32_t *)mmap(NULL, data_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

		if (cursor.wl_buffer) {
			// Clean up the old Wayland buffer.
			wl_buffer_destroy(cursor.wl_buffer);
		}

		// Create the Wayland buffer.
		struct wl_shm_pool *wl_shm_pool = wl_shm_create_pool(wls.globals.wl_shm, fd, texture_size.height * data_size);
		// TODO: Make sure that WL_SHM_FORMAT_ARGB8888 format is supported. It
		// technically isn't garaunteed to be supported, but I think that'd be a
		// pretty unlikely thing to stumble upon.
		cursor.wl_buffer = wl_shm_pool_create_buffer(wl_shm_pool, 0, texture_size.width, texture_size.height, texture_stride, WL_SHM_FORMAT_ARGB8888);
		wl_shm_pool_destroy(wl_shm_pool);

		// Fill the cursor buffer with the texture data.
		for (unsigned int index = 0; index < (unsigned int)(texture_size.width * texture_size.height); index++) {
			int row_index = floor(index / texture_size.width) + atlas_rect.position.y;
			int column_index = (index % int(texture_size.width)) + atlas_rect.position.x;

			if (atlas_texture.is_valid()) {
				column_index = MIN(column_index, atlas_rect.size.width - 1);
				row_index = MIN(row_index, atlas_rect.size.height - 1);
			}

			cursor.buffer_data[index] = image->get_pixel(column_index, row_index).to_argb32();

			// Wayland buffers, unless specified, require associated alpha, so we'll just
			// associate the alpha in-place.
			uint8_t *pixel_data = (uint8_t *)&cursor.buffer_data[index];
			pixel_data[0] = pixel_data[0] * pixel_data[3] / 255;
			pixel_data[1] = pixel_data[1] * pixel_data[3] / 255;
			pixel_data[2] = pixel_data[2] * pixel_data[3] / 255;
		}
	} else {
		// Reset to default system cursor.
		if (wls.custom_cursors.has(p_shape)) {
			wls.custom_cursors.erase(p_shape);
		}
	}

	WaylandThread::_wayland_state_update_cursor(wls);
}

int DisplayServerWayland::keyboard_get_layout_count() const {
	MutexLock mutex_lock(wayland_thread.mutex);

	if (wls.current_seat && wls.current_seat->xkb_keymap) {
		return xkb_keymap_num_layouts(wls.current_seat->xkb_keymap);
	}

	return 0;
}

int DisplayServerWayland::keyboard_get_current_layout() const {
	MutexLock mutex_lock(wayland_thread.mutex);

	if (wls.current_seat) {
		return wls.current_seat->current_layout_index;
	}

	return 0;
}

void DisplayServerWayland::keyboard_set_current_layout(int p_index) {
	MutexLock mutex_lock(wayland_thread.mutex);

	if (wls.current_seat) {
		wls.current_seat->current_layout_index = p_index;
	}
}

String DisplayServerWayland::keyboard_get_layout_language(int p_index) const {
	// xkbcommon exposes only the layout's name, which looks like it overlaps with
	// its language.
	return keyboard_get_layout_name(p_index);
}

String DisplayServerWayland::keyboard_get_layout_name(int p_index) const {
	MutexLock mutex_lock(wayland_thread.mutex);

	String ret;

	if (wls.current_seat && wls.current_seat->xkb_keymap) {
		ret.parse_utf8(xkb_keymap_layout_get_name(wls.current_seat->xkb_keymap, p_index));
	}

	return ret;
}

Key DisplayServerWayland::keyboard_get_keycode_from_physical(Key p_keycode) const {
	MutexLock mutex_lock(wayland_thread.mutex);

	xkb_keycode_t xkb_keycode = KeyMappingXKB::get_xkb_keycode(p_keycode);

	Key key = Key::NONE;

	if (wls.current_seat && wls.current_seat->xkb_state) {
		// NOTE: Be aware that this method will always return something, even if this
		// line might never be executed if the current seat doesn't have a keyboard.
		key = KeyMappingXKB::get_keycode(xkb_state_key_get_one_sym(wls.current_seat->xkb_state, xkb_keycode));
	}

	// If not found, fallback to QWERTY.
	// This should match the behavior of the event pump.
	if (key == Key::NONE) {
		return p_keycode;
	}

	if (key >= Key::A + 32 && key <= Key::Z + 32) {
		key -= 'a' - 'A';
	}

	// Make it consistent with the keys returned by `Input`.
	if (key == Key::BACKTAB) {
		key = Key::TAB;
	}

	return key;
}

void DisplayServerWayland::process_events() {
	MutexLock mutex_lock(wayland_thread.mutex);

	int werror = wl_display_get_error(wls.wl_display);

	if (werror) {
		if (werror == EPROTO) {
			struct wl_interface *wl_interface = nullptr;
			uint32_t id = 0;

			int error_code = wl_display_get_protocol_error(wls.wl_display, (const struct wl_interface **)&wl_interface, &id);
			print_error(vformat("Wayland protocol error %d on interface %s@%d.", error_code, wl_interface ? wl_interface->name : "unknown", id));
		} else {
			print_error(vformat("Wayland client error code %d.", werror));
		}
	}

	while (wayland_thread.has_message()) {
		Ref<WaylandThread::Message> msg = wayland_thread.pop_message();

		Ref<WaylandThread::WindowRectMessage> winrect_msg = msg;
		if (winrect_msg.is_valid()) {
			_resize_window(winrect_msg->rect.size);
		}

		Ref<WaylandThread::WindowEventMessage> winev_msg = msg;
		if (winev_msg.is_valid()) {
			_send_window_event(winev_msg->event);

			if (winev_msg->event == WINDOW_EVENT_FOCUS_IN) {
				if (OS::get_singleton()->get_main_loop()) {
					OS::get_singleton()->get_main_loop()->notification(MainLoop::NOTIFICATION_APPLICATION_FOCUS_IN);
				}
			} else if (winev_msg->event == WINDOW_EVENT_FOCUS_OUT) {
				if (OS::get_singleton()->get_main_loop()) {
					OS::get_singleton()->get_main_loop()->notification(MainLoop::NOTIFICATION_APPLICATION_FOCUS_OUT);
				}
			}
		}

		Ref<WaylandThread::InputEventMessage> inputev_msg = msg;
		if (inputev_msg.is_valid()) {
			Input::get_singleton()->parse_input_event(inputev_msg->event);
		}

		Ref<WaylandThread::DropFilesEventMessage> dropfiles_msg = msg;
		if (dropfiles_msg.is_valid()) {
			WindowData wd = main_window;

			if (wd.drop_files_callback.is_valid()) {
				Variant var_files = dropfiles_msg->files;
				Variant *arg = &var_files;

				Variant ret;
				Callable::CallError ce;

				wd.drop_files_callback.callp((const Variant **)&arg, 1, ret, ce);
			}
		}
	}

	if (!wls.current_seat) {
		return;
	}

	WaylandThread::SeatState &seat = *wls.current_seat;

	// TODO: Comment and document out properly this block of code.
	// In short, this implements key repeating.
	if (seat.repeat_key_delay_msec && seat.repeating_keycode != XKB_KEYCODE_INVALID) {
		uint64_t current_ticks = OS::get_singleton()->get_ticks_msec();
		uint64_t delayed_start_ticks = seat.last_repeat_start_msec + seat.repeat_start_delay_msec;

		if (seat.last_repeat_msec < delayed_start_ticks) {
			seat.last_repeat_msec = delayed_start_ticks;
		}

		if (current_ticks >= delayed_start_ticks) {
			uint64_t ticks_delta = current_ticks - seat.last_repeat_msec;

			int keys_amount = (ticks_delta / seat.repeat_key_delay_msec);

			for (int i = 0; i < keys_amount; i++) {
				Ref<InputEventKey> k;
				k.instantiate();

				if (!WaylandThread::_seat_state_configure_key_event(seat, k, seat.repeating_keycode, true)) {
					continue;
				}

				k->set_echo(true);

				Input::get_singleton()->parse_input_event(k);
			}

			seat.last_repeat_msec += ticks_delta - (ticks_delta % seat.repeat_key_delay_msec);
		}
	}

	Input::get_singleton()->flush_buffered_events();
}

void DisplayServerWayland::release_rendering_thread() {
#ifdef GLES3_ENABLED
	if (egl_manager) {
		egl_manager->release_current();
	}
#endif
}

void DisplayServerWayland::make_rendering_thread() {
#ifdef GLES3_ENABLED
	if (egl_manager) {
		egl_manager->make_current();
	}
#endif
}

void DisplayServerWayland::swap_buffers() {
#ifdef GLES3_ENABLED
	if (egl_manager) {
		egl_manager->swap_buffers();
	}
#endif
}

void DisplayServerWayland::set_context(Context p_context) {
	MutexLock mutex_lock(wayland_thread.mutex);

	DEBUG_LOG_WAYLAND(vformat("Setting context %d.", p_context));

	context = p_context;

	String app_id = _get_app_id_from_context(p_context);
	wayland_thread.window_set_app_id(MAIN_WINDOW_ID, app_id);
}

Vector<String> DisplayServerWayland::get_rendering_drivers_func() {
	Vector<String> drivers;

#ifdef VULKAN_ENABLED
	drivers.push_back("vulkan");
#endif

#ifdef GLES3_ENABLED
	drivers.push_back("opengl3");
#endif

	return drivers;
}

DisplayServer *DisplayServerWayland::create_func(const String &p_rendering_driver, WindowMode p_mode, VSyncMode p_vsync_mode, uint32_t p_flags, const Point2i *p_position, const Size2i &p_resolution, int p_screen, Error &r_error) {
	DisplayServer *ds = memnew(DisplayServerWayland(p_rendering_driver, p_mode, p_vsync_mode, p_flags, p_resolution, r_error));
	if (r_error != OK) {
		ERR_PRINT("Can't create the Wayland display server.");
		memdelete(ds);

		return nullptr;
	}
	return ds;
}

DisplayServerWayland::DisplayServerWayland(const String &p_rendering_driver, WindowMode p_mode, VSyncMode p_vsync_mode, uint32_t p_flags, const Vector2i &p_resolution, Error &r_error) {
#ifdef SOWRAP_ENABLED
#ifdef DEBUG_ENABLED
	int dylibloader_verbose = 1;
#else
	int dylibloader_verbose = 0;
#endif // DEBUG_ENABLED
#endif // SOWRAP_ENABLED

	r_error = ERR_UNAVAILABLE;

	// TODO: Remove this.
	wls.wayland_thread = &wayland_thread;

	Error thread_err = wayland_thread.init(wls);

	if (thread_err != OK) {
		r_error = thread_err;
		ERR_FAIL_MSG("Could not initialize the Wayland thread.");
	}

	// Input.
	Input::get_singleton()->set_event_dispatch_function(dispatch_input_events);

#ifdef SPEECHD_ENABLED
	// Init TTS
	tts = memnew(TTS_Linux);
#endif

#ifdef VULKAN_ENABLED
	if (p_rendering_driver == "vulkan") {
		context_vulkan = memnew(VulkanContextWayland);

		if (context_vulkan->initialize() != OK) {
			memdelete(context_vulkan);
			context_vulkan = nullptr;
			r_error = ERR_CANT_CREATE;
			ERR_FAIL_MSG("Could not initialize Vulkan.");
		}
	}
#endif

#ifdef GLES3_ENABLED
	if (p_rendering_driver == "opengl3") {
		egl_manager = memnew(EGLManagerWayland);

#ifdef SOWRAP_ENABLED
		if (initialize_wayland_egl(dylibloader_verbose) != 0) {
			WARN_PRINT("Can't load the Wayland EGL library.");
			return;
		}
#endif // SOWRAP_ENABLED

		if (egl_manager->initialize() != OK) {
			memdelete(egl_manager);
			egl_manager = nullptr;
			r_error = ERR_CANT_CREATE;
			ERR_FAIL_MSG("Could not initialize GLES3.");
		}

		RasterizerGLES3::make_current();
	}
#endif // GLES3_ENABLED
	const char *cursor_theme = OS::get_singleton()->get_environment("XCURSOR_THEME").utf8().ptr();

	int64_t cursor_size = OS::get_singleton()->get_environment("XCURSOR_SIZE").to_int();
	if (cursor_size <= 0) {
		print_verbose("Detected invalid cursor size preference, defaulting to 24.");
		cursor_size = 24;
	}

	print_verbose(vformat("Loading cursor theme \"%s\" size %d.", cursor_theme, cursor_size));
	wls.wl_cursor_theme = wl_cursor_theme_load(cursor_theme, cursor_size, wls.globals.wl_shm);

	ERR_FAIL_NULL_MSG(wls.wl_cursor_theme, "Can't find a cursor theme.");

	static const char *cursor_names[] = {
		"left_ptr",
		"xterm",
		"hand2",
		"cross",
		"watch",
		"left_ptr_watch",
		"fleur",
		"dnd-move",
		"crossed_circle",
		"v_double_arrow",
		"h_double_arrow",
		"size_bdiag",
		"size_fdiag",
		"move",
		"row_resize",
		"col_resize",
		"question_arrow"
	};

	static const char *cursor_names_fallback[] = {
		nullptr,
		nullptr,
		"pointer",
		"cross",
		"wait",
		"progress",
		"grabbing",
		"hand1",
		"forbidden",
		"ns-resize",
		"ew-resize",
		"fd_double_arrow",
		"bd_double_arrow",
		"fleur",
		"sb_v_double_arrow",
		"sb_h_double_arrow",
		"help"
	};

	for (int i = 0; i < CURSOR_MAX; i++) {
		struct wl_cursor *cursor = wl_cursor_theme_get_cursor(wls.wl_cursor_theme, cursor_names[i]);

		if (!cursor && cursor_names_fallback[i]) {
			cursor = wl_cursor_theme_get_cursor(wls.wl_cursor_theme, cursor_names[i]);
		}

		if (cursor && cursor->image_count > 0) {
			wls.cursor_images[i] = cursor->images[0];
			wls.cursor_bufs[i] = wl_cursor_image_get_buffer(cursor->images[0]);
		} else {
			wls.cursor_images[i] = nullptr;
			wls.cursor_bufs[i] = nullptr;
			print_verbose("Failed loading cursor: " + String(cursor_names[i]));
		}
	}

	cursor_set_shape(CURSOR_BUSY);

	WindowData &wd = main_window;

	wd.wls = &wls;
	wd.id = MAIN_WINDOW_ID;
	wd.mode = p_mode;
	wd.flags = p_flags;
	wd.vsync_mode = p_vsync_mode;
	wd.rect.size = p_resolution;
	wd.title = "Godot";

	_show_window();

#ifdef VULKAN_ENABLED
	if (p_rendering_driver == "vulkan") {
		rendering_device_vulkan = memnew(RenderingDeviceVulkan);
		rendering_device_vulkan->initialize(context_vulkan);

		RendererCompositorRD::make_current();
	}
#endif

#ifdef DBUS_ENABLED
	portal_desktop = memnew(FreeDesktopPortalDesktop);
	screensaver = memnew(FreeDesktopScreenSaver);
#endif

	screen_set_keep_on(GLOBAL_GET("display/window/energy_saving/keep_screen_on"));

	r_error = OK;
}

DisplayServerWayland::~DisplayServerWayland() {
	// TODO: Multiwindow support.
	if (main_window.visible) {
#ifdef VULKAN_ENABLED
		if (context_vulkan) {
			context_vulkan->window_destroy(MAIN_WINDOW_ID);
		}
#endif

#ifdef GLES3_ENABLED
		if (egl_manager) {
			egl_manager->window_destroy(MAIN_WINDOW_ID);
		}
#endif
	}

	if (main_window.wl_egl_window) {
		wl_egl_window_destroy(main_window.wl_egl_window);
	}

	wayland_thread.destroy();

	// Destroy all drivers.
#ifdef VULKAN_ENABLED
	if (rendering_device_vulkan) {
		rendering_device_vulkan->finalize();
		memdelete(rendering_device_vulkan);
	}

	if (context_vulkan) {
		memdelete(context_vulkan);
	}
#endif

#ifdef SPEECHD_ENABLED
	if (tts) {
		memdelete(tts);
	}
#endif

#ifdef DBUS_ENABLED
	if (portal_desktop) {
		memdelete(portal_desktop);
		memdelete(screensaver);
	}
#endif
}

void DisplayServerWayland::register_wayland_driver() {
	register_create_function("wayland", create_func, get_rendering_drivers_func);
}

#endif //WAYLAND_ENABLED
