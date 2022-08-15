/*************************************************************************/
/*  shader_editor_plugin.h                                               */
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

#ifndef SHADER_EDITOR_PLUGIN_H
#define SHADER_EDITOR_PLUGIN_H

#include "editor/editor_plugin.h"

class HSplitContainer;
class ItemList;
class ShaderCreateDialog;
class TabContainer;
class TextShaderEditor;

#include "modules/modules_enabled.gen.h" // For visual shader.
#ifdef MODULE_VISUAL_SHADER_ENABLED
class VisualShaderEditor;
#endif // MODULE_VISUAL_SHADER_ENABLED

class ShaderEditorPlugin : public EditorPlugin {
	GDCLASS(ShaderEditorPlugin, EditorPlugin);

	struct EditedShader {
		Ref<Shader> shader;
		Ref<ShaderInclude> shader_inc;
		TextShaderEditor *shader_editor = nullptr;
#ifdef MODULE_VISUAL_SHADER_ENABLED
		VisualShaderEditor *visual_shader_editor = nullptr;
#endif // MODULE_VISUAL_SHADER_ENABLED
	};

	LocalVector<EditedShader> edited_shaders;

	enum {
		FILE_NEW,
		FILE_NEW_INCLUDE,
		FILE_OPEN,
		FILE_OPEN_INCLUDE,
		FILE_SAVE,
		FILE_SAVE_AS,
		FILE_INSPECT,
		FILE_CLOSE,
		FILE_MAX
	};

	HSplitContainer *main_split = nullptr;
	ItemList *shader_list = nullptr;
	TabContainer *shader_tabs = nullptr;

	Button *button = nullptr;
	MenuButton *file_menu = nullptr;

	ShaderCreateDialog *shader_create_dialog = nullptr;

	void _update_shader_list();
	void _shader_selected(int p_index);
	void _shader_list_clicked(int p_item, Vector2 p_local_mouse_pos, MouseButton p_mouse_button_index);
	void _menu_item_pressed(int p_index);
	void _resource_saved(Object *obj);
	void _close_shader(int p_index);

	void _shader_created(Ref<Shader> p_shader);
	void _shader_include_created(Ref<ShaderInclude> p_shader_inc);
	void _update_shader_list_status();
	void _move_shader_tab(int p_from, int p_to);

	Variant get_drag_data_fw(const Point2 &p_point, Control *p_from);
	bool can_drop_data_fw(const Point2 &p_point, const Variant &p_data, Control *p_from) const;
	void drop_data_fw(const Point2 &p_point, const Variant &p_data, Control *p_from);

protected:
	static void _bind_methods();

public:
	virtual void edit(Object *p_object) override;
	virtual bool handles(Object *p_object) const override;
	virtual void make_visible(bool p_visible) override;
	virtual void selected_notify() override;

	TextShaderEditor *get_shader_editor(const Ref<Shader> &p_for_shader);
#ifdef MODULE_VISUAL_SHADER_ENABLED
	VisualShaderEditor *get_visual_shader_editor(const Ref<Shader> &p_for_shader);
#endif // MODULE_VISUAL_SHADER_ENABLED

	virtual void save_external_data() override;
	virtual void apply_changes() override;

	ShaderEditorPlugin();
	~ShaderEditorPlugin();
};

#endif // SHADER_EDITOR_PLUGIN_H
