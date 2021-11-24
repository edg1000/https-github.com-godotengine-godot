/*************************************************************************/
/*  shader_editor_plugin.cpp                                             */
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

#include "shader_editor_plugin.h"

#include "core/io/resource_loader.h"
#include "core/io/resource_saver.h"
#include "core/os/keyboard.h"
#include "core/os/os.h"
#include "core/version_generated.gen.h"
#include "editor/editor_node.h"
#include "editor/editor_scale.h"
#include "editor/editor_settings.h"
#include "editor/project_settings_editor.h"
#include "editor/property_editor.h"
#include "servers/display_server.h"
#include "servers/rendering/shader_preprocessor.h"
#include "servers/rendering/shader_types.h"

/*** SHADER SCRIPT EDITOR ****/

static bool saved_warnings_enabled = false;
static bool saved_treat_warning_as_errors = false;
static Map<ShaderWarning::Code, bool> saved_warnings;
static uint32_t saved_warning_flags = 0U;

Ref<Shader> ShaderTextEditor::get_edited_shader() const {
	return shader;
}

void ShaderTextEditor::set_edited_shader(const Ref<Shader> &p_shader) {
	set_edited_shader(p_shader, p_shader->get_code());
}

void ShaderTextEditor::set_edited_shader(const Ref<Shader> &p_shader, String code) {
	if (shader == p_shader) {
		return;
	}
	shader = p_shader;

	_load_theme_settings();

	get_text_editor()->set_text(code);
	get_text_editor()->clear_undo_history();
	get_text_editor()->call_deferred(SNAME("set_h_scroll"), 0);
	get_text_editor()->call_deferred(SNAME("set_v_scroll"), 0);

	_validate_script();
	_line_col_changed();
}

void ShaderTextEditor::reload_text() {
	ERR_FAIL_COND(shader.is_null());

	CodeEdit *te = get_text_editor();
	int column = te->get_caret_column();
	int row = te->get_caret_line();
	int h = te->get_h_scroll();
	int v = te->get_v_scroll();

	te->set_text(shader->get_code());
	te->set_caret_line(row);
	te->set_caret_column(column);
	te->set_h_scroll(h);
	te->set_v_scroll(v);

	te->tag_saved_version();

	update_line_and_column();
}

void ShaderTextEditor::set_warnings_panel(RichTextLabel *p_warnings_panel) {
	warnings_panel = p_warnings_panel;
}

void ShaderTextEditor::set_shader_editor(ShaderEditor *p_editor) {
	shader_editor = p_editor;
}

void ShaderTextEditor::set_shader_dependency_tree(Tree *p_tree) {
	shader_dependency_tree = p_tree;
}

void ShaderTextEditor::_clear_tree_item_backgrounds(TreeItem *p_node) {
	Array tree_children = p_node->get_children();
	for (int i = 0; i < tree_children.size(); i++) {
		auto child = tree_children[i];
		if (child.get_type() == Variant::Type::OBJECT) {
			Object *obj = child.get_validated_object();
			TreeItem *itemChild = obj->cast_to<TreeItem>(obj);
			itemChild->clear_custom_bg_color(0);

			_clear_tree_item_backgrounds(itemChild);
		}
	}
}

void ShaderTextEditor::_load_theme_settings() {
	CodeEdit *text_editor = get_text_editor();
	Color updated_marked_line_color = EDITOR_GET("text_editor/theme/highlighting/mark_color");
	if (updated_marked_line_color != marked_line_color) {
		for (int i = 0; i < text_editor->get_line_count(); i++) {
			if (text_editor->get_line_background_color(i) == marked_line_color) {
				text_editor->set_line_background_color(i, updated_marked_line_color);
			}
		}
		marked_line_color = updated_marked_line_color;
	}

	syntax_highlighter->set_number_color(EDITOR_GET("text_editor/theme/highlighting/number_color"));
	syntax_highlighter->set_symbol_color(EDITOR_GET("text_editor/theme/highlighting/symbol_color"));
	syntax_highlighter->set_function_color(EDITOR_GET("text_editor/theme/highlighting/function_color"));
	syntax_highlighter->set_member_variable_color(EDITOR_GET("text_editor/theme/highlighting/member_variable_color"));

	syntax_highlighter->clear_keyword_colors();

	List<String> keywords;
	ShaderLanguage::get_keyword_list(&keywords);
	const Color keyword_color = EDITOR_GET("text_editor/theme/highlighting/keyword_color");
	const Color control_flow_keyword_color = EDITOR_GET("text_editor/theme/highlighting/control_flow_keyword_color");

	for (const String &E : keywords) {
		if (ShaderLanguage::is_control_flow_keyword(E)) {
			syntax_highlighter->add_keyword_color(E, control_flow_keyword_color);
		} else {
			syntax_highlighter->add_keyword_color(E, keyword_color);
		}
	}

	// Colorize built-ins like `COLOR` differently to make them easier
	// to distinguish from keywords at a quick glance.

	List<String> built_ins;
	if (shader.is_valid()) {
		for (const KeyValue<StringName, ShaderLanguage::FunctionInfo> &E : ShaderTypes::get_singleton()->get_functions(RenderingServer::ShaderMode(shader->get_mode()))) {
			for (const KeyValue<StringName, ShaderLanguage::BuiltInInfo> &F : E.value.built_ins) {
				built_ins.push_back(F.key);
			}
		}

		const Vector<ShaderLanguage::ModeInfo> &modes = ShaderTypes::get_singleton()->get_modes(RenderingServer::ShaderMode(shader->get_mode()));

		for (int i = 0; i < modes.size(); i++) {
			const ShaderLanguage::ModeInfo &info = modes[i];

			if (!info.options.is_empty()) {
				for (int j = 0; j < info.options.size(); j++) {
					built_ins.push_back(String(info.name) + "_" + String(info.options[j]));
				}
			} else {
				built_ins.push_back(String(info.name));
			}
		}
	}

	const Color user_type_color = EDITOR_GET("text_editor/theme/highlighting/user_type_color");

	for (const String &E : built_ins) {
		syntax_highlighter->add_keyword_color(E, user_type_color);
	}

	// Colorize comments.
	const Color comment_color = EDITOR_GET("text_editor/theme/highlighting/comment_color");
	syntax_highlighter->clear_color_regions();
	syntax_highlighter->add_color_region("/*", "*/", comment_color, false);
	syntax_highlighter->add_color_region("//", "", comment_color, true);

	text_editor->clear_comment_delimiters();
	text_editor->add_comment_delimiter("/*", "*/", false);
	text_editor->add_comment_delimiter("//", "", true);

	if (!text_editor->has_auto_brace_completion_open_key("/*")) {
		text_editor->add_auto_brace_completion_pair("/*", "*/");
	}

	// Colorize shader preprocessor directives
	List<String> preprocessor_keywords;
	ShaderPreprocessor::get_keyword_list(&preprocessor_keywords);

	for (List<String>::Element *E = preprocessor_keywords.front(); E; E = E->next()) {
		syntax_highlighter->add_keyword_color(E->get(), keyword_color);
	}

	//colorize preprocessor include strings
	const Color string_color = EDITOR_GET("text_editor/theme/highlighting/string_color");
	syntax_highlighter->add_color_region("\"", "\"", string_color, false);

	if (warnings_panel) {
		// Warnings panel
		warnings_panel->add_theme_font_override("normal_font", EditorNode::get_singleton()->get_gui_base()->get_theme_font(SNAME("main"), SNAME("EditorFonts")));
		warnings_panel->add_theme_font_size_override("normal_font_size", EditorNode::get_singleton()->get_gui_base()->get_theme_font_size(SNAME("main_size"), SNAME("EditorFonts")));
	}
}

void ShaderTextEditor::_check_shader_mode() {
	String type = ShaderLanguage::get_shader_type(get_text_editor()->get_text());

	Shader::Mode mode;

	if (type == "canvas_item") {
		mode = Shader::MODE_CANVAS_ITEM;
	} else if (type == "particles") {
		mode = Shader::MODE_PARTICLES;
	} else if (type == "sky") {
		mode = Shader::MODE_SKY;
	} else if (type == "fog") {
		mode = Shader::MODE_FOG;
	} else {
		mode = Shader::MODE_SPATIAL;
	}

	if (shader->get_mode() != mode) {
		shader->set_code(get_text_editor()->get_text());
		_load_theme_settings();
	}
}

static ShaderLanguage::DataType _get_global_variable_type(const StringName &p_variable) {
	RS::GlobalVariableType gvt = RS::get_singleton()->global_variable_get_type(p_variable);
	return (ShaderLanguage::DataType)RS::global_variable_type_get_shader_datatype(gvt);
}

void ShaderTextEditor::_code_complete_script(const String &p_code, List<ScriptCodeCompletionOption> *r_options) {
	_check_shader_mode();

	ShaderLanguage sl;
	String calltip;

	ShaderLanguage::ShaderCompileInfo info;
	info.functions = ShaderTypes::get_singleton()->get_functions(RenderingServer::ShaderMode(shader->get_mode()));
	info.render_modes = ShaderTypes::get_singleton()->get_modes(RenderingServer::ShaderMode(shader->get_mode()));
	info.shader_types = ShaderTypes::get_singleton()->get_types();
	info.global_variable_type_func = _get_global_variable_type;

	sl.complete(p_code, info, r_options, calltip);

	get_text_editor()->set_code_hint(calltip);
}

void ShaderTextEditor::_validate_script() {
	_check_shader_mode();

	//List<StringName> params;
	//shader->get_param_list(&params);

	ShaderLanguage::ShaderCompileInfo info;
	info.functions = ShaderTypes::get_singleton()->get_functions(RenderingServer::ShaderMode(shader->get_mode()));
	info.render_modes = ShaderTypes::get_singleton()->get_modes(RenderingServer::ShaderMode(shader->get_mode()));
	info.shader_types = ShaderTypes::get_singleton()->get_types();
	info.global_variable_type_func = _get_global_variable_type;

	// force apply current shader.
	String code = get_text_editor()->get_text();
	shader->set_code(code);

	// reset code to parent shader code
	code = shader_editor->get_shader()->get_code();

	ShaderPreprocessor processor(code);
	String processed = processor.preprocess();

	PreprocessorState *state = processor.get_state();
	if (!state->error.is_empty()) {
		// couldn't preprocess, so no sense in validating. Need to feed back issues to user.
		return;
	}

	// shader_editor->shader_dependencies = ShaderDependencyGraph();
	shader_editor->shader_dependencies.populate(shader_editor->get_shader());
	shader_editor->_update_shader_dependency_tree();

	ShaderLanguage sl;

	sl.enable_warning_checking(saved_warnings_enabled);
	sl.set_warning_flags(saved_warning_flags);

	Error err = sl.compile(code, info);

	_clear_tree_item_backgrounds(shader_dependency_tree->get_root());

	if (err != OK) {
		ShaderDependencyNode *context;
		int adjusted_line = sl.get_error_line();
		for (ShaderDependencyNode *node : shader_editor->shader_dependencies.nodes) {
			adjusted_line = node->GetContext(sl.get_error_line(), &context);
			break;
		}

		bool highlight_error = false;
		error_shader_path = shader->get_path();
		if (context) {
			if (!context->path.is_empty()) {
				TreeItem *tree_item = shader_dependency_tree->get_item_with_text(context->path);
				tree_item->set_custom_bg_color(0, marked_line_color);

				error_shader_path = context->path;
			} else {
				highlight_error = true;
			}
		}

		String error_text = "error(" + error_shader_path + ":" + itos(adjusted_line) + "): " + sl.get_error_text();
		set_error(error_text);
		set_error_pos(adjusted_line - 1, 0);

		if (highlight_error) {
			for (int i = 0; i < get_text_editor()->get_line_count(); i++) {
				get_text_editor()->set_line_background_color(i, Color(0, 0, 0, 0));
			}
			get_text_editor()->set_line_background_color(adjusted_line - 1, marked_line_color);
		}
	} else {
		error_shader_path = "";

		for (int i = 0; i < get_text_editor()->get_line_count(); i++) {
			get_text_editor()->set_line_background_color(i, Color(0, 0, 0, 0));
		}
		set_error("");
	}

	// reset intensity and reset for lines that match unevaluated conditional macros
	for (int i = 0; i < get_text_editor()->get_line_count(); i++) {
		get_text_editor()->set_line_font_color_intensity(i, 1.0f);
	}

	if (!state->skipped_conditions.is_empty()) {
		auto val_elem = state->skipped_conditions.find(shader->get_path());
		if (val_elem) {
			for (auto &cond : val_elem->get()) {
				int end_line = cond->end_line;
				if (end_line < 0) {
					// set to end of file
					end_line = get_text_editor()->get_line_count() - cond->start_line;
				}

				for (int i = cond->start_line; i < cond->end_line; i++) {
					// TODO expose unevaluated macro conditional block intensity in editor settings
					get_text_editor()->set_line_font_color_intensity(i, 0.5f);
				}
			}
		}
	}

	if (warnings.size() > 0 || err != OK) {
		warnings_panel->clear();
	}
	warnings.clear();
	for (List<ShaderWarning>::Element *E = sl.get_warnings_ptr(); E; E = E->next()) {
		warnings.push_back(E->get());
	}
	if (warnings.size() > 0 && err == OK) {
		warnings.sort_custom<WarningsComparator>();
		_update_warning_panel();
	} else {
		set_warning_count(0);
	}
	emit_signal(SNAME("script_changed"));
}

void ShaderTextEditor::goto_error() {
	if (!error_shader_path.is_empty()) {
		shader_editor->open_path(error_shader_path);

		int error_line;
		int error_column;
		get_error_pos(error_line, error_column);

		for (int i = 0; i < get_text_editor()->get_line_count(); i++) {
			get_text_editor()->set_line_background_color(i, Color(0, 0, 0, 0));
		}
		get_text_editor()->set_line_background_color(error_line, marked_line_color);
	}

	CodeTextEditor::goto_error();
}

void ShaderTextEditor::_update_warning_panel() {
	int warning_count = 0;

	const Color &warning_color = warnings_panel->get_theme_color(SNAME("warning_color"), SNAME("Editor"));
	warnings_panel->push_table(2);
	for (int i = 0; i < warnings.size(); i++) {
		ShaderWarning &w = warnings[i];

		ShaderDependencyNode *context;
		int adjusted_line = w.get_line();
		for (ShaderDependencyNode *node : shader_editor->shader_dependencies.nodes) {
			adjusted_line = node->GetContext(w.get_line(), &context);
			break;
		}

		bool highlight_error = false;
		String warning_shader_path = shader->get_path();
		if (context) {
			if (!context->path.is_empty()) {
				TreeItem *tree_item = shader_dependency_tree->get_item_with_text(context->path);
				if (saved_treat_warning_as_errors) {
					if (warning_count == 0) {
						tree_item->set_custom_bg_color(0, marked_line_color);
					}
				} else {
					// TODO need a less intense yellow. set font coor instead maybe?
					// maybe use color * intensity used for preprocessor condition blocked code?
					// tree_item->set_custom_bg_color(0, warning_color);
				}

				warning_shader_path = context->path;
			} else {
				highlight_error = true;
			}
		}

		if (warning_count == 0) {
			if (saved_treat_warning_as_errors) {
				error_shader_path = warning_shader_path;
				String error_text = "error(" + warning_shader_path + ":" + itos(adjusted_line) + "): " + w.get_message() + " " + TTR("Warnings should be fixed to prevent errors.");
				set_error_pos(adjusted_line - 1, 0);
				set_error(error_text);

				if (highlight_error) {
					get_text_editor()->set_line_background_color(adjusted_line - 1, marked_line_color);
				}
			}
		}

		warning_count++;
		int line = w.get_line();

		Array warning_data_array;
		warning_data_array.resize(2);
		warning_data_array[0] = adjusted_line - 1;
		warning_data_array[1] = warning_shader_path;

		// First cell.
		warnings_panel->push_cell();
		warnings_panel->push_color(warning_color);
		if (adjusted_line != -1) {
			warnings_panel->push_meta(warning_data_array);
			warnings_panel->add_text(warning_shader_path + " - " + TTR("Line") + " " + itos(adjusted_line));
			warnings_panel->add_text(" (" + w.get_name() + "):");
		} else {
			warnings_panel->add_text(w.get_name() + ":");
		}
		warnings_panel->pop(); // Color.
		warnings_panel->pop(); // Cell.

		// Second cell.
		warnings_panel->push_cell();
		warnings_panel->add_text(w.get_message());
		warnings_panel->pop(); // Cell.
	}
	warnings_panel->pop(); // Table.

	set_warning_count(warning_count);
}

void ShaderTextEditor::_bind_methods() {
}

ShaderTextEditor::ShaderTextEditor() {
	syntax_highlighter.instantiate();
	get_text_editor()->set_syntax_highlighter(syntax_highlighter);
}

/*** SCRIPT EDITOR ******/

void ShaderEditor::_menu_option(int p_option) {
	switch (p_option) {
		case EDIT_UNDO: {
			shader_editor->get_text_editor()->undo();
		} break;
		case EDIT_REDO: {
			shader_editor->get_text_editor()->redo();
		} break;
		case EDIT_CUT: {
			shader_editor->get_text_editor()->cut();
		} break;
		case EDIT_COPY: {
			shader_editor->get_text_editor()->copy();
		} break;
		case EDIT_PASTE: {
			shader_editor->get_text_editor()->paste();
		} break;
		case EDIT_SELECT_ALL: {
			shader_editor->get_text_editor()->select_all();
		} break;
		case EDIT_MOVE_LINE_UP: {
			shader_editor->move_lines_up();
		} break;
		case EDIT_MOVE_LINE_DOWN: {
			shader_editor->move_lines_down();
		} break;
		case EDIT_INDENT_LEFT: {
			if (shader.is_null()) {
				return;
			}
			shader_editor->get_text_editor()->unindent_lines();
		} break;
		case EDIT_INDENT_RIGHT: {
			if (shader.is_null()) {
				return;
			}
			shader_editor->get_text_editor()->indent_lines();
		} break;
		case EDIT_DELETE_LINE: {
			shader_editor->delete_lines();
		} break;
		case EDIT_DUPLICATE_SELECTION: {
			shader_editor->duplicate_selection();
		} break;
		case EDIT_TOGGLE_COMMENT: {
			if (shader.is_null()) {
				return;
			}

			shader_editor->toggle_inline_comment("//");

		} break;
		case EDIT_COMPLETE: {
			shader_editor->get_text_editor()->request_code_completion();
		} break;
		case SEARCH_FIND: {
			shader_editor->get_find_replace_bar()->popup_search();
		} break;
		case SEARCH_FIND_NEXT: {
			shader_editor->get_find_replace_bar()->search_next();
		} break;
		case SEARCH_FIND_PREV: {
			shader_editor->get_find_replace_bar()->search_prev();
		} break;
		case SEARCH_REPLACE: {
			shader_editor->get_find_replace_bar()->popup_replace();
		} break;
		case SEARCH_GOTO_LINE: {
			goto_line_dialog->popup_find_line(shader_editor->get_text_editor());
		} break;
		case BOOKMARK_TOGGLE: {
			shader_editor->toggle_bookmark();
		} break;
		case BOOKMARK_GOTO_NEXT: {
			shader_editor->goto_next_bookmark();
		} break;
		case BOOKMARK_GOTO_PREV: {
			shader_editor->goto_prev_bookmark();
		} break;
		case BOOKMARK_REMOVE_ALL: {
			shader_editor->remove_all_bookmarks();
		} break;
		case HELP_DOCS: {
			OS::get_singleton()->shell_open(vformat("%s/tutorials/shaders/shader_reference/index.html", VERSION_DOCS_URL));
		} break;
	}
	if (p_option != SEARCH_FIND && p_option != SEARCH_REPLACE && p_option != SEARCH_GOTO_LINE) {
		shader_editor->get_text_editor()->call_deferred(SNAME("grab_focus"));
	}
}

void ShaderEditor::_notification(int p_what) {
	if (p_what == NOTIFICATION_WM_WINDOW_FOCUS_IN) {
		_check_for_external_edit();
	}
}

void ShaderEditor::_editor_settings_changed() {
	shader_editor->update_editor_settings();

	shader_editor->get_text_editor()->add_theme_constant_override("line_spacing", EditorSettings::get_singleton()->get("text_editor/appearance/whitespace/line_spacing"));
	shader_editor->get_text_editor()->set_draw_breakpoints_gutter(false);
	shader_editor->get_text_editor()->set_draw_executing_lines_gutter(false);
}

void ShaderEditor::_show_warnings_panel(bool p_show) {
	warnings_panel->set_visible(p_show);
}

void ShaderEditor::_warning_clicked(Variant p_data) {
	if (p_data.get_type() == Variant::ARRAY) {
		// open shader path
		Array data_array = p_data.operator Array();
		open_path(data_array[1].operator String());
		shader_editor->get_text_editor()->set_caret_line((data_array[0]).operator int64_t());
	}
}

void ShaderEditor::_bind_methods() {
	ClassDB::bind_method("_show_warnings_panel", &ShaderEditor::_show_warnings_panel);
	ClassDB::bind_method("_warning_clicked", &ShaderEditor::_warning_clicked);
}

void ShaderEditor::ensure_select_current() {
	/*
	if (tab_container->get_child_count() && tab_container->get_current_tab()>=0) {
		ShaderTextEditor *ste = Object::cast_to<ShaderTextEditor>(tab_container->get_child(tab_container->get_current_tab()));
		if (!ste)
			return;
		Ref<Shader> shader = ste->get_edited_shader();
		get_scene()->get_root_node()->call("_resource_selected",shader);
	}*/
}

void ShaderEditor::goto_line_selection(int p_line, int p_begin, int p_end) {
	shader_editor->goto_line_selection(p_line, p_begin, p_end);
}

void ShaderEditor::_project_settings_changed() {
	_update_warnings(true);
}

void ShaderEditor::_update_warnings(bool p_validate) {
	bool changed = false;

	bool warnings_enabled = GLOBAL_GET("debug/shader_language/warnings/enable").booleanize();
	if (warnings_enabled != saved_warnings_enabled) {
		saved_warnings_enabled = warnings_enabled;
		changed = true;
	}

	bool treat_warning_as_errors = GLOBAL_GET("debug/shader_language/warnings/treat_warnings_as_errors").booleanize();
	if (treat_warning_as_errors != saved_treat_warning_as_errors) {
		saved_treat_warning_as_errors = treat_warning_as_errors;
		changed = true;
	}

	bool update_flags = false;

	for (int i = 0; i < ShaderWarning::WARNING_MAX; i++) {
		ShaderWarning::Code code = (ShaderWarning::Code)i;
		bool value = GLOBAL_GET("debug/shader_language/warnings/" + ShaderWarning::get_name_from_code(code).to_lower());

		if (saved_warnings[code] != value) {
			saved_warnings[code] = value;
			update_flags = true;
			changed = true;
		}
	}

	if (update_flags) {
		saved_warning_flags = (uint32_t)ShaderWarning::get_flags_from_codemap(saved_warnings);
	}

	if (p_validate && changed && shader_editor && shader_editor->get_edited_shader().is_valid()) {
		shader_editor->validate_script();
	}
}

void ShaderEditor::_check_for_external_edit() {
	if (shader.is_null() || !shader.is_valid()) {
		return;
	}

	if (shader->is_built_in()) {
		return;
	}

	bool use_autoreload = bool(EDITOR_DEF("text_editor/behavior/files/auto_reload_scripts_on_external_change", false));
	if (shader->get_last_modified_time() != FileAccess::get_modified_time(shader->get_path())) {
		if (use_autoreload) {
			_reload_shader_from_disk();
		} else {
			disk_changed->call_deferred(SNAME("popup_centered"));
		}
	}
}

void ShaderEditor::_reload_shader_from_disk() {
	Ref<Shader> rel_shader = ResourceLoader::load(shader->get_path(), shader->get_class(), ResourceFormatLoader::CACHE_MODE_IGNORE);
	ERR_FAIL_COND(!rel_shader.is_valid());

	shader->set_code(rel_shader->get_code());
	shader->set_last_modified_time(rel_shader->get_last_modified_time());
	shader_editor->reload_text();
}

void ShaderEditor::edit(const Ref<Shader> &p_shader) {
	if (p_shader.is_null() || !p_shader->is_text_shader()) {
		return;
	}

	if (shader == p_shader) {
		return;
	}

	shader = p_shader;

	// create shader dependencies and update tree.
	// TODO need to call this on edit as well. if the script changes there could be new dpendencies.
	// this needs to be updated constantly
	shader_rolling_code.clear();
	shader_dependencies.populate(shader);
	_update_shader_dependency_tree();

	shader_rolling_code[shader->get_path()] = p_shader->get_code();

	shader_editor->set_edited_shader(p_shader);

	//vertex_editor->set_edited_shader(shader,ShaderLanguage::SHADER_MATERIAL_VERTEX);
	// see if already has it
}

void ShaderEditor::save_external_data(const String &p_str) {
	if (shader.is_null()) {
		disk_changed->hide();
		return;
	}

	apply_shaders();
	if (!shader->is_built_in()) {
		//external shader, save it
		auto edited_shader = shader_editor->get_edited_shader();
		ResourceSaver::save(edited_shader->get_path(), edited_shader);
		ResourceSaver::save(shader->get_path(), shader);
	}

	disk_changed->hide();
}

void ShaderEditor::apply_shaders() {
	if (shader.is_valid()) {
		Ref<Shader> currently_edited_shader = shader_editor->get_edited_shader();
		String editor_code = shader_editor->get_text_editor()->get_text();
		shader_rolling_code[currently_edited_shader->get_path()] = editor_code;
		if (currently_edited_shader == shader) {
			// TODO get cached version of root node shader code to apply. Then set all dep code too. Do this in reverse order up the tree?
			String shader_code = shader->get_code();
			if (shader_code != editor_code) {
				shader->set_code(editor_code);
				shader->set_edited(true);
			}
		} else {
			currently_edited_shader->set_code(editor_code);
			currently_edited_shader->set_edited(true);
		}

		ShaderPreprocessor::refresh_shader_dependencies(*shader);
	}
}

void ShaderEditor::_text_edit_gui_input(const Ref<InputEvent> &p_ev) {
	Ref<InputEventMouseButton> mb = p_ev;

	if (mb.is_valid()) {
		if (mb->get_button_index() == MouseButton::RIGHT && mb->is_pressed()) {
			CodeEdit *tx = shader_editor->get_text_editor();

			Point2i pos = tx->get_line_column_at_pos(mb->get_global_position() - tx->get_global_position());
			int row = pos.y;
			int col = pos.x;
			tx->set_move_caret_on_right_click_enabled(EditorSettings::get_singleton()->get("text_editor/behavior/navigation/move_caret_on_right_click"));

			if (tx->is_move_caret_on_right_click_enabled()) {
				if (tx->has_selection()) {
					int from_line = tx->get_selection_from_line();
					int to_line = tx->get_selection_to_line();
					int from_column = tx->get_selection_from_column();
					int to_column = tx->get_selection_to_column();

					if (row < from_line || row > to_line || (row == from_line && col < from_column) || (row == to_line && col > to_column)) {
						// Right click is outside the selected text
						tx->deselect();
					}
				}
				if (!tx->has_selection()) {
					tx->set_caret_line(row, true, false);
					tx->set_caret_column(col);
				}
			}
			_make_context_menu(tx->has_selection(), get_local_mouse_position());
		}
	}

	Ref<InputEventKey> k = p_ev;
	if (k.is_valid() && k->is_pressed() && k->is_action("ui_menu", true)) {
		CodeEdit *tx = shader_editor->get_text_editor();
		tx->adjust_viewport_to_caret();
		_make_context_menu(tx->has_selection(), (get_global_transform().inverse() * tx->get_global_transform()).xform(tx->get_caret_draw_pos()));
		context_menu->grab_focus();
	}
}

void ShaderEditor::_update_bookmark_list() {
	bookmarks_menu->clear();

	bookmarks_menu->add_shortcut(ED_GET_SHORTCUT("script_text_editor/toggle_bookmark"), BOOKMARK_TOGGLE);
	bookmarks_menu->add_shortcut(ED_GET_SHORTCUT("script_text_editor/remove_all_bookmarks"), BOOKMARK_REMOVE_ALL);
	bookmarks_menu->add_shortcut(ED_GET_SHORTCUT("script_text_editor/goto_next_bookmark"), BOOKMARK_GOTO_NEXT);
	bookmarks_menu->add_shortcut(ED_GET_SHORTCUT("script_text_editor/goto_previous_bookmark"), BOOKMARK_GOTO_PREV);

	Array bookmark_list = shader_editor->get_text_editor()->get_bookmarked_lines();
	if (bookmark_list.size() == 0) {
		return;
	}

	bookmarks_menu->add_separator();

	for (int i = 0; i < bookmark_list.size(); i++) {
		String line = shader_editor->get_text_editor()->get_line(bookmark_list[i]).strip_edges();
		// Limit the size of the line if too big.
		if (line.length() > 50) {
			line = line.substr(0, 50);
		}

		bookmarks_menu->add_item(String::num((int)bookmark_list[i] + 1) + " - \"" + line + "\"");
		bookmarks_menu->set_item_metadata(bookmarks_menu->get_item_count() - 1, bookmark_list[i]);
	}
}

void ShaderEditor::_bookmark_item_pressed(int p_idx) {
	if (p_idx < 4) { // Any item before the separator.
		_menu_option(bookmarks_menu->get_item_id(p_idx));
	} else {
		shader_editor->goto_line(bookmarks_menu->get_item_metadata(p_idx));
	}
}

void ShaderEditor::_make_context_menu(bool p_selection, Vector2 p_position) {
	context_menu->clear();
	if (p_selection) {
		context_menu->add_shortcut(ED_GET_SHORTCUT("ui_cut"), EDIT_CUT);
		context_menu->add_shortcut(ED_GET_SHORTCUT("ui_copy"), EDIT_COPY);
	}

	context_menu->add_shortcut(ED_GET_SHORTCUT("ui_paste"), EDIT_PASTE);
	context_menu->add_separator();
	context_menu->add_shortcut(ED_GET_SHORTCUT("ui_text_select_all"), EDIT_SELECT_ALL);
	context_menu->add_shortcut(ED_GET_SHORTCUT("ui_undo"), EDIT_UNDO);
	context_menu->add_shortcut(ED_GET_SHORTCUT("ui_redo"), EDIT_REDO);

	context_menu->add_separator();
	context_menu->add_shortcut(ED_GET_SHORTCUT("script_text_editor/indent_left"), EDIT_INDENT_LEFT);
	context_menu->add_shortcut(ED_GET_SHORTCUT("script_text_editor/indent_right"), EDIT_INDENT_RIGHT);
	context_menu->add_shortcut(ED_GET_SHORTCUT("script_text_editor/toggle_comment"), EDIT_TOGGLE_COMMENT);
	context_menu->add_shortcut(ED_GET_SHORTCUT("script_text_editor/toggle_bookmark"), BOOKMARK_TOGGLE);

	context_menu->set_position(get_screen_position() + p_position);
	context_menu->reset_size();
	context_menu->popup();
}

void ShaderEditor::open_path(String p_path) {
	// TODO pull from shader dependency graph which can hold cache data, or store directly in tree?
	// using only to pull code. how to cache data? hash set in editor panel with path lookup?
	RES res = ResourceLoader::load(p_path);
	if (!res.is_null()) {
		Shader *shader = Object::cast_to<Shader>(*res);
		if (shader != nullptr) {
			auto rollingCode = shader_rolling_code.find(shader->get_path());

			if (rollingCode) {
				shader_editor->set_edited_shader(shader, rollingCode->get());
			} else {
				String included = shader->get_code();
				shader_rolling_code[p_path] = included;
				shader_editor->set_edited_shader(shader);
			}
		}
	}
}

Ref<Shader> ShaderEditor::get_shader() {
	return shader;
}

void ShaderEditor::_tree_activate_shader() {
	TreeItem *selected = shader_dependency_tree->get_selected();
	if (selected) {
		String path = selected->get_metadata(0);

		open_path(path);
	}
}

void ShaderEditor::_update_shader_dependency_tree() {
	shader_dependency_tree->clear();
	TreeItem *root = shader_dependency_tree->create_item();
	root->select(0);

	for (ShaderDependencyNode *node : shader_dependencies.nodes) {
		TreeItem *shader_parent_item = shader_dependency_tree->create_item(root);
		shader_parent_item->set_text(0, TTR(node->get_path()));
		shader_parent_item->set_metadata(0, node->get_path());
		shader_parent_item->set_icon(0, get_theme_icon(SNAME("Shader"), SNAME("EditorIcons")));

		_update_shader_dependency_tree_items(shader_parent_item, node);
	}
}

void ShaderEditor::_update_shader_dependency_tree_items(TreeItem *p_parent_tree_item, ShaderDependencyNode *p_node) {
	for (ShaderDependencyNode *child_node : p_node->dependencies) {
		TreeItem *shader_child_item = shader_dependency_tree->create_item(p_parent_tree_item);
		shader_child_item->set_text(0, TTR(child_node->get_path()));
		shader_child_item->set_metadata(0, child_node->get_path());
		shader_child_item->set_icon(0, get_theme_icon(SNAME("Shader"), SNAME("EditorIcons")));

		_update_shader_dependency_tree_items(shader_child_item, child_node);
	}
}

ShaderEditor::ShaderEditor(EditorNode *p_node) {
	GLOBAL_DEF("debug/shader_language/warnings/enable", true);
	GLOBAL_DEF("debug/shader_language/warnings/treat_warnings_as_errors", false);
	for (int i = 0; i < (int)ShaderWarning::WARNING_MAX; i++) {
		GLOBAL_DEF("debug/shader_language/warnings/" + ShaderWarning::get_name_from_code((ShaderWarning::Code)i).to_lower(), true);
	}
	_update_warnings(false);

	shader_editor = memnew(ShaderTextEditor);
	shader_editor->set_shader_editor(this);

	shader_editor->set_v_size_flags(SIZE_EXPAND_FILL);
	shader_editor->add_theme_constant_override("separation", 0);
	shader_editor->set_anchors_and_offsets_preset(Control::PRESET_WIDE);

	shader_editor->connect("show_warnings_panel", callable_mp(this, &ShaderEditor::_show_warnings_panel));
	shader_editor->connect("script_changed", callable_mp(this, &ShaderEditor::apply_shaders));
	EditorSettings::get_singleton()->connect("settings_changed", callable_mp(this, &ShaderEditor::_editor_settings_changed));
	ProjectSettingsEditor::get_singleton()->connect("confirmed", callable_mp(this, &ShaderEditor::_project_settings_changed));

	shader_editor->get_text_editor()->set_code_hint_draw_below(EditorSettings::get_singleton()->get("text_editor/completion/put_callhint_tooltip_below_current_line"));

	shader_editor->get_text_editor()->set_symbol_lookup_on_click_enabled(true);
	shader_editor->get_text_editor()->set_context_menu_enabled(false);
	shader_editor->get_text_editor()->connect("gui_input", callable_mp(this, &ShaderEditor::_text_edit_gui_input));

	shader_editor->update_editor_settings();

	context_menu = memnew(PopupMenu);
	add_child(context_menu);
	context_menu->connect("id_pressed", callable_mp(this, &ShaderEditor::_menu_option));

	VBoxContainer *main_container = memnew(VBoxContainer);
	add_child(main_container);

	HBoxContainer *hbc = memnew(HBoxContainer);

	edit_menu = memnew(MenuButton);
	edit_menu->set_shortcut_context(this);
	edit_menu->set_text(TTR("Edit"));
	edit_menu->set_switch_on_hover(true);

	edit_menu->get_popup()->add_shortcut(ED_GET_SHORTCUT("ui_undo"), EDIT_UNDO);
	edit_menu->get_popup()->add_shortcut(ED_GET_SHORTCUT("ui_redo"), EDIT_REDO);
	edit_menu->get_popup()->add_separator();
	edit_menu->get_popup()->add_shortcut(ED_GET_SHORTCUT("ui_cut"), EDIT_CUT);
	edit_menu->get_popup()->add_shortcut(ED_GET_SHORTCUT("ui_copy"), EDIT_COPY);
	edit_menu->get_popup()->add_shortcut(ED_GET_SHORTCUT("ui_paste"), EDIT_PASTE);
	edit_menu->get_popup()->add_separator();
	edit_menu->get_popup()->add_shortcut(ED_GET_SHORTCUT("ui_text_select_all"), EDIT_SELECT_ALL);
	edit_menu->get_popup()->add_separator();
	edit_menu->get_popup()->add_shortcut(ED_GET_SHORTCUT("script_text_editor/move_up"), EDIT_MOVE_LINE_UP);
	edit_menu->get_popup()->add_shortcut(ED_GET_SHORTCUT("script_text_editor/move_down"), EDIT_MOVE_LINE_DOWN);
	edit_menu->get_popup()->add_shortcut(ED_GET_SHORTCUT("script_text_editor/indent_left"), EDIT_INDENT_LEFT);
	edit_menu->get_popup()->add_shortcut(ED_GET_SHORTCUT("script_text_editor/indent_right"), EDIT_INDENT_RIGHT);
	edit_menu->get_popup()->add_shortcut(ED_GET_SHORTCUT("script_text_editor/delete_line"), EDIT_DELETE_LINE);
	edit_menu->get_popup()->add_shortcut(ED_GET_SHORTCUT("script_text_editor/toggle_comment"), EDIT_TOGGLE_COMMENT);
	edit_menu->get_popup()->add_shortcut(ED_GET_SHORTCUT("script_text_editor/duplicate_selection"), EDIT_DUPLICATE_SELECTION);
	edit_menu->get_popup()->add_separator();
	edit_menu->get_popup()->add_shortcut(ED_GET_SHORTCUT("ui_text_completion_query"), EDIT_COMPLETE);
	edit_menu->get_popup()->connect("id_pressed", callable_mp(this, &ShaderEditor::_menu_option));

	search_menu = memnew(MenuButton);
	search_menu->set_shortcut_context(this);
	search_menu->set_text(TTR("Search"));
	search_menu->set_switch_on_hover(true);

	search_menu->get_popup()->add_shortcut(ED_GET_SHORTCUT("script_text_editor/find"), SEARCH_FIND);
	search_menu->get_popup()->add_shortcut(ED_GET_SHORTCUT("script_text_editor/find_next"), SEARCH_FIND_NEXT);
	search_menu->get_popup()->add_shortcut(ED_GET_SHORTCUT("script_text_editor/find_previous"), SEARCH_FIND_PREV);
	search_menu->get_popup()->add_shortcut(ED_GET_SHORTCUT("script_text_editor/replace"), SEARCH_REPLACE);
	search_menu->get_popup()->connect("id_pressed", callable_mp(this, &ShaderEditor::_menu_option));

	MenuButton *goto_menu = memnew(MenuButton);
	goto_menu->set_shortcut_context(this);
	goto_menu->set_text(TTR("Go To"));
	goto_menu->set_switch_on_hover(true);
	goto_menu->get_popup()->connect("id_pressed", callable_mp(this, &ShaderEditor::_menu_option));

	goto_menu->get_popup()->add_shortcut(ED_GET_SHORTCUT("script_text_editor/goto_line"), SEARCH_GOTO_LINE);
	goto_menu->get_popup()->add_separator();

	bookmarks_menu = memnew(PopupMenu);
	bookmarks_menu->set_name("Bookmarks");
	goto_menu->get_popup()->add_child(bookmarks_menu);
	goto_menu->get_popup()->add_submenu_item(TTR("Bookmarks"), "Bookmarks");
	_update_bookmark_list();
	bookmarks_menu->connect("about_to_popup", callable_mp(this, &ShaderEditor::_update_bookmark_list));
	bookmarks_menu->connect("index_pressed", callable_mp(this, &ShaderEditor::_bookmark_item_pressed));

	help_menu = memnew(MenuButton);
	help_menu->set_text(TTR("Help"));
	help_menu->set_switch_on_hover(true);
	help_menu->get_popup()->add_icon_item(p_node->get_gui_base()->get_theme_icon(SNAME("Instance"), SNAME("EditorIcons")), TTR("Online Docs"), HELP_DOCS);
	help_menu->get_popup()->connect("id_pressed", callable_mp(this, &ShaderEditor::_menu_option));

	add_child(main_container);
	main_container->add_child(hbc);
	hbc->add_child(search_menu);
	hbc->add_child(edit_menu);
	hbc->add_child(goto_menu);
	hbc->add_child(help_menu);
	hbc->add_theme_style_override("panel", p_node->get_gui_base()->get_theme_stylebox(SNAME("ScriptEditorPanel"), SNAME("EditorStyles")));

	// split container for code editor and dependency tree
	HSplitContainer *panel_split = memnew(HSplitContainer);
	panel_split->set_anchors_and_offsets_preset(Control::PRESET_WIDE);
	panel_split->set_v_size_flags(SIZE_EXPAND_FILL);
	main_container->add_child(panel_split);

	VSplitContainer *editor_box = memnew(VSplitContainer);
	panel_split->add_child(editor_box);
	editor_box->set_anchors_and_offsets_preset(Control::PRESET_WIDE);
	editor_box->set_v_size_flags(SIZE_EXPAND_FILL);
	editor_box->add_child(shader_editor);

	shader_dependency_tree = memnew(Tree);
	shader_dependency_tree->set_hide_root(true);
	shader_dependency_tree->set_allow_rmb_select(true);
	shader_dependency_tree->set_anchors_and_offsets_preset(Control::PRESET_TOP_RIGHT, Control::PRESET_MODE_KEEP_WIDTH);
	shader_dependency_tree->set_select_mode(Tree::SELECT_SINGLE);
	shader_dependency_tree->set_custom_minimum_size(Size2(100 * EDSCALE, 30 * EDSCALE));
	shader_dependency_tree->connect("item_activated", callable_mp(this, &ShaderEditor::_tree_activate_shader));
	panel_split->add_child(shader_dependency_tree);
	shader_editor->set_shader_dependency_tree(shader_dependency_tree);

	FindReplaceBar *bar = memnew(FindReplaceBar);
	editor_box->add_child(bar);
	bar->set_anchors_and_offsets_preset(Control::PRESET_LEFT_WIDE, Control::PRESET_MODE_KEEP_WIDTH);
	bar->hide();
	shader_editor->set_find_replace_bar(bar);

	warnings_panel = memnew(RichTextLabel);
	warnings_panel->set_custom_minimum_size(Size2(0, 100 * EDSCALE));
	warnings_panel->set_h_size_flags(SIZE_EXPAND_FILL);
	warnings_panel->set_meta_underline(true);
	warnings_panel->set_selection_enabled(true);
	warnings_panel->set_focus_mode(FOCUS_CLICK);
	warnings_panel->hide();
	warnings_panel->connect("meta_clicked", callable_mp(this, &ShaderEditor::_warning_clicked));
	editor_box->add_child(warnings_panel);
	shader_editor->set_warnings_panel(warnings_panel);

	goto_line_dialog = memnew(GotoLineDialog);
	add_child(goto_line_dialog);

	disk_changed = memnew(ConfirmationDialog);

	VBoxContainer *vbc = memnew(VBoxContainer);
	disk_changed->add_child(vbc);

	Label *dl = memnew(Label);
	dl->set_text(TTR("This shader has been modified on disk.\nWhat action should be taken?"));
	vbc->add_child(dl);

	disk_changed->connect("confirmed", callable_mp(this, &ShaderEditor::_reload_shader_from_disk));
	disk_changed->get_ok_button()->set_text(TTR("Reload"));

	disk_changed->add_button(TTR("Resave"), !DisplayServer::get_singleton()->get_swap_cancel_ok(), "resave");
	disk_changed->connect("custom_action", callable_mp(this, &ShaderEditor::save_external_data));

	add_child(disk_changed);

	_editor_settings_changed();
}

void ShaderEditorPlugin::edit(Object *p_object) {
	Shader *s = Object::cast_to<Shader>(p_object);
	shader_editor->edit(s);
}

bool ShaderEditorPlugin::handles(Object *p_object) const {
	Shader *shader = Object::cast_to<Shader>(p_object);
	return shader != nullptr && shader->is_text_shader();
}

void ShaderEditorPlugin::make_visible(bool p_visible) {
	if (p_visible) {
		button->show();
		editor->make_bottom_panel_item_visible(shader_editor);

	} else {
		button->hide();
		if (shader_editor->is_visible_in_tree()) {
			editor->hide_bottom_panel();
		}
		shader_editor->apply_shaders();
	}
}

void ShaderEditorPlugin::selected_notify() {
	shader_editor->ensure_select_current();
}

void ShaderEditorPlugin::save_external_data() {
	shader_editor->save_external_data();
}

void ShaderEditorPlugin::apply_changes() {
	shader_editor->apply_shaders();
}

ShaderEditorPlugin::ShaderEditorPlugin(EditorNode *p_node) {
	editor = p_node;
	shader_editor = memnew(ShaderEditor(p_node));

	shader_editor->set_custom_minimum_size(Size2(0, 300) * EDSCALE);
	button = editor->add_bottom_panel_item(TTR("Shader"), shader_editor);
	button->hide();

	_2d = false;
}

ShaderEditorPlugin::~ShaderEditorPlugin() {
}
