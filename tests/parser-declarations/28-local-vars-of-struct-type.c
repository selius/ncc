void f()
{
	struct {
		int x;
		int y;
	} a;

	struct ABC {
		float foo;
		float bar;
		struct {
			int a;
			int b;
		} baz;
	};

	struct ABC b;
}
