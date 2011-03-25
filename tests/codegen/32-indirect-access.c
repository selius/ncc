int main()
{
	struct SomeStruct {
		int a;
		float b;
	};

	struct ComplexStruct {
		int a;
		struct {
			int a;
		} b;
		float c;
	};

	struct SomeStruct s;
	struct ComplexStruct c;

	struct SomeStruct *ps;
	struct ComplexStruct *pc;

	ps = &s;
	pc = &c;

	ps->a = 5;
	ps->b = 3.78;

	__print_int(s.a);
	__print_float(s.b);

	pc->b.a = 99;

	__print_int(c.b.a);

	pc->a = pc->b.a;
	__print_int(c.a);

	__print_int(ps->a);
	__print_float(ps->b);
	__print_int(pc->b.a);
	__print_int(pc->a);
	
	return 0;
}
