/**************************************************************************/
/*  physics_server_2d_wrap_mt.cpp                                         */
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

#include "physics_server_2d_wrap_mt.h"

#include "core/os/os.h"

void PhysicsServer2DWrapMT::thread_step(real_t p_delta) {
	server_thread = Thread::get_caller_id();
	command_queue.flush_all();
	physics_server_2d->step(p_delta);
	command_queue.flush_all(); // Flush pending commands before and after
	server_thread = Thread::UNASSIGNED_ID;
}

void PhysicsServer2DWrapMT::_main_thread_sync() {
	if (task_id != WorkerThreadPool::INVALID_TASK_ID) {
		WorkerThreadPool::get_singleton()->wait_for_task_completion(task_id);
		task_id = WorkerThreadPool::INVALID_TASK_ID;
	}

	server_thread = Thread::MAIN_ID;
}

/* EVENT QUEUING */

void PhysicsServer2DWrapMT::step(real_t p_step) {
	if (create_thread) {
		task_id = WorkerThreadPool::get_singleton()->add_task(callable_mp(this, &PhysicsServer2DWrapMT::thread_step).bind(p_step), true);
	} else {
		command_queue.flush_all(); //flush all pending from other threads
		physics_server_2d->step(p_step);
	}
}

void PhysicsServer2DWrapMT::sync() {
	if (create_thread) {
		if (task_id != WorkerThreadPool::INVALID_TASK_ID) {
			WorkerThreadPool::get_singleton()->wait_for_task_completion(task_id);
			task_id = WorkerThreadPool::INVALID_TASK_ID;
		}
	}
	physics_server_2d->sync();
}

void PhysicsServer2DWrapMT::flush_queries() {
	physics_server_2d->flush_queries();
}

void PhysicsServer2DWrapMT::end_sync() {
	physics_server_2d->end_sync();
}

void PhysicsServer2DWrapMT::init() {
	physics_server_2d->init();
}

void PhysicsServer2DWrapMT::finish() {
	if (task_id != WorkerThreadPool::INVALID_TASK_ID) {
		WorkerThreadPool::get_singleton()->wait_for_task_completion(task_id);
		task_id = WorkerThreadPool::INVALID_TASK_ID;
	}
	physics_server_2d->finish();
}

PhysicsServer2DWrapMT::PhysicsServer2DWrapMT(PhysicsServer2D *p_contained, bool p_create_thread) :
		command_queue(p_create_thread) {
	physics_server_2d = p_contained;
	create_thread = p_create_thread;

	if (!p_create_thread) {
		server_thread = Thread::MAIN_ID;
	} else {
		server_thread = Thread::UNASSIGNED_ID;
	}

	main_thread = Thread::MAIN_ID;
}

PhysicsServer2DWrapMT::~PhysicsServer2DWrapMT() {
	memdelete(physics_server_2d);
	//finish();
}
