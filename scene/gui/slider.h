/**************************************************************************/
/*  slider.h                                                              */
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

#ifndef SLIDER_H
#define SLIDER_H

#include "scene/gui/range.h"

class JoypadHelper;

class Slider : public Range {
	GDCLASS(Slider, Range);

	struct Grab {
		int pos = 0;
		double uvalue = 0.0; // Value at `pos`.
		double value_before_dragging = 0.0;
		bool active = false;
	} grab;

	int ticks = 0;
	bool mouse_inside = false;
	Orientation orientation;
	double custom_step = -1.0;
	bool editable = true;
	bool scrollable = true;

	Ref<JoypadHelper> helper;
	void _value_move(const Vector2i &p_movement);

	struct ThemeCache {
		Ref<StyleBox> slider_style;
		Ref<StyleBox> grabber_area_style;
		Ref<StyleBox> grabber_area_hl_style;

		Ref<Texture2D> grabber_icon;
		Ref<Texture2D> grabber_hl_icon;
		Ref<Texture2D> grabber_disabled_icon;
		Ref<Texture2D> tick_icon;

		bool center_grabber = false;
		int grabber_offset = 0;
	} theme_cache;

protected:
	bool ticks_on_borders = false;

	virtual void gui_input(const Ref<InputEvent> &p_event) override;
	void _notification(int p_what);
	static void _bind_methods();

public:
	virtual Size2 get_minimum_size() const override;

	void set_custom_step(double p_custom_step);
	double get_custom_step() const;

	void set_ticks(int p_count);
	int get_ticks() const;

	void set_ticks_on_borders(bool);
	bool get_ticks_on_borders() const;

	void set_editable(bool p_editable);
	bool is_editable() const;

	void set_scrollable(bool p_scrollable);
	bool is_scrollable() const;

	Slider(Orientation p_orientation = VERTICAL);
	~Slider();
};

class HSlider : public Slider {
	GDCLASS(HSlider, Slider);

public:
	HSlider() :
			Slider(HORIZONTAL) { set_v_size_flags(0); }
};

class VSlider : public Slider {
	GDCLASS(VSlider, Slider);

public:
	VSlider() :
			Slider(VERTICAL) { set_h_size_flags(0); }
};

#endif // SLIDER_H
