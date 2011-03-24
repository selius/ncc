int main()
{
	float a;
	float b;

	a = 5.345;
	b = 5.345;

	__print_int(a == b);
	__print_int(a != b);
	__print_int(a == 5.345);
	__print_int(a != 5.345);
	__print_int(5.345 == b);
	__print_int(5.345 != b);

	a = -3.129;
	b = 4.768;

	__print_int(a == b);
	__print_int(a != b);
	__print_int(a == 0.0);
	__print_int(a != -3.129);
	__print_int(0.0 == b);
	__print_int(4.768 != b);

	__print_int(5.345 == 5.345);
	__print_int(5.345 != 5.345);
	
	return 0;
}
