int main()
{
	int i = 0, j = 0;
	int a;
	int b;
	int c;
	int d = 4;

	while (i < 5) {
		j = 0;
		while (j < 5) {
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
