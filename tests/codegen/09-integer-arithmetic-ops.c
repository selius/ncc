int printf(int *fmt, int d);

void test_out(int d)
{
	printf("%d\n", d);
}

int main()
{
	int a;
	int b;

	a = 7;
	b = 3;

	test_out(a + b);
	test_out(a - b);
	test_out(a * b);
	test_out(a / b);
	test_out(a % b);

	return 0;
}
