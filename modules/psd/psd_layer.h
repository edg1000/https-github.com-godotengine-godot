/**************************************************************************/
/*  psd_layer.h                                                           */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef PSD_LAYER_H
#define PSD_LAYER_H

#include "core/io/image.h"
#include "scene/resources/texture.h"
#include "psd_texture.h"

class PSDLayer : public Texture2D {
	GDCLASS(PSDLayer, Texture2D);
	RES_BASE_EXTENSION("psdl");

protected:
	Ref<PSDTexture> psd;
	String layer = "";

	static void _bind_methods();

public:
	virtual int get_width() const override;
	virtual int get_height() const override;
	virtual RID get_rid() const override;

	virtual bool has_alpha() const override;

	void set_psd_texture(const Ref<PSDTexture>& p_psd);
	Ref<PSDTexture> get_psd_texture() const;

	void set_layer(const String p_layer);
	String get_layer() const;

	virtual void draw(RID p_canvas_item, const Point2& p_pos, const Color& p_modulate = Color(1, 1, 1), bool p_transpose = false) const override;
	virtual void draw_rect(RID p_canvas_item, const Rect2& p_rect, bool p_tile = false, const Color& p_modulate = Color(1, 1, 1), bool p_transpose = false) const override;
	virtual void draw_rect_region(RID p_canvas_item, const Rect2& p_rect, const Rect2& p_src_rect, const Color& p_modulate = Color(1, 1, 1), bool p_transpose = false, bool p_clip_uv = true) const override;
	virtual bool get_rect_region(const Rect2& p_rect, const Rect2& p_src_rect, Rect2& r_rect, Rect2& r_src_rect) const override;

	bool is_pixel_opaque(int p_x, int p_y) const override;

	virtual Ref<Image> get_image() const override;

	PSDLayer();
};

#endif // PSD_LAYER_H
