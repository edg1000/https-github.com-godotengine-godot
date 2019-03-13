/*************************************************************************/
/*  animation_player.cpp                                                 */
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

#include "animation_player.h"

#include "core/engine.h"
#include "core/message_queue.h"
#include "scene/scene_string_names.h"
#include "servers/audio/audio_stream.h"

#include <iostream>

#ifdef TOOLS_ENABLED
void AnimatedValuesBackup::update_skeletons() {

	for (int i = 0; i < entries.size(); i++) {
		if (entries[i].bone_idx != -1) {
			Object::cast_to<Skeleton>(entries[i].object)->notification(Skeleton::NOTIFICATION_UPDATE_SKELETON);
		}
	}
}
#endif

bool AnimationPlayer::_set(const StringName &p_name, const Variant &p_value) {

	String name = p_name;

	if (name.begins_with("playback/play")) { // bw compatibility

		set_current_animation(p_value);

	} else if (name.begins_with("anims/")) {

		String which = name.get_slicec('/', 1);
		add_animation(which, p_value);

	} else if (name.begins_with("next/")) {

		String which = name.get_slicec('/', 1);
		animation_set_next(which, p_value);

	} else if (p_name == SceneStringNames::get_singleton()->blend_times) {

		Array array = p_value;
		int len = array.size();
		ERR_FAIL_COND_V(len % 3, false);

		for (int i = 0; i < len / 3; i++) {

			StringName from = array[i * 3 + 0];
			StringName to = array[i * 3 + 1];
			float time = array[i * 3 + 2];

			set_blend_time(from, to, time);
		}

	} else
		return false;

	return true;
}

bool AnimationPlayer::_get(const StringName &p_name, Variant &r_ret) const {

	String name = p_name;

	if (name == "playback/play") { // bw compatibility

		r_ret = get_current_animation();

	} else if (name.begins_with("anims/")) {

		String which = name.get_slicec('/', 1);
		r_ret = get_animation(which).get_ref_ptr();

	} else if (name.begins_with("next/")) {

		String which = name.get_slicec('/', 1);

		r_ret = animation_get_next(which);

	} else if (name == "blend_times") {

		Vector<BlendKey> keys;
		for (Map<BlendKey, float>::Element *E = blend_times.front(); E; E = E->next()) {

			keys.ordered_insert(E->key());
		}

		Array array;
		for (int i = 0; i < keys.size(); i++) {

			array.push_back(keys[i].from);
			array.push_back(keys[i].to);
			array.push_back(blend_times[keys[i]]);
		}

		r_ret = array;
	} else
		return false;

	return true;
}

void AnimationPlayer::_validate_property(PropertyInfo &property) const {

	if (property.name == "current_animation") {
		List<String> names;

		for (Map<StringName, AnimationData>::Element *E = animation_set.front(); E; E = E->next()) {
			names.push_back(E->key());
		}
		names.sort();
		names.push_front("[stop]");
		String hint;
		for (List<String>::Element *E = names.front(); E; E = E->next()) {

			if (E != names.front())
				hint += ",";
			hint += E->get();
		}

		property.hint_string = hint;
	}
}

void AnimationPlayer::_get_property_list(List<PropertyInfo> *p_list) const {

	List<PropertyInfo> anim_names;

	for (Map<StringName, AnimationData>::Element *E = animation_set.front(); E; E = E->next()) {

		anim_names.push_back(PropertyInfo(Variant::OBJECT, "anims/" + String(E->key()), PROPERTY_HINT_RESOURCE_TYPE, "Animation", PROPERTY_USAGE_NOEDITOR | PROPERTY_USAGE_INTERNAL | PROPERTY_USAGE_DO_NOT_SHARE_ON_DUPLICATE));
		if (E->get().next != StringName())
			anim_names.push_back(PropertyInfo(Variant::STRING, "next/" + String(E->key()), PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR | PROPERTY_USAGE_INTERNAL));
	}

	anim_names.sort();

	for (List<PropertyInfo>::Element *E = anim_names.front(); E; E = E->next()) {
		p_list->push_back(E->get());
	}

	p_list->push_back(PropertyInfo(Variant::ARRAY, "blend_times", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR | PROPERTY_USAGE_INTERNAL));
}

void AnimationPlayer::advance(float p_time) {

	_animation_process(p_time);
}

void AnimationPlayer::_notification(int p_what) {

	switch (p_what) {

		case NOTIFICATION_ENTER_TREE: {

			if (!processing) {
				//make sure that a previous process state was not saved
				//only process if "processing" is set
				set_physics_process_internal(false);
				set_process_internal(false);
			}
			//_set_process(false);
			clear_caches();
		} break;
		case NOTIFICATION_READY: {

			if (!Engine::get_singleton()->is_editor_hint() && animation_set.has(autoplay)) {
				play(autoplay);
				_animation_process(0);
			}
		} break;
		case NOTIFICATION_INTERNAL_PROCESS: {
			if (animation_process_mode == ANIMATION_PROCESS_PHYSICS)
				break;

			if (processing)
				_animation_process(get_process_delta_time());
		} break;
		case NOTIFICATION_INTERNAL_PHYSICS_PROCESS: {

			if (animation_process_mode == ANIMATION_PROCESS_IDLE)
				break;

			if (processing)
				_animation_process(get_physics_process_delta_time());
		} break;
		case NOTIFICATION_EXIT_TREE: {

			clear_caches();
		} break;
	}
}

void AnimationPlayer::_ensure_node_caches(AnimationData *p_anim) {

	// Already cached?
	if (p_anim->node_cache.size() == p_anim->animation->get_track_count())
		return;

	Node *parent = get_node(root);

	ERR_FAIL_COND(!parent);

	Animation *a = p_anim->animation.operator->();

	p_anim->node_cache.resize(a->get_track_count());

	for (int i = 0; i < a->get_track_count(); i++) {

		p_anim->node_cache.write[i] = NULL;
		RES resource;
		Vector<StringName> leftover_path;
		Node *child = parent->get_node_and_resource(a->track_get_path(i), resource, leftover_path);
		if (!child) {
			ERR_EXPLAIN("On Animation: '" + p_anim->name + "', couldn't resolve track:  '" + String(a->track_get_path(i)) + "'");
		}
		ERR_CONTINUE(!child); // couldn't find the child node
		uint32_t id = resource.is_valid() ? resource->get_instance_id() : child->get_instance_id();
		int bone_idx = -1;

		if (a->track_get_path(i).get_subname_count() == 1 && Object::cast_to<Skeleton>(child)) {

			Skeleton *sk = Object::cast_to<Skeleton>(child);
			bone_idx = sk->find_bone(a->track_get_path(i).get_subname(0));
			if (bone_idx == -1 || sk->is_bone_ignore_animation(bone_idx)) {

				continue;
			}
		}

		{
			if (!child->is_connected("tree_exiting", this, "_node_removed"))
				child->connect("tree_exiting", this, "_node_removed", make_binds(child), CONNECT_ONESHOT);
		}

		TrackNodeCacheKey key;
		key.id = id;
		key.bone_idx = bone_idx;

		if (!node_cache_map.has(key))
			node_cache_map[key] = TrackNodeCache();

		p_anim->node_cache.write[i] = &node_cache_map[key];
		p_anim->node_cache[i]->path = a->track_get_path(i);
		p_anim->node_cache[i]->node = child;
		p_anim->node_cache[i]->resource = resource;
		p_anim->node_cache[i]->node_2d = Object::cast_to<Node2D>(child);
		if (a->track_get_type(i) == Animation::TYPE_TRANSFORM) {
			// special cases and caches for transform tracks

			// cache spatial
			p_anim->node_cache[i]->spatial = Object::cast_to<Spatial>(child);
			// cache skeleton
			p_anim->node_cache[i]->skeleton = Object::cast_to<Skeleton>(child);
			if (p_anim->node_cache[i]->skeleton) {
				if (a->track_get_path(i).get_subname_count() == 1) {
					StringName bone_name = a->track_get_path(i).get_subname(0);

					p_anim->node_cache[i]->bone_idx = p_anim->node_cache[i]->skeleton->find_bone(bone_name);
					if (p_anim->node_cache[i]->bone_idx < 0) {
						// broken track (nonexistent bone)
						p_anim->node_cache[i]->skeleton = NULL;
						p_anim->node_cache[i]->spatial = NULL;
						ERR_CONTINUE(p_anim->node_cache[i]->bone_idx < 0);
					}
				} else {
					// no property, just use spatialnode
					p_anim->node_cache[i]->skeleton = NULL;
				}
			}
		}

		if (a->track_get_type(i) == Animation::TYPE_VALUE) {

			if (!p_anim->node_cache[i]->property_anim.has(a->track_get_path(i).get_concatenated_subnames())) {

				TrackNodeCache::PropertyAnim pa;
				pa.subpath = leftover_path;
				pa.object = resource.is_valid() ? (Object *)resource.ptr() : (Object *)child;
				pa.special = SP_NONE;
				pa.owner = p_anim->node_cache[i];
				if (false && p_anim->node_cache[i]->node_2d) {

					if (leftover_path.size() == 1 && leftover_path[0] == SceneStringNames::get_singleton()->transform_pos)
						pa.special = SP_NODE2D_POS;
					else if (leftover_path.size() == 1 && leftover_path[0] == SceneStringNames::get_singleton()->transform_rot)
						pa.special = SP_NODE2D_ROT;
					else if (leftover_path.size() == 1 && leftover_path[0] == SceneStringNames::get_singleton()->transform_scale)
						pa.special = SP_NODE2D_SCALE;
				}
				p_anim->node_cache[i]->property_anim[a->track_get_path(i).get_concatenated_subnames()] = pa;
			}
		}

		if (a->track_get_type(i) == Animation::TYPE_BEZIER && leftover_path.size()) {

			if (!p_anim->node_cache[i]->bezier_anim.has(a->track_get_path(i).get_concatenated_subnames())) {

				TrackNodeCache::BezierAnim ba;
				ba.bezier_property = leftover_path;
				ba.object = resource.is_valid() ? (Object *)resource.ptr() : (Object *)child;
				ba.owner = p_anim->node_cache[i];

				p_anim->node_cache[i]->bezier_anim[a->track_get_path(i).get_concatenated_subnames()] = ba;
			}
		}
	}
}

AnimationPlayer::BlockData AnimationPlayer::animation_track_get_overlapping(Animation *a, int p_track, int p_key, AnimationPlayer *player) {

	if ((p_key - 1) < 0) {
		return BlockData();
	}

	BlockData blockData_key_prev = get_animation_block(a, p_track, p_key - 1, player);
	BlockData blockData_key_curr = get_animation_block(a, p_track, p_key, player);

	float global_start_pos = blockData_key_curr.pos;

	float curr_end_pos = blockData_key_curr.pos + blockData_key_curr.length;
	if (a->track_get_key_count(p_track) > (p_key + 1)) {
		float pos_next = a->track_get_key_time(p_track, p_key + 1);
		curr_end_pos = MIN(curr_end_pos, pos_next);
	}

	float global_end_pos = MIN(blockData_key_prev.pos + blockData_key_prev.length, curr_end_pos);
	if (global_end_pos < global_start_pos) {
		return BlockData();
	}

	return BlockData(global_start_pos, global_end_pos - global_start_pos);
}

void AnimationPlayer::_animation_process_animation(AnimationData *p_anim, float p_time, float p_delta, float p_interp, bool p_is_current, bool p_seeked, bool p_started) {

	_ensure_node_caches(p_anim);
	ERR_FAIL_COND(p_anim->node_cache.size() != p_anim->animation->get_track_count());

	Animation *a = p_anim->animation.operator->();
	bool can_call = is_inside_tree() && !Engine::get_singleton()->is_editor_hint();

	for (int i = 0; i < a->get_track_count(); i++) {

		// If an animation changes this animation (or it animates itself)
		// we need to recreate our animation cache
		if (p_anim->node_cache.size() != a->get_track_count()) {
			_ensure_node_caches(p_anim);
		}

		TrackNodeCache *nc = p_anim->node_cache[i];

		if (!nc)
			continue; // no node cache for this track, skip it

		if (!a->track_is_enabled(i))
			continue; // do nothing if the track is disabled

		if (a->track_get_key_count(i) == 0)
			continue; // do nothing if track is empty

		switch (a->track_get_type(i)) {

			case Animation::TYPE_TRANSFORM: {

				if (!nc->spatial)
					continue;

				Vector3 loc;
				Quat rot;
				Vector3 scale;

				Error err = a->transform_track_interpolate(i, p_time, &loc, &rot, &scale);
				//ERR_CONTINUE(err!=OK); //used for testing, should be removed

				if (err != OK)
					continue;

				if (nc->accum_pass != accum_pass) {
					ERR_CONTINUE(cache_update_size >= NODE_CACHE_UPDATE_MAX);
					cache_update[cache_update_size++] = nc;
					nc->accum_pass = accum_pass;
					nc->loc_accum = loc;
					nc->rot_accum = rot;
					nc->scale_accum = scale;

				} else {

					nc->loc_accum = nc->loc_accum.linear_interpolate(loc, p_interp);
					nc->rot_accum = nc->rot_accum.slerp(rot, p_interp);
					nc->scale_accum = nc->scale_accum.linear_interpolate(scale, p_interp);
				}

			} break;
			case Animation::TYPE_VALUE: {

				if (!nc->node)
					continue;

				//StringName property=a->track_get_path(i).get_property();

				Map<StringName, TrackNodeCache::PropertyAnim>::Element *E = nc->property_anim.find(a->track_get_path(i).get_concatenated_subnames());
				ERR_CONTINUE(!E); //should it continue, or create a new one?

				TrackNodeCache::PropertyAnim *pa = &E->get();

				Animation::UpdateMode update_mode = a->value_track_get_update_mode(i);

				if (update_mode == Animation::UPDATE_CAPTURE) {

					if (p_started) {
						pa->capture = pa->object->get_indexed(pa->subpath);
					}

					int key_count = a->track_get_key_count(i);
					if (key_count == 0)
						continue; //eeh not worth it

					float first_key_time = a->track_get_key_time(i, 0);
					float transition = 1.0;
					int first_key = 0;

					if (first_key_time == 0.0) {
						//ignore, use for transition
						if (key_count == 1)
							continue; //with one key we can't do anything
						transition = a->track_get_key_transition(i, 0);
						first_key_time = a->track_get_key_time(i, 1);
						first_key = 1;
					}

					if (p_time < first_key_time) {
						float c = Math::ease(p_time / first_key_time, transition);
						Variant first_value = a->track_get_key_value(i, first_key);
						Variant interp_value;
						Variant::interpolate(pa->capture, first_value, c, interp_value);

						if (pa->accum_pass != accum_pass) {
							ERR_CONTINUE(cache_update_prop_size >= NODE_CACHE_UPDATE_MAX);
							cache_update_prop[cache_update_prop_size++] = pa;
							pa->value_accum = interp_value;
							pa->accum_pass = accum_pass;
						} else {
							Variant::interpolate(pa->value_accum, interp_value, p_interp, pa->value_accum);
						}

						continue; //handled
					}
				}

				if (update_mode == Animation::UPDATE_CONTINUOUS || update_mode == Animation::UPDATE_CAPTURE || (p_delta == 0 && update_mode == Animation::UPDATE_DISCRETE)) { //delta == 0 means seek

					Variant value = a->value_track_interpolate(i, p_time);

					if (value == Variant())
						continue;

					//thanks to trigger mode, this should be solved now..
					/*
					if (p_delta==0 && value.get_type()==Variant::STRING)
						continue; // doing this with strings is messy, should find another way
					*/
					if (pa->accum_pass != accum_pass) {
						ERR_CONTINUE(cache_update_prop_size >= NODE_CACHE_UPDATE_MAX);
						cache_update_prop[cache_update_prop_size++] = pa;
						pa->value_accum = value;
						pa->accum_pass = accum_pass;
					} else {
						Variant::interpolate(pa->value_accum, value, p_interp, pa->value_accum);
					}

				} else if (p_is_current && p_delta != 0) {

					List<int> indices;
					a->value_track_get_key_indices(i, p_time, p_delta, &indices);

					for (List<int>::Element *F = indices.front(); F; F = F->next()) {

						Variant value = a->track_get_key_value(i, F->get());
						switch (pa->special) {

							case SP_NONE: {
								bool valid;
								pa->object->set_indexed(pa->subpath, value, &valid); //you are not speshul
#ifdef DEBUG_ENABLED
								if (!valid) {
									ERR_PRINTS("Failed setting track value '" + String(pa->owner->path) + "'. Check if property exists or the type of key is valid. Animation '" + a->get_name() + "' at node '" + get_path() + "'.");
								}
#endif

							} break;
							case SP_NODE2D_POS: {
#ifdef DEBUG_ENABLED
								if (value.get_type() != Variant::VECTOR2) {
									ERR_PRINTS("Position key at time " + rtos(p_time) + " in Animation Track '" + String(pa->owner->path) + "' not of type Vector2(). Animation '" + a->get_name() + "' at node '" + get_path() + "'.");
								}
#endif
								static_cast<Node2D *>(pa->object)->set_position(value);
							} break;
							case SP_NODE2D_ROT: {
#ifdef DEBUG_ENABLED
								if (value.is_num()) {
									ERR_PRINTS("Rotation key at time " + rtos(p_time) + " in Animation Track '" + String(pa->owner->path) + "' not numerical. Animation '" + a->get_name() + "' at node '" + get_path() + "'.");
								}
#endif

								static_cast<Node2D *>(pa->object)->set_rotation(Math::deg2rad((double)value));
							} break;
							case SP_NODE2D_SCALE: {
#ifdef DEBUG_ENABLED
								if (value.get_type() != Variant::VECTOR2) {
									ERR_PRINTS("Scale key at time " + rtos(p_time) + " in Animation Track '" + String(pa->owner->path) + "' not of type Vector2()." + a->get_name() + "' at node '" + get_path() + "'.");
								}
#endif

								static_cast<Node2D *>(pa->object)->set_scale(value);
							} break;
						}
					}
				}

			} break;
			case Animation::TYPE_METHOD: {

				if (!nc->node)
					continue;
				if (p_delta == 0) {
					continue;
				}
				if (!p_is_current)
					break;

				List<int> indices;

				a->method_track_get_key_indices(i, p_time, p_delta, &indices);

				for (List<int>::Element *E = indices.front(); E; E = E->next()) {

					StringName method = a->method_track_get_name(i, E->get());
					Vector<Variant> params = a->method_track_get_params(i, E->get());

					int s = params.size();

					ERR_CONTINUE(s > VARIANT_ARG_MAX);
#ifdef DEBUG_ENABLED
					if (!nc->node->has_method(method)) {
						ERR_PRINTS("Invalid method call '" + method + "'. '" + a->get_name() + "' at node '" + get_path() + "'.");
					}
#endif

					if (can_call) {
						MessageQueue::get_singleton()->push_call(
								nc->node,
								method,
								s >= 1 ? params[0] : Variant(),
								s >= 2 ? params[1] : Variant(),
								s >= 3 ? params[2] : Variant(),
								s >= 4 ? params[3] : Variant(),
								s >= 5 ? params[4] : Variant());
					}
				}

			} break;
			case Animation::TYPE_BEZIER: {

				if (!nc->node)
					continue;

				Map<StringName, TrackNodeCache::BezierAnim>::Element *E = nc->bezier_anim.find(a->track_get_path(i).get_concatenated_subnames());
				ERR_CONTINUE(!E); //should it continue, or create a new one?

				TrackNodeCache::BezierAnim *ba = &E->get();

				float bezier = a->bezier_track_interpolate(i, p_time);
				if (ba->accum_pass != accum_pass) {
					ERR_CONTINUE(cache_update_bezier_size >= NODE_CACHE_UPDATE_MAX);
					cache_update_bezier[cache_update_bezier_size++] = ba;
					ba->bezier_accum = bezier;
					ba->accum_pass = accum_pass;
				} else {
					ba->bezier_accum = Math::lerp(ba->bezier_accum, bezier, p_interp);
				}

			} break;
			case Animation::TYPE_AUDIO: {
				if (!nc->node)
					continue;
				if (p_delta == 0) {
					continue;
				}

				bool stop = false;

				if (p_seeked)
					nc->audio_idx = -1;

				int idx = a->track_find_key(i, p_time);
				if (idx != -1) {
					Ref<AudioStream> stream = a->audio_track_get_key_stream(i, idx);
					if (stream.is_valid()) {
						BlockData audioblock = get_audio_play_block(a, i, idx, p_time);
						if (audioblock.length > 0) {
							if (nc->audio_idx != idx) {
								nc->audio_idx = idx;

								float local_pos = get_local_audio_pos(a, i, idx, p_time);

								nc->node->call("set_stream", stream);
								nc->node->call("play", local_pos);

								nc->audio_playing = true;
								playing_caches.insert(nc);

								nc->audio_start = audioblock.pos;
								nc->audio_len = audioblock.length;
							}
						} else {
							stop = true;
						}
					} else {
						stop = true;
					}
				}

				if (!stop && nc->audio_playing) {
					int dir = sgn(p_delta);
					if ((dir > 0 && p_time < nc->audio_start) || (dir > 0 && p_time >= a->get_length()) || (dir < 0 && p_time > nc->audio_start) || (dir < 0 && p_time <= 0)) {
						stop = true;

					} else {
						float audio_clamp_len = MAX(MIN((nc->audio_start + nc->audio_len), a->get_length()), 0);
						float value = audio_clamp_len - p_time;
						if (value <= 0) {
							stop = true;
						}
					}
				}

				if (stop) {
					//time to stop
					nc->node->call("stop");
					nc->audio_playing = false;
					playing_caches.erase(nc);
				}

			} break;
			case Animation::TYPE_ANIMATION: {
				AnimationPlayer *player = Object::cast_to<AnimationPlayer>(nc->node);
				if (!player)
					continue;

				bool stop = false;

				if (p_seeked)
					nc->animation_idx = -1;

				int idx = a->track_find_key(i, p_time);
				if (idx != -1) {
					StringName anim_name = a->animation_track_get_key_animation(i, idx);
					if (String(anim_name) != "[stop]" && player->has_animation(anim_name)) {
						BlockData animationblock = get_animation_play_block(a, i, idx, p_time, player);
						if (animationblock.length > 0) { //inside of clip
							if (p_delta == 0 || p_seeked) { //on seeking
								stop = true;

								float local_pos = get_local_animation_pos(a, i, idx, p_time);

								//set_blend_time(const StringName &p_animation1, const StringName &p_animation2, float p_time) {

								//float custom_blend = -1;
								//BlockData blockData = animation_track_get_overlapping(a, i, idx, player);
								//if (blockData.length > 0) {
								//	custom_blend = blockData.length - (p_time - blockData.pos); //custom_blend = blockData.length;
								//}

								player->set_assigned_animation(anim_name);
								player->seek(local_pos, true);
								//player->seek_blend(a, player, i, p_time, custom_blend, true);

							} else if (nc->animation_idx != idx) { //on playing
								nc->animation_idx = idx;

								float local_pos = get_local_animation_pos(a, i, idx, p_time);

								float custom_blend = -1;
								BlockData blockData = animation_track_get_overlapping(a, i, idx, player);
								if (blockData.length > 0) {
									custom_blend = blockData.length - (p_time - blockData.pos); //custom_blend = blockData.length;
								}

								player->play(anim_name, custom_blend);
								player->seek(local_pos);

								nc->animation_playing = true;
								playing_caches.insert(nc);

								nc->animation_start = animationblock.pos;
								nc->animation_len = animationblock.length;
							}

							/*
							BlockData blockData = animation_track_get_overlapping(a, i, idx, player);
							if (blockData.length > 0) {
								if (p_time > blockData.pos && p_time <= (blockData.pos + blockData.length)) {
									float value = Interpolate(blockData.pos, blockData.pos + blockData.length, 0.0, 1.0, p_time);
									if (value) {
										float local_pos_prev = get_local_animation_pos(a, i, idx - 1, p_time);
										float local_pos_curr = get_local_animation_pos(a, i, idx, p_time);

										float weight_prev = value;
										float weight_curr = 1.0 - value;

										//print_line(String("{0}, {1}, {2}, {3}").format(varray("BLOCK", idx, blockData.pos, blockData.length)));
										//print_line(String("{0}, {1}, {2}").format(varray("!", local_pos_prev, local_pos_curr)));
									}
								}
							}
							*/

						} else { //outside of clip
							if ((p_delta == 0 || p_seeked) || nc->animation_playing) {
								stop = true;

								//float custom_blend = -1;
								//BlockData blockData = animation_track_get_overlapping(a, i, idx, player);
								//if (blockData.length > 0) {
								//	custom_blend = blockData.length - (p_time - blockData.pos); //custom_blend = blockData.length;
								//}

								float local_length = get_local_animation_end_pos(a, i, idx, player);

								player->set_assigned_animation(anim_name);
								player->seek(local_length, true);
								//player->seek_blend(a, player, i, p_time, custom_blend, true);
							}
						}
					} else {
						stop = true;
					}
				}

				if (!stop && nc->animation_playing) {
					int dir = sgn(p_delta);
					if ((dir > 0 && p_time < nc->animation_start) || (dir > 0 && p_time >= a->get_length()) || (dir < 0 && p_time > nc->animation_start) || (dir < 0 && p_time <= 0)) {
						stop = true;

					} else {
						float animation_clamp_len = MAX(MIN((nc->animation_start + nc->animation_len), a->get_length()), 0);
						float value = animation_clamp_len - p_time;
						if (value <= 0) {
							stop = true;
						}
					}
				}

				if (stop) {
					if (playing_caches.has(nc)) {
						playing_caches.erase(nc);
						player->stop();
						nc->animation_playing = false;
					}
				}

			} break;
		}
	}
}

float AnimationPlayer::Interpolate(float p_min_in, float p_max_in, float p_min_out, float p_max_out, float p_value) {

	float normed_value = Normalize(p_value, p_min_in, p_max_in);
	if (normed_value < 0) {
		normed_value = 0;
	}
	if (normed_value > 1) {
		normed_value = 1;
	}

	return Lerp(p_min_out, p_max_out, normed_value);
}

float AnimationPlayer::Normalize(float p_value, float p_min_value, float p_max_value) {

	float normed_value = (1.0 / (p_max_value - p_min_value)) * (p_value - p_min_value);
	return normed_value;
}

float AnimationPlayer::Lerp(float p_min_value, float p_max_value, float p_alpha) {
	return p_min_value + (p_max_value - p_min_value) * p_alpha;
}

float AnimationPlayer::get_local_animation_pos(Animation *a, int p_track, int p_key, float p_time) {

	//return MAX(p_time - a->track_get_key_time(p_track, p_key), 0) + a->animation_track_get_key_start_offset(p_track, p_key);
	return (p_time - a->track_get_key_time(p_track, p_key)) + a->animation_track_get_key_start_offset(p_track, p_key);
}

float AnimationPlayer::get_local_animation_start_pos(Animation *a, int p_track, int p_key) {

	return a->animation_track_get_key_start_offset(p_track, p_key);
}

float AnimationPlayer::get_local_animation_end_pos(Animation *a, int p_track, int p_key, AnimationPlayer *player) {

	StringName anim_name = a->animation_track_get_key_animation(p_track, p_key);
	Ref<Animation> anim = player->get_animation(anim_name);
	if (String(anim_name) != "[stop]" && player->has_animation(anim_name)) {
		float end_ofs = a->animation_track_get_key_end_offset(p_track, p_key);
		float length = anim->get_length();

		return MIN(MAX(0, length - end_ofs), length);
	}

	return 0;
}

float AnimationPlayer::get_local_animation_length(Animation *a, int p_track, int p_key, float p_time, AnimationPlayer *player) {

	StringName anim_name = a->animation_track_get_key_animation(p_track, p_key);
	Ref<Animation> anim = player->get_animation(anim_name);
	if (String(anim_name) != "[stop]" && player->has_animation(anim_name)) {
		float start_ofs = a->animation_track_get_key_start_offset(p_track, p_key);
		float end_ofs = a->animation_track_get_key_end_offset(p_track, p_key);
		float length = anim->get_length();

		return MIN(MAX(0, length - start_ofs - end_ofs), length);
	}

	return 0;
}

AnimationPlayer::BlockData AnimationPlayer::get_animation_play_block(Animation *a, int p_track, int p_key, float p_time, AnimationPlayer *player) {

	BlockData blockData = get_animation_block(a, p_track, p_key, player);
	float local_pos = p_time - blockData.pos;
	float newLen = blockData.length - local_pos;

	return BlockData(p_time, newLen);
}

AnimationPlayer::BlockData AnimationPlayer::get_animation_block(Animation *a, int p_track, int p_key, AnimationPlayer *player) {

	StringName anim_name = a->animation_track_get_key_animation(p_track, p_key);
	Ref<Animation> anim = player->get_animation(anim_name);
	if (String(anim_name) != "[stop]" && player->has_animation(anim_name)) {
		float start_ofs = a->animation_track_get_key_start_offset(p_track, p_key);
		float end_ofs = a->animation_track_get_key_end_offset(p_track, p_key);
		float length = anim->get_length();

		length = MIN(MAX(0, length - start_ofs - end_ofs), length);
		return BlockData(a->track_get_key_time(p_track, p_key), length);
	}

	return BlockData(a->track_get_key_time(p_track, p_key), 0);
}

float AnimationPlayer::get_local_audio_pos(Animation *a, int p_track, int p_key, float p_time) {

	return (p_time - a->track_get_key_time(p_track, p_key)) + a->audio_track_get_key_start_offset(p_track, p_key);
	//return MAX(p_time - a->track_get_key_time(p_track, p_key), 0) + a->audio_track_get_key_start_offset(p_track, p_key);
}

AnimationPlayer::BlockData AnimationPlayer::get_audio_play_block(Animation *a, int p_track, int p_key, float p_time) {

	BlockData blockData = get_audio_block(a, p_track, p_key);
	float local_pos = p_time - blockData.pos;
	float newLen = blockData.length - local_pos;

	return BlockData(p_time, newLen);
}

AnimationPlayer::BlockData AnimationPlayer::get_audio_block(Animation *a, int p_track, int p_key) {

	Ref<AudioStream> stream = a->audio_track_get_key_stream(p_track, p_key);
	if (stream.is_valid()) {
		float start_ofs = a->audio_track_get_key_start_offset(p_track, p_key);
		float end_ofs = a->audio_track_get_key_end_offset(p_track, p_key);
		float length = stream->get_length();

		length = MIN(MAX(0, length - start_ofs - end_ofs), length);
		return BlockData(a->track_get_key_time(p_track, p_key), length);
	}

	return BlockData(a->track_get_key_time(p_track, p_key), 0);
}

void AnimationPlayer::_animation_process_data(PlaybackData &cd, float p_delta, float p_blend, bool p_seeked, bool p_started) {

	float delta = p_delta * speed_scale * cd.speed_scale;
	float next_pos = cd.pos + delta;

	float len = cd.from->animation->get_length();
	bool loop = cd.from->animation->has_loop();

	if (!loop) {

		if (next_pos < 0)
			next_pos = 0;
		else if (next_pos > len)
			next_pos = len;

		// fix delta
		delta = next_pos - cd.pos;

		if (&cd == &playback.current) {

			bool backwards = delta < 0;

			if (!backwards && cd.pos <= len && next_pos == len /*&& playback.blend.empty()*/) {
				//playback finished
				end_reached = true;
				end_notify = cd.pos < len; // Notify only if not already at the end
			}

			if (backwards && cd.pos >= 0 && next_pos == 0 /*&& playback.blend.empty()*/) {
				//playback finished
				end_reached = true;
				end_notify = cd.pos > 0; // Notify only if not already at the beginning
			}
		}

	} else {

		float looped_next_pos = Math::fposmod(next_pos, len);
		if (looped_next_pos == 0 && next_pos != 0) {
			// Loop multiples of the length to it, rather than 0
			// so state at time=length is previewable in the editor
			next_pos = len;

		} else {
			next_pos = looped_next_pos;
		}
	}

	//
	bool has_dir_changed = (sgn(delta) != sgn(next_pos - cd.pos));
	if (has_dir_changed || cd.pos == 0 || cd.pos == len) {
		p_seeked = true;
	}

	cd.pos = next_pos;

	_animation_process_animation(cd.from, cd.pos, delta, p_blend, &cd == &playback.current, p_seeked, p_started);
}
void AnimationPlayer::_animation_process2(float p_delta, bool p_started) {

	Playback &c = playback;

	accum_pass++;

	_animation_process_data(c.current, p_delta, 1.0f, c.seeked && p_delta != 0, p_started);
	if (p_delta != 0) {
		c.seeked = false;
	}

	List<Blend>::Element *prev = NULL;
	for (List<Blend>::Element *E = c.blend.back(); E; E = prev) {

		Blend &b = E->get();
		float blend = b.blend_left / b.blend_time;
		_animation_process_data(b.data, p_delta, blend, false, false);

		b.blend_left -= Math::absf(speed_scale * p_delta);

		prev = E->prev();
		if (b.blend_left < 0) {

			c.blend.erase(E);
		}
	}
}

void AnimationPlayer::_animation_update_transforms() {
	{
		Transform t;
		for (int i = 0; i < cache_update_size; i++) {

			TrackNodeCache *nc = cache_update[i];

			ERR_CONTINUE(nc->accum_pass != accum_pass);

			t.origin = nc->loc_accum;
			t.basis.set_quat_scale(nc->rot_accum, nc->scale_accum);
			if (nc->skeleton && nc->bone_idx >= 0) {

				nc->skeleton->set_bone_pose(nc->bone_idx, t);

			} else if (nc->spatial) {

				nc->spatial->set_transform(t);
			}
		}
	}

	cache_update_size = 0;

	for (int i = 0; i < cache_update_prop_size; i++) {

		TrackNodeCache::PropertyAnim *pa = cache_update_prop[i];

		ERR_CONTINUE(pa->accum_pass != accum_pass);

		switch (pa->special) {

			case SP_NONE: {
				bool valid;
				pa->object->set_indexed(pa->subpath, pa->value_accum, &valid); //you are not speshul
#ifdef DEBUG_ENABLED
				if (!valid) {
					ERR_PRINTS("Failed setting key at time " + rtos(playback.current.pos) + " in Animation '" + get_current_animation() + "' at Node '" + get_path() + "', Track '" + String(pa->owner->path) + "'. Check if property exists or the type of key is right for the property");
				}
#endif

			} break;
			case SP_NODE2D_POS: {
#ifdef DEBUG_ENABLED
				if (pa->value_accum.get_type() != Variant::VECTOR2) {
					ERR_PRINTS("Position key at time " + rtos(playback.current.pos) + " in Animation '" + get_current_animation() + "' at Node '" + get_path() + "', Track '" + String(pa->owner->path) + "' not of type Vector2()");
				}
#endif
				static_cast<Node2D *>(pa->object)->set_position(pa->value_accum);
			} break;
			case SP_NODE2D_ROT: {
#ifdef DEBUG_ENABLED
				if (pa->value_accum.is_num()) {
					ERR_PRINTS("Rotation key at time " + rtos(playback.current.pos) + " in Animation '" + get_current_animation() + "' at Node '" + get_path() + "', Track '" + String(pa->owner->path) + "' not numerical");
				}
#endif

				static_cast<Node2D *>(pa->object)->set_rotation(Math::deg2rad((double)pa->value_accum));
			} break;
			case SP_NODE2D_SCALE: {
#ifdef DEBUG_ENABLED
				if (pa->value_accum.get_type() != Variant::VECTOR2) {
					ERR_PRINTS("Scale key at time " + rtos(playback.current.pos) + " in Animation '" + get_current_animation() + "' at Node '" + get_path() + "', Track '" + String(pa->owner->path) + "' not of type Vector2()");
				}
#endif

				static_cast<Node2D *>(pa->object)->set_scale(pa->value_accum);
			} break;
		}
	}

	cache_update_prop_size = 0;

	for (int i = 0; i < cache_update_bezier_size; i++) {

		TrackNodeCache::BezierAnim *ba = cache_update_bezier[i];

		ERR_CONTINUE(ba->accum_pass != accum_pass);
		ba->object->set_indexed(ba->bezier_property, ba->bezier_accum);
	}

	cache_update_bezier_size = 0;
}

void AnimationPlayer::_animation_process(float p_delta) {

	if (playback.current.from) {

		end_reached = false;
		end_notify = false;
		_animation_process2(p_delta, playback.started);

		if (playback.started) {
			playback.started = false;
		}

		_animation_update_transforms();
		if (end_reached) {
			if (queued.size()) {
				String old = playback.assigned;
				play(queued.front()->get());
				String new_name = playback.assigned;
				queued.pop_front();
				if (end_notify)
					emit_signal(SceneStringNames::get_singleton()->animation_changed, old, new_name);
			} else {
				//stop();
				playing = false;
				_set_process(false);
				if (end_notify)
					emit_signal(SceneStringNames::get_singleton()->animation_finished, playback.assigned);
			}
			end_reached = false;
		}

	} else {
		_set_process(false);
	}
}

Error AnimationPlayer::add_animation(const StringName &p_name, const Ref<Animation> &p_animation) {

#ifdef DEBUG_ENABLED
	ERR_EXPLAIN("Invalid animation name: " + String(p_name));
	ERR_FAIL_COND_V(String(p_name).find("/") != -1 || String(p_name).find(":") != -1 || String(p_name).find(",") != -1 || String(p_name).find("[") != -1, ERR_INVALID_PARAMETER);
#endif

	ERR_FAIL_COND_V(p_animation.is_null(), ERR_INVALID_PARAMETER);

	if (animation_set.has(p_name)) {

		_unref_anim(animation_set[p_name].animation);
		animation_set[p_name].animation = p_animation;
		clear_caches();
	} else {

		AnimationData ad;
		ad.animation = p_animation;
		ad.name = p_name;
		animation_set[p_name] = ad;
	}

	_ref_anim(p_animation);
	_change_notify();
	return OK;
}

void AnimationPlayer::remove_animation(const StringName &p_name) {

	ERR_FAIL_COND(!animation_set.has(p_name));

	stop();
	_unref_anim(animation_set[p_name].animation);
	animation_set.erase(p_name);

	clear_caches();
	_change_notify();
}

void AnimationPlayer::_ref_anim(const Ref<Animation> &p_anim) {

	Ref<Animation>(p_anim)->connect(SceneStringNames::get_singleton()->tracks_changed, this, "_animation_changed", varray(), CONNECT_REFERENCE_COUNTED);
}

void AnimationPlayer::_unref_anim(const Ref<Animation> &p_anim) {

	Ref<Animation>(p_anim)->disconnect(SceneStringNames::get_singleton()->tracks_changed, this, "_animation_changed");
}

void AnimationPlayer::rename_animation(const StringName &p_name, const StringName &p_new_name) {

	ERR_FAIL_COND(!animation_set.has(p_name));
	ERR_FAIL_COND(String(p_new_name).find("/") != -1 || String(p_new_name).find(":") != -1);
	ERR_FAIL_COND(animation_set.has(p_new_name));

	stop();
	AnimationData ad = animation_set[p_name];
	ad.name = p_new_name;
	animation_set.erase(p_name);
	animation_set[p_new_name] = ad;

	List<BlendKey> to_erase;
	Map<BlendKey, float> to_insert;
	for (Map<BlendKey, float>::Element *E = blend_times.front(); E; E = E->next()) {

		BlendKey bk = E->key();
		BlendKey new_bk = bk;
		bool erase = false;
		if (bk.from == p_name) {
			new_bk.from = p_new_name;
			erase = true;
		}
		if (bk.to == p_name) {
			new_bk.to = p_new_name;
			erase = true;
		}

		if (erase) {
			to_erase.push_back(bk);
			to_insert[new_bk] = E->get();
		}
	}

	while (to_erase.size()) {

		blend_times.erase(to_erase.front()->get());
		to_erase.pop_front();
	}

	while (to_insert.size()) {
		blend_times[to_insert.front()->key()] = to_insert.front()->get();
		to_insert.erase(to_insert.front());
	}

	if (autoplay == p_name)
		autoplay = p_new_name;

	clear_caches();
	_change_notify();
}

bool AnimationPlayer::has_animation(const StringName &p_name) const {

	return animation_set.has(p_name);
}
Ref<Animation> AnimationPlayer::get_animation(const StringName &p_name) const {

	ERR_FAIL_COND_V(!animation_set.has(p_name), Ref<Animation>());

	const AnimationData &data = animation_set[p_name];

	return data.animation;
}
void AnimationPlayer::get_animation_list(List<StringName> *p_animations) const {

	List<String> anims;

	for (Map<StringName, AnimationData>::Element *E = animation_set.front(); E; E = E->next()) {

		anims.push_back(E->key());
	}

	anims.sort();

	for (List<String>::Element *E = anims.front(); E; E = E->next()) {

		p_animations->push_back(E->get());
	}
}

void AnimationPlayer::set_blend_time(const StringName &p_animation1, const StringName &p_animation2, float p_time) {

	ERR_FAIL_COND(p_time < 0);

	BlendKey bk;
	bk.from = p_animation1;
	bk.to = p_animation2;
	if (p_time == 0)
		blend_times.erase(bk);
	else
		blend_times[bk] = p_time;
}

float AnimationPlayer::get_blend_time(const StringName &p_animation1, const StringName &p_animation2) const {

	BlendKey bk;
	bk.from = p_animation1;
	bk.to = p_animation2;

	if (blend_times.has(bk))
		return blend_times[bk];
	else
		return 0;
}

void AnimationPlayer::queue(const StringName &p_name) {

	if (!is_playing())
		play(p_name);
	else
		queued.push_back(p_name);
}

PoolVector<String> AnimationPlayer::get_queue() {
	PoolVector<String> ret;
	for (List<StringName>::Element *E = queued.front(); E; E = E->next()) {
		ret.push_back(E->get());
	}

	return ret;
}

void AnimationPlayer::clear_queue() {
	queued.clear();
}

void AnimationPlayer::play_backwards(const StringName &p_name, float p_custom_blend) {

	play(p_name, p_custom_blend, -1, true);
}

void AnimationPlayer::play(const StringName &p_name, float p_custom_blend, float p_custom_scale, bool p_from_end) {

	StringName name = p_name;

	if (String(name) == "")
		name = playback.assigned;

	if (!animation_set.has(name)) {
		ERR_EXPLAIN("Animation not found: " + name);
		ERR_FAIL();
	}

	Playback &c = playback;

	if (c.current.from) {

		float blend_time = 0;
		// find if it can blend
		BlendKey bk;
		bk.from = c.current.from->name;
		bk.to = name;

		if (p_custom_blend >= 0) {
			blend_time = p_custom_blend;
		} else if (blend_times.has(bk)) {

			blend_time = blend_times[bk];
		} else {

			bk.from = "*";
			if (blend_times.has(bk)) {

				blend_time = blend_times[bk];
			} else {

				bk.from = c.current.from->name;
				bk.to = "*";

				if (blend_times.has(bk)) {

					blend_time = blend_times[bk];
				}
			}
		}

		if (p_custom_blend < 0 && blend_time == 0 && default_blend_time)
			blend_time = default_blend_time;
		if (blend_time > 0) {

			Blend b;
			b.data = c.current;
			b.blend_time = b.blend_left = blend_time;
			c.blend.push_back(b);
		}
	}

	_stop_playing_caches();

	c.current.from = &animation_set[name];

	if (c.assigned != name) { // reset
		c.current.pos = p_from_end ? c.current.from->animation->get_length() : 0;
	} else {
		if (p_from_end && c.current.pos == 0) {
			// Animation reset BUT played backwards, set position to the end
			c.current.pos = c.current.from->animation->get_length();
		} else if (!p_from_end && c.current.pos == c.current.from->animation->get_length()) {
			// Animation resumed but already ended, set position to the beggining
			c.current.pos = 0;
		}
	}

	c.current.speed_scale = p_custom_scale;
	c.assigned = name;
	c.seeked = false;
	c.started = true;

	if (!end_reached)
		queued.clear();
	_set_process(true); // always process when starting an animation
	playing = true;

	emit_signal(SceneStringNames::get_singleton()->animation_started, c.assigned);

	if (is_inside_tree() && Engine::get_singleton()->is_editor_hint())
		return; // no next in this case

	StringName next = animation_get_next(p_name);
	if (next != StringName() && animation_set.has(next)) {
		queue(next);
	}
}

bool AnimationPlayer::is_playing() const {

	return playing;
	/*
	if (playback.current.from==NULL)
		return false;

	float len=playback.current.from->animation->get_length();
	float pos = playback.current.pos;
	bool loop=playback.current.from->animation->has_loop();
	if (!loop && pos >= len) {
		return false;
	};

	return true;
	*/
}

void AnimationPlayer::set_current_animation(const String &p_anim) {

	if (p_anim == "[stop]" || p_anim == "") {
		stop();
	} else if (!is_playing() || playback.assigned != p_anim) {
		play(p_anim);
	} else {
		// Same animation, do not replay from start
	}
}

String AnimationPlayer::get_current_animation() const {

	return (is_playing() ? playback.assigned : "");
}

void AnimationPlayer::set_assigned_animation(const String &p_anim) {

	if (is_playing()) {
		play(p_anim);
	} else {
		ERR_FAIL_COND(!animation_set.has(p_anim));
		playback.current.pos = 0;
		playback.current.from = &animation_set[p_anim];
		playback.assigned = p_anim;
	}
}

String AnimationPlayer::get_assigned_animation() const {

	return playback.assigned;
}

void AnimationPlayer::stop(bool p_reset) {

	_stop_playing_caches();
	Playback &c = playback;
	c.blend.clear();
	if (p_reset) {
		c.current.from = NULL;
		c.current.speed_scale = 1;
		c.current.pos = 0;
	}
	_set_process(false);
	queued.clear();
	playing = false;
}

void AnimationPlayer::set_speed_scale(float p_speed) {

	speed_scale = p_speed;
}
float AnimationPlayer::get_speed_scale() const {

	return speed_scale;
}
float AnimationPlayer::get_playing_speed() const {

	if (!playing) {
		return 0;
	}
	return speed_scale * playback.current.speed_scale;
}

void AnimationPlayer::seek(float p_time, bool p_update) {

	if (!playback.current.from) {
		if (playback.assigned) {
			ERR_FAIL_COND(!animation_set.has(playback.assigned));
			playback.current.from = &animation_set[playback.assigned];
		}
		ERR_FAIL_COND(!playback.current.from);
	}

	playback.current.pos = p_time;
	playback.seeked = true;
	if (p_update) {
		_animation_process(0);
	}
}

//, float p_custom_scale, bool p_from_end
void AnimationPlayer::seek_blend(Animation *a, AnimationPlayer *player, int p_track, float p_time, float p_custom_blend, bool p_update) {
	int idx = a->track_find_key(p_track, p_time);
	if ((idx - 1) < 0) {
		StringName anim_name = a->animation_track_get_key_animation(p_track, idx);
		if (String(anim_name) != "[stop]" && has_animation(anim_name)) {
			set_assigned_animation(anim_name);

			float local_pos = get_local_animation_pos(a, p_track, idx, p_time);
			seek(local_pos, p_update);
		}

	} else {
		float local_pos = get_local_animation_pos(a, p_track, idx, p_time);

		float custom_blend = -1;
		BlockData blockData = animation_track_get_overlapping(a, p_track, idx, player);
		if (blockData.length > 0) {
			custom_blend = blockData.length - (p_time - blockData.pos); //custom_blend = blockData.length;
		}

		StringName anim_name = a->animation_track_get_key_animation(p_track, idx);
		player->stop();
		player->play(anim_name, custom_blend);
		player->seek(local_pos, true);
		

		//return;

		/*
		StringName curr_anim = a->animation_track_get_key_animation(p_track, idx);
		StringName prev_anim = a->animation_track_get_key_animation(p_track, idx - 1);

		ERR_FAIL_COND(!animation_set.has(prev_anim));

		playback.current.from = &animation_set[prev_anim];
		playback.assigned = curr_anim;

		playback.current.pos = p_time;

		Playback &c = playback;

		if (c.current.from) {

			float blend_time = 0;
			// find if it can blend
			BlendKey bk;
			bk.from = c.current.from->name;
			bk.to = curr_anim;

			if (p_custom_blend >= 0) {
				blend_time = p_custom_blend;
			} else if (blend_times.has(bk)) {

				blend_time = blend_times[bk];
			} else {

				bk.from = "*";
				if (blend_times.has(bk)) {

					blend_time = blend_times[bk];
				} else {

					bk.from = c.current.from->name;
					bk.to = "*";

					if (blend_times.has(bk)) {

						blend_time = blend_times[bk];
					}
				}
			}

			if (p_custom_blend < 0 && blend_time == 0 && default_blend_time)
				blend_time = default_blend_time;
			if (blend_time > 0) {

				Blend b;
				b.data = c.current;
				b.blend_time = b.blend_left = blend_time;
				c.blend.push_back(b);
			}
		}

		//_stop_playing_caches();

		//c.current.from = &animation_set[name];

		bool p_from_end = false;
		float p_custom_scale = 1.0;

		if (c.assigned != curr_anim) { // reset
			c.current.pos = p_from_end ? c.current.from->animation->get_length() : 0;
		} else {
			if (p_from_end && c.current.pos == 0) {
				// Animation reset BUT played backwards, set position to the end
				c.current.pos = c.current.from->animation->get_length();
			} else if (!p_from_end && c.current.pos == c.current.from->animation->get_length()) {
				// Animation resumed but already ended, set position to the beggining
				c.current.pos = 0;
			}
		}

		c.current.pos = p_time;
		c.current.speed_scale = p_custom_scale;
		c.assigned = curr_anim;
		c.seeked = true;
		//c.started = true;

		//if (!end_reached)
		//	queued.clear();

		if (p_update) {
			_animation_process(0);
		}
		*/
	}
}

void AnimationPlayer::seek_delta(float p_time, float p_delta) {

	if (!playback.current.from) {
		if (playback.assigned) {
			ERR_FAIL_COND(!animation_set.has(playback.assigned));
			playback.current.from = &animation_set[playback.assigned];
		}
		ERR_FAIL_COND(!playback.current.from);
	}

	playback.current.pos = p_time - p_delta;
	if (speed_scale != 0.0)
		p_delta /= speed_scale;
	_animation_process(p_delta);
	//playback.current.pos=p_time;
}

bool AnimationPlayer::is_valid() const {

	return (playback.current.from);
}

float AnimationPlayer::get_current_animation_position() const {

	ERR_FAIL_COND_V(!playback.current.from, 0);
	return playback.current.pos;
}

float AnimationPlayer::get_current_animation_length() const {

	ERR_FAIL_COND_V(!playback.current.from, 0);
	return playback.current.from->animation->get_length();
}

void AnimationPlayer::_animation_changed() {

	clear_caches();
	emit_signal("caches_cleared");
	if (is_playing()) {
		playback.seeked = true; //need to restart stuff, like audio
	}
}

void AnimationPlayer::_stop_playing_caches() {

	for (Set<TrackNodeCache *>::Element *E = playing_caches.front(); E; E = E->next()) {

		if (E->get()->node && E->get()->audio_playing) {
			E->get()->node->call("stop");
		}
		if (E->get()->node && E->get()->animation_playing) {
			AnimationPlayer *player = Object::cast_to<AnimationPlayer>(E->get()->node);
			if (!player)
				continue;
			player->stop();
		}
	}

	playing_caches.clear();
}

void AnimationPlayer::_node_removed(Node *p_node) {

	clear_caches(); // nodes contained here ar being removed, clear the caches
}

void AnimationPlayer::clear_caches() {

	_stop_playing_caches();

	node_cache_map.clear();

	for (Map<StringName, AnimationData>::Element *E = animation_set.front(); E; E = E->next()) {

		E->get().node_cache.clear();
	}

	cache_update_size = 0;
	cache_update_prop_size = 0;
	cache_update_bezier_size = 0;
}

void AnimationPlayer::set_active(bool p_active) {

	if (active == p_active)
		return;

	active = p_active;
	_set_process(processing, true);
}

bool AnimationPlayer::is_active() const {

	return active;
}

StringName AnimationPlayer::find_animation(const Ref<Animation> &p_animation) const {

	for (Map<StringName, AnimationData>::Element *E = animation_set.front(); E; E = E->next()) {

		if (E->get().animation == p_animation)
			return E->key();
	}

	return "";
}

void AnimationPlayer::set_autoplay(const String &p_name) {
	if (is_inside_tree() && !Engine::get_singleton()->is_editor_hint())
		WARN_PRINT("Setting autoplay after the node has been added to the scene has no effect.");

	autoplay = p_name;
}

String AnimationPlayer::get_autoplay() const {

	return autoplay;
}

void AnimationPlayer::set_animation_process_mode(AnimationProcessMode p_mode) {

	if (animation_process_mode == p_mode)
		return;

	bool pr = processing;
	if (pr)
		_set_process(false);
	animation_process_mode = p_mode;
	if (pr)
		_set_process(true);
}

AnimationPlayer::AnimationProcessMode AnimationPlayer::get_animation_process_mode() const {

	return animation_process_mode;
}

void AnimationPlayer::_set_process(bool p_process, bool p_force) {

	if (processing == p_process && !p_force)
		return;

	switch (animation_process_mode) {

		case ANIMATION_PROCESS_PHYSICS: set_physics_process_internal(p_process && active); break;
		case ANIMATION_PROCESS_IDLE: set_process_internal(p_process && active); break;
		case ANIMATION_PROCESS_MANUAL: break;
	}

	processing = p_process;
}

void AnimationPlayer::animation_set_next(const StringName &p_animation, const StringName &p_next) {

	ERR_FAIL_COND(!animation_set.has(p_animation));
	animation_set[p_animation].next = p_next;
}

StringName AnimationPlayer::animation_get_next(const StringName &p_animation) const {

	if (!animation_set.has(p_animation))
		return StringName();
	return animation_set[p_animation].next;
}

void AnimationPlayer::set_default_blend_time(float p_default) {

	default_blend_time = p_default;
}

float AnimationPlayer::get_default_blend_time() const {

	return default_blend_time;
}

void AnimationPlayer::set_root(const NodePath &p_root) {

	root = p_root;
	clear_caches();
}

NodePath AnimationPlayer::get_root() const {

	return root;
}

void AnimationPlayer::get_argument_options(const StringName &p_function, int p_idx, List<String> *r_options) const {

	String pf = p_function;
	if (p_function == "play" || p_function == "play_backwards" || p_function == "remove_animation" || p_function == "has_animation" || p_function == "queue") {
		List<StringName> al;
		get_animation_list(&al);
		for (List<StringName>::Element *E = al.front(); E; E = E->next()) {

			r_options->push_back("\"" + String(E->get()) + "\"");
		}
	}
	Node::get_argument_options(p_function, p_idx, r_options);
}

#ifdef TOOLS_ENABLED
AnimatedValuesBackup AnimationPlayer::backup_animated_values() {

	if (!playback.current.from)
		return AnimatedValuesBackup();

	_ensure_node_caches(playback.current.from);

	AnimatedValuesBackup backup;

	for (int i = 0; i < playback.current.from->node_cache.size(); i++) {
		TrackNodeCache *nc = playback.current.from->node_cache[i];
		if (!nc)
			continue;

		if (nc->skeleton) {
			if (nc->bone_idx == -1)
				continue;

			AnimatedValuesBackup::Entry entry;
			entry.object = nc->skeleton;
			entry.bone_idx = nc->bone_idx;
			entry.value = nc->skeleton->get_bone_pose(nc->bone_idx);
			backup.entries.push_back(entry);
		} else {
			if (nc->spatial) {
				AnimatedValuesBackup::Entry entry;
				entry.object = nc->spatial;
				entry.subpath.push_back("transform");
				entry.value = nc->spatial->get_transform();
				entry.bone_idx = -1;
				backup.entries.push_back(entry);
			} else {
				for (Map<StringName, TrackNodeCache::PropertyAnim>::Element *E = nc->property_anim.front(); E; E = E->next()) {
					AnimatedValuesBackup::Entry entry;
					entry.object = E->value().object;
					entry.subpath = E->value().subpath;
					bool valid;
					entry.value = E->value().object->get_indexed(E->value().subpath, &valid);
					entry.bone_idx = -1;
					if (valid)
						backup.entries.push_back(entry);
				}
			}
		}
	}

	return backup;
}

void AnimationPlayer::restore_animated_values(const AnimatedValuesBackup &p_backup) {

	for (int i = 0; i < p_backup.entries.size(); i++) {

		const AnimatedValuesBackup::Entry *entry = &p_backup.entries[i];
		if (entry->bone_idx == -1) {
			entry->object->set_indexed(entry->subpath, entry->value);
		} else {
			Object::cast_to<Skeleton>(entry->object)->set_bone_pose(entry->bone_idx, entry->value);
		}
	}
}
#endif

void AnimationPlayer::_bind_methods() {

	ClassDB::bind_method(D_METHOD("_node_removed"), &AnimationPlayer::_node_removed);
	ClassDB::bind_method(D_METHOD("_animation_changed"), &AnimationPlayer::_animation_changed);

	ClassDB::bind_method(D_METHOD("add_animation", "name", "animation"), &AnimationPlayer::add_animation);
	ClassDB::bind_method(D_METHOD("remove_animation", "name"), &AnimationPlayer::remove_animation);
	ClassDB::bind_method(D_METHOD("rename_animation", "name", "newname"), &AnimationPlayer::rename_animation);
	ClassDB::bind_method(D_METHOD("has_animation", "name"), &AnimationPlayer::has_animation);
	ClassDB::bind_method(D_METHOD("get_animation", "name"), &AnimationPlayer::get_animation);
	ClassDB::bind_method(D_METHOD("get_animation_list"), &AnimationPlayer::_get_animation_list);

	ClassDB::bind_method(D_METHOD("animation_set_next", "anim_from", "anim_to"), &AnimationPlayer::animation_set_next);
	ClassDB::bind_method(D_METHOD("animation_get_next", "anim_from"), &AnimationPlayer::animation_get_next);

	ClassDB::bind_method(D_METHOD("set_blend_time", "anim_from", "anim_to", "sec"), &AnimationPlayer::set_blend_time);
	ClassDB::bind_method(D_METHOD("get_blend_time", "anim_from", "anim_to"), &AnimationPlayer::get_blend_time);

	ClassDB::bind_method(D_METHOD("set_default_blend_time", "sec"), &AnimationPlayer::set_default_blend_time);
	ClassDB::bind_method(D_METHOD("get_default_blend_time"), &AnimationPlayer::get_default_blend_time);

	ClassDB::bind_method(D_METHOD("play", "name", "custom_blend", "custom_speed", "from_end"), &AnimationPlayer::play, DEFVAL(""), DEFVAL(-1), DEFVAL(1.0), DEFVAL(false));
	ClassDB::bind_method(D_METHOD("play_backwards", "name", "custom_blend"), &AnimationPlayer::play_backwards, DEFVAL(""), DEFVAL(-1));
	ClassDB::bind_method(D_METHOD("stop", "reset"), &AnimationPlayer::stop, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("is_playing"), &AnimationPlayer::is_playing);

	ClassDB::bind_method(D_METHOD("set_current_animation", "anim"), &AnimationPlayer::set_current_animation);
	ClassDB::bind_method(D_METHOD("get_current_animation"), &AnimationPlayer::get_current_animation);
	ClassDB::bind_method(D_METHOD("set_assigned_animation", "anim"), &AnimationPlayer::set_assigned_animation);
	ClassDB::bind_method(D_METHOD("get_assigned_animation"), &AnimationPlayer::get_assigned_animation);
	ClassDB::bind_method(D_METHOD("queue", "name"), &AnimationPlayer::queue);
	ClassDB::bind_method(D_METHOD("get_queue"), &AnimationPlayer::get_queue);
	ClassDB::bind_method(D_METHOD("clear_queue"), &AnimationPlayer::clear_queue);

	ClassDB::bind_method(D_METHOD("set_active", "active"), &AnimationPlayer::set_active);
	ClassDB::bind_method(D_METHOD("is_active"), &AnimationPlayer::is_active);

	ClassDB::bind_method(D_METHOD("set_speed_scale", "speed"), &AnimationPlayer::set_speed_scale);
	ClassDB::bind_method(D_METHOD("get_speed_scale"), &AnimationPlayer::get_speed_scale);
	ClassDB::bind_method(D_METHOD("get_playing_speed"), &AnimationPlayer::get_playing_speed);

	ClassDB::bind_method(D_METHOD("set_autoplay", "name"), &AnimationPlayer::set_autoplay);
	ClassDB::bind_method(D_METHOD("get_autoplay"), &AnimationPlayer::get_autoplay);

	ClassDB::bind_method(D_METHOD("set_root", "path"), &AnimationPlayer::set_root);
	ClassDB::bind_method(D_METHOD("get_root"), &AnimationPlayer::get_root);

	ClassDB::bind_method(D_METHOD("find_animation", "animation"), &AnimationPlayer::find_animation);

	ClassDB::bind_method(D_METHOD("clear_caches"), &AnimationPlayer::clear_caches);

	ClassDB::bind_method(D_METHOD("set_animation_process_mode", "mode"), &AnimationPlayer::set_animation_process_mode);
	ClassDB::bind_method(D_METHOD("get_animation_process_mode"), &AnimationPlayer::get_animation_process_mode);

	ClassDB::bind_method(D_METHOD("get_current_animation_position"), &AnimationPlayer::get_current_animation_position);
	ClassDB::bind_method(D_METHOD("get_current_animation_length"), &AnimationPlayer::get_current_animation_length);

	ClassDB::bind_method(D_METHOD("seek", "seconds", "update"), &AnimationPlayer::seek, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("advance", "delta"), &AnimationPlayer::advance);

	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "root_node"), "set_root", "get_root");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "current_animation", PROPERTY_HINT_ENUM, "", PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_ANIMATE_AS_TRIGGER), "set_current_animation", "get_current_animation");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "assigned_animation", PROPERTY_HINT_NONE, "", 0), "set_assigned_animation", "get_assigned_animation");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "autoplay", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR), "set_autoplay", "get_autoplay");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "current_animation_length", PROPERTY_HINT_NONE, "", 0), "", "get_current_animation_length");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "current_animation_position", PROPERTY_HINT_NONE, "", 0), "", "get_current_animation_position");

	ADD_GROUP("Playback Options", "playback_");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "playback_process_mode", PROPERTY_HINT_ENUM, "Physics,Idle,Manual"), "set_animation_process_mode", "get_animation_process_mode");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "playback_default_blend_time", PROPERTY_HINT_RANGE, "0,4096,0.01"), "set_default_blend_time", "get_default_blend_time");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "playback_active", PROPERTY_HINT_NONE, "", 0), "set_active", "is_active");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "playback_speed", PROPERTY_HINT_RANGE, "-64,64,0.01"), "set_speed_scale", "get_speed_scale");

	ADD_SIGNAL(MethodInfo("animation_finished", PropertyInfo(Variant::STRING, "anim_name")));
	ADD_SIGNAL(MethodInfo("animation_changed", PropertyInfo(Variant::STRING, "old_name"), PropertyInfo(Variant::STRING, "new_name")));
	ADD_SIGNAL(MethodInfo("animation_started", PropertyInfo(Variant::STRING, "anim_name")));
	ADD_SIGNAL(MethodInfo("caches_cleared"));

	BIND_ENUM_CONSTANT(ANIMATION_PROCESS_PHYSICS);
	BIND_ENUM_CONSTANT(ANIMATION_PROCESS_IDLE);
	BIND_ENUM_CONSTANT(ANIMATION_PROCESS_MANUAL);
}

AnimationPlayer::AnimationPlayer() {

	accum_pass = 1;
	cache_update_size = 0;
	cache_update_prop_size = 0;
	cache_update_bezier_size = 0;
	speed_scale = 1;
	end_reached = false;
	end_notify = false;
	animation_process_mode = ANIMATION_PROCESS_IDLE;
	processing = false;
	default_blend_time = 0;
	root = SceneStringNames::get_singleton()->path_pp;
	playing = false;
	active = true;
	playback.seeked = false;
	playback.started = false;
}

AnimationPlayer::~AnimationPlayer() {
}
