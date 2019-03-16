/*************************************************************************/
/*  test_plugin.h                                                        */
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

#ifndef TEST_PLUGIN_H
#define TEST_PLUGIN_H

#include "editor/editor_plugin.h"
#include "editor/editor_run.h"
#include "editor/script_editor_debugger.h"
#include "scene/gui/button.h"

class DebugButton : public Button {
	GDCLASS(DebugButton, Button);

public:

	DebugButton();
	virtual ~DebugButton();
	EditorRun::Status get_status() const;
	Error run();
	void run_native_notify() { m_editor_run.run_native_notify(); }
	void stop();

	OS::ProcessID get_pid() const { return m_editor_run.get_pid(); }

	void set_debug_collisions(bool p_debug);
	bool get_debug_collisions() const;

	void set_debug_navigation(bool p_debug);
	bool get_debug_navigation() const;

protected:
	static void _bind_methods();
	virtual void _notification(int p_what);

private:
	EditorRun m_editor_run;
	ScriptEditorDebugger *m_debugger;
};

class TestPanel : public HBoxContainer {
	GDCLASS(TestPanel, HBoxContainer);

public:
	TestPanel();

protected:
	static void _bind_methods();
};

class CoveragePanel : public HBoxContainer {
	GDCLASS(CoveragePanel, HBoxContainer);

public:
	CoveragePanel();

protected:
	static void _bind_methods();
};

class DocumentationPanel : public HBoxContainer {
	GDCLASS(DocumentationPanel, HBoxContainer);

public:
	DocumentationPanel();

protected:
	static void _bind_methods();
};

class TestPlugin : public EditorPlugin {
	GDCLASS(TestPlugin, EditorPlugin);

public:
	TestPlugin();
	virtual ~TestPlugin();

protected:
	static void _bind_methods();

private:
	DebugButton *m_debug_button;
	ToolButton *m_test_panel;
	ToolButton *m_coverage_panel;
	ToolButton *m_documentation_panel;
};

#endif // TEST_PLUGIN_H
