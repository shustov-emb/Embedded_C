#include <stdio.h>

//1. Вывести двоичное представление целого положительного числа, используя битовые операции(число вводится с клавиатуры).
void binary_representation(int input) {

	char bits_count = sizeof(input) * 8;

	for (char i = 1; i <= bits_count; i++) {
		printf("%d", (input >> (bits_count - i)) & 1);
		if (i % 4 == 0)
		{
			printf(" ");
		}
	}
}

/*2. Вывести двоичное представление целого отрицательного числа, используя битовые операции(число вводится с клавиатуры).
У меня первый метод выводит как отрицательные так и положительные, поэтому я сделал во втором задании преобразование в обратный код
Просто для себя.*/
void binary_negative(int input) {

	printf("\n input:\t\t");
	binary_representation(input);

	int result = (~input) + 1;
	printf("\n 2.1 res:\t");
	binary_representation(result);

#pragma region cycles
	/*Перебор побитово в цикле*/
	int inverted_result = 0;
	char bits_count = (sizeof(input) * 8) - 1;

	for (char i = 0; i <= bits_count; i++) {
		inverted_result = (inverted_result << 1) | (1 ^ ((input >> (bits_count - i)) & 1));
	}

	//printf("\n2.2 inverted:\t");
	//binary_representation(inverted_result);

	int result_cycle = 0;
	unsigned int carry = 1;

	for (char i = 0; i < 32; i++) {
		unsigned int bit = (inverted_result >> i) & 1;
		unsigned int sum = bit ^ carry;
		carry = bit & carry;
		result_cycle |= (sum << i);
	}
	printf("\n 2.2 res_cycle:\t");
	binary_representation(result_cycle);
#pragma endregion	

}

//3. Найти количество единиц в двоичном представлении целого положительного числа(число вводится с клавиатуры).
void binary_ones_count(int input) {
	char sum_ones = 0;
	char bits_count = sizeof(input) * 8;

	for (char i = 1; i <= bits_count; i++) {
		sum_ones += (input >> bits_count - i) & 1;
	}
	printf("Ones count: %u", sum_ones);
}

//4. Поменять в целом положительном числе(типа int) значение третьего байта на введенное пользователем число(изначальное число также вводится с клавиатуры).
void change_third_byte(int input, int replacement) {
	int replacement_for_cycles = replacement;
	int input_for_cycles = input;

	printf("\n input:\t\t");
	binary_representation(input);
	printf("\n replacement:\t");
	binary_representation(replacement);

	int mask = ~(255 << 16);
	input &= mask;
	replacement <<= 16;
	int result_mask = replacement | input;

	printf("\n 4.1 res_mask:\t");
	binary_representation(result_mask);

#pragma region cycle
	/*Перебор побитово в цикле*/
	int result_cycles = 0;
	char bits_count = sizeof(input) * 8;

	for (char i = 0; i < bits_count; ++i) {

		char final_bit = (input_for_cycles >> i) & 1;

		if (i >= 16 && i < 24) {
			final_bit = (replacement_for_cycles >> (i - 16)) & 1;
		}
		result_cycles |= (final_bit << i);
	}
	printf("\n 4.2 res_cycle:\t");
	binary_representation(result_cycles);
#pragma endregion

}

int main() {

	int input = 0;
	int replacement = 0;
	int scan_result = 0;

	while (scan_result != 1) {
		printf("Целое число: ");
		scan_result = scanf("%d", &input);
		//Про чистку буфера я не знал, этот способ был взят из интернета
		while (getchar() != '\n');
	}
	
	scan_result = 0;
	while (scan_result != 1) {
		printf("Замена (0-255): ");
		scan_result = scanf("%u", &replacement);
		if (replacement < 0 || replacement > 255) scan_result = 0;
		while (getchar() != '\n');
	}

	printf("\n1.Вывести двоичное представление целого положительного числа\n\t\t");
	binary_representation(input);printf("\n");
	printf("\n2.Вывести двоичное представление целого отрицательного числа.\t\t");
	binary_negative(input);printf("\n");
	printf("\n3.Найти количество единиц в двоичном представлении целого положительного числа\n\t\t");
	binary_ones_count(input);printf("\n");
	printf("\n4.Поменять в целом положительном числе(типа int) значение третьего байта на введенное пользователем число\t");
	change_third_byte(input, replacement);printf("\n");
}

