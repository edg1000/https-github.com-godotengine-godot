func convert_literal_int_to_float() -> float: return 76
func convert_arg_int_to_float(arg: int) -> float: return arg
func convert_var_int_to_float() -> float: var number := 59; return number

func convert_literal_array_to_packed() -> PackedStringArray: return ['46']
@warning_ignore("implicit_conversion_causes_copy")
func convert_arg_array_to_packed(arg: Array) -> PackedStringArray: return arg
@warning_ignore("implicit_conversion_causes_copy")
func convert_var_array_to_packed() -> PackedStringArray: var array := ['79']; return array

func test():
	var converted_literal_int := convert_literal_int_to_float()
	assert(typeof(converted_literal_int) == TYPE_FLOAT)
	assert(converted_literal_int == 76.0)

	var converted_arg_int := convert_arg_int_to_float(36)
	assert(typeof(converted_arg_int) == TYPE_FLOAT)
	assert(converted_arg_int == 36.0)

	var converted_var_int := convert_var_int_to_float()
	assert(typeof(converted_var_int) == TYPE_FLOAT)
	assert(converted_var_int == 59.0)

	var converted_literal_array := convert_literal_array_to_packed()
	assert(typeof(converted_literal_array) == TYPE_PACKED_STRING_ARRAY)
	assert(str(converted_literal_array) == '["46"]')

	var converted_arg_array := convert_arg_array_to_packed(['91'])
	assert(typeof(converted_arg_array) == TYPE_PACKED_STRING_ARRAY)
	assert(str(converted_arg_array) == '["91"]')

	var converted_var_array := convert_var_array_to_packed()
	assert(typeof(converted_var_array) == TYPE_PACKED_STRING_ARRAY)
	assert(str(converted_var_array) == '["79"]')

	print('ok')
