/*************************************************************************/
/*  texture_editor_plugin.h                                              */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2017 Juan Linietsky, Ariel Manzur.                 */
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
#ifndef TEXTURE_EDITOR_PLUGIN_H
#define TEXTURE_EDITOR_PLUGIN_H



#include "tools/editor/editor_plugin.h"
#include "tools/editor/editor_node.h"
#include "scene/resources/texture.h"


class TextureEditor : public Control {

	GDCLASS(TextureEditor, Control);


	Ref<Texture> texture;

protected:
	void _notification(int p_what);
	void _gui_input(InputEvent p_event);
	static void _bind_methods();
public:

	void edit(Ref<Texture> p_texture);
	TextureEditor();
};


class TextureEditorPlugin : public EditorPlugin {

	GDCLASS( TextureEditorPlugin, EditorPlugin );

	TextureEditor *texture_editor;
	EditorNode *editor;

public:

	virtual String get_name() const { return "Texture"; }
	bool has_main_screen() const { return false; }
	virtual void edit(Object *p_node);
	virtual bool handles(Object *p_node) const;
	virtual void make_visible(bool p_visible);

	TextureEditorPlugin(EditorNode *p_node);
	~TextureEditorPlugin();

};

#endif // TEXTURE_EDITOR_PLUGIN_H
