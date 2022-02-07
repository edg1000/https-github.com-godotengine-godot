/*************************************************************************/
/*  import_dock.h                                                        */
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

#ifndef IMPORTDOCK_H
#define IMPORTDOCK_H

#include "core/io/config_file.h"
#include "core/io/resource_importer.h"
#include "core/object/object.h"
#include "core/object/ref_counted.h"
#include "core/string/string_name.h"
#include "core/string/ustring.h"
#include "core/templates/list.h"
#include "core/templates/map.h"
#include "core/templates/vector.h"
#include "core/variant/variant.h"
#include "editor/editor_file_system.h"
#include "editor/editor_inspector.h"
#include "scene/gui/box_container.h"
#include "scene/gui/dialogs.h"
#include "scene/gui/menu_button.h"
#include "scene/gui/option_button.h"
#include "scene/gui/popup_menu.h"

class ImportDockParameters;
class Button;
class ConfirmationDialog;
class Control;
class EditorInspector;
class Label;
class MenuButton;
class OptionButton;

class ImportDock : public VBoxContainer {
	GDCLASS(ImportDock, VBoxContainer);

	Label *imported;
	OptionButton *import_as;
	MenuButton *preset;
	EditorInspector *import_opts;

	List<PropertyInfo> properties;
	Map<StringName, Variant> property_values;

	ConfirmationDialog *reimport_confirm;
	Label *label_warning;
	Button *import;

	Control *advanced_spacer;
	Button *advanced;

	ImportDockParameters *params;

	VBoxContainer *content;
	Label *select_a_resource;

	void _preset_selected(int p_idx);
	void _importer_selected(int i_idx);
	void _update_options(const String &p_path, const Ref<ConfigFile> &p_config = Ref<ConfigFile>());
	void _update_preset_menu();
	void _add_keep_import_option(const String &p_importer_name);

	void _property_edited(const StringName &p_prop);
	void _property_toggled(const StringName &p_prop, bool p_checked);
	void _set_dirty(bool p_dirty);
	void _reimport_attempt();
	void _reimport_and_restart();
	void _reimport();

	void _advanced_options();
	enum {
		ITEM_SET_AS_DEFAULT = 100,
		ITEM_LOAD_DEFAULT,
		ITEM_CLEAR_DEFAULT,
	};

private:
	static ImportDock *singleton;

public:
	static ImportDock *get_singleton() { return singleton; }

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void set_edit_path(const String &p_path);
	void set_edit_multiple_paths(const Vector<String> &p_paths);
	void initialize_import_options() const;
	void clear();

	ImportDock();
	~ImportDock();
};

#endif // IMPORTDOCK_H
