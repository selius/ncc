int main()
{
	int i;
	int a;

	for (i = 0; i < 5; i++) {
		__print_int(i);
	}

	a = 0;
	for (i = 1; i <= 1000; i++) {
		a += i;
	}
	__print_int(a);

	for (; 0;) {
		__print_int(999);
		break;
	}
	

	i = 0;
	for (;;) {
		if (i > 100) {
			break;
		}
		++i;
	}
	__print_int(i);

	a = 0;
	for (i = 0; i < 10; i++) {
		a += i;
		continue;
		a *= i;
	}
	__print_int(a);
	
	return 0;
}
