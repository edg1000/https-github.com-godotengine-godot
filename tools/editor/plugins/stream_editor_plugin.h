/*************************************************************************/
/*  stream_editor_plugin.h                                               */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2016 Juan Linietsky, Ariel Manzur.                 */
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
#ifndef STREAM_EDITOR_PLUGIN_H
#define STREAM_EDITOR_PLUGIN_H


#include "tools/editor/editor_plugin.h"
#include "tools/editor/editor_node.h"
#include "scene/audio/stream_player.h"

/**
	@author Juan Linietsky <reduzio@gmail.com>
*/

class StreamEditor : public Control {

	OBJ_TYPE(StreamEditor, Control );

	Button * play;
	Button * stop;

	Panel *panel;
	Node *node;

	void _play();
	void _stop();
protected:
	void _notification(int p_what);
	void _node_removed(Node *p_node);
	static void _bind_methods();
public:

	void edit(Node *p_stream);
	StreamEditor();
};

class StreamEditorPlugin : public EditorPlugin {

	OBJ_TYPE( StreamEditorPlugin, EditorPlugin );

	StreamEditor *stream_editor;
	EditorNode *editor;

public:

	virtual String get_name() const { return "Stream"; }
	bool has_main_screen() const { return false; }
	virtual void edit(Object *p_node);
	virtual bool handles(Object *p_node) const;
	virtual void make_visible(bool p_visible);

	StreamEditorPlugin(EditorNode *p_node);
	~StreamEditorPlugin();

};

#endif // STREAM_EDITOR_PLUGIN_H
