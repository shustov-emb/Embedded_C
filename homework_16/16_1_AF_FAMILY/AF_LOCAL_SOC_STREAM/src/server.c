/**
 * @file client.c
 * @author Шустов Александр
 * @brief Stream-сервер на базе AF_LOCAL и SOCK_STREAM.
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

#define PATH "/tmp/af_local_sock_stream"

int main(void)
{
    printf("\nAF_LOCAL SOC_STREAM SERVER\n");

    int server_fd, client_fd;
    struct sockaddr_un addr = {0};
    char buff[1024] = {0};

    // Создаем stream-сокет сервера для соединения с клиентом
    server_fd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (server_fd < 0)
        perror("socket error");

    // Заполняем адрес сервера: семейство локальных сокетов и путь к файлу сокета
    addr.sun_family = AF_LOCAL;
    strncpy(addr.sun_path, PATH, sizeof(addr.sun_path) - 1);

    // Удаляем старый файл сокета и привязываем новый сокет к PATH
    unlink(PATH);
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind error");
        close(server_fd);
    }

    // Переводим сокет в режим ожидания входящих подключений
    if (listen(server_fd, 5) < 0)
        perror("listen error");

    // Принимаем подключение клиента
    client_fd = accept(server_fd, NULL, NULL);

    // Получаем сообщение от подключенного клиента
    int received_bytes = recv(client_fd, buff, sizeof(buff) - 1, 0);
    if (received_bytes < 0)
    {
        perror("recv error");
        close(client_fd);
        close(server_fd);
        unlink(PATH);
        exit(EXIT_FAILURE);
    }
    buff[received_bytes] = '\0';

    // Выводим полученное сообщение
    printf("(server) %s\n", buff);

    // Отправляем ответ клиенту по принятому соединению
    const char *msg = "hello from server!";
    send(client_fd, msg, strlen(msg) + 1, 0);

    printf("\n");

    sleep(1);

    // Закрываем соединение, серверный сокет и удаляем файл сокета
    close(client_fd);
    close(server_fd);
    unlink(PATH);

    exit(EXIT_SUCCESS);
}
