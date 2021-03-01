/*************************************************************************/
/*  snappers.h                                                           */
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

#ifndef SNAPPERS_H
#define SNAPPERS_H

#include "core/math/math_funcs.h"
#include "core/math/vector2.h"
#include "scene/main/advanced_settings.h"

// generic class for handling 2d snapping
class Snapper2D {
public:
	void snap(Vector2 &r_pos) const {
		if (!_enabled) {
			return;
		}

		r_pos.x = snap_value(r_pos.x, _snap_mode_x);
		r_pos.y = snap_value(r_pos.y, _snap_mode_y);
	}

	void set_snap_modes(AdvancedSettings::RoundMode p_mode_x, AdvancedSettings::RoundMode p_mode_y) {
		_snap_mode_x = p_mode_x;
		_snap_mode_y = p_mode_y;
		_enabled = !((_snap_mode_x == AdvancedSettings::ROUND_MODE_DISABLED) && (_snap_mode_y == AdvancedSettings::ROUND_MODE_DISABLED));
	}

	bool is_enabled() const { return _enabled; }

private:
	real_t snap_value(real_t p_value, AdvancedSettings::RoundMode p_mode) const {
		switch (p_mode) {
			default:
				break;
			case AdvancedSettings::ROUND_MODE_FLOOR: {
				return Math::floor(p_value);
			} break;
			case AdvancedSettings::ROUND_MODE_CEILING: {
				return Math::ceil(p_value);
			} break;
			case AdvancedSettings::ROUND_MODE_ROUND: {
				return Math::round(p_value);
			} break;
		}
		return p_value;
	}

	bool _enabled = false;
	AdvancedSettings::RoundMode _snap_mode_x = AdvancedSettings::ROUND_MODE_DISABLED;
	AdvancedSettings::RoundMode _snap_mode_y = AdvancedSettings::ROUND_MODE_DISABLED;
};

// All the 2D snapping in one place.
// This is called from the various places it needs to be introduced, but the logic
// can be self contained here to make it easier to change / debug.
class Snappers {
public:
	void initialize(bool p_gpu_snap);
	void set_stretch_mode(String p_mode);
	void set_transform_snap_2d(AdvancedSettings::Snap2DType p_type, AdvancedSettings::RoundMode p_mode_x, AdvancedSettings::RoundMode p_mode_y);

	// for positioning of sprites etc, not the main draw call
	void snap_read_item(Vector2 &r_pos) const;

	Snapper2D snapper_canvas_item_pre;
	Snapper2D snapper_canvas_item_post;
	Snapper2D snapper_canvas_item_read;

	Snapper2D snapper_viewport_pre;
	Snapper2D snapper_viewport_post;
	Snapper2D snapper_viewport_parent_pre;

private:
	// local version
	bool _gpu_snap_enabled = false;
	bool _snap_transforms_enabled = false;
	bool _stretch_mode_viewport = false;
};

#endif // SNAPPERS_H
