/*************************************************************************/
/*  register_types.cpp                                                   */
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

#include "register_types.h"

#ifdef TOOLS_ENABLED
#include "navigation_mesh_editor_plugin.h"
#else
#include "core/engine.h"
#endif

#include "navigation_mesh_generator.h"

EditorNavigationMeshGenerator *_nav_mesh_generator = nullptr;

void register_recast_types() {
	ClassDB::APIType prev_api = ClassDB::get_current_api();
	ClassDB::set_current_api(ClassDB::API_EDITOR);

#ifdef TOOLS_ENABLED
	  EditorPlugins::add_by_type<NavigationMeshEditorPlugin>();
#endif
	_nav_mesh_generator = memnew(EditorNavigationMeshGenerator);

	ClassDB::register_class<EditorNavigationMeshGenerator>();

	Engine::get_singleton()->add_singleton(Engine::Singleton("NavigationMeshGenerator", EditorNavigationMeshGenerator::get_singleton()));

	ClassDB::set_current_api(prev_api);
}

void unregister_recast_types() {
	if (_nav_mesh_generator) {
		memdelete(_nav_mesh_generator);
	}
}

