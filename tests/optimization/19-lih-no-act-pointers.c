int main()
{
	int i;
	int a = 2;
	int *b = &a;

	for (i = 0; i < 5; i++) {
		*b = a * 5;
	}

	__print_int(a);

	return 0;
}
