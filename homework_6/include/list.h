/**
 * @file list.h
 * @author Шустов Александр
 * @brief 
 * @version 0.1
 * @date 2026-02-02
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef LIST_H
#define LIST_H
#include <stddef.h> //Для size_t
#include "subscriber.h" //Для возможности использовать структуру абонента

/**
 * @brief Структура содержащая информацию узла списка
 */
typedef struct Node
{
    Subscriber *data;       //Указатель на структуру с данными абонента
    struct Node *nextNode;  //Указатель на следующий узел списка
    struct Node *prevNode;  //Указатель на предыдущий узел списка
} Node;

/**
 * @brief Структура двусвязного списка
 */
typedef struct List
{
    size_t count;   //Хранит количество элеменотв списка
    Node *head;     //Указатель на начало списка
    Node *tail;     //Указатель на конец списка
} List;

/**
 * @brief Высвобождает выделенную под узел память  списка
 * 
 * @details Так же высвобождает абонента и его данные
 * 
 * @param[in] node Узел двусвязного списка который необходимо высвободить
 */
void FreeNode (Node *node);

/**
 * @brief Добавляет данные абонента в двусвязный список
 * 
 * @details Функция выделяет память для узла списка. Вызывающий код обязан освободить её с помощью free().
 * 
 * @param[in] list Список в который будет добавлен узел
 * @param[in] userData Данные азла списка
 */
void Push(List *list, Subscriber *userData);

/**
 * @brief Удаляет выбранный по ID элемент списка
 * 
 * @details Высвобождает память выделенную под выбранный узел
 * 
 * @param[in] id Номер узла для удаления
 * @param[in] list Список из которого будет удалятся узел
 */
void Pop(size_t id, List *list);

/**
 * @brief Полностью очищает двусвязыный список
 * 
 * @details Проходит по каждому узлу списка высвобождая выделенную память
 * 
 * @param[in] list Список для очистки
 */
void ClearList(List *list);

#endif //LILST_H