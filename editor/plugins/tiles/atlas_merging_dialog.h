/*************************************************************************/
/*  atlas_merging_dialog.h                                               */
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

#ifndef ATLAS_MERGING_DIALOG_H
#define ATLAS_MERGING_DIALOG_H

#include "core/math/vector2i.h"
#include "core/object/object.h"
#include "core/object/ref_counted.h"
#include "core/string/string_name.h"
#include "core/string/ustring.h"
#include "core/templates/local_vector.h"
#include "core/templates/map.h"
#include "core/templates/vector.h"
#include "core/variant/variant.h"
#include "editor/editor_node.h"
#include "editor/editor_properties.h"
#include "scene/gui/dialogs.h"
#include "scene/gui/item_list.h"
#include "scene/gui/texture_rect.h"
#include "scene/resources/tile_set.h"

class Button;
class EditorFileDialog;
class EditorPropertyInteger;
class EditorPropertyVector2i;
class ItemList;
class Label;
class TextureRect;
class UndoRedo;

class AtlasMergingDialog : public ConfirmationDialog {
	GDCLASS(AtlasMergingDialog, ConfirmationDialog);

private:
	int commited_actions_count = 0;
	bool delete_original_atlases = true;
	Ref<TileSetAtlasSource> merged;
	LocalVector<Map<Vector2i, Vector2i>> merged_mapping;
	Ref<TileSet> tile_set;

	UndoRedo *undo_redo = EditorNode::get_singleton()->get_undo_redo();

	// Settings.
	int next_line_after_column = 30;

	// GUI.
	ItemList *atlas_merging_atlases_list;
	EditorPropertyVector2i *texture_region_size_editor_property;
	EditorPropertyInteger *columns_editor_property;
	TextureRect *preview;
	Label *select_2_atlases_label;
	EditorFileDialog *editor_file_dialog;
	Button *merge_button;

	void _property_changed(const StringName &p_property, const Variant &p_value, const String &p_field, bool p_changing);

	void _generate_merged(Vector<Ref<TileSetAtlasSource>> p_atlas_sources, int p_max_columns);
	void _update_texture();
	void _merge_confirmed(String p_path);

protected:
	virtual void ok_pressed() override;
	virtual void cancel_pressed() override;
	virtual void custom_action(const String &) override;

	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;

public:
	void update_tile_set(Ref<TileSet> p_tile_set);

	AtlasMergingDialog();
};

#endif // ATLAS_MERGING_DIALOG_H
