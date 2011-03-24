int main()
{
	float a;
	float b;

	a = 4.39;

	b = ++a;
	__print_float(a);
	__print_float(b);

	++a;
	b = ++a;
	__print_float(a);
	__print_float(b);

	a = -2.087;

	b = --a;
	__print_float(a);
	__print_float(b);

	--a;
	b = --a;
	__print_float(a);
	__print_float(b);

	return 0;
}
