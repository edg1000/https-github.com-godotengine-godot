/*************************************************************************/
/*  texture_progress.h                                                   */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                 */
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
#ifndef TEXTURE_PROGRESS_H
#define TEXTURE_PROGRESS_H

#include "scene/gui/range.h"

class TextureProgress : public Range {

	OBJ_TYPE( TextureProgress, Range  );

	bool expand;
	Color modulate;

	Ref<Texture> under;
	Ref<Texture> progress;
	Ref<Texture> over;

protected:

	static void _bind_methods();
	void _notification(int p_what);
public:

	void set_under_texture(const Ref<Texture>& p_texture);
	Ref<Texture> get_under_texture() const;

	void set_progress_texture(const Ref<Texture>& p_texture);
	Ref<Texture> get_progress_texture() const;

	void set_over_texture(const Ref<Texture>& p_texture);
	Ref<Texture> get_over_texture() const;

	Size2 get_minimum_size() const;

	void set_modulate(const Color& p_tex);
	Color get_modulate() const;

	void set_expand(bool p_expand);
	bool has_expand() const;

	TextureProgress();
};

#endif // TEXTURE_PROGRESS_H
