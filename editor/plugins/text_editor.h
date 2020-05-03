/*************************************************************************/
/*  text_editor.h                                                        */
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

#ifndef TEXT_EDITOR_H
#define TEXT_EDITOR_H

#include "script_editor_plugin.h"

class TextEditor : public ScriptEditorBase {
	GDCLASS(TextEditor, ScriptEditorBase);

private:
	CodeTextEditor *code_editor;

	Ref<TextFile> text_file;

	HBoxContainer *edit_hb;
	MenuButton *edit_menu;
	PopupMenu *highlighter_menu;
	MenuButton *search_menu;
	PopupMenu *bookmarks_menu;
	PopupMenu *context_menu;

	GotoLineDialog *goto_line_dialog;

	enum {
		EDIT_UNDO,
		EDIT_REDO,
		EDIT_CUT,
		EDIT_COPY,
		EDIT_PASTE,
		EDIT_SELECT_ALL,
		EDIT_TRIM_TRAILING_WHITESAPCE,
		EDIT_CONVERT_INDENT_TO_SPACES,
		EDIT_CONVERT_INDENT_TO_TABS,
		EDIT_MOVE_LINE_UP,
		EDIT_MOVE_LINE_DOWN,
		EDIT_INDENT_RIGHT,
		EDIT_INDENT_LEFT,
		EDIT_DELETE_LINE,
		EDIT_CLONE_DOWN,
		EDIT_TO_UPPERCASE,
		EDIT_TO_LOWERCASE,
		EDIT_CAPITALIZE,
		EDIT_TOGGLE_FOLD_LINE,
		EDIT_FOLD_ALL_LINES,
		EDIT_UNFOLD_ALL_LINES,
		SEARCH_FIND,
		SEARCH_FIND_NEXT,
		SEARCH_FIND_PREV,
		SEARCH_REPLACE,
		SEARCH_IN_FILES,
		REPLACE_IN_FILES,
		SEARCH_GOTO_LINE,
		BOOKMARK_TOGGLE,
		BOOKMARK_GOTO_NEXT,
		BOOKMARK_GOTO_PREV,
		BOOKMARK_REMOVE_ALL,
	};

protected:
	static void _bind_methods();

	void _notification(int p_what);

	void _edit_option(int p_op);
	void _make_context_menu(bool p_selection, bool p_can_fold, bool p_is_folded, Vector2 p_position);
	void _text_edit_gui_input(const Ref<InputEvent> &ev);

	Map<String, Ref<EditorSyntaxHighlighter>> highlighters;
	void _change_syntax_highlighter(int p_idx);
	void _load_theme_settings();

	void _convert_case(CodeTextEditor::CaseStyle p_case);

	void _validate_script();

	void _update_bookmark_list();
	void _bookmark_item_pressed(int p_idx);

public:
	virtual void add_syntax_highlighter(Ref<EditorSyntaxHighlighter> p_highlighter) override;
	virtual void set_syntax_highlighter(Ref<EditorSyntaxHighlighter> p_highlighter) override;

	virtual String get_name() override;
	virtual Ref<Texture2D> get_theme_icon() override;
	virtual RES get_edited_resource() const override;
	virtual void set_edited_resource(const RES &p_res) override;
	void set_edited_file(const Ref<TextFile> &p_file);
	virtual void reload_text() override;
	virtual void apply_code() override;
	virtual bool is_unsaved() override;
	virtual Variant get_edit_state() override;
	virtual void set_edit_state(const Variant &p_state) override;
	virtual Vector<String> get_functions() override;
	virtual void get_breakpoints(List<int> *p_breakpoints) override;
	virtual void goto_line(int p_line, bool p_with_error = false) override;
	void goto_line_selection(int p_line, int p_begin, int p_end);
	virtual void set_executing_line(int p_line) override;
	virtual void clear_executing_line() override;
	virtual void trim_trailing_whitespace() override;
	virtual void insert_final_newline() override;
	virtual void convert_indent_to_spaces() override;
	virtual void convert_indent_to_tabs() override;
	virtual void ensure_focus() override;
	virtual void tag_saved_version() override;
	virtual void update_settings() override;
	virtual bool show_members_overview() override;
	virtual bool can_lose_focus_on_node_selection() override { return true; }
	virtual void set_debugger_active(bool p_active) override;
	virtual void set_tooltip_request_func(String p_method, Object *p_obj) override;
	virtual void add_callback(const String &p_function, PackedStringArray p_args) override;

	virtual Control *get_edit_menu() override;
	virtual void clear_edit_menu() override;

	virtual void validate() override;

	static void register_editor();

	TextEditor();
	~TextEditor();
};

#endif // TEXT_EDITOR_H
