/*************************************************************************/
/*  filesystem_dock.h                                                    */
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

#ifndef FILESYSTEM_DOCK_H
#define FILESYSTEM_DOCK_H

#include "editor/create_dialog.h"
#include "editor/dependency_editor.h"
#include "editor/editor_dir_dialog.h"
#include "editor/editor_file_system.h"
#include "editor/plugins/script_editor_plugin.h"
#include "editor/script_create_dialog.h"
#include "scene/gui/box_container.h"
#include "scene/gui/control.h"
#include "scene/gui/dialogs.h"
#include "scene/gui/item_list.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/menu_button.h"
#include "scene/gui/progress_bar.h"
#include "scene/gui/split_container.h"
#include "scene/gui/tree.h"

class SceneCreateDialog;
class ShaderCreateDialog;

class FileSystemDock : public VBoxContainer {
	GDCLASS(FileSystemDock, VBoxContainer);

public:
	enum FileListDisplayMode {
		FILE_LIST_DISPLAY_THUMBNAILS,
		FILE_LIST_DISPLAY_LIST
	};

	enum DisplayMode {
		DISPLAY_MODE_TREE_ONLY,
		DISPLAY_MODE_SPLIT,
	};

	enum FileSortOption {
		FILE_SORT_NAME = 0,
		FILE_SORT_NAME_REVERSE,
		FILE_SORT_TYPE,
		FILE_SORT_TYPE_REVERSE,
		FILE_SORT_MODIFIED_TIME,
		FILE_SORT_MODIFIED_TIME_REVERSE,
		FILE_SORT_MAX,
	};

private:
	enum FileMenu {
		FILE_OPEN,
		FILE_INHERIT,
		FILE_MAIN_SCENE,
		FILE_INSTANCE,
		FILE_ADD_FAVORITE,
		FILE_REMOVE_FAVORITE,
		FILE_DEPENDENCIES,
		FILE_OWNERS,
		FILE_MOVE,
		FILE_RENAME,
		FILE_REMOVE,
		FILE_DUPLICATE,
		FILE_REIMPORT,
		FILE_INFO,
		FILE_NEW_FOLDER,
		FILE_NEW_SCRIPT,
		FILE_NEW_SCENE,
		FILE_SHOW_IN_EXPLORER,
		FILE_COPY_PATH,
		FILE_COPY_UID,
		FILE_NEW_RESOURCE,
		FILE_NEW_TEXTFILE,
		FOLDER_EXPAND_ALL,
		FOLDER_COLLAPSE_ALL,
	};

	FileSortOption file_sort = FILE_SORT_NAME;

	VBoxContainer *scanning_vb = nullptr;
	ProgressBar *scanning_progress = nullptr;
	VSplitContainer *split_box = nullptr;
	VBoxContainer *file_list_vb = nullptr;

	HashSet<String> favorites;

	Button *button_toggle_display_mode = nullptr;
	Button *button_reload = nullptr;
	Button *button_file_list_display_mode = nullptr;
	Button *button_hist_next = nullptr;
	Button *button_hist_prev = nullptr;
	LineEdit *current_path = nullptr;

	HBoxContainer *toolbar2_hbc = nullptr;
	LineEdit *tree_search_box = nullptr;
	MenuButton *tree_button_sort = nullptr;

	LineEdit *file_list_search_box = nullptr;
	MenuButton *file_list_button_sort = nullptr;

	String searched_string;
	Vector<String> uncollapsed_paths_before_search;

	TextureRect *search_icon = nullptr;
	HBoxContainer *path_hb = nullptr;

	FileListDisplayMode file_list_display_mode;
	DisplayMode display_mode;
	DisplayMode old_display_mode;

	PopupMenu *file_list_popup = nullptr;
	PopupMenu *tree_popup = nullptr;

	DependencyEditor *deps_editor = nullptr;
	DependencyEditorOwners *owners_editor = nullptr;
	DependencyRemoveDialog *remove_dialog = nullptr;

	EditorDirDialog *move_dialog = nullptr;
	ConfirmationDialog *rename_dialog = nullptr;
	LineEdit *rename_dialog_text = nullptr;
	ConfirmationDialog *duplicate_dialog = nullptr;
	LineEdit *duplicate_dialog_text = nullptr;
	ConfirmationDialog *make_dir_dialog = nullptr;
	LineEdit *make_dir_dialog_text = nullptr;
	ConfirmationDialog *overwrite_dialog = nullptr;
	SceneCreateDialog *make_scene_dialog = nullptr;
	ScriptCreateDialog *make_script_dialog = nullptr;
	ShaderCreateDialog *make_shader_dialog = nullptr;
	CreateDialog *new_resource_dialog = nullptr;

	bool always_show_folders = false;

	class FileOrFolder {
	public:
		String path;
		bool is_file = false;

		FileOrFolder() {}
		FileOrFolder(const String &p_path, bool p_is_file) :
				path(p_path),
				is_file(p_is_file) {}
	};
	FileOrFolder to_rename;
	FileOrFolder to_duplicate;
	Vector<FileOrFolder> to_move;
	String to_move_path;

	Vector<String> history;
	int history_pos;
	int history_max_size;

	String path;

	bool initialized = false;

	bool updating_tree = false;
	int tree_update_id;
	Tree *tree = nullptr;
	ItemList *files = nullptr;
	bool import_dock_needs_update = false;

	bool holding_branch = false;
	Vector<TreeItem *> tree_items_selected_on_drag_begin;
	PackedInt32Array list_items_selected_on_drag_begin;

	void _tree_mouse_exited();
	void _reselect_items_selected_on_drag_begin(bool reset = false);

	Ref<Texture2D> _get_tree_item_icon(bool p_is_valid, String p_file_type);
	bool _create_tree(TreeItem *p_parent, EditorFileSystemDirectory *p_dir, Vector<String> &uncollapsed_paths, bool p_select_in_favorites, bool p_unfold_path = false);
	Vector<String> _compute_uncollapsed_paths();
	void _update_tree(const Vector<String> &p_uncollapsed_paths = Vector<String>(), bool p_uncollapse_root = false, bool p_select_in_favorites = false, bool p_unfold_path = false);
	void _navigate_to_path(const String &p_path, bool p_select_in_favorites = false);

	void _file_list_gui_input(Ref<InputEvent> p_event);
	void _tree_gui_input(Ref<InputEvent> p_event);

	void _update_file_list(bool p_keep_selection);
	void _toggle_file_display();
	void _set_file_display(bool p_active);
	void _fs_changed();

	void _select_file(const String &p_path, bool p_select_in_favorites = false);
	void _tree_activate_file();
	void _file_list_activate_file(int p_idx);
	void _file_multi_selected(int p_index, bool p_selected);
	void _tree_multi_selected(Object *p_item, int p_column, bool p_selected);

	void _get_imported_files(const String &p_path, Vector<String> &r_files) const;
	void _update_import_dock();

	void _get_all_items_in_dir(EditorFileSystemDirectory *p_efsd, Vector<String> &r_files, Vector<String> &r_folders) const;
	void _find_remaps(EditorFileSystemDirectory *p_efsd, const HashMap<String, String> &r_renames, Vector<String> &r_to_remaps) const;
	void _try_move_item(const FileOrFolder &p_item, const String &p_new_path, HashMap<String, String> &p_file_renames, HashMap<String, String> &p_folder_renames);
	void _try_duplicate_item(const FileOrFolder &p_item, const String &p_new_path) const;
	void _update_dependencies_after_move(const HashMap<String, String> &p_renames) const;
	void _update_resource_paths_after_move(const HashMap<String, String> &p_renames) const;
	void _save_scenes_after_move(const HashMap<String, String> &p_renames) const;
	void _update_favorites_list_after_move(const HashMap<String, String> &p_files_renames, const HashMap<String, String> &p_folders_renames) const;
	void _update_project_settings_after_move(const HashMap<String, String> &p_renames) const;

	void _file_removed(String p_file);
	void _folder_removed(String p_folder);

	void _resource_created();
	void _make_dir_confirm();
	void _make_scene_confirm();
	void _rename_operation_confirm();
	void _duplicate_operation_confirm();
	void _move_with_overwrite();
	Vector<String> _check_existing();
	void _move_operation_confirm(const String &p_to_path, bool p_overwrite = false);

	void _tree_rmb_option(int p_option);
	void _file_list_rmb_option(int p_option);
	void _file_option(int p_option, const Vector<String> &p_selected);

	void _fw_history();
	void _bw_history();
	void _update_history();
	void _push_to_history();

	void _set_scanning_mode();
	void _rescan();

	void _toggle_split_mode(bool p_active);

	void _search_changed(const String &p_text, const Control *p_from);

	MenuButton *_create_file_menu_button();
	void _file_sort_popup(int p_id);

	void _file_and_folders_fill_popup(PopupMenu *p_popup, Vector<String> p_paths, bool p_display_path_dependent_options = true);
	void _tree_rmb_select(const Vector2 &p_pos, MouseButton p_button);
	void _file_list_item_clicked(int p_item, const Vector2 &p_pos, MouseButton p_mouse_button_index);
	void _file_list_empty_clicked(const Vector2 &p_pos, MouseButton p_mouse_button_index);
	void _tree_empty_click(const Vector2 &p_pos, MouseButton p_button);
	void _tree_empty_selected();

	struct FileInfo {
		String name;
		String path;
		StringName type;
		Vector<String> sources;
		bool import_broken = false;
		uint64_t modified_time = 0;

		bool operator<(const FileInfo &fi) const {
			return NaturalNoCaseComparator()(name, fi.name);
		}
	};

	struct FileInfoTypeComparator;
	struct FileInfoModifiedTimeComparator;

	void _sort_file_info_list(List<FileSystemDock::FileInfo> &r_file_list);

	void _search(EditorFileSystemDirectory *p_path, List<FileInfo> *matches, int p_max_items);

	void _set_current_path_text(const String &p_path);

	Variant get_drag_data_fw(const Point2 &p_point, Control *p_from);
	bool can_drop_data_fw(const Point2 &p_point, const Variant &p_data, Control *p_from) const;
	void drop_data_fw(const Point2 &p_point, const Variant &p_data, Control *p_from);
	void _get_drag_target_folder(String &target, bool &target_favorites, const Point2 &p_point, Control *p_from) const;

	void _preview_invalidated(const String &p_path);
	void _file_list_thumbnail_done(const String &p_path, const Ref<Texture2D> &p_preview, const Ref<Texture2D> &p_small_preview, const Variant &p_udata);
	void _tree_thumbnail_done(const String &p_path, const Ref<Texture2D> &p_preview, const Ref<Texture2D> &p_small_preview, const Variant &p_udata);

	void _update_display_mode(bool p_force = false);

	Vector<String> _tree_get_selected(bool remove_self_inclusion = true) const;

	bool _is_file_type_disabled_by_feature_profile(const StringName &p_class);

	void _feature_profile_changed();
	static Vector<String> _remove_self_included_paths(Vector<String> selected_strings);

private:
	static FileSystemDock *singleton;

public:
	static FileSystemDock *get_singleton() { return singleton; }

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	Vector<String> get_selected_paths() const;

	String get_current_path() const;
	String get_current_directory() const;

	void navigate_to_path(const String &p_path);
	void focus_on_filter();

	ScriptCreateDialog *get_script_create_dialog() const;

	void fix_dependencies(const String &p_for_file);

	int get_split_offset() { return split_box->get_split_offset(); }
	void set_split_offset(int p_offset) { split_box->set_split_offset(p_offset); }
	void select_file(const String &p_file);

	void set_display_mode(DisplayMode p_display_mode);
	DisplayMode get_display_mode() { return display_mode; }

	void set_file_sort(FileSortOption p_file_sort);
	FileSortOption get_file_sort() { return file_sort; }

	void set_file_list_display_mode(FileListDisplayMode p_mode);
	FileListDisplayMode get_file_list_display_mode() { return file_list_display_mode; };

	FileSystemDock();
	~FileSystemDock();
};

#endif // FILESYSTEM_DOCK_H
