int main()
{
	int i = 0, j = 0;
	int a;
	int b;
	int c;
	int d = 4;

	for (i = 0; i < 5; i++) {
		j = 0;
		do {
			a = 5 * d;
			b = i * 5;
			c = i * j;
			j++;
		} while (j < 5);
	}

	__print_int(a);
	__print_int(b);
	__print_int(c);

	for (i = 0; i < 5; i++) {
		j = 0;
		while (j < 5) {
			a = 5 * d;
			b = i * 5;
			c = i * j;
			j++;
		}
	}

	__print_int(a);
	__print_int(b);
	__print_int(c);

	i = 0;
	while (i < 5) {
		for (j = 0; j < 5; j++) {
			a = 5 * d;
			b = i * 5;
			c = i * j;
			j++;
		}
		i++;
	}

	__print_int(a);
	__print_int(b);
	__print_int(c);

	return 0;
}
