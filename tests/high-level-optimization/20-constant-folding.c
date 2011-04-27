void pr(int a, float b)
{
	__print_int(a);
	__print_float(b);
}

int main()
{
	int a;
	float b;

	a = -27;
	b = -0.07;
	pr(a, b);

	a = !5;
	b = !3.4;
	pr(a, b);

	a = !0;
	b = !0.0;
	pr(a, b);

	a = ~5;
	__print_int(a);

	a = 2 + 6;
	b = 2 + 6.5;
	pr(a, b);

	a = 7 - 4;
	b = 7.35 - 4;
	pr(a, b);

	a = 3 * 5;
	b = 3.3 * 5.2;
	pr(a, b);

	a = 5 / 2;
	b = 5.0 / 2.0;
	pr(a, b);

	a = 7 % 3;
	__print_int(a);

	a = 4 << 2;
	__print_int(a);
	
	a = 28 >> 1;
	__print_int(a);

	__print_int(2 < 7);
	__print_int(4 <= 0.2);
	__print_int(3.94 > 5);
	__print_int(8.23 >= 8.23);

	__print_int(3 == 3);
	__print_int(5.3 == 2);
	__print_int(2 != 5.7);
	__print_int(8.4 != 8.4);

	__print_int(2 && 5);
	__print_int(4.3 && 0);

	__print_int(0 || 7.2);
	__print_int(0.0 || 0.0);

	__print_int(5 | 2);
	__print_int(6 & 5);
	__print_int(5 ^ 6);

	a = (3, 7);
	b = (5.4, 2.7);
	pr(a, b);

	a = 5 ? 2 : 4;
	b = 0 ? 5.6 : 3.8;
	pr(a, b);

	return 0;
}
