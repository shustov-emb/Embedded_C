/**
 * @file client.c
 * @author Шустов Александр
 * @brief Клиентская часть, multicast рассылки
 * @version 0.1
 * @date 2026-05-04
 *
 * @copyright Copyright (c) 2026
 *
 */

#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define SERVER_MULTICAST_PATH "239.0.0.1"
#define SERVER_PORT 9898

int main(void)
{
    printf("\nPID: %d\n", getpid());
    printf("MULTICAST | CLIENT!\n");

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int multicast_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (multicast_fd < 0)
    {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    //Говорим ядру чтобы пакеты на 224.0.0.1 оно не отбрасывало, а перенаправляло с к нам
    struct ip_mreqn mreqn = {0};
    if (inet_pton(AF_INET, SERVER_MULTICAST_PATH, &mreqn.imr_multiaddr) < 0)
    {
        perror("mreqn inet_pton error");
        exit(EXIT_FAILURE);
    }
    mreqn.imr_address.s_addr = htonl(INADDR_ANY);
    mreqn.imr_ifindex = 0;
    setsockopt(multicast_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreqn, sizeof(mreqn));

    bind(multicast_fd, (struct sockaddr *)&addr, (socklen_t)sizeof(addr));

    char buff[50];
    int index = 0;
    while (index < 10)
    {
        memset(buff, 0, sizeof(buff));
        socklen_t addr_size = sizeof(addr);
        recvfrom(multicast_fd, buff, sizeof(buff), 0, (struct sockaddr *)&addr, &addr_size);
        printf("%s\n", buff);
        index++;
    }

    printf("\n");
    exit(EXIT_SUCCESS);
}
