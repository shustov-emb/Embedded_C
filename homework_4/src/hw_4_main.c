/**
 * @file hw_4_main.c
 * @author Шустов Александр
 * @brief Файл вызова основной функции main
 * @version 0.1
 * @date 2025-12-10
 */
#include <stdio.h>
#include <locale.h>
#include "hw_4.h"

/**
 * @brief В основной функции программы, мы выводим меню спрашиваем пользователя выбрать действие
 */
int main()
{
    setlocale(LC_ALL, "ru");

    //Создаём массив из структур Subscriber
    struct Subscriber array[ARRAY_SIZE] = {0};

    int input = 0;
    int scan_result = 0;

    //Ввод пользователем цифры 5 завершает программу
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
            //Получаем строку от пользователя и обрабатываем её
            char name_to_find[STRING_SIZE] = {0};
            GetString("name to find", name_to_find);
            //Выводим список только совпавших пользователей
            PrintSubscribers(array, name_to_find);
            break;
        case 4:
            PrintSubscribers(array, 0);
            break;
        case 5:
            break;
        default:
            //В случае если были введены не нужные данные, просим пользователя повторить ввод! 
            printf("Повторите ввод!\n");
            break;
        }
    }

    return 0;
}