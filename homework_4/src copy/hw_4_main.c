#include <stdio.h>
#include <locale.h>
#include "hw_4.h"

int main()
{
    setlocale(LC_ALL, "ru");
    struct subscriber array[arraySize] = {0};

    int input = 0;
    int scan_result1 = 0;

    while (input != 5)
    {

        scan_result1 = 0;

        while (scan_result1 == 0)
        {
            printf("\n1. Add subscriber.\n");
            printf("2. Delete subscriber.\n");
            printf("3. Find subscriber.\n");
            printf("4. Print subscribers list.\n");
            printf("5. EXIT\n");
            printf("Enter value: ");
            scan_result1 = scanf("%d", &input);
            while (getchar() != '\n')
                ;
        }

        switch (input)
        {
        case 1:
            add_subscriber(array);
            break;
        case 2:
            delete_subscriber(array);
            break;
        case 3:
            char nameToFind[stringSize] = {0};
            get_string("name to find", nameToFind, stringSize);
            print_subscribers(array, nameToFind);
            break;
        case 4:
            print_subscribers(array, 0);
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