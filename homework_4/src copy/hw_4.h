#ifndef HOMEWORK_4_H
#define HOMEWORK_4_H
#define arraySize 100
#define stringSize 12

struct subscriber
{
    char notEmpty;
    char name[stringSize];
    char second_name[stringSize];
    char phoneNumber[stringSize];
};

void erase_char_array(char *charArray);
void erase_structure(struct subscriber *subscriber);
void get_string(const char *message, char *buffer, int bufferSize);
int strcompare(char *str1, char *str2);
void add_subscriber(struct subscriber array[]);
void delete_subscriber(struct subscriber array[]);
void print_subscribers(struct subscriber array[], char *nameToFind);

#endif