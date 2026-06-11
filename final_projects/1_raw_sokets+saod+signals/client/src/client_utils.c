/**
 * @file client_utils.c
 * @author Шустов Александр
 * @brief Реализация вспомогательных функций клиентского приложения
 * @version 0.1
 * @date 2026-06-10
 *
 * @copyright Copyright (c) 2026
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <signal.h>
#include "client_utils.h"

int LOCAL_PORT = 0;
int raw_fd = -1;
volatile sig_atomic_t stop_requested = 0;
struct sockaddr_in serv_addr = {0};

void HandleSigint(int signal_number)
{
    (void)signal_number;

    // Атомарный флаг, чтобы остановить приложение по sigint
    stop_requested = 1;

    close(STDIN_FILENO);
}

int InitSignalHandling()
{
    struct sigaction signal_action;

    memset(&signal_action, 0, sizeof(signal_action));
    signal_action.sa_handler = HandleSigint;
    if (sigemptyset(&signal_action.sa_mask) < 0)
        return -1;
    // Обрабатываем sigint (ctrl+c)
    return sigaction(SIGINT, &signal_action, NULL);
}

int SendMessage(char *message)
{
    // Определяем размеры сообщений, udp заголовка
    int udphdr_size = sizeof(struct udphdr);
    int msg_size = strlen(message);
    // Создаём буфер под размер пакета
    int total_size = udphdr_size + msg_size;
    char buffer[total_size];
    // Получаем указатели на заголовк и сообщение в буфере
    struct udphdr *udp_header = (struct udphdr *)buffer;
    char *data = buffer + udphdr_size;

    // Копируем сообщение в буфер
    memcpy(data, message, msg_size);

    // Создаём udp заголовок
    udp_header->dest = htons(SERVER_PORT);
    udp_header->source = htons(LOCAL_PORT);
    udp_header->len = htons(total_size);
    udp_header->check = 0;

    return sendto(raw_fd, buffer, total_size, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
}

int InitApp()
{
    LOCAL_PORT = 9000 + (getpid() % 50000);

    raw_fd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (raw_fd < 0)
        return -1;

    serv_addr.sin_port = htons(SERVER_PORT);
    serv_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) != 1)
        return -1;

    return 0;
}

void *HandleReceive(void *arg)
{
    (void)arg;

    /* Через setsockopt устанавливаем интервал времени для сокета, чтобы иметь возможность
       раз в секунду выходить из recvfrom, чтобы проверить не закрылся ли принимающий сокет*/
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    if (setsockopt(raw_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv)) < 0)
    {
        perror("setsockopt error");
        return NULL;
    }

    char recv_buffer[4096];

    while (1)
    {
        memset(recv_buffer, 0, sizeof(recv_buffer));

        struct sockaddr_in sender_addr;
        socklen_t sender_addr_size = sizeof(sender_addr);
        // Ждём сообщения
        int recv_bytes = recvfrom(raw_fd, recv_buffer, sizeof(recv_buffer), 0, (struct sockaddr *)&sender_addr, &sender_addr_size);
        if (recv_bytes < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                if (stop_requested)
                    break;
                continue;
            }

            // Если сокет закрылся, выходим из цикла
            if (errno == EBADF)
                break;
            if (errno == EINTR && stop_requested)
                break;

            perror("recvfrom error");
            continue;
        }

        // Если получили что-то - разбиваем на заголовки
        struct iphdr *ip_header = (struct iphdr *)recv_buffer;
        int ip_header_length = ip_header->ihl * 4;

        if (recv_bytes < ip_header_length + (int)sizeof(struct udphdr))
        {
            printf("recv bytes less than ip_header\n");
            continue;
        }

        // Определяем payload и отправителя
        char *data = (char *)(recv_buffer + ip_header_length + sizeof(struct udphdr));
        struct udphdr *udp_header = (struct udphdr *)(recv_buffer + ip_header_length);

        // Если сообщение пришло на наш порт - выводим его
        if (ntohs(udp_header->dest) == LOCAL_PORT)
        {
            int data_len = ntohs(ip_header->tot_len) - ip_header_length - sizeof(struct udphdr);
            if (data_len < 0)
                continue;

            printf("%.*s\n\n", data_len, data);
        }
    }

    return NULL;
}
