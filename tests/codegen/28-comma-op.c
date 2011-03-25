int main()
{
	int a, b, c;
	float d, e, f;

	a = 5;
	b = 6;

	c = (a += b, b - a);
	__print_int(c);

	d = 4.7;
	e = 9.2;
	
	f = (e -= d, e - d);
	__print_float(f);

	return 0;
}
