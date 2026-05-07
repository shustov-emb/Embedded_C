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
 * @brief Основная функция для общения с сервером, клиент делает коннект к г.серверу и ждёт сообщения с эндпоинтом обслуживающего сервера
 * Затем подключается к осблуживающему серверу и ждёт сообщения с датой и временем
 * @param args Структура Connection с клиентскими данными
 * @return void*
 */
void *TalkToServer(void *args)
{
    Connection *conn = (Connection *)args;
    char buff[100] = {0};
    char endpoint[32] = {0};
    char ip[16] = {0};
    unsigned short port = 0;
    // Подключамся к г.серверу
    if (connect(conn->fd, (struct sockaddr *)&conn->addr, sizeof(conn->addr)) < 0)
    {
        perror("connection error");
        printf("\n");
        return NULL;
    }

    // В ответ получаем эндпоинт, дочернего соервера
    int recv_bytes = recv(conn->fd, endpoint, sizeof(endpoint) - 1, 0);
    if (recv_bytes <= 0)
    {
        perror("recv error");
        close(conn->fd);
        return NULL;
    }
    endpoint[recv_bytes] = '\0';
    // Закрываем эндпоинт с г.сервером
    close(conn->fd);

    // Парсим ответ
    if (sscanf(endpoint, "%15[^:]:%hu", ip, &port) != 2)
    {
        fprintf(stderr, "invalid endpoint: %s\n", endpoint);
        return NULL;
    }

    // Получаем новый дескриптор для общения с дочерним сервером
    conn->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (conn->fd < 0)
    {
        return NULL;
    }

    // Перезаписываем структуру данными нового сервера
    conn->addr.sin_family = AF_INET;
    conn->addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &conn->addr.sin_addr) <= 0)
    {
        perror("inet_pton error");
        close(conn->fd);
        return NULL;
    }

    // Коннектимся к дочернему серверу
    if (connect(conn->fd, (struct sockaddr *)&conn->addr, sizeof(conn->addr)) < 0)
    {
        perror("connection error");
        printf("\n");
        return NULL;
    }

    // Принимаем данные от дочернего сервера
    recv_bytes = recv(conn->fd, buff, sizeof(buff) - 1, 0);
    if (recv_bytes < 0)
    {
        perror("recv error");
        close(conn->fd);
        return NULL;
    }

    buff[recv_bytes] = '\0';
    printf("%s\n", buff);

    close(conn->fd);
    return NULL;
}

int main(void)
{

    printf("\nSimple parallel | CLIENT!\n");
    Connection connections[CLIENT_COUNT] = {0};
    // Генерируем списко клиентов, по сути они все различаются только дескриптором
    GenerateClients(connections, CLIENT_COUNT);

    pthread_t threads[CLIENT_COUNT] = {0};

    for (size_t i = 0; i < CLIENT_COUNT; i++)
    {
        // Отправляем клиента в поток для мучения сервера
        if (pthread_create(&threads[i], NULL, TalkToServer, &connections[i]) != 0)
        {
            close(connections[i].fd);
            threads[i] = 0;
        }
    }

    for (size_t i = 0; i < CLIENT_COUNT; i++)
    {
        if (threads[i])
            pthread_join(threads[i], NULL);
    }

    printf("\n");
    exit(EXIT_SUCCESS);
}
