#ifndef HOMEWORK_4_H
#define HOMEWORK_4_H
#define ARRAY_SIZE 100
#define STRING_SIZE 12

struct Subscriber
{
    char not_empty;
    char name[STRING_SIZE];
    char second_name[STRING_SIZE];
    char phone_number[STRING_SIZE];
};

void EraseCharArray(char *char_array);
void EraseStructure(struct Subscriber *subscriber);
void GetString(const char *message, char *buffer);
int  StrCompare(char *str1, char *str2);
void AddSubscriber(struct Subscriber array[]);
void DeleteSubscriber(struct Subscriber array[]);
void PrintSubscribers(struct Subscriber array[], char *name_ot_find);

#endif