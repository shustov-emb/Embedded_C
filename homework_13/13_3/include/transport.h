/**
 * @file transport.h
 * @brief Интерфейс транспорта между клиентом и сервером
 * @version 0.1
 * @date 2026-04-13
 */

#ifndef TRANSPORT_H
#define TRANSPORT_H

#include "chat.h"

/**
 * @brief Обёртка над очередью сообщений
 */
typedef struct TransportQueue
{
    long desc;                  /* Дескриптор очереди */
    char path[CHAT_QUEUE_SIZE]; /* Имя очереди */
} TransportQueue;

/**
 * @brief Создаёт очередь для чтения сообщений
 * @param queue Структура очереди
 * @param path Имя очереди
 * @return 0 при успехе, -1 при ошибке
 */
int TransportCreateReadQueue(TransportQueue *queue, const char *path);

/**
 * @brief Открывает существующую очередь для записи
 * @param queue Структура очереди
 * @param path Имя очереди
 * @return 0 при успехе, -1 при ошибке
 */
int TransportOpenWriteQueue(TransportQueue *queue, const char *path);

/**
 * @brief Отправляет сообщение в очередь
 * @param queue Очередь получателя
 * @param message Сообщение для отправки
 * @return 0 при успехе, -1 при ошибке
 */
int TransportSend(TransportQueue *queue, const ChatMessage *message);

/**
 * @brief Получает сообщение из очереди
 * @param queue Очередь для чтения
 * @param message Структура, куда будет записано сообщение
 * @return 0 при успехе, -1 при ошибке
 */
int TransportReceive(TransportQueue *queue, ChatMessage *message);

/**
 * @brief Закрывает очередь
 * @param queue Очередь для закрытия
 */
void TransportClose(TransportQueue *queue);

/**
 * @brief Удаляет очередь по имени
 * @param path Имя очереди
 */
void TransportUnlink(const char *path);

#endif // TRANSPORT_H
