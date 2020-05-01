/*************************************************************************/
/*  gdscript_warning.h                                                   */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
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

#ifndef GDSCRIPT_WARNINGS
#define GDSCRIPT_WARNINGS

#ifdef DEBUG_ENABLED

#include "core/ustring.h"
#include "core/vector.h"

class GDScriptWarning {
public:
	enum Code {
		UNASSIGNED_VARIABLE, // Variable used but never assigned
		UNASSIGNED_VARIABLE_OP_ASSIGN, // Variable never assigned but used in an assignment operation (+=, *=, etc)
		UNUSED_VARIABLE, // Local variable is declared but never used
		SHADOWED_VARIABLE, // Variable name shadowed by other variable
		UNUSED_CLASS_VARIABLE, // Class variable is declared but never used in the file
		UNUSED_ARGUMENT, // Function argument is never used
		UNREACHABLE_CODE, // Code after a return statement
		STANDALONE_EXPRESSION, // Expression not assigned to a variable
		VOID_ASSIGNMENT, // Function returns void but it's assigned to a variable
		NARROWING_CONVERSION, // Float value into an integer slot, precision is lost
		FUNCTION_MAY_YIELD, // Typed assign of function call that yields (it may return a function state)
		VARIABLE_CONFLICTS_FUNCTION, // Variable has the same name of a function
		FUNCTION_CONFLICTS_VARIABLE, // Function has the same name of a variable
		FUNCTION_CONFLICTS_CONSTANT, // Function has the same name of a constant
		INCOMPATIBLE_TERNARY, // Possible values of a ternary if are not mutually compatible
		UNUSED_SIGNAL, // Signal is defined but never emitted
		RETURN_VALUE_DISCARDED, // Function call returns something but the value isn't used
		PROPERTY_USED_AS_FUNCTION, // Function not found, but there's a property with the same name
		CONSTANT_USED_AS_FUNCTION, // Function not found, but there's a constant with the same name
		FUNCTION_USED_AS_PROPERTY, // Property not found, but there's a function with the same name
		INTEGER_DIVISION, // Integer divide by integer, decimal part is discarded
		UNSAFE_PROPERTY_ACCESS, // Property not found in the detected type (but can be in subtypes)
		UNSAFE_METHOD_ACCESS, // Function not found in the detected type (but can be in subtypes)
		UNSAFE_CAST, // Cast used in an unknown type
		UNSAFE_CALL_ARGUMENT, // Function call argument is of a supertype of the require argument
		DEPRECATED_KEYWORD, // The keyword is deprecated and should be replaced
		STANDALONE_TERNARY, // Return value of ternary expression is discarded
		WARNING_MAX,
	};

	Code code = WARNING_MAX;
	int line = -1;
	Vector<String> symbols;

	String get_name() const;
	String get_message() const;
	static String get_name_from_code(Code p_code);
	static Code get_code_from_name(const String &p_name);
};

#endif // DEBUG_ENABLED

#endif // GDSCRIPT_WARNINGS
