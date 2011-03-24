int main()
{
	int a;
	int b;

	a = 3;
	b = 2;
	__print_int(a << b);
	__print_int(4 << b);
	__print_int(4 << 1);

	a = 28;
	b = 1;
	__print_int(a >> b);
	__print_int(a >> 2);
	__print_int(8 >> 3);
	
	return 0;
}
