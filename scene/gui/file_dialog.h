/*************************************************************************/
/*  file_dialog.h                                                        */
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

#ifndef FILE_DIALOG_H
#define FILE_DIALOG_H

#include "box_container.h"
#include "core/io/dir_access.h"
#include "core/object/object.h"
#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/templates/vector.h"
#include "core/variant/binder_common.h"
#include "scene/gui/dialogs.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/option_button.h"
#include "scene/gui/tree.h"

class Button;
class DirAccess;
class HBoxContainer;
class InputEvent;
class Label;
class LineEdit;
class OptionButton;
class Texture2D;
class Tree;
class VBoxContainer;

class FileDialog : public ConfirmationDialog {
	GDCLASS(FileDialog, ConfirmationDialog);

public:
	enum Access {
		ACCESS_RESOURCES,
		ACCESS_USERDATA,
		ACCESS_FILESYSTEM
	};

	enum FileMode {
		FILE_MODE_OPEN_FILE,
		FILE_MODE_OPEN_FILES,
		FILE_MODE_OPEN_DIR,
		FILE_MODE_OPEN_ANY,
		FILE_MODE_SAVE_FILE
	};

	typedef Ref<Texture2D> (*GetIconFunc)(const String &);
	typedef void (*RegisterFunc)(FileDialog *);

	static GetIconFunc get_icon_func;
	static GetIconFunc get_large_icon_func;
	static RegisterFunc register_func;
	static RegisterFunc unregister_func;

private:
	ConfirmationDialog *makedialog;
	LineEdit *makedirname;

	Button *makedir;
	Access access = ACCESS_RESOURCES;
	//Button *action;
	VBoxContainer *vbox;
	FileMode mode;
	LineEdit *dir;
	HBoxContainer *drives_container;
	HBoxContainer *shortcuts_container;
	OptionButton *drives;
	Tree *tree;
	HBoxContainer *file_box;
	LineEdit *file;
	OptionButton *filter;
	AcceptDialog *mkdirerr;
	AcceptDialog *exterr;
	DirAccess *dir_access;
	ConfirmationDialog *confirm_save;

	Label *message;

	Button *dir_prev;
	Button *dir_next;
	Button *dir_up;

	Button *refresh;
	Button *show_hidden;

	Vector<String> filters;

	Vector<String> local_history;
	int local_history_pos = 0;
	void _push_history();

	bool mode_overrides_title = true;

	static bool default_show_hidden_files;
	bool show_hidden_files = false;

	bool invalidated = true;

	void update_dir();
	void update_file_name();
	void update_file_list();
	void update_filters();

	void _focus_file_text();

	void _tree_multi_selected(Object *p_object, int p_cell, bool p_selected);
	void _tree_selected();

	void _select_drive(int p_idx);
	void _tree_item_activated();
	void _dir_submitted(String p_dir);
	void _file_submitted(const String &p_file);
	void _action_pressed();
	void _save_confirm_pressed();
	void _cancel_pressed();
	void _filter_selected(int);
	void _make_dir();
	void _make_dir_confirm();
	void _go_up();
	void _go_back();
	void _go_forward();

	void _update_drives(bool p_select = true);

	virtual void unhandled_input(const Ref<InputEvent> &p_event) override;

	bool _is_open_should_be_disabled();

	virtual void _post_popup() override;

protected:
	void _theme_changed();

	void _notification(int p_what);
	static void _bind_methods();
	//bind helpers
public:
	void popup_file_dialog();
	void clear_filters();
	void add_filter(const String &p_filter);
	void set_filters(const Vector<String> &p_filters);
	Vector<String> get_filters() const;

	void set_enable_multiple_selection(bool p_enable);
	Vector<String> get_selected_files() const;

	String get_current_dir() const;
	String get_current_file() const;
	String get_current_path() const;
	void set_current_dir(const String &p_dir);
	void set_current_file(const String &p_file);
	void set_current_path(const String &p_path);

	void set_mode_overrides_title(bool p_override);
	bool is_mode_overriding_title() const;

	void set_file_mode(FileMode p_mode);
	FileMode get_file_mode() const;

	VBoxContainer *get_vbox();
	LineEdit *get_line_edit() { return file; }

	void set_access(Access p_access);
	Access get_access() const;

	void set_show_hidden_files(bool p_show);
	bool is_showing_hidden_files() const;

	static void set_default_show_hidden_files(bool p_show);

	void invalidate();

	void deselect_all();

	FileDialog();
	~FileDialog();
};

VARIANT_ENUM_CAST(FileDialog::FileMode);
VARIANT_ENUM_CAST(FileDialog::Access);

#endif
