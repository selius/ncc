int main()
{
	int i = 0;
	int a = 0;
	int b = 10;
	int c = 0;

	do {
		a = b * 4;
		c = a * b;
		i++;
	} while (i < 5);

	__print_int(c);

	return 0;
}
