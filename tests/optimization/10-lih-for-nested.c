int main()
{
	int i, j;
	int a;
	int b;
	int c;
	int d = 4;

	for (i = 0; i < 5; i++) {
		for (j = 0; j < 5; j++) {
			a = 5 * d;
			b = i * 5;
			c = i * j;
		}
	}

	__print_int(a);
	__print_int(b);
	__print_int(c);

	return 0;
}
