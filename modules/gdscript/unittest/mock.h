/*************************************************************************/
/*  mock.h                                                               */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md)    */
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

#ifndef MOCK_H
#define MOCK_H

#include "method_override.h"

#include "gdscript.h"

#include "scene/main/node.h"

class Mock : public Node {
	GDCLASS(Mock, Node);

public:
	Mock(Ref<GDScriptNativeClass> p_base = NULL);
	virtual ~Mock();

	void bind_method(const String &p_name, const Variant& value);
	void add_property(const String &p_name, const StringName setter, const StringName getter);

	virtual Variant getvar(const Variant &p_key, bool *r_valid = NULL) const;
	virtual void setvar(const Variant &p_key, const Variant &p_value, bool *r_valid = NULL);
	virtual Variant call(const StringName &p_method, const Variant **p_args, int p_argcount, Variant::CallError &r_error);

protected:
	static void _bind_methods();

private:
	Object *m_instance;
	Ref<MethodOverride> m_override;

	Variant _handle_signal(const Variant **p_args, int p_argcount, Variant::CallError &r_error);
};

#endif // MOCK_H
