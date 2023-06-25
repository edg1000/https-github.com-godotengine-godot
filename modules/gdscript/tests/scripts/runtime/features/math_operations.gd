func test():
	print("# add")
	print(Vector2(0.1,1.1) + Vector2i(0,1))
	print(Vector3(0.1,1.1,2.1) + Vector3i(0,1,2))
	print(Vector4(0.1,1.1,2.1,3.1) + Vector4i(0,1,2,3))
	print(Vector2i(0,1) + Vector2(0.1,1.1))
	print(Vector3i(0,1,2) + Vector3(0.1,1.1,2.1))
	print(Vector4i(0,1,2,3) + Vector4(0.1,1.1,2.1,3.1))

	print("# subtract")
	print(Vector2(0.1,1.1) - Vector2i(0,1))
	print(Vector3(0.1,1.1,2.1) - Vector3i(0,1,2))
	print(Vector4(0.1,1.1,2.1,3.1) - Vector4i(0,1,2,3))
	print(Vector2i(0,1) - Vector2(0.1,1.1))
	print(Vector3i(0,1,2) - Vector3(0.1,1.1,2.1))
	print(Vector4i(0,1,2,3) - Vector4(0.1,1.1,2.1,3.1))

	print("# multiply")
	print(Vector2(0.1,1.1) * Vector2i(1,2))
	print(Vector3(0.1,1.1,2.1) * Vector3i(1,2,3))
	print(Vector4(0.1,1.1,2.1,3.1) * Vector4i(1,2,3,4))
	print(Vector2i(1,2) * Vector2(0.1,1.1))
	print(Vector3i(1,2,3) * Vector3(0.1,1.1,2.1))
	print(Vector4i(1,2,3,4) * Vector4(0.1,1.1,2.1,3.1))

	print("# divide")
	print(Vector2(0.1,0.2) / Vector2i(1,2))
	print(Vector3(0.1,0.2,0.5) / Vector3i(1,2,5))
	print(Vector4(0.1,0.2,0.5,1.0) / Vector4i(1,2,5,10))
	print(Vector2i(1,2) / Vector2(0.1,0.2))
	print(Vector3i(1,2,3) / Vector3(0.1,0.2,0.5))
	print(Vector4i(1,2,3,4) / Vector4(0.1,0.2,0.5,1.0))

	print("# equal")
	print(Vector2() == Vector2i())
	print(Vector3() == Vector3i())
	print(Vector4() == Vector4i())
	print(Rect2() == Rect2i())
	print(Vector2i() == Vector2())
	print(Vector3i() == Vector3())
	print(Vector4i() == Vector4())
	print(Rect2i() == Rect2())
	print(Vector2(0.1,0.1) == Vector2i())
	print(Vector3(0.1,0.1,0.1) == Vector3i())
	print(Vector4(0.1,0.1,0.1,0.1) == Vector4i())
	print(Rect2(0,0,0.1,0.1) == Rect2i())
	print(Vector2i() == Vector2(0.1,0.1))
	print(Vector3i() == Vector3(0.1,0.1,0.1))
	print(Vector4i() == Vector4(0.1,0.1,0.1,0.1))
	print(Rect2i() == Rect2(0,0,0.1,0.1))

	print("# not equal")
	print(Vector2() != Vector2i())
	print(Vector3() != Vector3i())
	print(Vector4() != Vector4i())
	print(Rect2() != Rect2i())
	print(Vector2i() != Vector2())
	print(Vector3i() != Vector3())
	print(Vector4i() != Vector4())
	print(Rect2i() != Rect2())
	print(Vector2(0.1,0.1) != Vector2i())
	print(Vector3(0.1,0.1,0.1) != Vector3i())
	print(Vector4(0.1,0.1,0.1,0.1) != Vector4i())
	print(Rect2(0,0,0.1,0.1) != Rect2i())
	print(Vector2i() != Vector2(0.1,0.1))
	print(Vector3i() != Vector3(0.1,0.1,0.1))
	print(Vector4i() != Vector4(0.1,0.1,0.1,0.1))
	print(Rect2i() != Rect2(0,0,0.1,0.1))

	print("# less")
	print(Vector2.ZERO < Vector2i.ZERO)
	print(Vector2i.ZERO < Vector2.ZERO)
	print(Vector2.ZERO < Vector2i.ONE)
	print(Vector2i.ZERO < Vector2(0.1,0.1))
	print(Vector3.ZERO < Vector3i.ZERO)
	print(Vector3i.ZERO < Vector3.ZERO)
	print(Vector3.ZERO < Vector3i.ONE)
	print(Vector3i.ZERO < Vector3(0.1,0.1,0.1))
	print(Vector4.ZERO < Vector4i.ZERO)
	print(Vector4i.ZERO < Vector4.ZERO)
	print(Vector4.ZERO < Vector4i.ONE)
	print(Vector4i.ZERO < Vector4(0.1,0.1,0.1,0.1))

	print("# less or equal")
	print(Vector2.ZERO <= Vector2i.ZERO)
	print(Vector2i.ZERO <= Vector2.ZERO)
	print(Vector2.ZERO <= Vector2i.ONE)
	print(Vector2i.ZERO <= Vector2(0.1,0.1))
	print(Vector2.ONE <= Vector2i.ZERO)
	print(Vector2i.ONE <= Vector2.ZERO)
	print(Vector3.ZERO <= Vector3i.ZERO)
	print(Vector3i.ZERO <= Vector3.ZERO)
	print(Vector3.ZERO <= Vector3i.ONE)
	print(Vector3i.ZERO <= Vector3(0.1,0.1,0.1))
	print(Vector3.ONE <= Vector3i.ZERO)
	print(Vector3i.ONE <= Vector3.ZERO)
	print(Vector4.ZERO <= Vector4i.ZERO)
	print(Vector4i.ZERO <= Vector4.ZERO)
	print(Vector4.ZERO <= Vector4i.ONE)
	print(Vector4i.ZERO <= Vector4(0.1,0.1,0.1,0.1))
	print(Vector4.ONE <= Vector4i.ZERO)
	print(Vector4i.ONE <= Vector4.ZERO)

	print("# greater")
	print(Vector2.ZERO > Vector2i.ZERO)
	print(Vector2i.ZERO > Vector2(-0.1,-0.1))
	print(Vector2.ONE > Vector2i.ZERO)
	print(Vector2i.ONE > Vector2.ZERO)
	print(Vector3.ZERO > Vector3i.ZERO)
	print(Vector3i.ZERO > Vector3(-0.1,-0.1,-0.1))
	print(Vector3.ONE > Vector3i.ZERO)
	print(Vector3i.ONE > Vector3.ZERO)
	print(Vector4.ZERO > Vector4i.ZERO)
	print(Vector4i.ZERO > Vector4(-0.1,-0.1,-0.1,-0.1))
	print(Vector4.ONE > Vector4i.ZERO)
	print(Vector4i.ONE > Vector4.ZERO)

	print("# greater or equal")
	print(Vector2.ZERO >= Vector2i.ZERO)
	print(Vector2i.ZERO >= Vector2(-0.1,-0.1))
	print(Vector2.ZERO >= Vector2i.ONE)
	print(Vector2i.ZERO >= Vector2.ONE)
	print(Vector2.ONE >= Vector2i.ZERO)
	print(Vector2i.ONE >= Vector2.ZERO)
	print(Vector3.ZERO >= Vector3i.ZERO)
	print(Vector3i.ZERO >= Vector3(-0.1,-0.1,-0.1))
	print(Vector3.ZERO >= Vector3i.ONE)
	print(Vector3i.ZERO >= Vector3.ONE)
	print(Vector3.ONE >= Vector3i.ZERO)
	print(Vector3i.ONE >= Vector3.ZERO)
	print(Vector4.ZERO >= Vector4i.ZERO)
	print(Vector4i.ZERO >= Vector4(-0.1,-0.1,-0.1,-0.1))
	print(Vector4.ZERO >= Vector4i.ONE)
	print(Vector4i.ZERO >= Vector4.ONE)
	print(Vector4.ONE >= Vector4i.ZERO)
	print(Vector4i.ONE >= Vector4.ZERO)

	# assignment
	var _v2:=Vector2()
	var _v2i:=Vector2i()
	_v2+=Vector2i()
	_v2i+=Vector2()
	_v2-=Vector2i()
	_v2i-=Vector2()
	_v2*=Vector2i()
	_v2i*=Vector2()
	_v2/=Vector2i.ONE
	_v2i/=Vector2.ONE

	var _v3:=Vector3()
	var _v3i:=Vector3i()
	_v3+=Vector3i()
	_v3i+=Vector3()
	_v3-=Vector3i()
	_v3i-=Vector3()
	_v3*=Vector3i()
	_v3i*=Vector3()
	_v3/=Vector3i.ONE
	_v3i/=Vector3.ONE

	var _v4:=Vector4()
	var _v4i:=Vector4i()
	_v4+=Vector4i()
	_v4i+=Vector4()
	_v4-=Vector4i()
	_v4i-=Vector4()
	_v4*=Vector4i()
	_v4i*=Vector4()
	_v4/=Vector4i.ONE
	_v4i/=Vector4.ONE
