/**
 * @file client.c
 * @author Шустов Александр
 * @brief Клиентская часть, broadcast рассылки
 * @version 0.1
 * @date 2026-05-04
 *
 * @copyright Copyright (c) 2026
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>

#define SERVER_PATH "255.255.255.255"
#define SERVER_PORT 9898

int main(void)
{
    printf("\nPID: %d\n", getpid());
    printf("BROADCAST | CLIENT!\n");

    struct sockaddr_in addr = {0};
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, SERVER_PATH, &addr.sin_addr) < 0)
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

    if (bind(broadcast_fd, (struct sockaddr *)&addr, (socklen_t)sizeof(addr)) < 0)
    {
        perror("bind error");
        exit(EXIT_FAILURE);
    }

    char buff[50];

    int index = 0;
    while (index < 10)
    {
        memset(buff, 0, sizeof(buff));

        socklen_t addr_size = sizeof(addr);
        if (recvfrom(broadcast_fd, buff, sizeof(buff), 0, (struct sockaddr *)&addr, &addr_size) <= 0)
        {
            continue;
        }

        printf("%s\n", buff);
        index++;
    }

    printf("\n");
    exit(EXIT_SUCCESS);
}
