int main()
{
	int i;
	int a;

	i = 0;
	while (i < 5) {
		__print_int(i);
		++i;
	}

	i = 0;
	while (0) {
		++i;
	}
	__print_int(i);

	i = 0;
	while (1) {
		++i;
		if (i > 999) {
			break;
		}
	}
	__print_int(i);

	a = 0;
	i = -1;
	while (i < 9) {
		++i;
		a += i;
		continue;
		a *= i;
	}
	__print_int(a);

	return 0;
}
