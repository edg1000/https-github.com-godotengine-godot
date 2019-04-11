/*************************************************************************/
/*  sprite.h                                                             */
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

#ifndef SPRITE_H
#define SPRITE_H

#include "scene/2d/node_2d.h"
#include "scene/resources/texture.h"

class Sprite : public Node2D {

	GDCLASS(Sprite, Node2D);

	Ref<Texture> texture;
	Ref<Texture> normal_map;
	Size2 prev_scale;

	bool centered;
	Point2 offset;

	bool hflip;
	bool vflip;
	bool region;
	Rect2 region_rect;
	bool region_filter_clip;

	int frame;

	int vframes;
	int hframes;

	void _get_rects(Rect2 &r_src_rect, Rect2 &r_dst_rect, bool &r_filter_clip) const;

protected:
	void _notification(int p_what);

	static void _bind_methods();

	virtual void _validate_property(PropertyInfo &property) const;

	virtual void _changed_callback(Object *p_changed, const char *p_prop);

public:
	virtual Dictionary _edit_get_state() const;
	virtual void _edit_set_state(const Dictionary &p_state);

	virtual void _edit_set_pivot(const Point2 &p_pivot);
	virtual Point2 _edit_get_pivot() const;
	virtual bool _edit_use_pivot() const;
	virtual bool _edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const;

	bool is_pixel_opaque(const Point2 &p_point) const;

	virtual Rect2 _edit_get_rect() const;
	virtual bool _edit_use_rect() const;

	void set_texture(const Ref<Texture> &p_texture);
	Ref<Texture> get_texture() const;

	void set_normal_map(const Ref<Texture> &p_texture);
	Ref<Texture> get_normal_map() const;

	void set_pixel_size(const Size2 &p_size);
	Size2 get_pixel_size() const;

	void set_centered(bool p_center);
	bool is_centered() const;

	void set_offset(const Point2 &p_offset);
	Point2 get_offset() const;

	void set_flip_h(bool p_flip);
	bool is_flipped_h() const;

	void set_flip_v(bool p_flip);
	bool is_flipped_v() const;

	void set_region(bool p_region);
	bool is_region() const;

	void set_region_filter_clip(bool p_enable);
	bool is_region_filter_clip_enabled() const;

	void set_region_rect(const Rect2 &p_region_rect);
	Rect2 get_region_rect() const;

	void set_frame(int p_frame);
	int get_frame() const;

	void set_vframes(int p_amount);
	int get_vframes() const;

	void set_hframes(int p_amount);
	int get_hframes() const;

	Rect2 get_rect() const;
	virtual Rect2 get_anchorable_rect() const;

	Sprite();
	~Sprite();
};

#endif // SPRITE_H
