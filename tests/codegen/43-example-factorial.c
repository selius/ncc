int factorial_recursive(int n)
{
	if (n <= 1) {
		return 1;
	}

	return n * factorial_recursive(n - 1);
}

int factorial_loop(int n)
{
	int i;
	int res;
	res = 1;

	for (i = 2; i <= n; i++) {
		res *= i;
	}
	
	return res;
}

int main()
{
	__print_int(factorial_recursive(5));
	__print_int(factorial_recursive(7));

	__print_int(factorial_loop(5));
	__print_int(factorial_loop(7));


	return 0;
}
