/*************************************************************************/
/*  thread_dummy.cpp                                                     */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2015 Juan Linietsky, Ariel Manzur.                 */
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
#include "thread_dummy.h"

#include "memory.h"

Thread* ThreadDummy::create(ThreadCreateCallback p_callback,void * p_user,const Thread::Settings& p_settings) {
	return memnew(ThreadDummy);
};

void ThreadDummy::make_default() {
	Thread::create_func = &ThreadDummy::create;
};

Mutex* MutexDummy::create(bool p_recursive) {
	return memnew(MutexDummy);
};

void MutexDummy::make_default() {
	Mutex::create_func = &MutexDummy::create;
};


Semaphore* SemaphoreDummy::create() {
	return memnew(SemaphoreDummy);
};

void SemaphoreDummy::make_default() {
	Semaphore::create_func = &SemaphoreDummy::create;
};

void TlsDummy::create_func_dummy(ID& p_tls_key) {
}

void TlsDummy::delete_func_dummy(ID& p_tls_key) {
}

void *TlsDummy::get_func_dummy(ID& p_tls_key) {

	return NULL;
}

void TlsDummy::set_func_dummy(ID& p_tls_key, void *p_ptr) {
}

void TlsDummy::make_default() {

	create_func=create_func_dummy;
	delete_func=delete_func_dummy;
	get_func=get_func_dummy;
	set_func=set_func_dummy;
}


