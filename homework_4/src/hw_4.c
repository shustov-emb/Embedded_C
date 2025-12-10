#include <stdio.h>
#include "hw_4.h"

void EraseCharArray(char *char_array)
{
    for (int i = 0; i < STRING_SIZE; i++)
    {
        char_array[i] = 0;
    }
};

void EraseStructure(struct Subscriber *subscriber)
{
    EraseCharArray(subscriber->name);
    EraseCharArray(subscriber->second_name);
    EraseCharArray(subscriber->phone_number);
    subscriber->not_empty = 0;
    
};

void GetString(const char *message, char *buffer)
{

    char *scan_result = 0;
    int length = 0;

    printf("\tEnter %s: ", message);

    scan_result = fgets(buffer, STRING_SIZE, stdin);

    if (scan_result != NULL)
    {
        while (buffer[length] != '\0')
        {
            length++;
        }

        if (length > 0 && buffer[length - 1] == '\n')
        {
            buffer[length - 1] = '\0';
        }

        else if (length == STRING_SIZE - 1 && buffer[STRING_SIZE - 2] != '\n')
        {
            while (getchar() != '\n')
                ;
        }
    }
};

int StrCompare(char *str1, char *str2)
{

    for (int i = 0; i < STRING_SIZE; i++)
    {
        if (str1[i] != str2[i])
        {
            return 0;
        }
    }
    return 1;
};

void AddSubscriber(struct Subscriber array[])
{
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        if (!array[i].not_empty)
        {
            array[i].not_empty = 1;
            GetString("name", array[i].name);
            GetString("second name", array[i].second_name);
            GetString("phone number", array[i].phone_number);
            return;
        }
    }

    printf("Subscriber limit reached!\n");
};

void DeleteSubscriber(struct Subscriber array[])
{

    int input = 0;
    int scan_result = 0;

    if (array[0].not_empty != 1)
    {
        printf("Nothing to delete!\n");
        return;
    }

    while (scan_result == 0 || (input >= ARRAY_SIZE && input < 0))
    {
        printf("\tEnter user number: ");
        scan_result = scanf("%d", &input);
        while (getchar() != '\n')
            ;
    }

    for (int i = input - 1; i < ARRAY_SIZE - 1; i++)
    {
        array[i] = array[i + 1];
    }

    EraseStructure(&array[ARRAY_SIZE - 1]);
};

void PrintSubscribers(struct Subscriber array[], char *name_to_find)
{

    int search_filter = 0;

    if (name_to_find == NULL || *name_to_find == '\0')
        search_filter = 0;
    else
        search_filter = 1;

    for (int i = 0; i <= ARRAY_SIZE; i++)
    {
        if (array[i].not_empty)
        {
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