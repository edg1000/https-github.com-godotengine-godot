/*************************************************************************/
/*  egl_manager.cpp                                                      */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
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

#include "egl_manager.h"

#ifdef EGL_ENABLED

// Creates and caches a GLDisplay. Returns -1 on error.
int EGLManager::_get_gldisplay_id(void *p_display) {
	// Look for a cached GLDisplay.
	for (unsigned int i = 0; i < displays.size(); i++) {
		if (displays[i].display == p_display) {
			return i;
		}
	}

	// We didn't find any, so we'll have to create one, along with its own
	// EGLDisplay and EGLContext.
	GLDisplay new_gldisplay;
	new_gldisplay.display = p_display;

	new_gldisplay.egl_display = eglGetPlatformDisplay(_get_platform_extension_enum(), new_gldisplay.display, NULL);
	ERR_FAIL_COND_V(eglGetError() != EGL_SUCCESS, -1);

	ERR_FAIL_COND_V_MSG(new_gldisplay.egl_display == EGL_NO_DISPLAY, -1, "Can't create an EGL display.");

	// TODO: Check EGL version?
	if (!eglInitialize(new_gldisplay.egl_display, NULL, NULL)) {
		ERR_FAIL_V_MSG(-1, "Can't initialize an EGL display.");
	}

	if (!eglBindAPI(EGL_OPENGL_API)) {
		ERR_FAIL_V_MSG(-1, "OpenGL not supported.");
	}

	Error err = _gldisplay_create_context(new_gldisplay);

	if (err != OK) {
		eglTerminate(new_gldisplay.egl_display);
		ERR_FAIL_V(-1);
	}

	displays.push_back(new_gldisplay);

	// Return the new GLDisplay's ID.
	return displays.size() - 1;
}

Error EGLManager::_gldisplay_create_context(GLDisplay &p_gldisplay) {
	EGLint attribs[] = {
		EGL_RED_SIZE,
		1,
		EGL_BLUE_SIZE,
		1,
		EGL_GREEN_SIZE,
		1,
		EGL_DEPTH_SIZE,
		24,
		EGL_NONE,
	};

	EGLint attribs_layered[] = {
		EGL_RED_SIZE,
		8,
		EGL_GREEN_SIZE,
		8,
		EGL_GREEN_SIZE,
		8,
		EGL_ALPHA_SIZE,
		8,
		EGL_DEPTH_SIZE,
		24,
		EGL_NONE,
	};

	EGLint config_count = 0;

	if (OS::get_singleton()->is_layered_allowed()) {
		eglChooseConfig(p_gldisplay.egl_display, attribs_layered, &p_gldisplay.egl_config, 1, &config_count);
	} else {
		eglChooseConfig(p_gldisplay.egl_display, attribs, &p_gldisplay.egl_config, 1, &config_count);
	}

	ERR_FAIL_COND_V(eglGetError() != EGL_SUCCESS, ERR_BUG);

	ERR_FAIL_COND_V(config_count == 0, ERR_UNCONFIGURED);

	EGLint context_attribs[] = {
		EGL_CONTEXT_MAJOR_VERSION, 3,
		EGL_CONTEXT_MINOR_VERSION, 3,
		EGL_NONE
	};

	p_gldisplay.egl_context = eglCreateContext(p_gldisplay.egl_display, p_gldisplay.egl_config, EGL_NO_CONTEXT, context_attribs);
	ERR_FAIL_COND_V_MSG(p_gldisplay.egl_context == EGL_NO_CONTEXT, ERR_CANT_CREATE, vformat("Can't create an EGL context. Error code: %d", eglGetError()));

	return OK;
}

int EGLManager::display_get_native_visual_id(void *p_display) {
	int gldisplay_id = _get_gldisplay_id(p_display);
	ERR_FAIL_COND_V(gldisplay_id < 0, ERR_CANT_CREATE);

	GLDisplay gldisplay = displays[gldisplay_id];

	EGLint native_visual_id = -1;

	if (!eglGetConfigAttrib(gldisplay.egl_display, gldisplay.egl_config, EGL_NATIVE_VISUAL_ID, &native_visual_id)) {
		ERR_FAIL_V(-1);
	}

	return native_visual_id;
}

Error EGLManager::window_create(DisplayServer::WindowID p_window_id, void *p_display, void *p_native_window, int p_width, int p_height) {
	int gldisplay_id = _get_gldisplay_id(p_display);
	ERR_FAIL_COND_V(gldisplay_id < 0, ERR_CANT_CREATE);

	GLDisplay &gldisplay = displays[gldisplay_id];

	// In order to ensure a fast lookup, make sure we got enough elements in the
	// windows local vector to use the window id as an index.
	if (p_window_id >= (int)windows.size()) {
		windows.resize(p_window_id + 1);
	}

	GLWindow &glwindow = windows[p_window_id];
	glwindow.gldisplay_id = gldisplay_id;

	glwindow.egl_surface = eglCreatePlatformWindowSurface(gldisplay.egl_display, gldisplay.egl_config, p_native_window, NULL);

	if (glwindow.egl_surface == EGL_NO_SURFACE) {
		return ERR_CANT_CREATE;
	}

	glwindow.initialized = true;

	window_make_current(p_window_id);

	return OK;
}

void EGLManager::window_destroy(DisplayServer::WindowID p_window_id) {
	ERR_FAIL_INDEX(p_window_id, (int)windows.size());

	GLWindow &glwindow = windows[p_window_id];

	if (!glwindow.initialized) {
		return;
	}

	glwindow.initialized = false;

	ERR_FAIL_INDEX(glwindow.gldisplay_id, (int)displays.size());
	GLDisplay &gldisplay = displays[glwindow.gldisplay_id];

	if (glwindow.egl_surface != EGL_NO_SURFACE) {
		eglDestroySurface(gldisplay.egl_display, glwindow.egl_surface);
		glwindow.egl_surface = nullptr;
	}
}

void EGLManager::release_current() {
	if (!current_window) {
		return;
	}

	GLDisplay &current_display = displays[current_window->gldisplay_id];

	eglMakeCurrent(current_display.egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

void EGLManager::make_current() {
	if (!current_window) {
		return;
	}

	if (!current_window->initialized) {
		WARN_PRINT("Current OpenGL window is uninitialized!");
		return;
	}

	GLDisplay &current_display = displays[current_window->gldisplay_id];

	eglMakeCurrent(current_display.egl_display, current_window->egl_surface, current_window->egl_surface, current_display.egl_context);
}

void EGLManager::swap_buffers() {
	if (!current_window) {
		return;
	}

	if (!current_window->initialized) {
		WARN_PRINT("Current OpenGL window is uninitialized!");
		return;
	}

	GLDisplay &current_display = displays[current_window->gldisplay_id];

	eglSwapBuffers(current_display.egl_display, current_window->egl_surface);
}

void EGLManager::window_make_current(DisplayServer::WindowID p_window_id) {
	if (p_window_id == DisplayServer::INVALID_WINDOW_ID) {
		return;
	}

	GLWindow &glwindow = windows[p_window_id];

	if (&glwindow == current_window || !glwindow.initialized) {
		return;
	}

	current_window = &glwindow;

	GLDisplay &current_display = displays[current_window->gldisplay_id];

	eglMakeCurrent(current_display.egl_display, current_window->egl_surface, current_window->egl_surface, current_display.egl_context);
}

void EGLManager::set_use_vsync(bool p_use) {
	// Force vsync in the editor for now, as a safety measure.
	bool is_editor = Engine::get_singleton()->is_editor_hint();
	if (is_editor) {
		p_use = true;
	}

	// We need an active window to get a display to set the vsync.
	if (!current_window) {
		return;
	}

	GLDisplay &disp = displays[current_window->gldisplay_id];

	// TODO: Warn when vsync isn't available.

	int swap_interval = p_use ? 1 : 0;

	eglSwapInterval(disp.egl_display, swap_interval);

	use_vsync = p_use;
}

bool EGLManager::is_using_vsync() const {
	return use_vsync;
}

Error EGLManager::initialize() {
	String extensions_string = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
	// The above method should always work. If it doesn't, something's very wrong.
	ERR_FAIL_COND_V(eglGetError() != EGL_SUCCESS, ERR_BUG);

	const char *platform = _get_platform_extension_name();
	if (extensions_string.split(" ").find(platform) < 0) {
		ERR_FAIL_V_MSG(ERR_UNAVAILABLE, vformat("EGL platform extension \"%s\" not found.", platform));
	}

	return OK;
}

EGLManager::EGLManager() {
}

EGLManager::~EGLManager() {
	for (unsigned int i = 0; i < displays.size(); i++) {
		eglTerminate(displays[i].egl_display);
	}
}

#endif // EGL_ENABLED
