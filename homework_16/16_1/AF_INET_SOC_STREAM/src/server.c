/**
 * @file client.c
 * @author Шустов Александр
 * @brief TCP-сервер на базе AF_INET и SOCK_STREAM.
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

    printf("\n(AF_INET SOCK_STREAM) SERVER\n");
    struct sockaddr_in server_addr = {0};
    char buff[100] = {0};

    // Создаем TCP-сокет сервера
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // Заполняем IPv4-адрес сервера и порт, на котором сервер будет ждать подключение
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9931);
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) < 0)
    {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }
    // server_addr.sin_addr.s_addr = INADDR_ANY;

    // Привязываем сокет к адресу 127.0.0.1:9931
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind error");
    }

    // Переводим сокет в режим ожидания входящих подключений
    listen(server_fd, 5);

    // Принимаем подключение клиента
    int client_fd = accept(server_fd, NULL, NULL);

    // Получаем сообщение от клиента
    int recv_bytes = recv(client_fd, buff, sizeof(buff) - 1, 0);
    if (recv_bytes < 0)
    {
        perror("recv error");
        close(client_fd);
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    buff[recv_bytes] = '\0';

    printf("(server) %s\n\n", buff);

    // Отправляем ответ клиенту
    const char *msg = "hello from server!";
    send(client_fd, msg, strlen(msg) + 1, 0);

    sleep(2);

    // Закрываем серверный сокет и соединение с клиентом
    close(server_fd);
    close(client_fd);

    exit(EXIT_SUCCESS);
}
