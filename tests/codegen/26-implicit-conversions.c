int main()
{
	int a;
	float b;

	b = 3.645;
	a = b;
	__print_int(a);

	b = a;
	__print_float(b);

	a = 975;
	b = a;
	__print_float(b);

	a = b;
	__print_int(a);

	return 0;
}
