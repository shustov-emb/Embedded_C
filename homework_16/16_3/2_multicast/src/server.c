/**
 * @file server.c
 * @author Шустов Александр
 * @brief Серверная часть, multicast рассылки
 * @version 0.1
 * @date 2026-05-04
 *
 * @copyright Copyright (c) 2026
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 9898
#define MULTICAST_PATH "239.0.0.1"
#define MSG_COUNT 10

int main(void)
{

    printf("\nPID: %d\n", getpid());
    printf("MULTICAST | SERVER!\n");

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, MULTICAST_PATH, &addr.sin_addr) < 0)
    {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }

    int multicast_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (multicast_fd < 0)
    {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    char buff[50] = {0};
    for (size_t i = 0; i < MSG_COUNT; i++)
    {
        snprintf(buff, sizeof(buff), "%s: %d", "MULTICAST MESSAGE", (int)i + 1);
        sendto(multicast_fd, buff, sizeof(buff), 0, (struct sockaddr *)&addr, (socklen_t)sizeof(addr));
        printf("multicast sent: %s\n", buff);
    }

    printf("\n");
    exit(EXIT_SUCCESS);
}
