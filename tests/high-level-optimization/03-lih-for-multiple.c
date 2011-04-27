int main()
{
	int i;
	int a = 0;
	int b = 10;
	int c = 0;

	for (i = 0; i < 5; i++) {
		a = b * 4;
		c = a * b;
	}

	__print_int(c);

	return 0;
}
