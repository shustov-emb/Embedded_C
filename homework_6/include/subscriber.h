/**
 * @file subscriber.h
 * @author Шустов Александр
 * @brief 
 * @version 0.1
 * @date 2026-02-02
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H
#include <stddef.h> //Для size_t

/**
 * @brief Даём определение структуры двусвязного списка
 * 
 * @details Изначально планировалось подключить list.h, но в него уже включён subscriber.h
 * и получается циклическое включение данных файлов друг в друга.
 * Пришлось прибегнуть к простому созданию алиаса для данной структуры
 */
struct List; 
typedef struct List List; 

struct Node; 
typedef struct Node Node; 

/**
 * @brief Структура содержащая данные абонента
 */
typedef struct Subscriber
{
    char *name;         //Имя абонента
    char *second_name;  //Фамилия абонента
    char *phone_number; //Номер телефона
} Subscriber;

/**
 * @brief Выводит данные всех узлов списка
 * 
 * @param[in] list Список для вывода данных узлов
 */
void PrintList(List *list);

/**
 * @brief Вспомогательная функция для вывода в консоль данных выбранного узла
 * 
 * @param[in] node Узел для вывода его данных в консоль
 * @param[in] index Индек для вывода его в консоль вместе с датой
 */
void PrintNode (Node *node, size_t index);


/**
 * @brief Производит поиск по переданному имени в списке абонентов
 * 
 * @param[in] name Имя для поиска в списке
 * @param[in] list Список в котором будет производится поиск
 */
void SearchSubscriber(char *name, List *list);

/**
 * @brief Получет данные абонента
 * 
 * @return Возвращает ссылку на структуру с данными абонента
 */
struct Subscriber *GetUserData();

#endif //SUBSCRIBER_H