int printf(const int *fmt);

void a()
{
	printf("a() called\n");
}

void b(int a)
{
	printf("b() called\n");
	__print_int(a);
}

int c()
{
	printf("c() called\n");
	return 10;
}

float d(float a)
{
	printf("d() called\n");
	return a * 5;
}

int main()
{
	a();

	b(42);

	__print_int(c());
	__print_int(c() + 5);

	__print_float(d(0.5));

	__print_float(d(c()));
	
	return 0;
}
