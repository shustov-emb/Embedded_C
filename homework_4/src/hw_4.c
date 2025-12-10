#include <stdio.h>
#include "hw_4.h"

void erase_char_array(char *charArray)
{
    for (int i = 0; i < stringSize; i++)
    {
        charArray[i] = 0;
    }
};

void erase_structure(struct subscriber *subscriber)
{
    erase_char_array(subscriber->name);
    erase_char_array(subscriber->second_name);
    erase_char_array(subscriber->phoneNumber);
    subscriber->notEmpty = 0;
    
};

void get_string(const char *message, char *buffer, int bufferSize)
{

    char *scanResult = 0;
    int length = 0;

    printf("\tEnter %s: ", message);

    scanResult = fgets(buffer, stringSize, stdin);

    if (scanResult != NULL)
    {
        while (buffer[length] != '\0')
        {
            length++;
        }

        if (length > 0 && buffer[length - 1] == '\n')
        {
            buffer[length - 1] = '\0';
        }

        else if (length == stringSize - 1 && buffer[stringSize - 2] != '\n')
        {
            while (getchar() != '\n')
                ;
        }
    }
};

int strcompare(char *str1, char *str2)
{

    for (int i = 0; i < stringSize; i++)
    {
        if (str1[i] != str2[i])
        {
            return 0;
        }
    }
    return 1;
};

void add_subscriber(struct subscriber array[])
{
    for (int i = 0; i < arraySize; i++)
    {
        if (!array[i].notEmpty)
        {
            array[i].notEmpty = 1;
            get_string("name", array[i].name, stringSize);
            get_string("second name", array[i].second_name, stringSize);
            get_string("phone number", array[i].phoneNumber, stringSize);
            return;
        }
    }

    printf("Subscriber limit reached!\n");
};

void delete_subscriber(struct subscriber array[])
{

    int input = 0;
    int scan_result = 0;

    if (array[0].notEmpty != 1)
    {
        printf("Nothing to delete!\n");
        return;
    }

    while (scan_result == 0 || (input >= arraySize && input < 0))
    {
        printf("\tEnter user number: ");
        scan_result = scanf("%d", &input);
        while (getchar() != '\n')
            ;
    }

    for (int i = input - 1; i < arraySize - 1; i++)
    {
        array[i] = array[i + 1];
    }

    erase_structure(&array[arraySize - 1]);
};

void print_subscribers(struct subscriber array[], char *nameToFind)
{

    int searchFilter = 0;

    if (nameToFind == NULL || *nameToFind == '\0')
        searchFilter = 0;
    else
        searchFilter = 1;

    for (int i = 0; i <= arraySize; i++)
    {
        if (array[i].notEmpty)
        {
            if (!searchFilter || strcompare(nameToFind, array[i].name))
            {
                    printf("\nNumber:\t\t%d\nName:\t\t%s\nSecond name:\t%s\nPhone number:\t%s\n",
                           i + 1,
                           array[i].name,
                           array[i].second_name,
                           array[i].phoneNumber);
            }
        }
    }
};