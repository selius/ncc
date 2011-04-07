void f()
{
	typedef int foo;
	int a;
	{
		int b;
	}
	{
		int a;
		foo b;
		{
			typedef float foo;
			float a;
			foo c;
		}
	}
}

void g()
{
	int *a;
}
