/**
 * @file client.c
 * @author Шустов Александр
 * @brief UDP-сервер на базе AF_INET и SOCK_DGRAM.
 * @version 0.1
 * @date 2026-04-20
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/un.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(void)
{
    printf("\n(AF_INET SOCK_DGRAM) SERVER\n");

    char buff[100] = {0};
    struct sockaddr_in server_addr = {0};
    struct sockaddr_in client_addr = {0};

    // Заполняем IPv4-адрес сервера и порт, на котором сервер будет принимать сообщения
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9932);
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0)
    {
        perror("inet_pton error");
        exit(EXIT_FAILURE);
    }

    // Создаем UDP-сокет сервера
    int server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_fd < 0)
    {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    // Привязываем сокет к адресу 127.0.0.1:9932
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind error");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Ждем датаграмму от клиента и сохраняем адрес отправителя
    socklen_t client_addr_size = sizeof(client_addr);

    int recv_bytes = recvfrom(server_fd, buff, sizeof(buff) - 1, 0, (struct sockaddr *)&client_addr, &client_addr_size);
    if (recv_bytes < 0)
    {
        perror("recvfrom error");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    buff[recv_bytes] = '\0';

    printf("(server) %s\n\n", buff);

    // Отправляем ответ на адрес клиента, полученный через recvfrom
    const char *msg = "hello from server!";
    if (sendto(server_fd, msg, strlen(msg) + 1, 0, (struct sockaddr *)&client_addr, client_addr_size) < 0)
    {
        perror("sendto error");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Закрываем серверный сокет
    close(server_fd);

    exit(EXIT_SUCCESS);
}
