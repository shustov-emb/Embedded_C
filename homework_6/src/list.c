#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#include "subscriber.h"


void FreeNode (Node *node){
    free(node->data->name); 
    free(node->data->second_name);
    free(node->data->phone_number);
    free(node->data);
    free(node);
}

void Push(List *list, Subscriber *userData)
{

    //Выделяем память под узел
    struct Node *currentNode = malloc(sizeof(Node));

    
    if (!currentNode){
        free(currentNode);
        return;
    }

    //Если головы узла ещё нет
    if (!list->head)
    {
        //То делаем текущий узел головой
        list->head = currentNode;
        //Зануляем предыдущий узел головы
        currentNode->prevNode = NULL;
    }
    else
    {
        //Если узел не голова, у текущего узла в левый узел устанавляваем хвост
        currentNode->prevNode = list->tail;
        //Следующим узлом последнего узла списка, устанавливаем текущий узел
        list->tail->nextNode = currentNode;
    }

    //Добавляем данные в текущий узел
    currentNode->data = userData;
    //Поскольку 
    currentNode->nextNode = NULL;
    //И делаем текущий узел последним узлом списка
    list->tail = currentNode;
    list->count += 1;
}

void Pop(size_t id, List *list)
{

    if (id <= 0 || list->count < 1 || id > list->count )
        return;


    if (list->count == 1)
    {
        FreeNode(list->head);
        return;
    }

    /*Вычисляем центральное число по количеству элементов в списке.
      Это потребуется для определения с какого конца нам надо будет искать элемент для удаления */
    size_t median = list->count / 2;
    Node *currentNode;
    unsigned char backSearch;
    size_t i;

    //Если удаляется первый элемент, то освобождаем его и перепривязываем голову на следующий
    if(id == 1) {
        list->head->nextNode->prevNode = NULL;
        currentNode = list->head;
        list->head = list->head->nextNode;
        FreeNode(currentNode);
        list->count -= 1;
        return;
    }

    //С последним элементом также, освобождаем хвост и перепривязываем его на предыдущий элемент
    if(id == list->count) {
        list->tail->prevNode->nextNode = NULL;
        currentNode = list->tail;
        list->tail = list->tail->prevNode;
        FreeNode(currentNode);
        list->count -= 1;
        return; 
    }

    /*Если номер для удаления больше центрального числа, то переходить к нужном элементу мы начнём с конца,
      так как он будет ближе к концу и преходов потребуется меньше*/
    if (id > median){
        currentNode = list->tail;
        backSearch = 1;
        i = list->count;
    }
    //В противном случае переходить к нужному узлу будем сначала
    else{
        currentNode = list->head;
        backSearch = 0;
        i = 1;
    }

    //Прыгаем по узлам, пока не дойдём до искомого
    while (i != id)
    {
        //Переход с конца, до искомого
        if (backSearch){
            currentNode = currentNode->prevNode;
            i--;
        }
        //Переходим от начала до искогмого
        else{
            currentNode = currentNode->nextNode;
            i++;
        }
    }

    // Левый узел теперь считает своим правым узлом, узел справа от удаляемого
    currentNode->prevNode->nextNode = currentNode->nextNode;

    // Правый узел теперь считает своим левым узлом, узел слева от удаляемого
    currentNode->nextNode->prevNode = currentNode->prevNode;
    
    //Высвобождаем память узлаа
    FreeNode(currentNode);
    list->count -= 1;

}

//Удаляем всы узлы начиная с головы, высвобождаем память
void ClearList(List *list){
    
    if (list->count < 1) return;

    Node *nodeToDelete;

    do
    {
        nodeToDelete = list->head;
        list->head = list->head->nextNode;
        FreeNode(nodeToDelete);
    } while (list->head != NULL);
    
}