/**
 * @file server.c
 * @author Шустов Александр
 * @brief Слушающий сервер принимает клиентские подключения, кладёт их в очередь,
 * а дочерние сервера забирают заявки и обслуживают клиентов
 * @version 0.1
 * @date 2026-05-04
 *
 * @copyright Copyright (c) 2026
 *
 */

#define _POSIX_C_SOURCE 200809L
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

#define SERVER_IP "127.0.0.1"
#define MAIN_SERVER_PORT 9898
#define MAIN_SERVER_LISTEN_COUNT 10
#define CHILD_SERVER_COUNT 3
#define REQUEST_QUEUE_SIZE 5

/**
 * @brief Структура для хранения данных о соединении
 */
typedef struct Connection
{
    int fd;
    struct sockaddr_in addr;
} Connection;

/**
 * @brief Данные дочернего сервера
 *
 */
typedef struct ChildServerState
{
    int id;
    int busy;
} ChildServerState;

/**
 * @brief Структура очереди заявок
 *
 */
typedef struct RequestQueue
{
    Connection requests[REQUEST_QUEUE_SIZE];
    int head;
    int tail;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t condvar;
} RequestQueue;

/**
 * @brief Данные дочернего потока
 *
 */
typedef struct ChildServerData
{
    ChildServerState state;
    RequestQueue *queue;
} ChildServerData;

/**
 * @brief Данные консольного потока
 *
 */
typedef struct ConsoleData
{
    RequestQueue *queue;
} ConsoleData;

volatile sig_atomic_t keep_running = 1;

/**
 * @brief Получить дату и время с миллискундами для наглядности
 *
 * @param [out] output Буфер в который копируется строка
 * @param output_size Размер буфера
 * @return int 0 - Успех, -1 - Ошибка
 */
int GetTime(char *output, size_t output_size)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    struct tm info = {0};
    if (localtime_r(&tv.tv_sec, &info) == NULL)
    {
        return -1;
    }

    strftime(output, output_size, "%d.%m.%y %H:%M:%S", &info);
    sprintf(output + 17, ".%03d", (int)(tv.tv_usec / 1000));
    return 0;
}

/**
 * @brief Инициализация главного сервера
 *
 * @param [out] connection Структура с даннымы сервера для заполнения
 * @param listen_count Сколько подключений слушать
 * @return int 0 - Успех, -1 - Ошибка
 */
int InitServer(Connection *connection, int listen_count)
{
    connection->addr.sin_family = AF_INET;
    connection->addr.sin_port = htons(MAIN_SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &connection->addr.sin_addr) <= 0)
    {
        return -1;
    }

    connection->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (connection->fd < 0)
    {
        return -1;
    }

    // Задаём для сокета настройку, которая позволяет использовать порт сразу после закрытия соединения
    int opt = 1;
    if (setsockopt(connection->fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        close(connection->fd);
        return -1;
    }

    // Раз в секунду accept будет просыпаться и проверять флаг завершения
    struct timeval tv = {0};
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    if (setsockopt(connection->fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
    {
        close(connection->fd);
        return -1;
    }

    socklen_t server_addr_size = sizeof(connection->addr);
    if (bind(connection->fd, (struct sockaddr *)&connection->addr, server_addr_size) < 0)
    {
        close(connection->fd);
        return -1;
    }

    if (listen(connection->fd, listen_count) < 0)
    {
        close(connection->fd);
        return -1;
    }

    return 0;
}

/**
 * @brief Инициализирует очередь заявок
 *
 * @param [out] queue Очередь для заполнения
 * @return int 0 - Успех, -1 - Ошибка
 */
int InitQueue(RequestQueue *queue)
{
    memset(queue, 0, sizeof(RequestQueue));

    if (pthread_mutex_init(&queue->mutex, NULL) != 0)
    {
        return -1;
    }

    if (pthread_cond_init(&queue->condvar, NULL) != 0)
    {
        pthread_mutex_destroy(&queue->mutex);
        return -1;
    }

    return 0;
}

/**
 * @brief Удаляет очередь заявок
 *
 * @param queue Очередь заявок
 */
void DestroyQueue(RequestQueue *queue)
{
    pthread_cond_destroy(&queue->condvar);
    pthread_mutex_destroy(&queue->mutex);
}

/**
 * @brief Добавляет заявку в очередь
 *
 * @param queue Очередь заявок
 * @param client_info Данные клиента
 * @return int 0 - Успех, -1 - Ошибка
 */
int PushRequest(RequestQueue *queue, Connection *client_info)
{
    pthread_mutex_lock(&queue->mutex);

    if (queue->count == REQUEST_QUEUE_SIZE)
    {
        pthread_mutex_unlock(&queue->mutex);
        return -1;
    }

    // Кладём новую заявку в хвост очереди
    queue->requests[queue->tail] = *client_info;
    // Сдвигаем хвост по кругу, чтобы после конца массива вернуться в начало
    queue->tail = (queue->tail + 1) % REQUEST_QUEUE_SIZE;
    // Увеличиваем текущее количество заявок в очереди
    queue->count++;

    pthread_cond_signal(&queue->condvar);
    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

/**
 * @brief Извлекает заявку из очереди
 *
 * @param queue Очередь заявок
 * @param [out] client_info Данные клиента
 * @return int 0 - Успех, -1 - Ошибка
 */
int PopRequest(RequestQueue *queue, Connection *client_info)
{
    pthread_mutex_lock(&queue->mutex);

    // Если заявок нет, поток ждёт пока главный поток не добавит новую заявку
    while (!queue->count && keep_running)
    {
        pthread_cond_wait(&queue->condvar, &queue->mutex);
    }

    if (!queue->count && !keep_running)
    {
        pthread_mutex_unlock(&queue->mutex);
        return -1;
    }

    // Забираем самую старую заявку из головы очереди
    *client_info = queue->requests[queue->head];
    // Сдвигаем голову по кругу
    queue->head = (queue->head + 1) % REQUEST_QUEUE_SIZE;
    // Уменьшаем текущее количество заявок в очереди
    queue->count--;

    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

/**
 * @brief Обработка заявки дочерним потоком
 *
 * Поток ждёт появления заявки в очереди, обслуживает клиента и после
 * завершения обработки помечает себя как свободный
 *
 * @param arg Данные дочернего потока
 * @return void* NULL
 */
void *HandleChildServer(void *arg)
{
    ChildServerData *child_server_data = (ChildServerData *)arg;

    while (keep_running)
    {
        Connection client_info = {0};
        if (PopRequest(child_server_data->queue, &client_info) < 0)
        {
            return NULL;
        }

        child_server_data->state.busy = 1;

        char buffer[100] = {0};
        if (GetTime(buffer, sizeof(buffer)) == 0)
        {
            if (send(client_info.fd, buffer, strlen(buffer), 0) < 0)
            {
                perror("HandleChildServer send error");
            }
        }

        close(client_info.fd);
        child_server_data->state.busy = 0;
    }

    return NULL;
}

/**
 * @brief Вспомогательная функция для считывания пользовательского ввода с клавиатуры
 * Нужна для обработки выхода из приложения
 * @param arg Пустой аргумент
 * @return void*
 */
void *HandleConsole(void *arg)
{
    ConsoleData *console_data = (ConsoleData *)arg;
    char command[16] = {0};

    while (fgets(command, sizeof(command), stdin))
    {
        if (command[0] == 'q')
        {
            keep_running = 0;

            // Будим все дочерние потоки, которые спят на пустой очереди
            pthread_mutex_lock(&console_data->queue->mutex);
            pthread_cond_broadcast(&console_data->queue->condvar);
            pthread_mutex_unlock(&console_data->queue->mutex);
            return NULL;
        }
    }

    return NULL;
}

int main()
{
    printf("PID: %d\n", getpid());
    printf("Pull of servers + queue | SERVER!\n");

    // Инициализируем очередь заявок
    RequestQueue queue = {0};
    if (InitQueue(&queue) < 0)
    {
        perror("InitQueue create error");
        exit(EXIT_FAILURE);
    }

    // Запускаем поток, который слушает команду q из консоли
    ConsoleData console_data = {0};
    console_data.queue = &queue;

    pthread_t console_thread = {0};
    if (pthread_create(&console_thread, NULL, HandleConsole, &console_data) != 0)
    {
        DestroyQueue(&queue);
        perror("HandleConsole thread create error");
        exit(EXIT_FAILURE);
    }

    // Запускаем дочерние потоки обработки
    ChildServerData child_servers[CHILD_SERVER_COUNT] = {0};
    pthread_t child_server_threads[CHILD_SERVER_COUNT] = {0};
    for (int i = 0; i < CHILD_SERVER_COUNT; i++)
    {
        child_servers[i].state.id = i;
        child_servers[i].state.busy = 0;
        child_servers[i].queue = &queue;

        if (pthread_create(&child_server_threads[i], NULL, HandleChildServer, &child_servers[i]) != 0)
        {
            keep_running = 0;

            // Будим уже созданные потоки и дожидаемся их завершения
            pthread_mutex_lock(&queue.mutex);
            pthread_cond_broadcast(&queue.condvar);
            pthread_mutex_unlock(&queue.mutex);

            for (int j = 0; j < i; j++)
            {
                pthread_join(child_server_threads[j], NULL);
            }

            DestroyQueue(&queue);
            perror("HandleChildServer thread create error");
            exit(EXIT_FAILURE);
        }
    }

    // Инициализируем главный сервер
    Connection listen_server_data = {0};
    if (InitServer(&listen_server_data, MAIN_SERVER_LISTEN_COUNT) < 0)
    {
        DestroyQueue(&queue);
        perror("InitServer error");
        exit(EXIT_FAILURE);
    }

    while (keep_running)
    {
        // Принимаем входящие заявки от клиентов
        Connection client_info = {0};
        socklen_t client_addr_size = sizeof(client_info.addr);
        client_info.fd = accept(listen_server_data.fd, (struct sockaddr *)&client_info.addr, &client_addr_size);
        if (client_info.fd < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                continue;
            else if (errno == EINTR)
                break;
            else
                continue;
        }

        // Если очередь заполнена, отправляем клиенту "BUSY"
        if (PushRequest(&queue, &client_info) < 0)
        {
            send(client_info.fd, "BUSY", 5, 0);
            close(client_info.fd);
            continue;
        }
    }

    close(listen_server_data.fd);

    // Будим все дочерние потоки и завершаем их работу
    pthread_mutex_lock(&queue.mutex);
    pthread_cond_broadcast(&queue.condvar);
    pthread_mutex_unlock(&queue.mutex);

    for (int i = 0; i < CHILD_SERVER_COUNT; i++)
    {
        pthread_join(child_server_threads[i], NULL);
    }

    pthread_cancel(console_thread);
    pthread_join(console_thread, NULL);

    DestroyQueue(&queue);
    printf("\nServer shutting down!\n");
    exit(EXIT_SUCCESS);
}
