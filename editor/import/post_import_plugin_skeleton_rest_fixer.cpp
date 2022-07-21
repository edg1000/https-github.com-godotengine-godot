/*************************************************************************/
/*  post_import_plugin_skeleton_rest_fixer.cpp                           */
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

#include "post_import_plugin_skeleton_rest_fixer.h"

#include "editor/import/scene_import_settings.h"
#include "scene/3d/importer_mesh_instance_3d.h"
#include "scene/3d/skeleton_3d.h"
#include "scene/animation/animation_player.h"
#include "scene/resources/animation.h"
#include "scene/resources/bone_map.h"

void PostImportPluginSkeletonRestFixer::get_internal_import_options(InternalImportCategory p_category, List<ResourceImporter::ImportOption> *r_options) {
	if (p_category == INTERNAL_IMPORT_CATEGORY_SKELETON_3D_NODE) {
		r_options->push_back(ResourceImporter::ImportOption(PropertyInfo(Variant::BOOL, "retarget/rest_fixer/overwrite_axis"), true));

		r_options->push_back(ResourceImporter::ImportOption(PropertyInfo(Variant::BOOL, "retarget/rest_fixer/fix_silhouette/enable"), false));
		r_options->push_back(ResourceImporter::ImportOption(PropertyInfo(Variant::FLOAT, "retarget/rest_fixer/fix_silhouette/threshold"), 15));

		// TODO: PostImportPlugin need to be implemented such as validate_option(PropertyInfo &property, const Dictionary &p_options).
		// get_internal_option_visibility() is not sufficient because it can only retrieve options implemented in the core and can only read option values.
		// r_options->push_back(ResourceImporter::ImportOption(PropertyInfo(Variant::ARRAY, "retarget/rest_fixer/filter", PROPERTY_HINT_ARRAY_TYPE, vformat("%s/%s:%s", Variant::STRING_NAME, PROPERTY_HINT_ENUM, "Hips,Spine,Chest")), Array()));
		r_options->push_back(ResourceImporter::ImportOption(PropertyInfo(Variant::ARRAY, "retarget/rest_fixer/fix_silhouette/filter", PROPERTY_HINT_ARRAY_TYPE, "StringName"), Array()));
	}
}

void PostImportPluginSkeletonRestFixer::internal_process(InternalImportCategory p_category, Node *p_base_scene, Node *p_node, Ref<Resource> p_resource, const Dictionary &p_options) {
	if (p_category == INTERNAL_IMPORT_CATEGORY_SKELETON_3D_NODE) {
		// Prepare objects.
		Object *map = p_options["retarget/bone_map"].get_validated_object();
		if (!map) {
			return;
		}
		BoneMap *bone_map = Object::cast_to<BoneMap>(map);
		Ref<SkeletonProfile> profile = bone_map->get_profile();
		if (!profile.is_valid()) {
			return;
		}
		Skeleton3D *src_skeleton = Object::cast_to<Skeleton3D>(p_node);
		if (!src_skeleton) {
			return;
		}
		bool is_renamed = bool(p_options["retarget/bone_renamer/rename_bones"]);
		Array filter = p_options["retarget/rest_fixer/fix_silhouette/filter"];
		bool is_rest_changed = false;

		// Build profile skeleton.
		Skeleton3D *prof_skeleton = memnew(Skeleton3D);
		{
			int prof_bone_len = profile->get_bone_size();
			// Add single bones.
			for (int i = 0; i < prof_bone_len; i++) {
				prof_skeleton->add_bone(profile->get_bone_name(i));
				prof_skeleton->set_bone_rest(i, profile->get_reference_pose(i));
			}
			// Set parents.
			for (int i = 0; i < prof_bone_len; i++) {
				int parent = profile->find_bone(profile->get_bone_parent(i));
				if (parent >= 0) {
					prof_skeleton->set_bone_parent(i, parent);
				}
			}
		}

		// Complement Rotation track for compatibility between different rests.
		{
			TypedArray<Node> nodes = p_base_scene->find_children("*", "AnimationPlayer");
			while (nodes.size()) {
				AnimationPlayer *ap = Object::cast_to<AnimationPlayer>(nodes.pop_back());
				List<StringName> anims;
				ap->get_animation_list(&anims);
				for (const StringName &name : anims) {
					Ref<Animation> anim = ap->get_animation(name);
					int track_len = anim->get_track_count();

					// Detect does the animation have skeleton's TRS track.
					String track_path;
					bool found_skeleton = false;
					for (int i = 0; i < track_len; i++) {
						if (anim->track_get_path(i).get_subname_count() != 1 || !(anim->track_get_type(i) == Animation::TYPE_POSITION_3D || anim->track_get_type(i) == Animation::TYPE_ROTATION_3D || anim->track_get_type(i) == Animation::TYPE_SCALE_3D)) {
							continue;
						}
						track_path = String(anim->track_get_path(i).get_concatenated_names());
						Node *node = (ap->get_node(ap->get_root()))->get_node(NodePath(track_path));
						if (node) {
							Skeleton3D *track_skeleton = Object::cast_to<Skeleton3D>(node);
							if (track_skeleton && track_skeleton == src_skeleton) {
								found_skeleton = true;
								break;
							}
						}
					}

					if (found_skeleton) {
						// Search and insert rot track if it doesn't exist.
						for (int prof_idx = 0; prof_idx < prof_skeleton->get_bone_count(); prof_idx++) {
							String bone_name = is_renamed ? prof_skeleton->get_bone_name(prof_idx) : String(bone_map->get_skeleton_bone_name(prof_skeleton->get_bone_name(prof_idx)));
							if (bone_name == String()) {
								continue;
							}
							int src_idx = src_skeleton->find_bone(bone_name);
							if (src_idx == -1) {
								continue;
							}
							String insert_path = track_path + ":" + bone_name;
							int rot_track = anim->find_track(insert_path, Animation::TYPE_ROTATION_3D);
							if (rot_track == -1) {
								int track = anim->add_track(Animation::TYPE_ROTATION_3D);
								anim->track_set_path(track, insert_path);
								anim->rotation_track_insert_key(track, 0, src_skeleton->get_bone_rest(src_idx).basis.get_rotation_quaternion());
							}
						}
					}
				}
			}
		}

		// Fix silhouette.
		Vector<Transform3D> silhouette_diff; // Transform values to be ignored when overwrite axis.
		silhouette_diff.resize(src_skeleton->get_bone_count());
		Transform3D *silhouette_diff_w = silhouette_diff.ptrw();
		if (bool(p_options["retarget/rest_fixer/fix_silhouette/enable"])) {
			LocalVector<Transform3D> old_skeleton_global_rest;
			for (int i = 0; i < src_skeleton->get_bone_count(); i++) {
				old_skeleton_global_rest.push_back(src_skeleton->get_bone_global_rest(i));
			}

			Vector<int> bones_to_process = prof_skeleton->get_parentless_bones();
			while (bones_to_process.size() > 0) {
				int prof_idx = bones_to_process[0];
				bones_to_process.erase(prof_idx);
				Vector<int> prof_children = prof_skeleton->get_bone_children(prof_idx);
				for (int i = 0; i < prof_children.size(); i++) {
					bones_to_process.push_back(prof_children[i]);
				}

				// Calc virtual/looking direction with origins.
				bool is_filtered = false;
				for (int i = 0; i < filter.size(); i++) {
					if (String(filter[i]) == prof_skeleton->get_bone_name(prof_idx)) {
						is_filtered = true;
						break;
					}
				}
				if (is_filtered) {
					continue;
				}

				int src_idx = src_skeleton->find_bone(is_renamed ? prof_skeleton->get_bone_name(prof_idx) : String(bone_map->get_skeleton_bone_name(prof_skeleton->get_bone_name(prof_idx))));
				if (src_idx < 0 || profile->get_tail_direction(prof_idx) == SkeletonProfile::TAIL_DIRECTION_END) {
					continue;
				}
				Vector3 prof_tail;
				Vector3 src_tail;
				if (profile->get_tail_direction(prof_idx) == SkeletonProfile::TAIL_DIRECTION_AVERAGE_CHILDREN) {
					PackedInt32Array prof_bone_children = prof_skeleton->get_bone_children(prof_idx);
					int children_size = prof_bone_children.size();
					if (children_size == 0) {
						continue;
					}
					bool exist_all_children = true;
					for (int i = 0; i < children_size; i++) {
						int prof_child_idx = prof_bone_children[i];
						int src_child_idx = src_skeleton->find_bone(is_renamed ? prof_skeleton->get_bone_name(prof_child_idx) : String(bone_map->get_skeleton_bone_name(prof_skeleton->get_bone_name(prof_child_idx))));
						if (src_child_idx < 0) {
							exist_all_children = false;
							break;
						}
						prof_tail = prof_tail + prof_skeleton->get_bone_global_rest(prof_child_idx).origin;
						src_tail = src_tail + src_skeleton->get_bone_global_rest(src_child_idx).origin;
					}
					if (!exist_all_children) {
						continue;
					}
					prof_tail = prof_tail / children_size;
					src_tail = src_tail / children_size;
				}
				if (profile->get_tail_direction(prof_idx) == SkeletonProfile::TAIL_DIRECTION_SPECIFIC_CHILD) {
					int prof_tail_idx = prof_skeleton->find_bone(profile->get_bone_tail(prof_idx));
					if (prof_tail_idx < 0) {
						continue;
					}
					int src_tail_idx = src_skeleton->find_bone(is_renamed ? prof_skeleton->get_bone_name(prof_tail_idx) : String(bone_map->get_skeleton_bone_name(prof_skeleton->get_bone_name(prof_tail_idx))));
					if (src_tail_idx < 0) {
						continue;
					}
					prof_tail = prof_skeleton->get_bone_global_rest(prof_tail_idx).origin;
					src_tail = src_skeleton->get_bone_global_rest(src_tail_idx).origin;
				}

				Vector3 prof_head = prof_skeleton->get_bone_global_rest(prof_idx).origin;
				Vector3 src_head = src_skeleton->get_bone_global_rest(src_idx).origin;

				Vector3 prof_dir = prof_tail - prof_head;
				Vector3 src_dir = src_tail - src_head;

				// Rotate rest.
				if (Math::abs(Math::rad2deg(src_dir.angle_to(prof_dir))) > float(p_options["retarget/rest_fixer/fix_silhouette/threshold"])) {
					// Get rotation difference.
					Vector3 up_vec; // Need to rotate other than roll axis.
					switch (Vector3(abs(src_dir.x), abs(src_dir.y), abs(src_dir.z)).min_axis_index()) {
						case Vector3::AXIS_X: {
							up_vec = Vector3(1, 0, 0);
						} break;
						case Vector3::AXIS_Y: {
							up_vec = Vector3(0, 1, 0);
						} break;
						case Vector3::AXIS_Z: {
							up_vec = Vector3(0, 0, 1);
						} break;
					}
					Basis src_b;
					src_b = src_b.looking_at(src_dir, up_vec);
					Basis prof_b;
					prof_b = src_b.looking_at(prof_dir, up_vec);
					if (prof_b.is_equal_approx(Basis())) {
						continue; // May not need to rotate.
					}
					Basis diff_b = prof_b * src_b.inverse();

					// Apply rotation difference as global transform to skeleton.
					Basis src_pg;
					int src_parent = src_skeleton->get_bone_parent(src_idx);
					if (src_parent >= 0) {
						src_pg = src_skeleton->get_bone_global_rest(src_parent).basis;
					}
					Transform3D fixed_rest = Transform3D(src_pg.inverse() * diff_b * src_pg * src_skeleton->get_bone_rest(src_idx).basis, src_skeleton->get_bone_rest(src_idx).origin);
					src_skeleton->set_bone_rest(src_idx, fixed_rest);
				}
			}

			// For skin modification in overwrite rest.
			for (int i = 0; i < src_skeleton->get_bone_count(); i++) {
				silhouette_diff_w[i] = old_skeleton_global_rest[i] * src_skeleton->get_bone_global_rest(i).inverse();
			}

			is_rest_changed = true;
		}

		// Overwrite axis.
		if (bool(p_options["retarget/rest_fixer/overwrite_axis"])) {
			LocalVector<Transform3D> old_skeleton_rest;
			LocalVector<Transform3D> old_skeleton_global_rest;
			for (int i = 0; i < src_skeleton->get_bone_count(); i++) {
				old_skeleton_rest.push_back(src_skeleton->get_bone_rest(i));
				old_skeleton_global_rest.push_back(src_skeleton->get_bone_global_rest(i));
			}

			Vector<Basis> diffs;
			diffs.resize(src_skeleton->get_bone_count());
			Basis *diffs_w = diffs.ptrw();

			Vector<int> bones_to_process = src_skeleton->get_parentless_bones();
			while (bones_to_process.size() > 0) {
				int src_idx = bones_to_process[0];
				bones_to_process.erase(src_idx);
				Vector<int> src_children = src_skeleton->get_bone_children(src_idx);
				for (int i = 0; i < src_children.size(); i++) {
					bones_to_process.push_back(src_children[i]);
				}

				Basis tgt_rot;
				StringName src_bone_name = is_renamed ? StringName(src_skeleton->get_bone_name(src_idx)) : bone_map->find_profile_bone_name(src_skeleton->get_bone_name(src_idx));
				if (src_bone_name != StringName()) {
					Basis src_pg;
					int src_parent_idx = src_skeleton->get_bone_parent(src_idx);
					if (src_parent_idx >= 0) {
						src_pg = src_skeleton->get_bone_global_rest(src_parent_idx).basis;
					}

					int prof_idx = profile->find_bone(src_bone_name);
					if (prof_idx >= 0) {
						tgt_rot = src_pg.inverse() * prof_skeleton->get_bone_global_rest(prof_idx).basis; // Mapped bone uses reference pose.
					}
					/*
					// If there is rest-relative animation, this logic may be work fine, but currently not so...
					} else {
						// tgt_rot = src_pg.inverse() * old_skeleton_global_rest[src_idx].basis; // Non-Mapped bone keeps global rest.
					}
					*/
				}

				if (src_skeleton->get_bone_parent(src_idx) >= 0) {
					diffs_w[src_idx] = tgt_rot.inverse() * diffs[src_skeleton->get_bone_parent(src_idx)] * src_skeleton->get_bone_rest(src_idx).basis;
				} else {
					diffs_w[src_idx] = tgt_rot.inverse() * src_skeleton->get_bone_rest(src_idx).basis;
				}

				Basis diff;
				if (src_skeleton->get_bone_parent(src_idx) >= 0) {
					diff = diffs[src_skeleton->get_bone_parent(src_idx)];
				}
				src_skeleton->set_bone_rest(src_idx, Transform3D(tgt_rot, diff.xform(src_skeleton->get_bone_rest(src_idx).origin)));
			}

			// Fix skin.
			{
				TypedArray<Node> nodes = p_base_scene->find_children("*", "ImporterMeshInstance3D");
				while (nodes.size()) {
					ImporterMeshInstance3D *mi = Object::cast_to<ImporterMeshInstance3D>(nodes.pop_back());
					Ref<Skin> skin = mi->get_skin();
					if (skin.is_valid()) {
						Node *node = mi->get_node(mi->get_skeleton_path());
						if (node) {
							Skeleton3D *mesh_skeleton = Object::cast_to<Skeleton3D>(node);
							if (mesh_skeleton && node == src_skeleton) {
								int skin_len = skin->get_bind_count();
								for (int i = 0; i < skin_len; i++) {
									StringName bn = skin->get_bind_name(i);
									int bone_idx = src_skeleton->find_bone(bn);
									if (bone_idx >= 0) {
										Transform3D new_rest = silhouette_diff[i] * src_skeleton->get_bone_global_rest(bone_idx);
										skin->set_bind_pose(i, new_rest.inverse());
									}
								}
							}
						}
					}
				}
			}

			// Fix animation.
			{
				TypedArray<Node> nodes = p_base_scene->find_children("*", "AnimationPlayer");
				while (nodes.size()) {
					AnimationPlayer *ap = Object::cast_to<AnimationPlayer>(nodes.pop_back());
					List<StringName> anims;
					ap->get_animation_list(&anims);
					for (const StringName &name : anims) {
						Ref<Animation> anim = ap->get_animation(name);
						int track_len = anim->get_track_count();
						for (int i = 0; i < track_len; i++) {
							if (anim->track_get_path(i).get_subname_count() != 1 || anim->track_get_type(i) != Animation::TYPE_ROTATION_3D) {
								continue;
							}

							if (anim->track_is_compressed(i)) {
								continue; // TODO: Adopt to compressed track.
							}

							String track_path = String(anim->track_get_path(i).get_concatenated_names());
							Node *node = (ap->get_node(ap->get_root()))->get_node(NodePath(track_path));
							if (node) {
								Skeleton3D *track_skeleton = Object::cast_to<Skeleton3D>(node);
								if (track_skeleton && track_skeleton == src_skeleton) {
									StringName bn = anim->track_get_path(i).get_subname(0);
									if (bn) {
										int bone_idx = src_skeleton->find_bone(bn);

										Quaternion old_rest = old_skeleton_rest[bone_idx].basis.get_rotation_quaternion();
										Quaternion new_rest = src_skeleton->get_bone_rest(bone_idx).basis.get_rotation_quaternion();
										Quaternion old_pg;
										Quaternion new_pg;
										int parent_idx = src_skeleton->get_bone_parent(bone_idx);
										if (parent_idx >= 0) {
											old_pg = old_skeleton_global_rest[parent_idx].basis.get_rotation_quaternion();
											new_pg = src_skeleton->get_bone_global_rest(parent_idx).basis.get_rotation_quaternion();
										}

										int key_len = anim->track_get_key_count(i);
										for (int j = 0; j < key_len; j++) {
											Quaternion qt = static_cast<Quaternion>(anim->track_get_key_value(i, j));
											anim->track_set_key_value(i, j, new_pg.inverse() * old_pg * qt * old_rest.inverse() * old_pg.inverse() * new_pg * new_rest);
										}
									}
								}
							}
						}
					}
				}
			}

			is_rest_changed = true;
		}

		// Init skeleton pose to new rest.
		if (is_rest_changed) {
			for (int i = 0; i < src_skeleton->get_bone_count(); i++) {
				Transform3D fixed_rest = src_skeleton->get_bone_rest(i);
				src_skeleton->set_bone_pose_position(i, fixed_rest.origin);
				src_skeleton->set_bone_pose_rotation(i, fixed_rest.basis.get_rotation_quaternion());
				src_skeleton->set_bone_pose_scale(i, fixed_rest.basis.get_scale());
			}
		}

		memdelete(prof_skeleton);
	}
}

PostImportPluginSkeletonRestFixer::PostImportPluginSkeletonRestFixer() {
}
