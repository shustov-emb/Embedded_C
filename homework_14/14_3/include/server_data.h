/**
 * @file server_data.h
 * @brief Структуры данных, которые использует сервер
 * @version 0.1
 * @date 2026-04-13
 */

#ifndef SERVER_DATA_H
#define SERVER_DATA_H

#include <stddef.h>
#include "chat.h"

/**
 * @brief Информация об одном подключенном клиенте
 */
typedef struct ClientInfo
{
    char queue[CHAT_QUEUE_SIZE]; /* Имя личного объекта разделяемой памяти клиента */
} ClientInfo;

/**
 * @brief Список подключенных клиентов
 */
typedef struct ConnectionList
{
    ClientInfo clients[MAX_CLIENTS]; /* Массив клиентов */
    size_t count;                    /* Количество клиентов */
} ConnectionList;

/**
 * @brief История сообщений комнаты
 */
typedef struct MessageList
{
    ChatMessage messages[MAX_HISTORY]; /* Массив сообщений */
    size_t count;                      /* Количество сообщений */
} MessageList;

#endif // SERVER_DATA_H
