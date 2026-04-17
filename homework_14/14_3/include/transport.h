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
 * @brief Обёртка над объектом разделяемой памяти
 */
typedef struct TransportQueue
{
    int fd;                     /* Дескриптор объекта разделяемой памяти */
    void *data;                 /* Адрес отображённой памяти */
    void *sem_empty;            /* Семафор свободной ячейки */
    void *sem_full;             /* Семафор заполненной ячейки */
    char path[CHAT_QUEUE_SIZE]; /* Имя объекта разделяемой памяти */
} TransportQueue;

/**
 * @brief Создаёт объект разделяемой памяти для чтения сообщений
 * @param queue Структура транспорта
 * @param path Имя объекта разделяемой памяти
 * @return 0 при успехе, -1 при ошибке
 */
int TransportCreateReadQueue(TransportQueue *queue, const char *path);

/**
 * @brief Открывает существующий объект разделяемой памяти для записи
 * @param queue Структура транспорта
 * @param path Имя объекта разделяемой памяти
 * @return 0 при успехе, -1 при ошибке
 */
int TransportOpenWriteQueue(TransportQueue *queue, const char *path);

/**
 * @brief Отправляет сообщение через разделяемую память
 * @param queue Транспорт получателя
 * @param message Сообщение для отправки
 * @return 0 при успехе, -1 при ошибке
 */
int TransportSend(TransportQueue *queue, const ChatMessage *message);

/**
 * @brief Получает сообщение из разделяемой памяти
 * @param queue Транспорт для чтения
 * @param message Структура, куда будет записано сообщение
 * @return 0 при успехе, -1 при ошибке
 */
int TransportReceive(TransportQueue *queue, ChatMessage *message);

/**
 * @brief Закрывает объект разделяемой памяти
 * @param queue Транспорт для закрытия
 */
void TransportClose(TransportQueue *queue);

/**
 * @brief Удаляет объект разделяемой памяти по имени
 * @param path Имя объекта разделяемой памяти
 */
void TransportUnlink(const char *path);

#endif // TRANSPORT_H
