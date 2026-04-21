/**
 * @file client.c
 * @author Шустов Александр
 * @brief Stream-клиент на базе AF_LOCAL и SOCK_STREAM.
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

#define PATH "/tmp/af_local_sock_stream"

int main(void)
{

    printf("\nAF_LOCAL SOC_STREAM CLIENT\n");

    struct sockaddr_un addr = {0};
    char buff[1024] = {0};

    // Создаем stream-сокет клиента
    int server_fd = socket(AF_LOCAL, SOCK_STREAM, 0);

    // Заполняем адрес сервера, к которому нужно подключиться
    addr.sun_family = AF_LOCAL;
    strncpy(addr.sun_path, PATH, sizeof(addr.sun_path) - 1);

    // Подключаемся к серверному локальному сокету
    connect(server_fd, (struct sockaddr *)&addr, sizeof(addr));

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

    printf("(client) %s\n", buff);

    // Закрываем сокет
    close(server_fd);

    exit(EXIT_SUCCESS);
}
