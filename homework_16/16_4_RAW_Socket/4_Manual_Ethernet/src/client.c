/**
 * @file client.c
 * @author Шустов Александр
 * @brief Простая реализация формирования ETH+IP+UDP заголовков
 * ПРИМЕЧАНИЕ: Тесты проводил в двух докер контейнерах, и прописывал айпи и мак адреса именно этих контейнеров
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
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>

#define SRC_IP "172.18.0.3"
#define SRC_MAC "fa:c7:ca:1b:35:4e"
#define DST_IP "172.18.0.2"
#define DST_MAC "66:e3:d2:0b:f8:31"
#define IF_NAME "eth0"
#define SRC_PORT 9899
#define DST_PORT 9898

/**
 * @brief Простенькая структура для хранения отправляемого ethernet кадра
 *
 */
typedef struct PacketBuffer
{
    char *data;
    int size;
} PacketBuffer;

/**
 * @brief Функция для подсчёта контрольной суммы
 *
 * @param data void указатель, принимать мы можем разные данне
 * @param len Длинна, в данном случае заголовка ip
 * @return unsigned short - 2 байтное значение
 */
static unsigned short IPChecksum(const void *data, size_t len)
{
    // Говорим что data надо читать как слова по 2 байта
    const unsigned short *words = data;
    // Инт для переполнения суммы
    unsigned int sum = 0;

    // Если длинна больше 1 - суммируем, потом смещаем указатель на следующие два байта
    while (len > 1)
    {
        sum += *words;
        words++;
        len -= 2;
    }

    // Если длинна не чётная и остаётся 1 байт, то мы приводим word к однобайтному чару и прибавляем к сумме
    if (len == 1)
    {
        sum += *(const unsigned char *)words;
    }

    // Пока сумма больше 16 бит
    while (sum > 0xFFFF)
    {
        unsigned int low = sum & 0xFFFF; // Берём нижнюю часть
        unsigned int high = sum >> 16;   // Берём верхнюю часть
        sum = low + high;                // Складываем
    }

    // Инвертируем
    sum = ~sum;
    // Возвращаем 16 бит контрольной суммы
    return (unsigned short)sum;
}

/**
 * @brief Получить бинарный mac адрес из строки
 *
 * @param [out] buffer Буффер куда будет записан адрес в бинарном виде
 * @param string_mac_addres Строка с mac адресом
 * @return int
 */
int GetBinaryMACFromString(unsigned char *buffer, char *string_mac_addres)
{
    return sscanf(string_mac_addres, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                  &buffer[0], &buffer[1], &buffer[2], &buffer[3], &buffer[4], &buffer[5]);
}

/**
 * @brief Заполняет Ethernet заголовок
 *
 * @param [out] eth Указатель на ehthd стурктуру
 * @return int 0 - Успех, -1 - Ошибка
 */
int FillEthernetHeader(struct ethhdr *eth)
{
    // Обнуляем
    memset(eth, 0, sizeof(*eth));

    // Получаем мак адрес источника
    if (GetBinaryMACFromString(eth->h_source, SRC_MAC) != 6)
    {

        return -1;
    }

    // Получаем мак адрес назначения
    if (GetBinaryMACFromString(eth->h_dest, DST_MAC) != 6)
    {

        return -1;
    }

    // Говорим что будем передавать ip пакет
    eth->h_proto = htons(ETH_P_IP);

    return 0;
}

/**
 * @brief Заполняет IP заголовок
 *
 * @param [out] ip Указатель на iphdr структуру
 * @param ip_size размер всего пакета включая udp с payload
 * @return int 0 - Успех, -1 - Ошибка
 */
int FillIPHeader(struct iphdr *ip, int ip_size) //, int ip_size
{
    memset(ip, 0, sizeof(*ip));

    // Заполняем заголовок, не забывая что некоторые данные идут в litle-endian
    ip->version = 4;
    ip->ihl = 5;
    ip->tos = 0;
    ip->tot_len = htons(ip_size); // Можно не заполнять, заполнится ядром
    ip->id = htons(0);            // Можно не заполнять, заполнится ядром
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

    ip->check = IPChecksum(ip, ip->ihl * 4);

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
    int ip_size = sizeof(struct iphdr) + udp_size;       // размер ip заголовка + (udp заголовка + payload)
    int eth_size = sizeof(struct ethhdr) + ip_size;      // размер eth заголовка + (ip заголовок + udp заголовка + payload)

    char *buffer = malloc(eth_size);
    if (buffer == NULL)
    {
        return -1;
    }

    // Тут примерно тоже самое что и с размерами, только мы в этом буфере распределяем указатели
    // Переводим указатель в (struct *hdr *) для удобства обращения
    struct ethhdr *eth = (struct ethhdr *)buffer;
    struct iphdr *ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));                           // Самое начало буфера - будет ip заголовком
    struct udphdr *udp = (struct udphdr *)(buffer + sizeof(struct ethhdr) + sizeof(struct iphdr)); // Далее мы смещаемся на sizeof(struct iphdr) и это будет UDP заголовок
    char *payload = buffer + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr); // И последний указатель под payload - смещаем на  IP и UDP заголовки

    if (FillEthernetHeader(eth) != 0)
    {
        free(buffer);
        return -1;
    }

    // Передаём указатель на ip в FillIPHeader для заполнения
    if (FillIPHeader(ip, ip_size) != 0)
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
    packet->size = eth_size;
    return 0;
}

/**
 * @brief Отправляем пакет на указанный адрес
 *
 * @param raw_fd Сокет
 * @param packet Структура PacketBuffer, в котором есть буфер с заголовками и payload
 * в котором лежит собранный нами пакет, и его размер
 * @return int 0 - Успех, -1 - Ошибка
 */
int SendPacket(int raw_fd, PacketBuffer *packet)
{

    struct sockaddr_ll addr = {0};

    // ПОлучаем мак адрес получатея
    if (GetBinaryMACFromString(addr.sll_addr, DST_MAC) != 6)
    {
        return -1;
    }
    addr.sll_family = AF_PACKET;
    addr.sll_halen = 6;
    addr.sll_ifindex = if_nametoindex(IF_NAME);
    addr.sll_protocol = htons(ETH_P_ALL);

    return sendto(raw_fd, packet->data, packet->size, 0, (struct sockaddr *)&addr, (socklen_t)sizeof(addr));
}

int main(void)
{
    const char *message = "Hello server! im gonna text you a message, but it will be later tonight";

    int raw_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (raw_fd < 0)
    {
        perror("socket error");
        return EXIT_FAILURE;
    }

    // Поскольку мы работаем с эзернет кадрами, то и ip заголовок мы заполням в ручную
    // Следовательно IP_HDRINCL нам в данном случае не нужно!
    // int one = 1;
    // if (setsockopt(raw_fd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0)
    // {
    //     perror("setsockopt error");
    //     close(raw_fd);
    //     return EXIT_FAILURE;
    // }

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

    // Данные для приёма пакетов, сокет у нас AF_PACKET поэтому и принимать будем sockaddr_ll
    struct sockaddr_ll sender_addr;
    socklen_t addr_size;

    struct udphdr *udp_header;

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

        // struct ethhdr *eth_header = (struct ethhdr *)buffer;
        struct iphdr *ip_header = (struct iphdr *)(buffer + sizeof(struct ethhdr));
        // ihl показывает длину ip заголовка, в int обычно в 32 битных словах
        // Надо перевести в байты
        int ip_header_length = ip_header->ihl * 4;

        // Если полученное количество байт меньше чем хотя бы три необходимых заголовка, то пропускаем пакет
        if (recv_bytes < ip_header_length + (int)sizeof(struct udphdr) + (int)sizeof(struct ethhdr))
        {
            printf("recv bytes less than ip_header");
            continue;
        }

        // Достаём данные, смещаем начало буфера на длины eth, ip и udp заголовка, udp заголовок - 8 байт
        // Поэтому его не нужно вычислять как длину ip заголовка, которая может варьироваться
        char *data = (char *)(buffer + sizeof(struct ethhdr) + ip_header_length + sizeof(struct udphdr));

        // Из udp заголовка можно достать порты назначения, и источника
        udp_header = (struct udphdr *)(buffer + sizeof(struct ethhdr) + ip_header_length);

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
    close(raw_fd);

    exit(EXIT_SUCCESS);
}