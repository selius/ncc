int f(int a);
int g();

int main()
{
	return f(5);
}

int f(int a)
{
	return g() * a;
}

int g()
{
	return 2;
}
