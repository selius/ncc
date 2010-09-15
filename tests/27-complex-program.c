void some_function();
int another_function(float a);

int main()
{
	some_function();

	return 0;
}

void some_function()
{
	float c = 10.0f;

	if (another_function(c) < 25) {
		c *= 2;
	} else {
		c *= 3;
	}
}

int another_function(float a)
{
	return 5 * a;
}
