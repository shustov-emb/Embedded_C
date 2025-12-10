#include <stdio.h>
#define SIZE 5

void print_array(int *array, int actualSize)
{

    for (int i = 0; i < actualSize; i++)
    {
        int value = *((char *)array + i * sizeof(int));

        if (i % SIZE == 0)
            printf("\n\t");

        printf("%d\t", value);
    }
}

void task_1_print_simple_array()
{

    printf("1.Вывести квадратную матрицу по заданному N");
    int array[SIZE][SIZE];
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            int value = (i * SIZE) + j + 1;
            array[i][j] = value;
        }
    }

    print_array(&array[0][0], SIZE * SIZE);
}

void task_2_swap_values()
{

    printf("2.Вывести заданный массив размером N в обратном порядке, меняя первый и последний местами");
    int array[SIZE];
    for (int i = 0; i < SIZE; i++)
    {
        array[i] = i + 1;
    }
    print_array(array, SIZE);

    for (int i = 0; i < SIZE / 2; i++)
    {
        int temp = array[SIZE - (i + 1)];
        array[SIZE - (i + 1)] = array[i];
        array[i] = temp;
    }

    print_array(array, SIZE);
}

void task_3_triangle_matrix()
{
    printf("3.Заполнить верхний треугольник матрицы 1, а нижний 0");
    int array[SIZE][SIZE];
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            if (j < (SIZE - 1) - i)
                array[i][j] = 0;
            else
                array[i][j] = 1;
        }
    }

    print_array(&array[0][0], SIZE * SIZE);
}

void task_4_square_snail()
{
    printf("4.Заполнить матрицу числами от 1 до N - Улиткой");
    int arr[SIZE][SIZE];
    int upperBorder = 0;
    int rightBorder = SIZE-1;
    int lowerBorder = SIZE-1;
    int leftBorder = 0;
    int value = 1;
    while (upperBorder <= lowerBorder && leftBorder <= rightBorder)
    {

        for (int i = leftBorder; i <= rightBorder; i++)
        {
            arr[upperBorder][i] = value;
            value++;
        }
        upperBorder++;

        for (int i = upperBorder; i <= lowerBorder; i++)
        {
            arr[i][rightBorder] = value;
            value++;
        }
        rightBorder--;

        if (upperBorder <= lowerBorder)
        {
            for (int i = rightBorder; i >= leftBorder; i--)
            {
                arr[lowerBorder][i] = value;
                value++;
            }
            lowerBorder--;
        }

        if (leftBorder <= rightBorder)
        {
            for (int i = lowerBorder; i >= upperBorder; i--)
            {
                arr[i][leftBorder] = value;
                value++;
            }
            leftBorder++;
        }
    }

    print_array(&arr[0][0], SIZE*SIZE);
}

int main()
{
    task_1_print_simple_array(); printf("\n\n");
    task_2_swap_values(); printf("\n\n");
    task_3_triangle_matrix(); printf("\n\n");
    task_4_square_snail(); printf("\n\n");
}