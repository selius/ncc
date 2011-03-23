int printf(int *fmt, int d);

void test_out(int d)
{
	printf("%d\n", d);
}

int main()
{
	int a;

	a = 7;
	test_out(!a);

	a = 0;
	test_out(!a);

	test_out(a && 10);
	test_out(10 && a);

	a = 5;

	test_out(a && 10);
	test_out(10 && a);

	test_out(a && 0);
	test_out(0 && a);

	test_out(a || 0);
	test_out(0 || a);

	a = 0;

	test_out(a || 0);
	test_out(0 || a);
	
	return 0;
}
