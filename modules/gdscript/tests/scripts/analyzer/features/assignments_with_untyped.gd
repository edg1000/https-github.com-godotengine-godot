func test():
	var one_0 = 0
	one_0 = 1
	var one_1 := one_0
	print(one_1)

	var two: Variant = 0
	two += 2
	print(two)

	var three_0: Variant = 1
	var three_1: int = 2
	three_0 += three_1
	print(three_0)

	var four_0: int = 3
	var four_1: Variant = 1
	four_0 += four_1
	print(four_0)
