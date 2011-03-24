int main()
{
	int a;
	int b;
	a = -2;
	b = 5;

	__print_int(a < b);
	__print_int(a > b);
	__print_int(a < 5);
	__print_int(a > 5);
	__print_int(-2 < b);
	__print_int(-2 > b);

	__print_int(a <= b);
	__print_int(a >= b);
	__print_int(a <= 5);
	__print_int(a >= 5);
	__print_int(-2 <= b);
	__print_int(-2 >= b);

	a = 7;
	b = 7;

	__print_int(a < b);
	__print_int(a > b);
	__print_int(a <= b);
	__print_int(a >= b);
	
	return 0;
}
