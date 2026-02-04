#include <stdio.h>
#include "calc.h"

//Вспомогательная функция для получения пользовательского ввода
int get_input(int *a, int *b)
{
    printf("Enter two values:\n");
    if (scanf("%d %d", a, b) != 2)
    {
        printf("Wrong input!\n");
        while (getchar() != '\n')
            ;
        return 0;
    }

    return 1;
}

int main()
{
    int choice = 0;
    int a;
    int b;
    int running = 1;
    while (running)
    {
        printf("\n1. Addition\n2. Subtraction\n3. Multiplication\n4. Division\n5. EXIT\n");

        printf("Enter value: "); 
        scanf("%d", &choice);
        switch (choice)
        {
        case 1:
            if (get_input(&a, &b))
                printf("%d + %d = %d\n", a, b, add(a, b));
            break;
        case 2:
            if (get_input(&a, &b))
                printf("%d - %d = %d\n", a, b, sub(a, b));
            break;
        case 3:
            if (get_input(&a, &b))
                printf("%d * %d = %d\n", a, b, mul(a, b));
            break;
        case 4:
            if (get_input(&a, &b))
            {
                //Проверка деления на ноль
                if (b == 0)
                {
                    printf("Cannot divide by zero\n");
                    break;
                }
                printf("%d / %d = %d\n", a, b, div(a, b));
            }
            break;
        case 5:
            running = 0;
            break;
        default:
            printf("Wrong option!\n");
        }
    }

    return 0;
}