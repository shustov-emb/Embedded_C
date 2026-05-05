/**
 * @file server.c
 * @author Шустов Александр
 * @brief Серверная часть, broadcast рассылки
 * @version 0.1
 * @date 2026-05-04
 *
 * @copyright Copyright (c) 2026
 *
 */

#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MSG_COUNT 10

int main(void)
{

    printf("\nPID: %d\n", getpid());
    printf("BROADCAST | SERVER!\n");

    struct sockaddr_in addr = {0};
    addr.sin_port = htons(9898);
    addr.sin_family = AF_INET;

    //Говорим что вещать мы будем на 255.255.255.255
    if (inet_pton(AF_INET, "255.255.255.255", &addr.sin_addr) < 0)
    {
        perror("inet_pton error");
        exit(EXIT_FAILURE);
    }

    int broadcast_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (broadcast_fd < 0)
    {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    //Разрешаем отправку бродкастов
    int flag = 1;
    if (setsockopt(broadcast_fd, SOL_SOCKET, SO_BROADCAST, &flag, sizeof(flag)) < 0)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    char buffer[50] = {0};

    socklen_t addr_size = sizeof(addr);

    for (size_t i = 0; i < MSG_COUNT; i++)
    {
        snprintf(buffer, sizeof(buffer), "%s: %d", "BROADCAST MESSAGE", (int)i + 1);
        printf("Broadcast sent: %s\n", buffer);
        if (sendto(broadcast_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, addr_size) < 0)
        {
            perror("sendto");
            exit(EXIT_FAILURE);
        }
    }

    printf("\n");
    exit(EXIT_SUCCESS);
}
