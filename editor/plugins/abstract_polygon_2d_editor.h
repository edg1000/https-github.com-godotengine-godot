/*************************************************************************/
/*  abstract_polygon_2d_editor.cpp                                       */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2017 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2017 Godot Engine contributors (cf. AUTHORS.md)    */
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
#ifndef ABSTRACT_POLYGON_2D_EDITOR_H
#define ABSTRACT_POLYGON_2D_EDITOR_H

#include "editor/editor_node.h"
#include "editor/editor_plugin.h"
#include "scene/2d/polygon_2d.h"
#include "scene/2d/editable_polygon_2d.h"
#include "scene/gui/button_group.h"
#include "scene/gui/tool_button.h"

/**
	@author Juan Linietsky <reduzio@gmail.com>
*/
class CanvasItemEditor;

class AbstractPolygon2DEditor : public HBoxContainer {

	GDCLASS(AbstractPolygon2DEditor, HBoxContainer);

	ToolButton *button_create;
	ToolButton *button_edit;

	int edited_polygon;
	int edited_point;
	Vector2 edited_point_pos;
	Vector<Vector2> pre_move_edit;
	Vector<Vector2> wip;
	bool wip_active;

protected:
	enum {

		MODE_CREATE,
		MODE_EDIT,
		MODE_CONT,

	};

	int mode;

	UndoRedo *undo_redo;

	CanvasItemEditor *canvas_item_editor;
	EditorNode *editor;
	Panel *panel;
	ConfirmationDialog *create_res;

	EditablePolygon2D *editable;
	Node2D *node;

	void _menu_option(int p_option);
	void _wip_close();
	void _canvas_draw();

	void _notification(int p_what);
	void _node_removed(Node *p_node);
	static void _bind_methods();

	virtual EditablePolygon2D *_get_editable(Node *p_node) const = 0;
	virtual void _create_res();

public:
	bool forward_gui_input(const Ref<InputEvent> &p_event);
	void edit(Node *p_polygon);
	AbstractPolygon2DEditor(EditorNode *p_editor);
};

class AbstractPolygon2DEditorPlugin : public EditorPlugin {

	GDCLASS(AbstractPolygon2DEditorPlugin, EditorPlugin);

	AbstractPolygon2DEditor *polygon_editor;
	EditorNode *editor;
	String klass;

public:
	virtual bool forward_canvas_gui_input(const Transform2D &p_canvas_xform, const Ref<InputEvent> &p_event) { return polygon_editor->forward_gui_input(p_event); }

	bool has_main_screen() const { return false; }
	virtual String get_name() const { return klass; }
	virtual void edit(Object *p_object);
	virtual bool handles(Object *p_object) const;
	virtual void make_visible(bool p_visible);

	AbstractPolygon2DEditorPlugin(EditorNode *p_node, AbstractPolygon2DEditor *p_polygon_editor, String p_class);
	~AbstractPolygon2DEditorPlugin();
};

#endif // ABSTRACT_POLYGON_2D_EDITOR_H
