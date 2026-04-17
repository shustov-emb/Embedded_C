#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <mqueue.h>
#include <stdio.h>
#include <string.h>
#include "transport.h"

//Подготовка параметров очереди сообщений
static struct mq_attr BuildQueueAttr(void)
{
    struct mq_attr attr;

    //Размер сообщения задаётся по общей структуре чата
    memset(&attr, 0, sizeof(attr));
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(ChatMessage);

    return attr;
}

int TransportCreateReadQueue(TransportQueue *queue, const char *path)
{
    struct mq_attr attr = BuildQueueAttr();

    //Перед созданием удаляется старая очередь с таким же именем
    strncpy(queue->path, path, sizeof(queue->path) - 1);
    queue->path[sizeof(queue->path) - 1] = '\0';

    mq_unlink(path);
    queue->desc = (long)mq_open(path, O_CREAT | O_EXCL | O_RDWR, 0664, &attr);
    if ((mqd_t)queue->desc == (mqd_t)-1)
    {
        perror("Queue create error");
        return -1;
    }

    return 0;
}

int TransportOpenWriteQueue(TransportQueue *queue, const char *path)
{
    //Отправитель открывает уже существующую очередь только на запись
    strncpy(queue->path, path, sizeof(queue->path) - 1);
    queue->path[sizeof(queue->path) - 1] = '\0';

    queue->desc = (long)mq_open(path, O_WRONLY);
    if ((mqd_t)queue->desc == (mqd_t)-1)
    {
        perror("Queue open error");
        return -1;
    }

    return 0;
}

int TransportSend(TransportQueue *queue, const ChatMessage *message)
{
    //При отправке передаётся вся структура сообщения целиком
    if (mq_send((mqd_t)queue->desc, (const char *)message, sizeof(*message), 1) == -1)
    {
        if (errno != EBADF)
            perror("Send message error");
        return -1;
    }

    return 0;
}

int TransportReceive(TransportQueue *queue, ChatMessage *message)
{
    //Приём блокирует поток, пока в очереди не появится сообщение
    ssize_t result = mq_receive((mqd_t)queue->desc, (char *)message, sizeof(*message), NULL);

    if (result == -1)
    {
        if (errno != EINTR)
            perror("Receive message error");
        return -1;
    }

    return 0;
}

void TransportClose(TransportQueue *queue)
{
    //Закрытие дескриптора не удаляет саму очередь
    if ((mqd_t)queue->desc != (mqd_t)-1)
    {
        mq_close((mqd_t)queue->desc);
        queue->desc = -1;
    }
}

void TransportUnlink(const char *path)
{
    //Удаление очереди выполняется отдельно от закрытия
    mq_unlink(path);
}
