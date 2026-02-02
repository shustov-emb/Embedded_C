#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "subscriber.h"
#include "list.h"
#include "io.h"


void PrintNode (Node *node, size_t index){
    printf("\nId:\t\t%zu\n", index);
    printf("Name:\t\t%s\n", node->data->name);
    printf("Second name:\t%s\n", node->data->second_name);
    printf("Phone number:\t%s\n\n", node->data->phone_number);
}

void PrintList(List *list)
{
    if (list->count < 1)
        return;

    Node *currentNode = list->head;
    size_t index = 1;

    do
    {
        PrintNode (currentNode, index++);
        currentNode = currentNode->nextNode;
    } while (currentNode != NULL);
}


struct Subscriber *GetUserData()
{
    //Решил calloc'ом сразу занулять выделяемую память
    Subscriber *subscriber = calloc(1, sizeof(Subscriber));
    printf("Name: ");
    subscriber->name = ReadString();

    printf("Second name: ");
    subscriber->second_name = ReadString();

    printf("Phone number: ");
    subscriber->phone_number = ReadString();
    
    return subscriber;
}

/*Тут мы прыгаем с головы и до первого узла у которого имя абонента совпадаем со строкой переданной пользоваетлем*/
void SearchSubscriber(char *name, List *list){
       
    if (list->count < 1 || name == NULL) return;

    Node *currentNode = list->head;
    size_t index = 1;
   
    do
    {

        if (strcmp(currentNode->data->name, name) == 0 ){
            PrintNode(currentNode, index);
            return;
        } 

       currentNode = currentNode->nextNode;
       index++;

    } while (currentNode != NULL);

}

