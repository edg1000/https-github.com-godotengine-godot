/*************************************************************************/
/*  packed_scene_translation_parser_plugin.h                             */
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

#ifndef PACKED_SCENE_TRANSLATION_PARSER_PLUGIN_H
#define PACKED_SCENE_TRANSLATION_PARSER_PLUGIN_H

#include "editor/editor_translation_parser.h"

class PackedSceneEditorTranslationParserPlugin : public EditorTranslationParserPlugin {
	GDCLASS(PackedSceneEditorTranslationParserPlugin, EditorTranslationParserPlugin);

	// Scene Node's properties that contain translation strings.
	Set<String> lookup_properties;

public:
	virtual Error parse_file(const String &p_path, Vector<String> *r_extracted_strings) override;
	virtual void get_recognized_extensions(List<String> *r_extensions) const override;

	PackedSceneEditorTranslationParserPlugin();
};

#endif // PACKED_SCENE_TRANSLATION_PARSER_PLUGIN_H
