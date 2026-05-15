/**
 * @file client.c
 * @author Шустов Александр
 * @brief Простая реализация формирования UDP заголовка
 * 
 * @version 0.1
 * @date 2026-05-06
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

/**
 * @brief Отправляет сообщение на сервер,
 *
 * @param raw_fd Дескриптор
 * @return int -1 - Ошибка,  0 - Успех
 */
int SendMessage(int raw_fd)
{

    struct udphdr *udp_header = {0};
    // Место под заголовко (Можно было сразу написать 8, но чтоб уж наверняка)
    int udphdr_size = sizeof(struct udphdr);
    // Место под сообщение
    int msg_size = 14;

    // Собираем данне для отправки
    struct sockaddr_in serv_addr = {0};
    serv_addr.sin_port = htons(9898);
    serv_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) < 0)
    {
        return -1;
    }

    char buffer[udphdr_size + msg_size];
    // memset(buffer, 0, sizeof(buffer));

    // Заголовок кладём в начало буфера
    udp_header = (struct udphdr *)buffer;
    // Вычисляем указатель начала сообщения
    char *data = (char *)buffer + sizeof(struct udphdr);

    // КОпируем сообщение в буфер прямо за заголовком
    snprintf(data, msg_size, "Hello server!");

    // Настраиваем заголовок
    udp_header->dest = htons(9898);
    udp_header->source = htons(9899);
    udp_header->len = htons(sizeof(buffer));
    udp_header->check = 0; // У меня из без check = 0 работает все хорошо

    // Отправляем сообщение
    return sendto(raw_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&serv_addr, (socklen_t)sizeof(serv_addr));
}

int main(void)
{

    // Создаём raw сокет
    int raw_fd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (raw_fd < 0)
    {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    struct udphdr *udp_header = {0};
    if (SendMessage(raw_fd) < 0)
    {
        perror("SendMessage error");
        exit(EXIT_FAILURE);
    }

    // Данные для приёма пакетов
    struct sockaddr_in sender_addr;
    socklen_t addr_size;

    char buffer[4096];

    printf("\n");

    while (1)
    {
        // Обнуляем буфер, на всякий случай
        memset(buffer, 0, sizeof(buffer));

        // Получаем пакет
        addr_size = sizeof(sender_addr);
        int recv_bytes = recvfrom(raw_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&sender_addr, &addr_size);
        if (recv_bytes < 0)
        {
            perror("recvfrom error");
            continue;
        }

        struct iphdr *ip_header = (struct iphdr *)buffer;
        // ihl показывает длину ip заголовка, в int обычно в 32 битных словах
        // Надо перевести в байты
        int ip_header_length = ip_header->ihl * 4;

        // Если полученное количество байт меньше чем хотя бы два необходимых заголовка, то пропускаем пакет
        if (recv_bytes < ip_header_length + (int)sizeof(struct udphdr))
        {
            printf("recv bytes less than ip_header");
            continue;
        }

        // Достаём данные, смешаем начало буфера на длины ip и udp заголовка, udp заголовок - 8 байт
        // Поэтому его не нужно вычислять как длину ip заголовка, которая может варьироваться
        char *data = (char *)(buffer + ip_header_length + sizeof(struct udphdr));

        // Из udp заголовка можно достать порты назначения, и источника
        udp_header = (struct udphdr *)(buffer + ip_header_length);

        // Слушаем нужный порт, ждем ответа от сервера
        if (ntohs(udp_header->source) == 9898)
        {

            // Из всего пакета, мы вычитаем заголовки, это и будет длинна наших данных
            int data_len = ntohs(ip_header->tot_len) - ip_header_length - sizeof(struct udphdr);

            struct in_addr src_addr, dest_addr;
            src_addr.s_addr = ip_header->saddr;
            dest_addr.s_addr = ip_header->daddr;

            char src_str[INET_ADDRSTRLEN];
            char dest_str[INET_ADDRSTRLEN];
            // Переводим адреса в удобоваримый вариант, пользуемся удобным определением INET_ADDRSTRLEN
            inet_ntop(AF_INET, &src_addr, src_str, INET_ADDRSTRLEN);
            inet_ntop(AF_INET, &dest_addr, dest_str, INET_ADDRSTRLEN);

            // Выводим пакет
            printf("%-10s: %s:%d -> %s:%d\n", "Packet",
                   src_str, ntohs(udp_header->source),
                   dest_str, ntohs(udp_header->dest));
            printf("%-10s: %d\n", "Data len", data_len);
            printf("%-10s: %.*s\n\n", "Payload", data_len, data);
            break;
        }
    }

    // printf("\n");
    exit(EXIT_SUCCESS);
}