/*************************************************************************/
/*  navigation_path_query_parameters_2d.h                                */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
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

#ifndef NAVIGATION_PATH_QUERY_PARAMETERS_2D_H
#define NAVIGATION_PATH_QUERY_PARAMETERS_2D_H

#include "core/object/ref_counted.h"
#include "servers/navigation/navigation_utilities.h"

class NavigationPathQueryParameters2D : public RefCounted {
	GDCLASS(NavigationPathQueryParameters2D, RefCounted);

	NavigationUtilities::PathQueryParameters parameters;

protected:
	static void _bind_methods();

public:
	enum PathfindingAlgorithm {
		PATHFINDING_ALGORITHM_ASTAR = 0,
	};

	enum PathPostProcessing {
		PATH_POSTPROCESSING_CORRIDORFUNNEL = 0,
		PATH_POSTPROCESSING_EDGECENTERED,
	};

	const NavigationUtilities::PathQueryParameters &get_parameters() const { return parameters; }

	void set_pathfinding_algorithm(const PathfindingAlgorithm p_pathfinding_algorithm);
	PathfindingAlgorithm get_pathfinding_algorithm() const;

	void set_path_postprocessing(const PathPostProcessing p_path_postprocessing);
	PathPostProcessing get_path_postprocessing() const;

	void set_map(const RID &p_map);
	const RID &get_map() const;

	void set_start_position(const Vector2 p_start_position);
	Vector2 get_start_position() const;

	void set_target_position(const Vector2 p_target_position);
	Vector2 get_target_position() const;

	void set_navigation_layers(uint32_t p_navigation_layers);
	uint32_t get_navigation_layers() const;
};

VARIANT_ENUM_CAST(NavigationPathQueryParameters2D::PathfindingAlgorithm);
VARIANT_ENUM_CAST(NavigationPathQueryParameters2D::PathPostProcessing);

#endif // NAVIGATION_PATH_QUERY_PARAMETERS_2D_H
