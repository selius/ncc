int main()
{
	int a[5];
	float b[5][4][2];
	int i;
	int j;

	a[3] = 8;
	__print_int(a[3]);

	i = 2;
	j = 1;
	b[1 * i][j * 3][0] = 3.98;
	__print_float(b[1 * i][j * 3][0]);

	return 0;
}
