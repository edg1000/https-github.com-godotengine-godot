/*************************************************************************/
/*  os_tvos.h                                                            */
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

#ifndef OS_TVOS_H
#define OS_TVOS_H

#include "platform/uikit/uikit_os.h"

#include "drivers/coreaudio/audio_driver_coreaudio.h"
#include "drivers/unix/os_unix.h"
#include "servers/audio_server.h"
#include "servers/rendering/renderer_compositor.h"
#include "tvos.h"

#if defined(VULKAN_ENABLED)
#include "drivers/vulkan/rendering_device_vulkan.h"
#include "platform/uikit/uikit_vulkan_context.h"
#endif

class OSAppleTV : public OS_UIKit {
private:
	static HashMap<String, void *> dynamic_symbol_lookup_table;
	friend void register_dynamic_symbol(char *name, void *address);

	AudioDriverCoreAudio audio_driver;

	tvOS *tvos;

	virtual void initialize() override;

	virtual void finalize() override;

	bool is_focused = false;

	bool overrides_menu_button = true;

public:
	static OSAppleTV *get_singleton();

	OSAppleTV(String p_data_dir, String p_cache_dir);
	~OSAppleTV();

	virtual void alert(const String &p_alert, const String &p_title = "ALERT!") override;

	virtual Error get_dynamic_library_symbol_handle(void *p_library_handle, const String p_name, void *&p_symbol_handle, bool p_optional = false) override;

	virtual String get_name() const override;
	virtual String get_model_name() const override;

	virtual bool _check_internal_feature_support(const String &p_feature) override;

	void on_focus_out();
	void on_focus_in();

	bool get_overrides_menu_button() const;
	void set_overrides_menu_button(bool p_flag);

	void initialize_modules();
};

#endif // OS_TVOS_H
