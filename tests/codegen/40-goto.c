int main()
{
	__print_int(1);

	goto abc;
	__print_int(0);

ghi:
	__print_int(3);
	return 0;

abc:
	__print_int(2);

	goto ghi;
	__print_int(0);
}
