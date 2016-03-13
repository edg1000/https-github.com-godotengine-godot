/*************************************************************************/
/*  call_dialog.h                                                        */
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
#ifndef CALL_DIALOG_H
#define CALL_DIALOG_H

#include "scene/gui/popup.h"
#include "scene/gui/button.h"
#include "scene/gui/tree.h"
#include "scene/gui/label.h"
#include "scene/gui/line_edit.h"
#include "tools/editor/property_editor.h"
/**
	@author Juan Linietsky <reduzio@gmail.com>
*/


class CallDialogParams;

class CallDialog : public Popup {

	OBJ_TYPE( CallDialog, Popup );


	Label* method_label;
	Tree *tree;
	Button *call;
	Button *cancel;

	CallDialogParams *call_params;
	PropertyEditor *property_editor;

	Label *return_label;
	LineEdit *return_value;
	Object *object;
	StringName selected;

	Vector<MethodInfo> methods;


	void _item_selected();
	void _update_method_list();
	void _call();
	void _cancel();

protected:
	static void _bind_methods();
	void _notification(int p_what);
public:

	void set_object(Object *p_object,StringName p_selected="");

	CallDialog();
	~CallDialog();

};

#endif
