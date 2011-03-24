int main()
{
	float a;
	float b;
	a = -2.3;
	b = 5.9;

	__print_int(a < b);
	__print_int(a > b);
	__print_int(a < 5.9);
	__print_int(a > 5.9);
	__print_int(-2.3 < b);
	__print_int(-2.3 > b);

	__print_int(a <= b);
	__print_int(a >= b);
	__print_int(a <= 5.9);
	__print_int(a >= 5.9);
	__print_int(-2.3 <= b);
	__print_int(-2.3 >= b);

	a = 7.7;
	b = 7.7;

	__print_int(a < b);
	__print_int(a > b);
	__print_int(a <= b);
	__print_int(a >= b);
	
	return 0;
}
