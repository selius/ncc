int main()
{
	struct {
		int a;
		float b;
	} s;

	struct {
		int a;
		struct {
			int a;
		} b;
		float c;
	} c;

	s.a = 5;
	s.b = 3.78;

	__print_int(s.a);
	__print_float(s.b);

	c.b.a = 99;

	__print_int(c.b.a);

	c.a = c.b.a;
	__print_int(c.a);
	
	return 0;
}
