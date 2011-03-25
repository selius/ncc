int main()
{
	int a;
	a = 45;

	if (a) {
		__print_int(1);
	}

	if (!a) {
		__print_int(0);
	}

	if (a) {
		__print_int(1);
	} else {
		__print_int(0);
	}

	if (!a) {
		__print_int(0);
	} else {
		__print_int(1);
	}
	
	return 0;
}
