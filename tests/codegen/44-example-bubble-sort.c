void swap(float *a, float *b)
{
	float temp;
	temp = *a;
	*a = *b;
	*b = temp;
}

void bubble_sort(float *arr, int n)
{
	int i, j, sorted;
	sorted = 1;

	for (i = n - 1; i > 0; i--) {
		for (j = 0; j < i; j++) {
			if (arr[j] > arr[j + 1]) {
				swap(&arr[j], &arr[j + 1]);
				sorted = 0;
			}
		}

		if (sorted) {
			break;
		}
	}
}

int main()
{
	float a[5];
	int i;

	a[0] = 4.6;
	a[1] = 23.7;
	a[2] = -5.5;
	a[3] = 7.6;
	a[4] = 2.9;

	bubble_sort(a, 5);

	for (i = 0; i < 5; i++) {
		__print_float(a[i]);
	}

	return 0;
}
