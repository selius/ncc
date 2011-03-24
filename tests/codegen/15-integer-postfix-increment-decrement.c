int main()
{
	int a;
	int b;

	a = 4;

	b = a++;
	__print_int(a);
	__print_int(b);

	a++;
	b = a++;
	__print_int(a);
	__print_int(b);

	a = -2;

	b = a--;
	__print_int(a);
	__print_int(b);

	a--;
	b = a--;
	__print_int(a);
	__print_int(b);

	return 0;
}
