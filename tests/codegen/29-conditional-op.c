int main()
{
	int a, b;
	float c, d;

	a = 5;
	b = 0;

	__print_int(a ? 2 : 4);
	__print_int(b ? 1 : 3);

	c = 6.34;
	d = 0.0;

	__print_int(c ? 7 : 9);
	__print_float(d ? 4.5 : 1.1);

	return 0;
}
