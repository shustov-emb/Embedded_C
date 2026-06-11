/**
 * @file server_utils.c
 * @author Шустов Александр
 * @brief Реализация вспомогательных функций серверного приложения
 * @version 0.1
 * @date 2026-06-10
 *
 * @copyright Copyright (c) 2026
 *
 */

// #include <sys/un.h>
#include <stdio.h>
#include <string.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include "server_utils.h"

ClientInfo client_list[MAX_CLIENTS];
int raw_fd = -1;
volatile sig_atomic_t stop_requested = 0;

void HandleSigint(int signal_number)
{
    (void)signal_number;

    // Атомарный флаг, чтобы остановить приложение по sigint
    stop_requested = 1;

    if (raw_fd >= 0)
    {
        close(raw_fd);
        raw_fd = -1;
    }

    close(STDIN_FILENO);
}

int InitSignalHandling()
{
    struct sigaction signal_action;

    memset(&signal_action, 0, sizeof(signal_action));
    signal_action.sa_handler = HandleSigint;
    if (sigemptyset(&signal_action.sa_mask) < 0)
        return -1;
    // обрабатываем sigint (ctrl+c)
    return sigaction(SIGINT, &signal_action, NULL);
}

int SendMessage(int raw_fd, struct ClientInfo *client, char *message)
{
    struct sockaddr_in client_addr = {0};

    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = client->client_ip;
    client_addr.sin_port = client->client_port;

    // Определяем размеры сообщений, udp заголовка
    int udphdr_size = sizeof(struct udphdr);
    int msg_size = strlen(message);
    // Создаём буфер под размер пакета
    int total_size = udphdr_size + msg_size;
    char buffer[total_size];

    // Получаем указатели на заголовк и сообщение в буфере
    struct udphdr *udp_header = (struct udphdr *)buffer;
    char *data = buffer + udphdr_size;

    // Создаём udp заголовок
    udp_header->dest = client->client_port;
    udp_header->source = htons(SERVER_PORT);
    udp_header->len = htons(total_size);
    udp_header->check = 0;

    memcpy(data, message, msg_size);

    return sendto(raw_fd, buffer, total_size, 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
}

int GetClientIndex(uint32_t client_ip, uint16_t client_port)
{
    for (size_t i = 0; i < MAX_CLIENTS; i++)
    {
        // Если порт и ip адрес из параметров совпадают с этими же полями из массива - клиент найден возвращаем индекс
        if (client_list[i].is_used && client_list[i].client_ip == client_ip && client_list[i].client_port == client_port)
        {
            return i;
        }
    }

    return -1;
}

int AddNewClient(uint32_t client_ip, uint16_t client_port)
{
    for (size_t i = 0; i < MAX_CLIENTS; i++)
    {
        // Записываем данные клиента в первую попашуюся свободную ячейку массива
        if (!client_list[i].is_used)
        {
            client_list[i].client_ip = client_ip;
            client_list[i].client_port = client_port;
            client_list[i].is_used = 1;
            client_list[i].msg_count = 0;
            return (int)i;
        }
    }

    return -1;
}

int RemoveClient(uint32_t client_ip, uint16_t client_port)
{

    // Получаем индекс и затираем клиента
    int client_index = GetClientIndex(client_ip, client_port);
    if (client_index >= 0)
    {
        memset(&client_list[client_index], 0, sizeof(ClientInfo));
        return 0;
    }

    return -1;
}

void *HandleRecive(void *arg)
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

    char send_buffer[4096] = {0};
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
            printf("recv bytes less than ip_header + udp header\n");
            continue;
        }

        // Определяем payload и отправителя
        char *data = (char *)(recv_buffer + ip_header_length + sizeof(struct udphdr));
        struct udphdr *udp_header = (struct udphdr *)(recv_buffer + ip_header_length);

        // Если сообщение пришло на наш порт - выводим его
        if (ntohs(udp_header->dest) == SERVER_PORT)
        {
            // Для удобного вывода ip получателя, по сути бессмысленно, потому что все делаем локально, но пусть будет
            struct in_addr src_addr;
            src_addr.s_addr = ip_header->saddr;
            char src_str[INET_ADDRSTRLEN];
            if (inet_ntop(AF_INET, &src_addr, src_str, INET_ADDRSTRLEN) == NULL)
            {
                perror("inet_ntop error");
                continue;
            }

            int data_len = ntohs(ip_header->tot_len) - ip_header_length - sizeof(struct udphdr);
            if (data_len < 0)
                continue;

            // Если клиента ещё не было в списке - добавляем
            int client_index = GetClientIndex(ip_header->saddr, udp_header->source);
            if (client_index < 0)
            {
                client_index = AddNewClient(ip_header->saddr, udp_header->source);
                if (client_index < 0)
                {
                    printf("client list is full\n");
                    continue;
                }
            }

            printf("%-2d | (%s:%d): %.*s\n", client_index, src_str, ntohs(udp_header->source), data_len, data);

            // Если от клиента пришла команда exit - удаляем из списка
            if (data_len == 4 && memcmp(data, "exit", 4) == 0)
            {
                RemoveClient(ip_header->saddr, udp_header->source);
                continue;
            }

            client_list[client_index].msg_count++;

            // Копируем в буфер отправки сообщение клиента с припиской message <msg_count>
            if (snprintf(send_buffer, sizeof(send_buffer), "%.*s %u", data_len, data, client_list[client_index].msg_count) >= (int)sizeof(send_buffer))
            {
                printf("message is too long\n");
                memset(send_buffer, 0, sizeof(send_buffer));
                continue;
            }

            // Отправляем сообщение клиенту
            if (SendMessage(raw_fd, &client_list[client_index], send_buffer) < 0)
                perror("sendto error");

            memset(send_buffer, 0, sizeof(send_buffer));
        }
    }

    return NULL;
}
