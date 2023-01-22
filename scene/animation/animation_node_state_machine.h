/**************************************************************************/
/*  animation_node_state_machine.h                                        */
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

#ifndef ANIMATION_NODE_STATE_MACHINE_H
#define ANIMATION_NODE_STATE_MACHINE_H

#include "core/math/expression.h"
#include "scene/animation/animation_tree.h"

class AnimationNodeStateMachineTransition : public Resource {
	GDCLASS(AnimationNodeStateMachineTransition, Resource);

public:
	enum SwitchMode {
		SWITCH_MODE_IMMEDIATE,
		SWITCH_MODE_SYNC,
		SWITCH_MODE_AT_END,
	};

	enum AdvanceMode {
		ADVANCE_MODE_DISABLED,
		ADVANCE_MODE_ENABLED,
		ADVANCE_MODE_AUTO,
	};

private:
	SwitchMode switch_mode = SWITCH_MODE_IMMEDIATE;
	AdvanceMode advance_mode = ADVANCE_MODE_ENABLED;
	StringName advance_condition;
	StringName advance_condition_name;
	float xfade_time = 0.0;
	Ref<Curve> xfade_curve;
	bool reset = true;
	int priority = 1;
	String advance_expression;

	friend class AnimationNodeStateMachinePlayback;
	Ref<Expression> expression;

protected:
	static void _bind_methods();

public:
	void set_switch_mode(SwitchMode p_mode);
	SwitchMode get_switch_mode() const;

	void set_advance_mode(AdvanceMode p_mode);
	AdvanceMode get_advance_mode() const;

	void set_advance_condition(const StringName &p_condition);
	StringName get_advance_condition() const;

	StringName get_advance_condition_name() const;

	void set_advance_expression(const String &p_expression);
	String get_advance_expression() const;

	void set_xfade_time(float p_xfade);
	float get_xfade_time() const;

	void set_reset(bool p_reset);
	bool is_reset() const;

	void set_xfade_curve(const Ref<Curve> &p_curve);
	Ref<Curve> get_xfade_curve() const;

	void set_priority(int p_priority);
	int get_priority() const;

	AnimationNodeStateMachineTransition();
};

VARIANT_ENUM_CAST(AnimationNodeStateMachineTransition::SwitchMode)
VARIANT_ENUM_CAST(AnimationNodeStateMachineTransition::AdvanceMode)

class AnimationNodeStateMachine;

class AnimationNodeStateMachinePlayback : public Resource {
	GDCLASS(AnimationNodeStateMachinePlayback, Resource);

	friend class AnimationNodeStateMachine;

	struct AStarCost {
		float distance = 0.0;
		StringName prev;
	};

	struct Transition {
		StringName from;
		StringName to;
		StringName next;
	};

	double len_current = 0.0;
	double pos_current = 0.0;
	bool end_loop = false;

	StringName current;
	Transition current_transition;
	Ref<Curve> current_curve;
	bool force_auto_advance = false;

	StringName fading_from;
	float fading_time = 0.0;
	float fading_pos = 0.0;

	Vector<StringName> path;
	bool playing = false;

	StringName start_request;
	StringName travel_request;
	bool reset_request = false;
	bool reset_request_on_teleport = false;
	bool next_request = false;
	bool stop_request = false;

	bool _travel(AnimationNodeStateMachine *p_state_machine, const StringName &p_travel);
	void _start(const StringName &p_state);
	double _process(AnimationNodeStateMachine *p_state_machine, double p_time, bool p_seek, bool p_is_external_seeking);

	double process(AnimationNodeStateMachine *p_state_machine, double p_time, bool p_seek, bool p_is_external_seeking);

	bool _check_advance_condition(const Ref<AnimationNodeStateMachine> p_state_machine, const Ref<AnimationNodeStateMachineTransition> p_transition) const;

protected:
	static void _bind_methods();

public:
	void travel(const StringName &p_state, bool p_reset_on_teleport = true);
	void start(const StringName &p_state, bool p_reset = true);
	void next();
	void stop();
	bool is_playing() const;
	StringName get_current_node() const;
	StringName get_fading_from_node() const;
	Vector<StringName> get_travel_path() const;
	float get_current_play_pos() const;
	float get_current_length() const;

	AnimationNodeStateMachinePlayback();
};

class AnimationNodeStateMachine : public AnimationRootNode {
	GDCLASS(AnimationNodeStateMachine, AnimationRootNode);

private:
	friend class AnimationNodeStateMachinePlayback;

	struct State {
		Ref<AnimationRootNode> node;
		Vector2 position;
	};

	HashMap<StringName, State> states;

	struct Transition {
		StringName from;
		StringName to;
		StringName local_from;
		StringName local_to;
		Ref<AnimationNodeStateMachineTransition> transition;
	};

	Vector<Transition> transitions;

	StringName playback = "playback";
	StringName state_machine_name;
	AnimationNodeStateMachine *prev_state_machine = nullptr;
	bool updating_transitions = false;

	Vector2 graph_offset;

	void _tree_changed();
	void _remove_transition(const Ref<AnimationNodeStateMachineTransition> p_transition);
	void _rename_transitions(const StringName &p_name, const StringName &p_new_name);
	bool _can_connect(const StringName &p_name, Vector<AnimationNodeStateMachine *> p_parents = Vector<AnimationNodeStateMachine *>());
	StringName _get_shortest_path(const StringName &p_path) const;

protected:
	static void _bind_methods();

	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;
	bool _check_advance_condition(const Ref<AnimationNodeStateMachine> p_state_machine, const Ref<AnimationNodeStateMachineTransition> p_transition) const;

	virtual void reset_state() override;

public:
	StringName start_node = "Start";
	StringName end_node = "End";

	virtual void get_parameter_list(List<PropertyInfo> *r_list) const override;
	virtual Variant get_parameter_default_value(const StringName &p_parameter) const override;

	void add_node(const StringName &p_name, Ref<AnimationNode> p_node, const Vector2 &p_position = Vector2());
	void replace_node(const StringName &p_name, Ref<AnimationNode> p_node);
	Ref<AnimationNode> get_node(const StringName &p_name) const;
	void remove_node(const StringName &p_name);
	void rename_node(const StringName &p_name, const StringName &p_new_name);
	bool has_node(const StringName &p_name) const;
	StringName get_node_name(const Ref<AnimationNode> &p_node) const;
	void get_node_list(List<StringName> *r_nodes) const;

	void set_node_position(const StringName &p_name, const Vector2 &p_position);
	Vector2 get_node_position(const StringName &p_name) const;

	virtual void get_child_nodes(List<ChildNode> *r_child_nodes) override;

	bool has_transition(const StringName &p_from, const StringName &p_to) const;
	bool has_local_transition(const StringName &p_from, const StringName &p_to) const;
	int find_transition(const StringName &p_from, const StringName &p_to) const;
	void add_transition(const StringName &p_from, const StringName &p_to, const Ref<AnimationNodeStateMachineTransition> &p_transition);
	Ref<AnimationNodeStateMachineTransition> get_transition(int p_transition) const;
	StringName get_transition_from(int p_transition) const;
	StringName get_transition_to(int p_transition) const;
	int get_transition_count() const;
	void remove_transition_by_index(const int p_transition);
	void remove_transition(const StringName &p_from, const StringName &p_to);

	bool can_edit_node(const StringName &p_name) const;

	AnimationNodeStateMachine *get_prev_state_machine() const;

	void set_graph_offset(const Vector2 &p_offset);
	Vector2 get_graph_offset() const;

	virtual double process(double p_time, bool p_seek, bool p_is_external_seeking) override;
	virtual String get_caption() const override;

	virtual Ref<AnimationNode> get_child_by_name(const StringName &p_name) override;

	AnimationNodeStateMachine();
};

#endif // ANIMATION_NODE_STATE_MACHINE_H
