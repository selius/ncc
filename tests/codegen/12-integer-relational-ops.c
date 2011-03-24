int printf(int *fmt, int d);

void test_out(int d)
{
	printf("%d\n", d);
}

int main()
{
	int a;
	int b;
	a = -2;
	b = 5;

	test_out(a < b);
	test_out(a > b);
	test_out(a < 5);
	test_out(a > 5);
	test_out(-2 < b);
	test_out(-2 > b);

	test_out(a <= b);
	test_out(a >= b);
	test_out(a <= 5);
	test_out(a >= 5);
	test_out(-2 <= b);
	test_out(-2 >= b);

	a = 7;
	b = 7;

	test_out(a < b);
	test_out(a > b);
	test_out(a <= b);
	test_out(a >= b);
	
	return 0;
}
