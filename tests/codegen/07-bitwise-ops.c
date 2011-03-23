int printf(int *fmt, int d);

void test_out(int d)
{
	printf("%d\n", d);
}

int main()
{
	int a;

	a = 5;
	a = ~a;

	test_out(a);

	a = 5 | 2;

	test_out(a);

	a = 6 & 5;

	test_out(a);

	a = 5 ^ 6;

	test_out(a);
	
	return 0;
}
