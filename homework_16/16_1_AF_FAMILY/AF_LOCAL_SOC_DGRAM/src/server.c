 /**
 * @file client.c
 * @author Шустов Александр
 * @brief UDP-сервер на базе AF_LOCAL и SOCK_DGRAM.
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

#define PATH "/tmp/af_local_soc_dgram"
#define SIZE 1024

int main(void)
{
    printf("\n(AF_LOCAL/SOCK_DGRAM) SERVER\n");

    // Удаляем старый файл сокета, если он остался после прошлого запуска
    unlink(PATH);

    struct sockaddr_un server_addr = {0};
    struct sockaddr_un client_addr = {0};

    char buff[SIZE] = {0};

    // Заполняем адрес сервера: семейство локальных сокетов и путь к файлу сокета
    server_addr.sun_family = AF_LOCAL;
    strncpy(server_addr.sun_path, PATH, sizeof(server_addr.sun_path) - 1);

    // Создаем datagram-сокет для обмена отдельными сообщениями
    int server_fd = socket(AF_LOCAL, SOCK_DGRAM, 0);

    // Привязываем сокет к адресу, чтобы клиент мог отправить сообщение по PATH
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind error");
        exit(EXIT_FAILURE);
    }

    // Ждем сообщение от клиента и сохраняем адрес клиента для обратного ответа
    socklen_t addr_len = sizeof(client_addr);
    if (recvfrom(server_fd, buff, SIZE, 0, (struct sockaddr *)&client_addr, &addr_len) <= 0)
        perror("recvfrom error\n");

    printf("(server) %s\n", buff);

    // Отправляем ответ по адресу клиента, который был получен через recvfrom
    const char *msg = "hello from server!";
    if (sendto(server_fd, msg, strlen(msg) + 1, 0, (struct sockaddr *)&client_addr, addr_len) < 0)
        perror("sendto error");

    sleep(2);

    // Закрываем сокет и удаляем файл локального сокета
    close(server_fd);
    unlink(PATH);

    exit(EXIT_SUCCESS);
}
