/**
 * @file chat.h
 * @author Шустов Александр
 * @brief Общие константы и структура сообщения для клиента и сервера
 * @version 0.1
 * @date 2026-04-13
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef CHAT_H
#define CHAT_H

/**
 * @brief Максимальная длина текста сообщения
 */
#define CHAT_TEXT_SIZE 1024

/**
 * @brief Максимальная длина имени объекта разделяемой памяти клиента
 */
#define CHAT_QUEUE_SIZE 100

/**
 * @brief Максимальное количество клиентов в комнате
 */
#define MAX_CLIENTS 64

/**
 * @brief Максимальное количество сообщений в истории
 */
#define MAX_HISTORY 1024

/**
 * @brief Имя общего объекта разделяемой памяти сервера
 */
#define SERVER_QUEUE "/14_3_Server"

/**
 * @brief Тип сообщения, по которому сервер и клиент понимают действие
 */
typedef enum MessageType
{
    MSG_OPEN,   /* Подключение клиента */
    MSG_CLOSE,  /* Отключение клиента */
    MSG_TEXT,   /* Обычное сообщение в чат */
    MSG_SYSTEM, /* Служебное сообщение для вывода в чат */
    MSG_USERS   /* Служебное сообщение со списком клиентов */
} MessageType;

/**
 * @brief Сообщение, передаваемое через транспорт
 */
typedef struct ChatMessage
{
    MessageType msg_type;                 /* Тип сообщения */
    char client_queue[CHAT_QUEUE_SIZE];   /* Имя объекта разделяемой памяти клиента */
    char text[CHAT_TEXT_SIZE];            /* Текст сообщения или служебные данные */
} ChatMessage;

#endif // CHAT_H
