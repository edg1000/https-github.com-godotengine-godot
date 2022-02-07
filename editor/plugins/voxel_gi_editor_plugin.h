/*************************************************************************/
/*  voxel_gi_editor_plugin.h                                             */
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

#ifndef VOXEL_GIEDITORPLUGIN_H
#define VOXEL_GIEDITORPLUGIN_H

#include "core/object/object.h"
#include "core/string/ustring.h"
#include "editor/editor_node.h"
#include "editor/editor_plugin.h"
#include "scene/3d/voxel_gi.h"
#include "scene/resources/material.h"

class Button;
class EditorFileDialog;
class EditorNode;
class HBoxContainer;
class VoxelGI;
struct EditorProgress;

class VoxelGIEditorPlugin : public EditorPlugin {
	GDCLASS(VoxelGIEditorPlugin, EditorPlugin);

	VoxelGI *voxel_gi;

	HBoxContainer *bake_hb;
	Button *bake;
	EditorNode *editor;

	EditorFileDialog *probe_file;

	static EditorProgress *tmp_progress;
	static void bake_func_begin(int p_steps);
	static void bake_func_step(int p_step, const String &p_description);
	static void bake_func_end();

	void _bake();
	void _voxel_gi_save_path_and_bake(const String &p_path);

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	virtual String get_name() const override { return "VoxelGI"; }
	bool has_main_screen() const override { return false; }
	virtual void edit(Object *p_object) override;
	virtual bool handles(Object *p_object) const override;
	virtual void make_visible(bool p_visible) override;

	VoxelGIEditorPlugin(EditorNode *p_node);
	~VoxelGIEditorPlugin();
};

#endif // VOXEL_GIEDITORPLUGIN_H
