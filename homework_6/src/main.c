#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include "io.h"
#include "list.h"
#include "subscriber.h"

int main()
{
    List *list = malloc(sizeof(List));

    if (list == NULL) return -1;

    list->head = NULL;
    list->tail = NULL;
    list->count = 0;

    int running = 1;
    while (running) {
        printf("\n1. Add subscriber\n2. Delete subscriber\n3. Find subscriber\n4. Print subscriber list\n5. EXIT\n");
        size_t choice = GetNumber("Enter value: ");

        switch (choice) {
            case 1:
                Push(list, GetUserData());
                break;
            case 2:
                Pop(GetNumber("Id to delete: "), list);
                break;
            case 3:
                printf("Name to find: ");
                char *name = ReadString();
                if (name && name[0] != '\0') 
                    SearchSubscriber(name, list);
                free(name);
                break;
            case 4:
                PrintList(list);
                break;
            case 5:
                ClearList(list);
                free(list);
                running = 0;
                break;
            default:
                printf("Wrong option!\n");
        }
    }
    return 0;
}
