int main()
{
	float a;

	a = 7.4;
	__print_int(!a);

	a = 0.0;
	__print_int(!a);

	__print_int(a && 10.3);
	__print_int(10.3 && a);

	a = 5.7;

	__print_int(a && 10.3);
	__print_int(10.3 && a);

	__print_int(a && 0.0);
	__print_int(0.0 && a);

	__print_int(a || 0.0);
	__print_int(0.0 || a);

	a = 0.0;

	__print_int(a || 0.0);
	__print_int(0.0 || a);
	
	return 0;
}
