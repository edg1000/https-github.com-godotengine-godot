/*************************************************************************/
/*  gdscript_translation_parser_plugin.h                                 */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
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

#ifndef GDSCRIPT_TRANSLATION_PARSER_PLUGIN_H
#define GDSCRIPT_TRANSLATION_PARSER_PLUGIN_H

#include "editor/editor_translation_parser.h"
#include "modules/regex/regex.h"

class GDScriptEditorTranslationParserPlugin : public EditorTranslationParserPlugin {
	GDCLASS(GDScriptEditorTranslationParserPlugin, EditorTranslationParserPlugin);

	// Regex and search patterns that are used to match translation strings.
	const String text = "((?:[^\"\\\\]|\\\\[\\s\\S])*(?:\"[\\s\\\\]*\\+[\\s\\\\]*\"(?:[^\"\\\\]|\\\\[\\s\\S])*)*)";
	const String tr_pattern = "tr[\\s\\\\]*\\([\\s\\\\]*\"" + text + "\"(?:[\\s\\\\]*,[\\s\\\\]*\"" + text + "\")?[\\s\\\\]*\\)";
	const String trn_pattern = "tr_n[\\s\\\\]*\\([\\s\\\\]*\"" + text + "\"[\\s\\\\]*,[\\s\\\\]*\"" + text + "\".*?(?:\"" + text + "\"[\\s\\\\]*)?\\)";
	RegEx regex;
	Vector<String> patterns;
	Vector<String> file_dialog_patterns;

	void _parse_file_dialog(const String &p_source_code, Vector<String> *r_output);
	void _get_captured_strings(const Array &p_results, Vector<String> *r_output);
	void _handle_tr_pattern(const String &p_source_code, const String &p_pattern, Vector<Vector<String>> *r_ids_ctx_plural);

public:
	virtual Error parse_file(const String &p_path, Vector<String> *r_ids, Vector<Vector<String>> *r_ids_ctx_plural) override;
	virtual void get_recognized_extensions(List<String> *r_extensions) const override;

	GDScriptEditorTranslationParserPlugin();
};

#endif // GDSCRIPT_TRANSLATION_PARSER_PLUGIN_H
