/**
 * @file hw_4.c
 * @author Шустов Александр
 * @brief Файл реализации
 * @version 0.1
 * @date 2025-12-10
 */
#include <stdio.h>
#include "homework_4.h"

void EraseCharArray(char *char_array)
{
    // Цикл обнуляющий массив символов поэлементно
    for (int i = 0; i < STRING_SIZE; i++)
    {
        char_array[i] = 0;
    }
};

void CopyArray(char *src, char *dst)
{
    //Цикл пожлементно копирующий один массив символов в другой
    for (int i = 0; i < STRING_SIZE; i++)
    {
        dst[i] = src[i];
    }
}

void EraseStructure(struct Subscriber *subscriber)
{
    // Передаём символьные массивы с данными структуру, для того чтобы их обнулить!
    EraseCharArray(subscriber->name);
    EraseCharArray(subscriber->second_name);
    EraseCharArray(subscriber->phone_number);
    subscriber->not_empty = 0; // Удаем метку о заполненности массива
};

void CopyStructure(struct Subscriber *src, struct Subscriber *dst)
{

    CopyArray(src->name, dst->name);
    CopyArray(src->second_name, dst->second_name);
    CopyArray(src->phone_number, dst->phone_number);
    dst->not_empty = src->not_empty;
}

void GetString(const char *message, char *buffer)
{

    char *scan_result = 0;
    int length = 0;

    // Говорим пользователю какие данные нам от него нужны
    printf("\tEnter %s: ", message);

    // Получаем строку введённую пользователем
    scan_result = fgets(buffer, STRING_SIZE, stdin);

    if (scan_result != NULL)
    {
        // Узнаём длинну "строки"
        while (buffer[length] != '\0')
        {
            length++;
        }

        // fgets вожет последним символом вернуть перенос строки, надо убедиться что последний символ будет нулём
        if (length > 0 && buffer[length - 1] == '\n')
        {
            buffer[length - 1] = '\0';
        }

        // Если пользователь ввёл больше положенноо то чистим буфер
        else if (length == STRING_SIZE - 1 && buffer[STRING_SIZE - 2] != '\n')
        {
            while (getchar() != '\n')
                ;
        }
    }
};

int StrCompare(char *str1, char *str2)
{
    // Сравниваем поэлементно два символьных массива
    for (int i = 0; i < STRING_SIZE; i++)
    {
        if (str1[i] != str2[i])
        {
            return 0; // Ввозвращаем 0 если найдено хоть одно различие
        }
    }
    return 1; // Ввозвращаем 0 если различий не нашлось
};

void AddSubscriber(struct Subscriber array[])
{
    // массив подписчиков и ищем первую пустую структуру
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        // Если пустая структура находится, то добавляем данные введённые пользователем в эту структуру
        if (!array[i].not_empty)
        {
            array[i].not_empty = 1;
            GetString("name", array[i].name);
            GetString("second name", array[i].second_name);
            GetString("phone number", array[i].phone_number);
            return; // Выходим из функции после добавления данных в массив
        }
    }

    printf("Subscriber limit reached!\n"); // Если мы пробежались по массиву и не нашли свободных мест, то значит массив полный и добавлять некуда
};

void DeleteSubscriber(struct Subscriber array[])
{

    int input = 0;
    int scan_result = 0;

    // Если первый элемент массива абонентов пуст, то массив пустой и удалять нечего
    if (array[0].not_empty != 1)
    {
        printf("Nothing to delete!\n");
        return;
    }

    // Спрашиваем номер пользователя который нужно удалить
    while (scan_result == 0 || (input >= ARRAY_SIZE && input < 0))
    {
        printf("\tEnter user number: ");
        scan_result = scanf("%d", &input);
        while (getchar() != '\n')
            ;
    }

    // Удаляем даныне если они не пустые
    if (array[input].not_empty)
    {
        // Смещаем массив влево на место удалённой пользователем ячейки
        for (int i = input - 1; i < ARRAY_SIZE - 1; i++)
        {
            //CopyStructure(&array[i+1],&array[i]);
            array[i] = array[i + 1];
        }

        // Затираем последнюю структуру в списке, потому что она не сместится налево и будет дублировать предыдущую
        EraseStructure(&array[ARRAY_SIZE - 1]);
    }
};

void PrintSubscribers(struct Subscriber array[], char *name_to_find)
{

    int search_filter = 0;

    // Если фильтр по имени будет пустым, то будем выводить все записи!
    if (name_to_find == NULL || *name_to_find == '\0')
        search_filter = 0;
    else
        search_filter = 1;

    for (int i = 0; i <= ARRAY_SIZE; i++)
    {
        // На всякий случай пропускаем пустые структуры в массиве
        if (array[i].not_empty)
        {

            /* Условие вывода записи на экран.
             *
             * Запись будет напечатана в двух случаях:
             * 1. Фильтр по имени отключен (!search_filter истинно).
             * 2. Фильтр по имени включен, и при этом имя текущего абонента совпадает с искомым именем (результат StrCompare ненулевой).
             */

            if (!search_filter || StrCompare(name_to_find, array[i].name))
            {
                printf("\nNumber:\t\t%d\nName:\t\t%s\nSecond name:\t%s\nPhone number:\t%s\n",
                       i + 1,
                       array[i].name,
                       array[i].second_name,
                       array[i].phone_number);
            }
        }
    }
};