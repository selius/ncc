int main()
{
	int *a;
	int b;
	int c;

	float *d;
	float e;

	a = &b;
	b = 7;
	__print_int(*a);

	c = 10;
	a = &c;
	__print_int(*a);

	d = &e;
	e = 56.34;
	__print_float(*d);

	__print_float(*&e);

	return 0;
}
