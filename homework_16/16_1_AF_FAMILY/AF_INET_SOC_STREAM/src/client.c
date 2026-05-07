/**
 * @file client.c
 * @author Шустов Александр
 * @brief TCP-клиент на базе AF_INET и SOCK_STREAM.
 * @version 0.1
 * @date 2026-04-20
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

int main(void)
{

    printf("\n(AF_INET SOCK_STREAM) CLIENT\n");

    char buff[100] = {0};
    struct sockaddr_in server_addr = {0};

    // Заполняем IPv4-адрес сервера и порт для подключения
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9931);
    inet_pton(AF_INET,"127.0.0.1",&server_addr.sin_addr);

    // Создаем TCP-сокет клиента
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // Подключаемся к серверу
    connect(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    // Отправляем сообщение серверу
    const char *msg = "hello from client!";
    send(server_fd, msg, strlen(msg) + 1, 0);

    // Принимаем ответ от сервера
    int recv_bytes = recv(server_fd, buff, sizeof(buff) - 1, 0);
    if (recv_bytes < 0)
    {
        perror("recv error");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    buff[recv_bytes] = '\0';

    printf("(client) %s\n\n", buff);

    // Закрываем сокет
    close(server_fd);

    exit(EXIT_SUCCESS);
}
