/**
 * @file server.c
 * @author Шустов Александр
 * @brief Серверная часть на epoll реализации!
 * @version 0.1
 * @date 2026-04-30
 *
 * @copyright Copyright (c) 2026
 *
 */

// #define _POSIX_C_SOURCE 200809L
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <pthread.h>
#include <errno.h>
#include <sys/epoll.h>
#include <signal.h>

#define SERVER_IP "127.0.0.1"
#define TCP_SERVER_PORT 9898
#define UDP_SERVER_PORT 9899
#define TCP_SERVER_LISTEN_COUNT 10

// Атомарная неоптимизированная переменная, для идникации завершения программы
volatile sig_atomic_t keep_running = 1;

/**
 * @brief Перечисление для определения вида протокола
 */
typedef enum ConnectionType
{
    TCP,
    UDP
} ConnectionType;

/**
 * @brief Структура для хранения данных о соединении
 */
typedef struct Connection
{
    int fd;
    ConnectionType type;
    struct sockaddr_in addr;
} Connection;

/**
 * @brief Получить дату и время с миллискундами для наглядности
 *
 * @param [out] output Буфер в который копируется строка
 * @param output_size Размер буфера
 * @return int 0 - Успех, -1 - Ошибка
 */
int GetTime(char *output, size_t output_size)
{
    struct timeval tv;
    // Получаем текущее время
    gettimeofday(&tv, NULL);

    struct tm info = {0};
    // Преобразуем секунды в структуру локального времени
    if (localtime_r(&tv.tv_sec, &info) == NULL)
    {
        return -1;
    }

    // Форматируем дату и время до секунд
    strftime(output, output_size, "%d.%m.%y %H:%M:%S", &info);

    // Дописываем в конец строки точку и миллисекунды (переводим микросекунды в милисекунды)
    // Запись идет со смещением  в 17, чтобы не затереть основную дату
    sprintf(output + 17, ".%03d", (int)(tv.tv_usec / 1000));

    return 0;
}

/**
 * @brief Инициализация сервера
 *
 * @param [out] connection Структура с даннымы сервера для заполнения
 * @param listen_count Количество клиентов для прослушки в TCP сервере
 * @param port Порт сервера
 * @param SOCKET_TYPE Тип сокета
 * @return int 0 - Успех, -1 - Ошибка
 */
int InitServer(Connection *connection, int listen_count, int port, int SOCKET_TYPE)
{
    connection->addr.sin_family = AF_INET;
    connection->addr.sin_port = htons(port);

    if (inet_pton(AF_INET, SERVER_IP, &connection->addr.sin_addr) <= 0)
    {
        return -1;
    }

    connection->fd = socket(AF_INET, SOCKET_TYPE, 0);
    if (connection->fd < 0)
    {
        return -1;
    }

    int opt = 1;
    if (setsockopt(connection->fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        close(connection->fd);
        return -1;
    }

    struct timeval tv = {0};
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    if (setsockopt(connection->fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
    {
        perror("setsockopt SO_RCVTIMEO error");
    }

    socklen_t server_addr_size = sizeof(connection->addr);
    if (bind(connection->fd, (struct sockaddr *)&connection->addr, server_addr_size) < 0)
    {
        close(connection->fd);
        return -1;
    }

    if (port == 0)
    {
        if (getsockname(connection->fd, (struct sockaddr *)&connection->addr, &server_addr_size) < 0)
        {
            close(connection->fd);
            return -1;
        }
    }

    if (SOCKET_TYPE == SOCK_STREAM && listen_count != 0)
    {
        if (listen(connection->fd, listen_count) < 0)
        {
            close(connection->fd);
            return -1;
        }
        connection->type = TCP;
    }
    else
        connection->type = UDP;

    return 0;
}

/**
 * @brief Обработка UDP соединения
 *
 * @param conn Структура с данными о подключении
 * @return int 0 - Успех, -1 - Ошибка
 */
int HandleUDPConnetion(Connection *conn)
{

    char buffer[100] = {0};

    struct sockaddr_in client_addr = {0};
    socklen_t client_len = sizeof(client_addr);
    recvfrom(conn->fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &client_len);

    buffer[sizeof(buffer) - 1] = '\0';
    printf("SERVER UDP | %s\n", buffer);

    memset(buffer, 0, sizeof(buffer));

    if (GetTime(buffer, sizeof(buffer)) == 0)
    {
        if (sendto(conn->fd, buffer, strlen(buffer), 0, (struct sockaddr *)&client_addr, client_len) < 1)
        {
            perror("send udp error");
            return -1;
        }
    }
    else
    {
        return -1;
    }

    return 0;
}

/**
 * @brief Обработка TCP соединения
 *
 * @param conn Структура с данными о подключении
 * @return int 0 - Успех, -1 - Ошибка
 */
int HandleTCPConnection(Connection *conn)
{
    Connection client_info = {0};
    socklen_t client_addr_size = sizeof(client_info.addr);

    // Принимаем коннект
    int client_fd = accept(conn->fd, (struct sockaddr *)&client_info.addr, &client_addr_size);
    if (client_fd < 0)
    {
        return -1;
    }

    char buffer[100] = {0};

    // ПОлучаем сообщение от клиента
    int recv_bytes = recv(client_fd, buffer, sizeof(buffer), 0);
    if (recv_bytes <= 0)
    {
        perror("TCP recv error");
        close(client_fd);
        return -1;
    }
    buffer[recv_bytes - 1] = '\0';
    printf("SERVER TCP | %s\n", buffer);

    // ПОлучаем дату время и отправляем клиенту в ответ
    if (GetTime(buffer, sizeof(buffer)) == 0)
    {
        if (send(client_fd, buffer, strlen(buffer), 0) < 1)
        {
            perror("send tcp error");
            close(client_fd);
            return -1;
        }
    }
    else
    {
        close(client_fd);
        return -1;
    }

    close(client_fd);
    return 0;
}

/**
 * @brief Вспомогательная функция для считывания пользовательского ввода с клавиатуры
 * Нужна для обработки выхода из приложения
 * @param arg Пустой аргумент
 * @return void*
 */
void *HandleConsole(void *arg)
{
    (void)arg; // Чтобы компилятор не ругался на unused параметр
    char command[16] = {0};

    // Считываем ввод, и при полученном q или Q установить keep_running в 0
    while (fgets(command, sizeof(command), stdin))
    {
        if (command[0] == 'q' || command[0] == 'Q')
        {
            keep_running = 0;
            return NULL;
        }
    }
    return NULL;
}

int main()
{
    // Получаем pid для простоты завершения процесса если что-то пойдёт не так
    printf("PID: %d\n", getpid());
    printf("EPOLL | SERVER!\n");

    // Поток для отдельной функции считывания данных с клавиатуры, для выхода из приложения по q
    pthread_t console_thread = {0};
    if (pthread_create(&console_thread, NULL, HandleConsole, NULL) != 0)
    {
        perror("HandleConsole thread create error");
        exit(EXIT_FAILURE);
    }

    // Инициализируем tcp сервер
    Connection tcp_server_data = {0};
    if (InitServer(&tcp_server_data, 5, TCP_SERVER_PORT, SOCK_STREAM) < 0)
    {
        perror("InitServer tcp error");
        exit(EXIT_FAILURE);
    }

    // Инициализируем udp сервер
    Connection udp_server_data = {0};
    if (InitServer(&udp_server_data, 0, UDP_SERVER_PORT, SOCK_DGRAM) < 0)
    {
        perror("InitServer udp error");
        exit(EXIT_FAILURE);
    }

    // Создаём epoll и массив событий
    int epoll_fd = epoll_create1(0);
    struct epoll_event e_events[2] = {0};

    // Добавляем события и дескрипторы для прослушки!
    e_events[0].events = EPOLLIN;
    e_events[0].data.ptr = &tcp_server_data;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, tcp_server_data.fd, &e_events[0]);

    e_events[1].events = EPOLLIN;
    e_events[1].data.ptr = &udp_server_data;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, udp_server_data.fd, &e_events[1]);

    while (keep_running)
    {

        // Ловим события с таймаутом в сеунду,
        // чтобы мы могли проверить переменную keep_running и выйти из цикла
        int ready = epoll_wait(epoll_fd, e_events, 2, 1000);
        if (ready > 0)
        {
            for (size_t i = 0; i < (size_t)ready; i++)
            {
                Connection *conn = e_events[i].data.ptr;
                // Определяем тип подключения и обрабатываем его!
                if (conn->type == TCP)
                {
                    if (HandleTCPConnection(conn) < 0)
                        keep_running = 0;
                }
                else
                {
                    if (HandleUDPConnetion(conn) < 0)
                        break;
                }
            }
        }
    }

    close(tcp_server_data.fd);
    close(udp_server_data.fd);

    printf("\nServer shutting down!\n");
    exit(EXIT_SUCCESS);
}
