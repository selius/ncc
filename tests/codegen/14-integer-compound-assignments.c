int main()
{
	int a;
	a = 5;

	__print_int(a += 3);
	__print_int(a -= 4);
	__print_int(a *= 3);
	__print_int(a /= 5);
	__print_int(a <<= 2);
	__print_int(a %= 3);
	__print_int(a |= 5);
	__print_int(a ^= 31);
	__print_int(a &= 8);
	__print_int(a >>= 1);

	return 0;
}
