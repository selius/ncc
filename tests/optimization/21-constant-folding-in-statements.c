void pr(int a, float b)
{
	__print_int(a);
	__print_float(b);
}

int func()
{
	return 2 + 3 * (6 - 4) - 1;
}

int main()
{
	int a;
	float b;

	{
		a = 2 + 5 * (3 - 1);
		b = 3.4 * (6.4 - 3.2);
	}
	pr(a, b);

	if (2 * 5 == 20 / 2) {
		__print_int(1);
	} else {
		__print_int(0);
	}

	for (a = 2 * (5 / 2); a < 20 / 2; ++a) {
		__print_int(a);
	}

	a = 10;
	while (a > 2 * 3) {
		__print_int(a--);
	}

	a = 3;
	do {
		__print_int(a++);
	} while (a < 4 + 3);

	__print_int(func());

	switch (6 + 3 * 2) {
	case 13 - 1:
		__print_int(1);
		break;
	case 6 / 5:
		__print_int(0);
		break;
	case 2 * 0:
		__print_int(0);
		break;
	}
	
	return 0;
}
