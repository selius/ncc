struct A {
	struct B {
		int a;
		float b;
	};
};

struct {
	struct  {
		int foo;
		int bar;
	};
};

struct {
	int a;
	struct {
		struct {
			int x;
			int y;
		};
		float c;
	};
	float b;
};
