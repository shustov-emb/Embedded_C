#include "../src/homework_3.h"
#include <stdio.h>
#include <locale.h>
#include <stdlib.h>

int main()
{
	setlocale(LC_ALL, "");

    printf("\n1. Поменять в целом положительном числе (типа int) значение третьего байта\n");
	int input = 0;
	int replacement = 0;
	int scan_result = 0;

	while (scan_result != 1)
	{
		printf("\tЦелое число: ");
		scan_result = scanf("%d", &input);
		while (getchar() != '\n')
			;
	}

	scan_result = 0;
	while (scan_result != 1)
	{
		printf("\tЗамена (0-255): ");
		scan_result = scanf("%d", &replacement);
		if (replacement < 0 || replacement > 255)
			scan_result = 0;
		while (getchar() != '\n');
	}
	task_1_pointer_change_third_byte(input, replacement);
	
    printf("\n\n2.Измените только одну строку\n");
	task_2_pointer_change_code();
    
    printf("\n\n3. Используйте указатель для вывода элементов массива на консоль\n\t");
	task_3_pinter_print_array();

	printf("\n\n4.Напишите программу, которая ищет введенной строке введенную подстроку\n");
	// const int SIZE = 1000;
	char abs[100];
	char input_substring[100];
	int scan_result1 = 0;

	while (scan_result1 == 0)
	{
		printf("\tinput string: ");
		scan_result1 = scanf("%999[^\n]", abs);
		while (getchar() != '\n');
	}

	scan_result1 = 0;
	while (scan_result1 == 0)
	{
		printf("\tinput substring: ");
		scan_result1 = scanf("%999[^\n]", input_substring);
		while (getchar() != '\n');
	}

	char* result = task_4_find_substring(abs, input_substring);
	if (result != NULL)
		printf("\tfirst letter of substring: %c\n", *result);
}