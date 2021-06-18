/*************************************************************************/
/*  script_debugger.cpp                                                  */
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

#include "script_debugger.h"

#include "core/debugger/engine_debugger.h"

void ScriptDebugger::set_lines_left(int p_left) {
	lines_left = p_left;
}

int ScriptDebugger::get_lines_left() const {
	return lines_left;
}

void ScriptDebugger::set_depth(int p_depth) {
	depth = p_depth;
}

int ScriptDebugger::get_depth() const {
	return depth;
}

void ScriptDebugger::insert_breakpoint(int p_line, const StringName &p_source) {
	if (!breakpoints.has(p_line)) {
		breakpoints[p_line] = Set<StringName>();
	}
	breakpoints[p_line].insert(p_source);
}

void ScriptDebugger::remove_breakpoint(int p_line, const StringName &p_source) {
	if (!breakpoints.has(p_line)) {
		return;
	}

	breakpoints[p_line].remove(p_source);
	if (breakpoints[p_line].size() == 0) {
		breakpoints.remove(p_line);
	}
}

bool ScriptDebugger::is_breakpoint(int p_line, const StringName &p_source) const {
	if (!breakpoints.has(p_line)) {
		return false;
	}
	return breakpoints[p_line].has(p_source);
}

bool ScriptDebugger::is_breakpoint_line(int p_line) const {
	return breakpoints.has(p_line);
}

String ScriptDebugger::breakpoint_find_source(const String &p_source) const {
	return p_source;
}

void ScriptDebugger::clear_breakpoints() {
	breakpoints.clear();
}

void ScriptDebugger::set_skip_breakpoints(bool p_skip_breakpoints) {
	skip_breakpoints = p_skip_breakpoints;
}

bool ScriptDebugger::is_skipping_breakpoints() {
	return skip_breakpoints;
}

void ScriptDebugger::debug(ScriptLanguage *p_lang, bool p_can_continue, bool p_is_error_breakpoint) {
	ScriptLanguage *prev = break_lang;
	break_lang = p_lang;
	EngineDebugger::get_singleton()->debug(p_can_continue, p_is_error_breakpoint);
	break_lang = prev;
}

void ScriptDebugger::send_error(const String &p_func, const String &p_file, int p_line, const String &p_err, const String &p_descr, ErrorHandlerType p_type, const Vector<StackInfo> &p_stack_info) {
	// Store stack info, this is ugly, but allows us to separate EngineDebugger and ScriptDebugger. There might be a better way.
	error_stack_info.append_array(p_stack_info);
	EngineDebugger::get_singleton()->send_error(p_func, p_file, p_line, p_err, p_descr, p_type);
	error_stack_info.resize(0);
}

Vector<ScriptLanguage::StackInfo> ScriptDebugger::get_error_stack_info() const {
	return error_stack_info;
}

ScriptLanguage *ScriptDebugger::get_break_language() const {
	return break_lang;
}
