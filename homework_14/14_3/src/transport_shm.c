#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "transport.h"

//В каждом объекте разделяемой памяти хранится одно сообщение
typedef struct SharedChannel
{
    ChatMessage message;
} SharedChannel;

//Формирование имени семафора по имени объекта разделяемой памяти
static void BuildSemName(char *dst, size_t dst_size, const char *path, const char *suffix)
{
    snprintf(dst, dst_size, "%s_%s", path, suffix);
}

//Открытие двух семафоров, которые отвечают за пустую и заполненную ячейку
static int OpenSemaphores(TransportQueue *queue, int create)
{
    char empty_name[CHAT_QUEUE_SIZE + 16];
    char full_name[CHAT_QUEUE_SIZE + 16];
    int flags = create ? (O_CREAT | O_EXCL) : 0;

    BuildSemName(empty_name, sizeof(empty_name), queue->path, "empty");
    BuildSemName(full_name, sizeof(full_name), queue->path, "full");

    if (create)
    {
        sem_unlink(empty_name);
        sem_unlink(full_name);
    }

    queue->sem_empty = sem_open(empty_name, flags, 0664, 1);
    if (queue->sem_empty == SEM_FAILED)
    {
        perror("sem_open empty");
        queue->sem_empty = NULL;
        return -1;
    }

    queue->sem_full = sem_open(full_name, flags, 0664, 0);
    if (queue->sem_full == SEM_FAILED)
    {
        perror("sem_open full");
        sem_close((sem_t *)queue->sem_empty);
        queue->sem_empty = NULL;
        queue->sem_full = NULL;
        return -1;
    }

    return 0;
}

//Отображение объекта разделяемой памяти в адресное пространство процесса
static int MapChannel(TransportQueue *queue)
{
    queue->data = mmap(NULL,
                       sizeof(SharedChannel),
                       PROT_READ | PROT_WRITE,
                       MAP_SHARED,
                       queue->fd,
                       0);

    if (queue->data == MAP_FAILED)
    {
        perror("mmap");
        queue->data = NULL;
        return -1;
    }

    return 0;
}

int TransportCreateReadQueue(TransportQueue *queue, const char *path)
{
    memset(queue, 0, sizeof(*queue));
    queue->fd = -1;

    strncpy(queue->path, path, sizeof(queue->path) - 1);
    queue->path[sizeof(queue->path) - 1] = '\0';

    //Перед созданием удаляется старый объект с таким же именем
    TransportUnlink(path);

    queue->fd = shm_open(path, O_CREAT | O_EXCL | O_RDWR, 0664);
    if (queue->fd == -1)
    {
        perror("shm_open create");
        return -1;
    }

    if (ftruncate(queue->fd, sizeof(SharedChannel)) == -1)
    {
        perror("ftruncate");
        TransportClose(queue);
        TransportUnlink(path);
        return -1;
    }

    if (MapChannel(queue) == -1)
    {
        TransportClose(queue);
        TransportUnlink(path);
        return -1;
    }

    if (OpenSemaphores(queue, 1) == -1)
    {
        TransportClose(queue);
        TransportUnlink(path);
        return -1;
    }

    return 0;
}

int TransportOpenWriteQueue(TransportQueue *queue, const char *path)
{
    memset(queue, 0, sizeof(*queue));
    queue->fd = -1;

    strncpy(queue->path, path, sizeof(queue->path) - 1);
    queue->path[sizeof(queue->path) - 1] = '\0';

    queue->fd = shm_open(path, O_RDWR, 0664);
    if (queue->fd == -1)
    {
        perror("shm_open open");
        return -1;
    }

    if (MapChannel(queue) == -1)
    {
        TransportClose(queue);
        return -1;
    }

    if (OpenSemaphores(queue, 0) == -1)
    {
        TransportClose(queue);
        return -1;
    }

    return 0;
}

int TransportSend(TransportQueue *queue, const ChatMessage *message)
{
    SharedChannel *channel = queue->data;

    //Ждём, пока ячейка станет свободной для записи
    if (sem_wait((sem_t *)queue->sem_empty) == -1)
    {
        if (errno != EINTR)
            perror("sem_wait empty");
        return -1;
    }

    memcpy(&channel->message, message, sizeof(*message));

    //Сообщаем получателю, что сообщение появилось
    sem_post((sem_t *)queue->sem_full);

    return 0;
}

int TransportReceive(TransportQueue *queue, ChatMessage *message)
{
    SharedChannel *channel = queue->data;

    //Ждём, пока отправитель запишет сообщение
    if (sem_wait((sem_t *)queue->sem_full) == -1)
    {
        if (errno != EINTR)
            perror("sem_wait full");
        return -1;
    }

    memcpy(message, &channel->message, sizeof(*message));

    //После чтения ячейка снова свободна
    sem_post((sem_t *)queue->sem_empty);

    return 0;
}

void TransportClose(TransportQueue *queue)
{
    if (queue->data != NULL)
    {
        munmap(queue->data, sizeof(SharedChannel));
        queue->data = NULL;
    }

    if (queue->sem_empty != NULL)
    {
        sem_close((sem_t *)queue->sem_empty);
        queue->sem_empty = NULL;
    }

    if (queue->sem_full != NULL)
    {
        sem_close((sem_t *)queue->sem_full);
        queue->sem_full = NULL;
    }

    if (queue->fd != -1)
    {
        close(queue->fd);
        queue->fd = -1;
    }
}

void TransportUnlink(const char *path)
{
    char empty_name[CHAT_QUEUE_SIZE + 16];
    char full_name[CHAT_QUEUE_SIZE + 16];

    BuildSemName(empty_name, sizeof(empty_name), path, "empty");
    BuildSemName(full_name, sizeof(full_name), path, "full");

    shm_unlink(path);
    sem_unlink(empty_name);
    sem_unlink(full_name);
}
