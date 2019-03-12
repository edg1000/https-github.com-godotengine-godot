/*************************************************************************/
/*  test_case.cpp                                                        */
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

#include "test_case.h"

#include "core/math/math_funcs.h"
#include "core/script_language.h"

class AssertGuard {
public:
	AssertGuard(bool *can_assert) :
			m_can_assert(can_assert) {
		*m_can_assert = true;
	}

	virtual ~AssertGuard() {
		*m_can_assert = false;
	}

private:
	bool *m_can_assert;
};

void assert(Dictionary &input, const String &custom_msg, const String &default_msg) {
	String msg = default_msg;
	if (!custom_msg.empty()) {
		msg = custom_msg;
	} else {
		input["msg"] = "Assertion : ";
	}
	throw Failure{ msg.format(input) };
}

void assert(const Variant &x, const String &custom_msg, const String &default_msg) {
	Dictionary input;
	input["x"] = x;
	assert(input, custom_msg, default_msg);
}

void assert(const Variant &a, const Variant &b, const String &custom_msg, const String &default_msg) {
	Dictionary input;
	input["a"] = a;
	input["b"] = b;
	assert(input, custom_msg, default_msg);
}

bool TestCase::State::next_test() {
	while (m_method_info) {
		if (m_method_info->get().name.begins_with("test_")) {
			return true;
		}
		m_method_info = m_method_info->next();
	}
	return false;
}

bool TestCase::State::init(Object *object) {
	object->get_method_list(&m_methods);
	m_method_info = m_methods.front();
	return next_test();
}

const String &TestCase::State::get() {
	return m_method_info->get().name;
}

bool TestCase::State::next() {
	m_method_info = m_method_info->next();
	return next_test();
}

void TestCase::init() {
}

bool TestCase::iteration(float p_time) {
	return true;
}

void TestCase::finish() {
}

void TestCase::setup() {
	if (get_script_instance() && get_script_instance()->has_method("setup")) {
		get_script_instance()->call("setup");
	}
}

void TestCase::teardown() {
	if (get_script_instance() && get_script_instance()->has_method("teardown")) {
		get_script_instance()->call("teardown");
	}
}

void TestCase::run(Ref<TestResult> test_result) {
	if (test_result.is_null()) {
		test_result.instance();
	}
	test_result->start_test(this);
	Ref<State> state = memnew(State);
	bool has_next = state->init(this);
	while (has_next) {
		setup();
		try {
			AssertGuard guard(&m_can_assert);
			get_script_instance()->call(state->get());
		} catch (const Failure &failure) {
			TestError *error = memnew(TestError);
			test_result->add_failure(this, error);
		}
		teardown();
		test_result->add_success(this);
		has_next = state->next();
	}
	test_result->stop_test(this);
}

void TestCase::assert_equal(const Variant &a, const Variant &b, const String &msg) {
	ERR_FAIL_COND(!m_can_assert);
	if (a != b) {
		assert(a, b, "{msg}: {a} != {b}", msg);
	}
}

void TestCase::assert_not_equal(const Variant &a, const Variant &b, const String &msg) {
	ERR_FAIL_COND(!m_can_assert);
	if (a != b) {
		assert(a, b, "{msg}: {a} == {b}", msg);
	}
}

void TestCase::assert_true(const Variant &x, const String &msg) {
	ERR_FAIL_COND(!m_can_assert);
	if (!x) {
	}
}

void TestCase::assert_false(const Variant &x, const String &msg) {
	ERR_FAIL_COND(!m_can_assert);
	if (x) {
	}
}

void TestCase::assert_is(const Variant &a, const Ref<GDScriptNativeClass> b, const String &msg) {
	ERR_FAIL_COND(!m_can_assert);
	Object *obj = cast_to<Object>(a);
	const String &klass = obj->get_class();
	const String &inherits = obj->get_class();
	if (!ClassDB::is_parent_class(klass, inherits)) {
		assert(klass, inherits, msg, "{a} is not {b}");
	}
}

void TestCase::assert_is_not(const Variant &a, const Ref<GDScriptNativeClass> b, const String &msg) {
	ERR_FAIL_COND(!m_can_assert);
	Object *obj = cast_to<Object>(a);
	const String &klass = obj->get_class();
	const String &inherits = obj->get_class();
	if (!ClassDB::is_parent_class(klass, inherits)) {
		assert(klass, inherits, msg, "{a} is {b}");
	}
}

void TestCase::assert_is_nil(const Variant &x, const String &msg) {
	ERR_FAIL_COND(!m_can_assert);
	if (Variant::NIL != x.get_type()) {
	}
}

void TestCase::assert_is_not_nil(const Variant &x, const String &msg) {
	ERR_FAIL_COND(!m_can_assert);
	if (Variant::NIL == x.get_type()) {
	}
}

void TestCase::assert_in(const Variant &a, const Variant &b, const String &msg) {
	ERR_FAIL_COND(!m_can_assert);
}

void TestCase::assert_not_in(const Variant &a, const Variant &b, const String &msg) {
	ERR_FAIL_COND(!m_can_assert);
}

void TestCase::assert_is_type(const Variant &a, const Variant::Type type, const String &msg) {
	ERR_FAIL_COND(!m_can_assert);
	if (a.get_type() != type) {
	}
}

void TestCase::assert_is_not_type(const Variant &a, const Variant::Type type, const String &msg) {
	ERR_FAIL_COND(!m_can_assert);
	if (a.get_type() == type) {
	}
}

void TestCase::assert_almost_equal(const Variant &a, const Variant &b, const String &msg) {
	ERR_FAIL_COND(!m_can_assert);
	if (!Math::is_equal_approx(a, b)) {
	}
}

void TestCase::assert_not_almost_equal(const Variant &a, const Variant &b, const String &msg) {
	ERR_FAIL_COND(!m_can_assert);
	if (Math::is_equal_approx(a, b)) {
	}
}

void TestCase::_bind_methods() {
	ClassDB::bind_method(D_METHOD("assert_equal", "a", "b", "msg"), &TestCase::assert_equal, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("assert_not_equal", "a", "b", "msg"), &TestCase::assert_not_equal, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("assert_true", "a", "msg"), &TestCase::assert_true, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("assert_false", "a", "msg"), &TestCase::assert_false, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("assert_is", "a", "b", "msg"), &TestCase::assert_is, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("assert_is_not", "a", "b", "msg"), &TestCase::assert_is_not, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("assert_is_null", "a", "msg"), &TestCase::assert_is_nil, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("assert_is_not_null", "a", "msg"), &TestCase::assert_is_not_nil, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("assert_in", "a", "b", "msg"), &TestCase::assert_in, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("assert_not_in", "a", "b", "msg"), &TestCase::assert_not_in, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("assert_is_type", "a", "b", "msg"), &TestCase::assert_is_type, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("assert_is_not_type", "a", "b", "msg"), &TestCase::assert_is_not_type, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("assert_almost_equal", "a", "b", "msg"), &TestCase::assert_almost_equal, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("assert_not_almost_equal", "a", "b", "msg"), &TestCase::assert_not_almost_equal, DEFVAL(""));
}
