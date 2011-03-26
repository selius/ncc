int main()
{
	int i;
	int a;

	i = 0;
	do {
		__print_int(i);
		++i;
	} while (i < 5);

	i = 0;
	do {
		++i;
	} while (0);
	__print_int(i);

	i = 0;
	do {
		++i;
		if (i > 999) {
			break;
		}
	} while (1);
	__print_int(i);

	a = 0;
	i = -1;
	do {
		++i;
		a += i;
		continue;
		a *= i;
	} while (i < 9);
	__print_int(a);
	
	return 0;
}
