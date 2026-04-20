/**
 * @file client_ui.h
 * @author Шустов Александр
 * @brief Описание интерфейса клиента на ncurses
 * @version 0.1
 * @date 2026-04-13
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef CLIENT_UI_H
#define CLIENT_UI_H

#include <stddef.h>
#include <ncurses.h>
#include "chat.h"

/**
 * @brief Окна, из которых состоит интерфейс клиента
 */
typedef struct ClientUi
{
    WINDOW *clients_win; /* Родительское окно списка клиентов справа */
    WINDOW *chat_win;    /* Родительское окно сообщений слева */
    WINDOW *input_win;   /* Окно строки ввода команд снизу */
    WINDOW *clients_sub; /* Внутреннее окно списка клиентов */
    WINDOW *chat_sub;    /* Внутреннее окно сообщений */
} ClientUi;

/**
 * @brief Инициализирует ncurses и создаёт окна
 * @param ui Структура интерфейса
 */
void InitUi(ClientUi *ui);

/**
 * @brief Удаляет созданные окна
 * @param ui Структура интерфейса
 */
void DestroyUi(ClientUi *ui);

/**
 * @brief Пересоздаёт окна после старта или изменения размера терминала
 * @param ui Структура интерфейса
 */
void RecreateUi(ClientUi *ui);

/**
 * @brief Отрисовывает окно сообщений, список клиентов и строку ввода
 * @param ui Структура интерфейса
 * @param messages История сообщений
 * @param messages_count Количество сообщений в истории
 * @param clients Список клиентов
 * @param clients_count Количество клиентов
 * @param input Текущая строка ввода
 */
void DrawUi(ClientUi *ui,
            ChatMessage *messages,
            size_t messages_count,
            char clients[][CHAT_QUEUE_SIZE],
            size_t clients_count,
            const char *input);

#endif // CLIENT_UI_H
