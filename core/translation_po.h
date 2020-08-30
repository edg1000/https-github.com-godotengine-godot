/*************************************************************************/
/*  translation_po.h                                                     */
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

#ifndef TRANSLATION_PO_H
#define TRANSLATION_PO_H

//#define DEBUG_TRANSLATION_PO

#include "core/math/expression.h"
#include "core/translation.h"

class TranslationPO : public Translation {
	GDCLASS(TranslationPO, Translation);

	// TLDR: Maps context to a list of source strings and translated strings. In PO terms, maps msgctxt to a list of msgid and msgstr.
	// The first key corresponds to context, and the second key (of the contained HashMap) corresponds to source string.
	// The value Vector<StringName> in the second map stores the translated strings. Index 0, 1, 2 matches msgstr[0], msgstr[1], msgstr[2]... in the case of plurals.
	// Otherwise index 0 mathes to msgstr in a singular translation.
	// Strings without context have "" as first key.
	HashMap<StringName, HashMap<StringName, Vector<StringName>>> translation_map;

	Vector<String> _get_message_list() const override;
	Dictionary _get_messages() const override;
	void _set_messages(const Dictionary &p_messages) override;

public:
	void get_message_list(List<StringName> *r_messages) const override;
	int get_message_count() const override;
	void add_message(const StringName &p_src_text, const StringName &p_xlated_text, const StringName &p_context = "") override;
	void add_plural_message(const StringName &p_src_text, const Vector<String> &p_plural_xlated_texts, const StringName &p_context = "") override;
	StringName get_message(const StringName &p_src_text, const StringName &p_context = "") const override;
	StringName get_plural_message(const StringName &p_src_text, const StringName &p_plural_text, int p_n, const StringName &p_context = "") const override;
	void erase_message(const StringName &p_src_text, const StringName &p_context = "") override;

#ifdef DEBUG_TRANSLATION_PO
	void print_translation_map();
#endif

	TranslationPO() {}
};

#endif // TRANSLATION_PO_H
