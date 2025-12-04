#include <stdio.h>
#include "homework_3.h"

void binary_representation(int input)
{
	char bits_count = sizeof(input) * 8;

	for (char i = 1; i <= bits_count; i++)
	{
		printf("%d", (input >> (bits_count - i)) & 1);
		if (i % 4 == 0)
		{
			printf(" ");
		}
	}
}

void task_1_pointer_change_third_byte(int input, int replacement)
{
	printf("\replacement:\t\t%d\t|\t", replacement);
	binary_representation(replacement);
	printf("\n");
	printf("\tinput:\t\t%d\t|\t", input);
	binary_representation(input);
	printf("\n");

	char *pointer = ((char *)&input) + 2;
	*pointer = replacement;

	printf("\tinput+repl:\t%d\t|\t", input);
	binary_representation(input);
}

void task_2_pointer_change_code()
{
	float x = 5.0;
	printf("x = %f, ", x);
	float y = 6.0;
	printf("y = %f\n", y);
	float *xp = &y; // TODO: отредактируйте эту строку, и только правую часть уравнения
	float *yp = &y;
	printf("Результат: %f\n", *xp + *yp);
}

void task_3_pointer_print_array()
{
	int array[10];
	int SIZE = sizeof(array) / sizeof(array[0]);

	for (int i = 0; i < SIZE; i++)
	{
		*(array + i) = i+1;
	}
	
	for (int i = 0; i < 10; i++)
	{
		printf("%d ",*(array+i));
	}
}

char *task_4_pointer_find_substring(char* input_string, char* input_substring)
{
	char *current_str_ptr = input_string;
	char *sub_ptr;
	int match_length = 0;

	while (*current_str_ptr != '\0')
	{
		sub_ptr = input_substring;
		match_length = 0;

		while (*(current_str_ptr + match_length) != '\0' 
		&& *sub_ptr != '\0' 
		&& *(current_str_ptr + match_length) == *sub_ptr)
		{
			match_length++;
			sub_ptr++;
		}

		if (*sub_ptr == '\0')
			return current_str_ptr;

		current_str_ptr++;
	}

	return NULL;
}


