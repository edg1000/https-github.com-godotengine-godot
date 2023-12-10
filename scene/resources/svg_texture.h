/**************************************************************************/
/*  svg_texture.h                                                         */
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

#ifndef SVG_TEXTURE_H
#define SVG_TEXTURE_H

#include "core/templates/lru.h"
#include "scene/resources/texture.h"

class BitMap;

class SVGTexture : public Texture2D {
	GDCLASS(SVGTexture, Texture2D);

	String source;
	float base_scale = 1.0;
	float saturation = 1.0;
	Dictionary color_map;
	Size2 size_override;

	class TextureCache : public RefCounted {
	public:
		RID scaled_texture;

		TextureCache() {}
		TextureCache(const RID &p_rid) {
			scaled_texture = p_rid;
		}
		~TextureCache() {
			if (scaled_texture.is_valid()) {
				RenderingServer::get_singleton()->free(scaled_texture);
			}
		}
	};

	mutable LRUCache<float, Ref<TextureCache>> scaled_textures;
	mutable Ref<BitMap> alpha_cache;
	mutable HashMap<Color, Color> cmap;
	mutable Size2 size;

	RID _ensure_scale(float p_scale, bool p_set_size = false) const;
	void _update_texture();

protected:
	static void _bind_methods();

public:
	static Ref<SVGTexture> create_from_string(const String &p_source, float p_scale = 1.0, float p_saturation = 1.0, const Dictionary &p_color_map = Dictionary());

	void set_source(const String &p_source);
	String get_source() const;

	void set_base_scale(float p_scale);
	float get_base_scale() const;

	void set_color_map(const Dictionary &p_color_map);
	Dictionary get_color_map() const;

	void set_saturation(float p_saturation);
	float get_saturation() const;

	Ref<Image> get_image() const override;

	int get_width() const override;
	int get_height() const override;

	virtual RID get_rid() const override;
	virtual RID get_scaled_rid(float p_scale) const;

	bool has_alpha() const override;
	virtual void draw(RID p_canvas_item, const Point2 &p_pos, const Color &p_modulate = Color(1, 1, 1), bool p_transpose = false) const override;
	virtual void draw_rect(RID p_canvas_item, const Rect2 &p_rect, bool p_tile = false, const Color &p_modulate = Color(1, 1, 1), bool p_transpose = false) const override;
	virtual void draw_rect_region(RID p_canvas_item, const Rect2 &p_rect, const Rect2 &p_src_rect, const Color &p_modulate = Color(1, 1, 1), bool p_transpose = false, bool p_clip_uv = true) const override;

	void set_size_override(const Size2i &p_size);
	bool is_pixel_opaque(int p_x, int p_y) const override;

	SVGTexture();
	~SVGTexture();
};

#endif // SVG_TEXTURE_H
