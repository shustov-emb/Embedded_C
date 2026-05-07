/**
 * @file client.c
 * @author Шустов Александр
 * @brief Генератор клиентов для теста сервера
 * @version 0.1
 * @date 2026-05-04
 *
 * @copyright Copyright (c) 2026
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SERVER_PATH "127.0.0.1"
#define SERVER_PORT 9898
#define CLIENT_COUNT 20

/**
 * @brief Структура для хранения данных о соединении
 */
typedef struct Connection
{
    int fd;
    struct sockaddr_in addr;
} Connection;

/**
 * @brief Генерирует клиентские соединения заполняя ими список Connetion
 *
 * @param [out] connections Количество клиентов для генерации
 * @param count Количество клиентов для заполнеия
 * @return int 0 - Успех, -1 - Ошибка
 */
int GenerateClients(Connection *connections, int count)
{
    int i = 0;
    while (i < count)
    {
        int client_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (client_fd < 0)
        {
            return -1;
        }
        connections[i].fd = client_fd;
        connections[i].addr.sin_family = AF_INET;

        connections[i].addr.sin_port = htons(SERVER_PORT);
        if (inet_pton(AF_INET, SERVER_PATH, &connections[i].addr.sin_addr) < 0)
        {
            perror("inet_pton error");
            close(client_fd);
            return -1;
        }

        i++;
    }

    return 0;
}

/**
 * @brief Основная функция для общения с сервером, клиент делает коннект к г.серверу,
 * заявку из очереди берёт дочерний сервер, и отвечает клиенту
 * @param args Структура Connection с клиентскими данными
 * @return void*
 */
void *TalkToServer(void *args)
{
    Connection *conn = (Connection *)args;
    char buffer[100] = {0};

    // Делаем коннект к г.серверу
    if (connect(conn->fd, (struct sockaddr *)&conn->addr, sizeof(conn->addr)) < 0)
    {
        perror("connection error");
        return NULL;
    }

    // Получаем данные уже от дочернего сервера
    int recv_bytes = recv(conn->fd, buffer, sizeof(buffer) - 1, 0);
    if (recv_bytes <= 0)
    {
        perror("recv error");
        close(conn->fd);
        return NULL;
    }

    buffer[recv_bytes] = '\0';

    // Если в ответ прищло BUSY,
    // то значит очередь заполнена
    // Прекращаеаем работу клиента
    if (!strcmp(buffer, "BUSY"))
    {
        printf("Queue is full\n");
    }
    else
    {
        printf("%s\n", buffer);
    }

    close(conn->fd);
    return NULL;
}

int main(void)
{
    printf("\nPull of servers + queue | CLIENT!\n");

    Connection connections[CLIENT_COUNT] = {0};
    // Генерируем списко клиентов, по сути они все различаются только дескриптором
    GenerateClients(connections, CLIENT_COUNT);

    pthread_t threads[CLIENT_COUNT] = {0};
    for (int i = 0; i < CLIENT_COUNT; i++)
    {
        if (pthread_create(&threads[i], NULL, TalkToServer, &connections[i]) != 0)
        {
            close(connections[i].fd);
            threads[i] = 0;
        }
    }

    for (int i = 0; i < CLIENT_COUNT; i++)
    {
        if (threads[i])
            pthread_join(threads[i], NULL);
    }

    printf("\n");
    exit(EXIT_SUCCESS);
}
