void main()
{
	int i;
	int j;
	int min;
	int temp;

	int[10] array;

	print("Please enter 10 numbers:");
	print_nl();

	i = 0;
	while (i < 10) {
		print("#");
		print_int(i);
		print(": ");

		array[i] = read_int();
		i = i + 1;
	}

	j = 0;

	while (j < 9) {
		min = j;
		i = j + 1;

		while (i < 10) {
			if (array[i] < array[min])
				min = i;
			i = i + 1;
		}

		if (min != j) {
			temp = array[j];
			array[j] = array[min];
			array[min] = temp;
		}

		j = j + 1;
	}

	print("Sorted:");
	print_nl();

	i = 0;
	while (i < 10) {
		print_int(array[i]);
		print_nl();
		i = i + 1;
	}
}
