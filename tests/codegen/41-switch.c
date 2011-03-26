int main()
{
	int i;

	i = 1;

	switch (i) {
	}

	switch (i) {
	case 1:
		__print_int(i);
	}

	switch (i) {
	case 1:
		__print_int(1);
		break;
	case 2:
		__print_int(2);
		break;
	default:
		__print_int(999);
	}

	i = 45;

	switch (i) {
	case 1:
		__print_int(i);
	}

	switch (i) {
	case 1:
		__print_int(1);
		break;
	case 2:
		__print_int(2);
		break;
	default:
		__print_int(999);
	}

	switch (i) {
	default:
		__print_int(999);
	}

	return 0;
}
