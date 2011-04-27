int main()
{
	int i = 0;
	int a = 0;
	int b = 3;
	int c = 0;
	int d = 4;

	while (i < 5) {
		a = 5 * b / (2 - b) + d * 2;
		c = i * 2;
		i++;
	}

	__print_int(a);
	__print_int(c);

	return 0;
}
