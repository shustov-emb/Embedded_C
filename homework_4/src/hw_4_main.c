#include <stdio.h>
#include <locale.h>
#include "hw_4.h"

int main()
{
    setlocale(LC_ALL, "ru");
    struct Subscriber array[ARRAY_SIZE] = {0};

    int input = 0;
    int scan_result = 0;

    while (input != 5)
    {

        scan_result = 0;

        while (scan_result == 0)
        {
            printf("\n1. Add subscriber.\n");
            printf("2. Delete subscriber.\n");
            printf("3. Find subscriber.\n");
            printf("4. Print subscribers list.\n");
            printf("5. EXIT\n");
            printf("Enter value: ");
            scan_result = scanf("%d", &input);
            while (getchar() != '\n')
                ;
        }

        switch (input)
        {
        case 1:
            AddSubscriber(array);
            break;
        case 2:
            DeleteSubscriber(array);
            break;
        case 3:
            char name_to_find[STRING_SIZE] = {0};
            GetString("name to find", name_to_find);
            PrintSubscribers(array, name_to_find);
            break;
        case 4:
            PrintSubscribers(array, 0);
            break;
        case 5:
            break;
        default:
            printf("Повторите ввод!\n");
            break;
        }
    }

    return 0;
}