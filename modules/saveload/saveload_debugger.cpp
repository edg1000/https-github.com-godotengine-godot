/**************************************************************************/
/*  saveload_debugger.cpp                                                 */
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

#include "saveload_debugger.h"

#include "saveload_synchronizer.h"
#include "scene_saveload_config.h"

#include "core/debugger/engine_debugger.h"
#include "scene/main/node.h"

List<Ref<EngineProfiler>> saveload_profilers;

void SaveloadDebugger::initialize() {
	Ref<BandwidthProfiler> bandwidth;
	bandwidth.instantiate();
	bandwidth->bind("saveload:bandwidth");
	saveload_profilers.push_back(bandwidth);

	Ref<SaveloadProfiler> saveload_profiler;
	saveload_profiler.instantiate();
	saveload_profiler->bind("saveload:saveload");
	saveload_profilers.push_back(saveload_profiler);

	EngineDebugger::register_message_capture("saveload", EngineDebugger::Capture(nullptr, &_capture));
}

void SaveloadDebugger::deinitialize() {
	saveload_profilers.clear();
}

Error SaveloadDebugger::_capture(void *p_user, const String &p_msg, const Array &p_args, bool &r_captured) {
	if (p_msg == "cache") {
		Array out;
		for (int i = 0; i < p_args.size(); i++) {
			ObjectID id = p_args[i].operator ObjectID();
			Object *obj = ObjectDB::get_instance(id);
			ERR_CONTINUE(!obj);
			if (Object::cast_to<SceneSaveloadConfig>(obj)) {
				out.push_back(id);
				out.push_back(obj->get_class());
				out.push_back(((SceneSaveloadConfig *)obj)->get_path());
			} else if (Object::cast_to<Node>(obj)) {
				out.push_back(id);
				out.push_back(obj->get_class());
				out.push_back(String(((Node *)obj)->get_path()));
			} else {
				ERR_FAIL_V(FAILED);
			}
		}
		EngineDebugger::get_singleton()->send_message("saveload:cache", out);
		return OK;
	}
	ERR_FAIL_V(FAILED);
}

// BandwidthProfiler

int SaveloadDebugger::BandwidthProfiler::bandwidth_usage(const Vector<BandwidthFrame> &p_buffer, int p_pointer) {
	ERR_FAIL_COND_V(p_buffer.size() == 0, 0);
	int total_bandwidth = 0;

	uint64_t timestamp = OS::get_singleton()->get_ticks_msec();
	uint64_t final_timestamp = timestamp - 1000;

	int i = (p_pointer + p_buffer.size() - 1) % p_buffer.size();

	while (i != p_pointer && p_buffer[i].packet_size > 0) {
		if (p_buffer[i].timestamp < final_timestamp) {
			return total_bandwidth;
		}
		total_bandwidth += p_buffer[i].packet_size;
		i = (i + p_buffer.size() - 1) % p_buffer.size();
	}

	ERR_FAIL_COND_V_MSG(i == p_pointer, total_bandwidth, "Reached the end of the bandwidth profiler buffer, values might be inaccurate.");
	return total_bandwidth;
}

void SaveloadDebugger::BandwidthProfiler::toggle(bool p_enable, const Array &p_opts) {
	if (!p_enable) {
		bandwidth_in.clear();
		bandwidth_out.clear();
	} else {
		bandwidth_in_ptr = 0;
		bandwidth_in.resize(16384); // ~128kB
		for (int i = 0; i < bandwidth_in.size(); ++i) {
			bandwidth_in.write[i].packet_size = -1;
		}
		bandwidth_out_ptr = 0;
		bandwidth_out.resize(16384); // ~128kB
		for (int i = 0; i < bandwidth_out.size(); ++i) {
			bandwidth_out.write[i].packet_size = -1;
		}
	}
}

void SaveloadDebugger::BandwidthProfiler::add(const Array &p_data) {
	ERR_FAIL_COND(p_data.size() < 3);
	const String inout = p_data[0];
	int time = p_data[1];
	int size = p_data[2];
	if (inout == "in") {
		bandwidth_in.write[bandwidth_in_ptr].timestamp = time;
		bandwidth_in.write[bandwidth_in_ptr].packet_size = size;
		bandwidth_in_ptr = (bandwidth_in_ptr + 1) % bandwidth_in.size();
	} else if (inout == "out") {
		bandwidth_out.write[bandwidth_out_ptr].timestamp = time;
		bandwidth_out.write[bandwidth_out_ptr].packet_size = size;
		bandwidth_out_ptr = (bandwidth_out_ptr + 1) % bandwidth_out.size();
	}
}

void SaveloadDebugger::BandwidthProfiler::tick(double p_frame_time, double p_process_time, double p_physics_time, double p_physics_frame_time) {
	uint64_t pt = OS::get_singleton()->get_ticks_msec();
	if (pt - last_bandwidth_time > 200) {
		last_bandwidth_time = pt;
		int incoming_bandwidth = bandwidth_usage(bandwidth_in, bandwidth_in_ptr);
		int outgoing_bandwidth = bandwidth_usage(bandwidth_out, bandwidth_out_ptr);

		Array arr;
		arr.push_back(incoming_bandwidth);
		arr.push_back(outgoing_bandwidth);
		EngineDebugger::get_singleton()->send_message("saveload:bandwidth", arr);
	}
}

// SaveloadProfiler

SaveloadDebugger::SyncInfo::SyncInfo(SaveloadSynchronizer *p_sync) {
	ERR_FAIL_COND(!p_sync);
	synchronizer = p_sync->get_instance_id();
	if (p_sync->get_saveload_config().is_valid()) {
		config = p_sync->get_saveload_config()->get_instance_id();
	}
	if (p_sync->get_root_node()) {
		root_node = p_sync->get_root_node()->get_instance_id();
	}
}

void SaveloadDebugger::SyncInfo::write_to_array(Array &r_arr) const {
	r_arr.push_back(synchronizer);
	r_arr.push_back(config);
	r_arr.push_back(root_node);
	r_arr.push_back(incoming_syncs);
	r_arr.push_back(incoming_size);
	r_arr.push_back(outgoing_syncs);
	r_arr.push_back(outgoing_size);
}

bool SaveloadDebugger::SyncInfo::read_from_array(const Array &p_arr, int p_offset) {
	ERR_FAIL_COND_V(p_arr.size() - p_offset < 7, false);
	synchronizer = int64_t(p_arr[p_offset]);
	config = int64_t(p_arr[p_offset + 1]);
	root_node = int64_t(p_arr[p_offset + 2]);
	incoming_syncs = p_arr[p_offset + 3];
	incoming_size = p_arr[p_offset + 4];
	outgoing_syncs = p_arr[p_offset + 5];
	outgoing_size = p_arr[p_offset + 6];
	return true;
}

Array SaveloadDebugger::SaveloadFrame::serialize() {
	Array arr;
	arr.push_back(infos.size() * 7);
	for (const KeyValue<ObjectID, SyncInfo> &E : infos) {
		E.value.write_to_array(arr);
	}
	return arr;
}

bool SaveloadDebugger::SaveloadFrame::deserialize(const Array &p_arr) {
	ERR_FAIL_COND_V(p_arr.size() < 1, false);
	uint32_t size = p_arr[0];
	ERR_FAIL_COND_V(size % 7, false);
	ERR_FAIL_COND_V((uint32_t)p_arr.size() != size + 1, false);
	int idx = 1;
	for (uint32_t i = 0; i < size / 7; i++) {
		SyncInfo info;
		if (!info.read_from_array(p_arr, idx)) {
			return false;
		}
		infos[info.synchronizer] = info;
		idx += 7;
	}
	return true;
}

void SaveloadDebugger::SaveloadProfiler::toggle(bool p_enable, const Array &p_opts) {
	sync_data.clear();
}

void SaveloadDebugger::SaveloadProfiler::add(const Array &p_data) {
	ERR_FAIL_COND(p_data.size() != 3);
	const String what = p_data[0];
	const ObjectID id = p_data[1];
	const uint64_t size = p_data[2];
	SaveloadSynchronizer *sync = Object::cast_to<SaveloadSynchronizer>(ObjectDB::get_instance(id));
	ERR_FAIL_COND(!sync);
	if (!sync_data.has(id)) {
		sync_data[id] = SyncInfo(sync);
	}
	SyncInfo &info = sync_data[id];
	if (what == "sync_in") {
		info.incoming_syncs++;
		info.incoming_size += size;
	} else if (what == "sync_out") {
		info.outgoing_syncs++;
		info.outgoing_size += size;
	}
}

void SaveloadDebugger::SaveloadProfiler::tick(double p_frame_time, double p_process_time, double p_physics_time, double p_physics_frame_time) {
	uint64_t pt = OS::get_singleton()->get_ticks_msec();
	if (pt - last_profile_time > 100) {
		last_profile_time = pt;
		SaveloadFrame frame;
		for (const KeyValue<ObjectID, SyncInfo> &E : sync_data) {
			frame.infos[E.key] = E.value;
		}
		sync_data.clear();
		EngineDebugger::get_singleton()->send_message("multiplayer:syncs", frame.serialize());
	}
}
