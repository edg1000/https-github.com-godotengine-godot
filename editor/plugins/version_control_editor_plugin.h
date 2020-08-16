/*************************************************************************/
/*  version_control_editor_plugin.h                                      */
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

#ifndef VERSION_CONTROL_EDITOR_PLUGIN_H
#define VERSION_CONTROL_EDITOR_PLUGIN_H

#include "editor/editor_plugin.h"
#include "editor/editor_vcs_interface.h"
#include "scene/gui/container.h"
#include "scene/gui/menu_button.h"
#include "scene/gui/rich_text_label.h"
#include "scene/gui/tab_container.h"
#include "scene/gui/text_edit.h"
#include "scene/gui/tree.h"

class VersionControlEditorPlugin : public EditorPlugin {
	GDCLASS(VersionControlEditorPlugin, EditorPlugin)

public:
	enum ButtonType {
		BUTTON_TYPE_OPEN = 0,
		BUTTON_TYPE_DISCARD = 1,
	};

private:
	static VersionControlEditorPlugin *singleton;

	List<StringName> available_addons;

	PopupMenu *version_control_actions;
	AcceptDialog *set_up_dialog;
	OptionButton *set_up_choice;
	PanelContainer *set_up_init_settings;
	Button *set_up_init_button;
	LineEdit *set_up_username;
	LineEdit *set_up_password;

	HashMap<EditorVCSInterface::ChangeType, String> change_type_to_strings;
	HashMap<EditorVCSInterface::ChangeType, Color> change_type_to_color;
	HashMap<EditorVCSInterface::ChangeType, Ref<Texture>> change_type_to_icon;

	TabContainer *dock_vbc;
	VBoxContainer *version_commit_dock;
	Tree *staged_files;
	Tree *unstaged_files;
	Tree *commit_list;

	OptionButton *branch_select;

	Button *fetch_button;
	Button *pull_button;
	Button *push_button;

	Button *stage_all_button;
	Button *unstage_all_button;
	Button *discard_all_button;
	Button *refresh_button;
	TextEdit *commit_message;
	Button *commit_button;

	PanelContainer *version_control_dock;
	Button *version_control_dock_button;
	VBoxContainer *diff_vbc;
	HBoxContainer *diff_hbc;
	Button *diff_refresh_button;
	Label *diff_file_name;
	Label *diff_heading;
	RichTextLabel *diff;

	void _initialize_vcs();
	void _selected_a_vcs(int p_id);
	void _populate_available_vcs_names();

	void _pull();
	void _push();
	void _fetch();
	void _commit();
	void _discard_all();
	void _refresh_stage_area();
	void _refresh_branch_list();
	void _refresh_commit_list();
	void _move_all(Object *p_tree);
	int _get_item_count(Tree *p_tree);
	void _view_file_diff(Object *p_tree);
	void _item_activated(Object *p_tree);
	void _branch_item_selected(int index);
	void _move_item(Tree *p_tree, TreeItem *p_itme);
	void _discard_file(String p_file_path, EditorVCSInterface::ChangeType change);
	void _cell_button_pressed(Object *p_item, int column, int id);
	void _add_new_item(Tree *p_tree, String p_file_path, EditorVCSInterface::ChangeType change);

	void _clear_file_diff();
	void _refresh_file_diff();
	void _display_file_diff(String p_file_path);

	friend class EditorVCSInterface;

protected:
	static void _bind_methods();

public:
	static VersionControlEditorPlugin *get_singleton();

	void popup_vcs_set_up_dialog(const Control *p_gui_base);
	void set_version_control_tool_button(Button *p_button) { version_control_dock_button = p_button; }

	PopupMenu *get_version_control_actions_panel() const { return version_control_actions; }
	VBoxContainer *get_version_commit_dock() const { return version_commit_dock; }
	PanelContainer *get_version_control_dock() const { return version_control_dock; }

	List<StringName> get_available_vcs_names() const { return available_addons; }
	bool is_vcs_initialized() const;
	const String get_vcs_name() const;

	void register_editor();
	void fetch_available_vcs_addon_names();
	void shut_down();

	VersionControlEditorPlugin();
	~VersionControlEditorPlugin();
};

#endif // !VERSION_CONTROL_EDITOR_PLUGIN_H
