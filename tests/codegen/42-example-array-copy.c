void array_copy(int *dest, int *source, int n)
{
	int i;
	for (i = 0; i < n; i++) {
		dest[i] = source[i];
	}
}

void reset_array(int *arr, int n)
{
	int i;
	for (i = 0; i < n; i++) {
		arr[i] = 0;
	}
}

void print_array(int *arr, int n)
{
	int i;
	for (i = 0; i < n; i++) {
		__print_int(arr[i]);
	}
}

int main()
{
	int a[5];
	int b[5];
	int *c;
	int *d;
	int i;

	c = a;
	d = b;

	for (i = 0; i < 5; i++) {
		a[i] = i;
	}

	reset_array(b, 5);
	for (i = 0; i < 5; i++) {
		b[i] = a[i];
	}
	print_array(b, 5);

	reset_array(b, 5);
	for (i = 0; i < 5; i++) {
		d[i] = c[i];
	}
	print_array(b, 5);

	reset_array(b, 5);
	for (i = 0; i < 5; i++) {
		b[i] = c[i];
	}
	print_array(d, 5);

	reset_array(b, 5);
	array_copy(b, a, 5);
	print_array(b, 5);

	reset_array(b, 5);
	array_copy(d, c, 5);
	print_array(b, 5);

	reset_array(b, 5);
	array_copy(d, a, 5);
	print_array(b, 5);

	return 0;
}
