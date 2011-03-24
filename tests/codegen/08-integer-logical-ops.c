int main()
{
	int a;

	a = 7;
	__print_int(!a);

	a = 0;
	__print_int(!a);

	__print_int(a && 10);
	__print_int(10 && a);

	a = 5;

	__print_int(a && 10);
	__print_int(10 && a);

	__print_int(a && 0);
	__print_int(0 && a);

	__print_int(a || 0);
	__print_int(0 || a);

	a = 0;

	__print_int(a || 0);
	__print_int(0 || a);
	
	return 0;
}
