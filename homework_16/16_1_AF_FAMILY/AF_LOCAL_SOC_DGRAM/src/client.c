/**
 * @file client.c
 * @author Шустов Александр
 * @brief UDP-клиент на базе AF_LOCAL и SOCK_DGRAM.
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
#include <sys/un.h>
#include <sys/socket.h>

#define PATH "/tmp/af_local_soc_dgram"
#define CLIENT_PATH "/tmp/af_local_soc_dgram_client"
#define SIZE 1024

int main(void)
{
    printf("\n(AF_LOCAL/SOCK_DGRAM) CLIENT\n");

    struct sockaddr_un server_addr = {0};
    struct sockaddr_un client_addr = {0};
    char buff[SIZE] = {0};

    // Создаем сокет клиента
    int server_fd = socket(AF_LOCAL, SOCK_DGRAM, 0);

    // Готовим адрес клиента для приема ответа от сервера
    client_addr.sun_family = AF_LOCAL;
    unlink(CLIENT_PATH);
    strncpy(client_addr.sun_path, CLIENT_PATH, sizeof(client_addr.sun_path) - 1);

    // Привязываем клиентский сокет к CLIENT_PATH
    if (bind(server_fd, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
        perror("bind errror");

    // Заполняем адрес сервера, к которому будет отправлено сообщение
    server_addr.sun_family = AF_LOCAL;
    strncpy(server_addr.sun_path, PATH, sizeof(server_addr.sun_path) - 1);

    // Фиксируем адрес сервера для дальнейших send и recv
    if (connect(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        perror("connect error");

    // Отправляем сообщение серверу
    const char *msg = "hello from client!";
    if (send(server_fd, msg, strlen(msg) + 1, 0) < 0)
        perror("send error");

    // Принимаем ответ от сервера
    memset(buff, 0, SIZE);
    int n = recv(server_fd, buff, SIZE - 1, 0);
    if (n < 0)
    {
        perror("recv error");
        close(server_fd);
        unlink(CLIENT_PATH);
        exit(EXIT_FAILURE);
    }
    buff[n] = '\0';

    printf("(client) %s\n\n", buff);

    // Закрываем сокет и удаляем файл клиентского сокета
    close(server_fd);
    unlink(CLIENT_PATH);

    exit(EXIT_SUCCESS);
}
