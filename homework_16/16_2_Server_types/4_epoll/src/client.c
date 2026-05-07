/**
 * @file client.c
 * @author Шустов Александр
 * @brief Клиентская часть для сервера на epoll, в потоках рандомно создаётся tcp/udp покдлючение, и цепляестя к серверу
 * @version 0.1
 * @date 2026-04-30
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
#include <sys/time.h>

#define SERVER_PATH "127.0.0.1"
#define TCP_SERVER_PORT 9898
#define UDP_SERVER_PORT 9899
#define CLIENT_COUNT 20

/**
 * @brief Перечисление для определения вида протокола
 */
typedef enum ConnectionType
{
    TCP,
    UDP
} ConnectionType;

/**
 * @brief Структура для хранения данных о соединении
 */
typedef struct Connection
{
    int fd;
    ConnectionType type;
    struct sockaddr_in addr;
} Connection;

/**
 * @brief Функция, для генерации клиентов для подключения к серверу
 *
 * @param count Количество клиентов для генерации
 * @return Connection* Структура с данными для подключения
 */
Connection *GenerateClients(int count)
{
    // МОжно было и без выделения памяти, но как-то изначально забыл поменять так и осталось
    Connection *connections = malloc(sizeof(Connection) * count);
    if (!connections)
        return NULL;

    srand(time(NULL));

    for (int i = 0; i < count; i++)
    {

        // Рандомим тип протокола, и вешаем на соответсвенный порт, соответсвующего сервера
        int type, port;
        connections[i].type = rand() % 2;
        if (connections[i].type == TCP)
        {
            type = SOCK_STREAM;
            port = TCP_SERVER_PORT;
        }
        else
        {
            type = SOCK_DGRAM;
            port = UDP_SERVER_PORT;
        }

        connections[i].fd = socket(AF_INET, type, 0);
        if (connections[i].fd < 0)
        {
            free(connections);
            return NULL;
        }

        // Настраиваем таймаут (SO_RCVTIMEO) на сокете через setsockopt, если сервер недоступен recvfrom будет ждать вечно
        // И таймаут нужен чтобы избежать таких ситуаций
        struct timeval tv = {0};
        tv.tv_sec = 2;
        tv.tv_usec = 0;

        if (setsockopt(connections[i].fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
        {
            perror("setsockopt SO_RCVTIMEO error");
        }

        connections[i].addr.sin_family = AF_INET;
        connections[i].addr.sin_port = htons(port);
        if (inet_pton(AF_INET, SERVER_PATH, &connections[i].addr.sin_addr) <= 0)
        {
            close(connections[i].fd);
            free(connections);
            return NULL;
        }
    }    
    
    return connections;
}

/**
 * @brief Поточная функция для подключения к TCP серверу, и получению от него даты
 *
 * @param args В параметры передаётся структура типа Connection *
 * @return void*
 */
void *TalkToTCPServer(void *args)
{
    Connection *conn = (Connection *)args;
    char buffer[100] = {0};
    // Копируем сообщение для сервера в буфер
    snprintf(buffer, sizeof(buffer), "%s", "TCP CLIENT\0");

    if (connect(conn->fd, (struct sockaddr *)&conn->addr, sizeof(conn->addr)) < 0)
    {
        perror("TCP connection error");
        return NULL; // return -1;
    }

    if (send(conn->fd, buffer, sizeof(buffer), 0) <= 0)
    {
        perror("TCP send error");
        close(conn->fd);
        return NULL; // return -1;
    }

    int recv_bytes = recv(conn->fd, buffer, sizeof(buffer), 0);
    if (recv_bytes <= 0)
    {
        perror("TCP recv error");
        close(conn->fd);
        return NULL; // return -1;
    }

    buffer[recv_bytes - 1] = '\0';
    printf("(TCP) %s\n", buffer);

    close(conn->fd);

    return NULL; // return 0;
}

/**
 * @brief Поточная функция для подключения к UDP серверу, и получению от него даты
 *
 * @param args В параметры передаётся структура типа Connection *
 * @return void*
 */
void *TalkToUDPServer(void *args)
{
    Connection *conn = (Connection *)args;

    char buffer[100] = {0};
    snprintf(buffer, sizeof(buffer), "%s", "UDP CLIENT\0");

    socklen_t addr_len = sizeof(conn->addr);
    if (sendto(conn->fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&conn->addr, addr_len) < 0)
    {
        perror("UDP sendto error");
        return NULL;
    }

    memset(buffer, 0, sizeof(buffer));

    if (recvfrom(conn->fd, buffer, sizeof(buffer), 0, NULL, NULL) < 0)
    {
        perror("UDP sendto error");
        return NULL;
    }

    printf("(UDP) %s\n", buffer);

    close(conn->fd);

    return NULL; // return 0;
}

int main(void)
{

    printf("\nEPOLL | CLIENT!\n");

    // Генерируем клиентов
    Connection *connections = GenerateClients(CLIENT_COUNT);
    if (!connections)
    {
        exit(EXIT_FAILURE);
    }

    // В зависимости от типа протокола, назначаем потоку нужную функцию
    pthread_t threads[CLIENT_COUNT] = {0};
    for (int i = 0; i < CLIENT_COUNT; i++)
    {

        void *func = (connections[i].type == TCP) ? TalkToTCPServer : TalkToUDPServer;

        if (pthread_create(&threads[i], NULL, func, &connections[i]) != 0)
        {
            close(connections[i].fd);
            threads[i] = 0;
        }
    }

    // Ждём потоки
    for (int i = 0; i < CLIENT_COUNT; i++)
    {
        if (threads[i])
            pthread_join(threads[i], NULL);
    }

    free(connections);
    printf("\n");
    exit(EXIT_SUCCESS);
}
