class Foo: pass
class Bar extends Foo: pass
class Baz extends Foo: pass

func test():
	var typed: Array[Bar] = [Baz.new() as Foo]
	print('not ok')
