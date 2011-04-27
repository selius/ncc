int main()
{
	int i;
	int a = 0;
	int b = 3;
	int c = 0;
	int d = 4;

	for (i = 0; i < 5; i++) {
		a = 5 * b / (2 - b) + d * 2;
		c = i * 2;
	}

	__print_int(a);
	__print_int(c);

	return 0;
}
