int main()
{
	int a;
	int b;

	a = 5;
	b = 5;

	__print_int(a == b);
	__print_int(a != b);
	__print_int(a == 5);
	__print_int(a != 5);
	__print_int(5 == b);
	__print_int(5 != b);

	a = -3;
	b = 4;

	__print_int(a == b);
	__print_int(a != b);
	__print_int(a == 0);
	__print_int(a != -3);
	__print_int(0 == b);
	__print_int(4 != b);

	__print_int(5 == 5);
	__print_int(5 != 5);
	
	return 0;
}
