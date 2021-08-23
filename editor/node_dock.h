/*************************************************************************/
/*  node_dock.h                                                          */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
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

#ifndef NODE_DOCK_H
#define NODE_DOCK_H

#include "connections_dialog.h"
#include "groups_editor.h"

class NodeDock : public VBoxContainer {
	GDCLASS(NodeDock, VBoxContainer);

	struct ModeItem {
		Control *control = nullptr;
		Button *button = nullptr;
	};

	int current_idx = 0;
	Vector<ModeItem> mode_items;

	Node *current_node = nullptr;

	GridContainer *mode_container;
	Button *connections_button;
	Button *groups_button;

	ConnectionsDock *connections;
	GroupsEditor *groups;
	Label *select_a_node;

	void _set_current(bool p_toggled, int p_idx);

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	static NodeDock *singleton;

	void update_lists();

	// Getters
	ConnectionsDock *get_connections_dock();
	GroupsEditor *get_group_editor();

	// Modifiers
	void set_node(Node *p_node);

	Button *add_control(Control *p_control, const String &p_title);
	void remove_control(Control *p_control);
	void show_control(Control *p_control);

	NodeDock();
};

#endif // NODE_DOCK_H
