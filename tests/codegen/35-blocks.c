int main()
{
	int a;
	a = 2;
	{

	}

	{
		{
		}
	}

	{
		__print_int(111);
	}

	{
		int b;
		b = 4;
		__print_int(a);
		__print_int(b);
		{
			int c;
			c = 7;
			a = 6;
			__print_int(c);
			__print_int(b);
			__print_int(a);
			return 0;
		}
		a = b;
	}

	return a;
}
