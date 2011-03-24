int main()
{
	int a;

	a = 5;
	a = ~a;

	__print_int(a);

	a = 5 | 2;

	__print_int(a);

	a = 6 & 5;

	__print_int(a);

	a = 5 ^ 6;

	__print_int(a);
	
	return 0;
}
