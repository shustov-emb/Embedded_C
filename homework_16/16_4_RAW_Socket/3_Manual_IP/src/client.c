/**
 * @file client.c
 * @author Шустов Александр
 * @brief Простая реализация формирования IP+UDP заголовков
 * @version 0.1
 * @date 2026-05-13
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

#define SRC_IP "127.0.0.1"
#define DST_IP "127.0.0.1"
#define SRC_PORT 9899
#define DST_PORT 9898

/**
 * @brief Простенькая структура для хранения ip пакета и его размера
 *
 */
typedef struct PacketBuffer
{
    char *data;
    int size;
} PacketBuffer;

/**
 * @brief Заполняет IP заголовок
 *
 * @param [out] ip Указатель на iphdr структуру
 * @param packet_size размер всего пакета включая udp с payload
 * @return int 0 - Успех, -1 - Ошибка
 */
int FillIPHeader(struct iphdr *ip, int packet_size) //, int packet_size
{
    memset(ip, 0, sizeof(*ip));

    // Заполняем заголовок, не забывая что некоторые данные идут в litle-endian
    ip->version = 4;
    ip->ihl = 5;
    ip->tos = 0;
    ip->tot_len = htons(packet_size); // Можно не заполнять, заполнится ядром
    ip->id = htons(0);                // Можно не заполнять, заполнится ядром
    ip->frag_off = htons(0);
    ip->ttl = 64;
    ip->protocol = IPPROTO_UDP; // МОжно просто 17, но так понятнее

    if (inet_pton(AF_INET, SRC_IP, &ip->saddr) != 1) // Можно не заполнять, заполнится ядром
    {
        return -1;
    }

    if (inet_pton(AF_INET, DST_IP, &ip->daddr) != 1)
    {
        return -1;
    }

    ip->check = 0; // Можно не заполнять, заполнится ядром

    return 0;
}

/**
 * @brief Заполняет UDP заголвок
 *
 * @param [out] udp Указатель на udphdr структуру
 * @param udp_size развмер UDP заголовка + payload
 */
void FillUDPHeader(struct udphdr *udp, int udp_size)
{
    memset(udp, 0, sizeof(*udp));
    udp->source = htons(SRC_PORT);
    udp->dest = htons(DST_PORT);
    udp->len = htons(udp_size);
    udp->check = 0;
}

/**
 * @brief Собирает IP пакет для дальнейшей отправки
 *
 * @param [out] packet Указатель на стурктуру PacketBuffer
 * @param message сообщение для отправки
 * @return int 0 - Успех, -1 - Ошибка
 */
int BuildPacket(PacketBuffer *packet, const char *message)
{
    // Определям размеры
    int payload_size = strlen(message);                  // размер сообщения (оно же payload)
    int udp_size = sizeof(struct udphdr) + payload_size; // размер udp заголовка + payload
    int packet_size = sizeof(struct iphdr) + udp_size;   // размер ip заголовка + (udp заголовка + payload)

    char *buffer = malloc(packet_size);
    if (buffer == NULL)
    {
        return -1;
    }

    // Тут примерно тоже самое что и с размерами, только мы в этом буфере распределяем указатели
    // Переводим указатель в (struct *hdr *) для удобства обращения
    struct iphdr *ip = (struct iphdr *)buffer;                             // Самое начало буфера - будет ip заголовком
    struct udphdr *udp = (struct udphdr *)(buffer + sizeof(struct iphdr)); // Далее мы смещаемся на sizeof(struct iphdr) и это будет UDP заголовок
    char *payload = buffer + sizeof(struct iphdr) + sizeof(struct udphdr); // И последний указатель под payload - смещаем на  IP и UDP заголовки

    // Передаём указатель на ip в FillIPHeader для заполнения
    if (FillIPHeader(ip, packet_size) != 0)
    {
        free(buffer);
        return -1;
    }

    // Передаём указатель на UDP в FillIPHeader для заполнения
    FillUDPHeader(udp, udp_size);

    // Копируем сообщение в конец буфера
    memcpy(payload, message, payload_size);

    // Заполняем структуру
    packet->data = buffer;
    packet->size = packet_size;
    return 0;
}

int SendPacket(int raw_fd, PacketBuffer *packet)
{
    struct sockaddr_in addr = {0};

    addr.sin_family = AF_INET;

    if (inet_pton(AF_INET, DST_IP, &addr.sin_addr) != 1)
    {
        return -1;
    }

    return sendto(raw_fd, packet->data, packet->size, 0, (struct sockaddr *)&addr, (socklen_t)sizeof(addr));
}

int main(void)
{
    const char *message = "Hello server!";

    int raw_fd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (raw_fd < 0)
    {
        perror("socket error");
        return EXIT_FAILURE;
    }

    //Говорим системе, что мы IP заголовок заполняем сами
    int one = 1;
    if (setsockopt(raw_fd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0)
    {
        perror("setsockopt error");
        close(raw_fd);
        return EXIT_FAILURE;
    }

    PacketBuffer packet = {0};
    if (BuildPacket(&packet, message) != 0)
    {
        perror("BuildPacket error");
        close(raw_fd);
        return EXIT_FAILURE;
    }

    if (SendPacket(raw_fd, &packet) < 0)
    {
        perror("SendPacket error");
        free(packet.data);
        close(raw_fd);
        return EXIT_FAILURE;
    }

    free(packet.data);

    // Данные для приёма пакетов
    struct sockaddr_in sender_addr;
    socklen_t addr_size;

    struct udphdr *udp_header = {0};

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