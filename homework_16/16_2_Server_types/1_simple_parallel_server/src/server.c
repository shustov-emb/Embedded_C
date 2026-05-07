/**
 * @file server.c
 * @author Шустов Александр
 * @brief При подключении клиента слушающий сервер создаёт поток с обслуживающим сервером, передаёт эндпоинт дочернего сервера клиенту.
 * Дочерний сервер ждёт повторного подключения клиента и обрабатывает запрос.
 * @version 0.1
 * @date 2026-05-04
 *
 * @copyright Copyright (c) 2026
 *
 */

#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

volatile sig_atomic_t keep_running = 1;

/**
 * @brief Структура для хранения данных о соединении
 */
typedef struct Connection
{
    int fd;
    struct sockaddr_in addr;
} Connection;

/**
 * @brief Обрабатываем сигнал sigint прерывая главный цикл
 *
 * @param sig
 */
void HandleSignal(int sig)
{
    (void)sig;
    keep_running = 0;
}

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
 * @brief Обработка входящего клиентского подключения
 *
 * @param arg Структура с данными о подключении
 * @return void*
 */
void *HandleCLient(void *arg)
{

    Connection *server = (Connection *)arg;
    pthread_detach(pthread_self());

    Connection client_info = {0};
    socklen_t client_addr_size = sizeof(client_info.addr);
    // Дескриптор берём из данных подключения, и принимаем соединение
    client_info.fd = accept(server->fd, (struct sockaddr *)&client_info.addr, &client_addr_size);
    if (client_info.fd < 0)
    {
        close(server->fd);
        perror("accept error");
        free(server);
        return NULL;
    }

    // Отправляем дату и время
    char buffer[100] = {0};
    if (GetTime(buffer, sizeof(buffer)) == 0)
    {
        if (send(client_info.fd, buffer, strlen(buffer), 0) < 0)
        {
            perror("send error");
        }
    }

    close(client_info.fd);
    close(server->fd);

    free(server);
    return NULL;
}

/**
 * @brief Отправляет клиенту эндпоинт дочернего сервера в виде строки
 * @param client_fd Дескриптор клиента
 * @param server Данные дочернего сервера
 * @return int 0 - Успех, -1 - Ошибка
 */
int SendEndpoint(int client_fd, const Connection *server)
{
    char endpoint[32] = {0};
    char ip[INET_ADDRSTRLEN] = {0};
    int port = ntohs(server->addr.sin_port);

    if (inet_ntop(AF_INET, &server->addr.sin_addr, ip, sizeof(ip)) == NULL)
    {
        return -1;
    }

    if (snprintf(endpoint, sizeof(endpoint), "%s:%d", ip, port) >= (int)sizeof(endpoint))
    {
        return -1;
    }

    return send(client_fd, endpoint, strlen(endpoint) + 1, 0);
}

/**
 * @brief Инициализация сервера
 *
 * @param [out] connection Структура с даннымы сервера для заполнения
 * @param main_server Булево, главный сервер это или нет
 * @return int 0 - Успех, -1 - Ошибка
 */
int InitServer(Connection *connection, int main_server)
{
    int port;

    if (main_server)
        port = 9898;
    else
        port = 0;

    connection->addr.sin_family = AF_INET;
    connection->addr.sin_port = htons(port);
    if (inet_pton(AF_INET, "127.0.0.1", &connection->addr.sin_addr) < 0)
    {
        // perror("inet_pton error");
        return -1;
    }

    connection->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (connection->fd < 0)
    {
        // perror("socket error");
        return -1;
    }

    int opt = 1;
    // Задаём для сокета настройку, которая позволяет использовать порт сразу после закрытия соединения
    // Нужна чисто для того чтобы не выжидать пока порт снова станет доступен, а запусить сервер сразу после закрытия
    if (setsockopt(connection->fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        close(connection->fd);
        return -1;
    }

    socklen_t server_addr_size = sizeof(connection->addr);
    if (bind(connection->fd, (struct sockaddr *)&connection->addr, server_addr_size) < 0)
    {
        close(connection->fd);
        // perror("bind error");
        return -1;
    }

    // Если инициализируется дочерний сервер, то там порт назначается системой
    // И чтобы узнать какой порт нам назначила система, пользуемся getsockname
    // Нужно для дальнейшей отправки клиенту
    if (!main_server)
    {
        if (getsockname(connection->fd, (struct sockaddr *)&connection->addr, &server_addr_size) < 0)
        {
            close(connection->fd);
            // perror("getsockname error");
            return -1;
        }
    }

    // Дочернему серверу больше одного подключеие и не нужно
    // Так как он сразу заканчивает работу после ответа
    int listen_count = (main_server) ? 10 : 1;

    if (listen(connection->fd, listen_count) < 0)
    {
        close(connection->fd);
        // perror("listen error");
        return -1;
    }

    return 0;
}

/**
 * @brief Создаёт дочерний сервер, отправляет эндпоинт клиенту, и создаёт поток для обработки последующего соединения
 *
 * @param client_fd Дескриптор клиента
 * @return int
 */
int PrepareChildServer(int client_fd)
{

    Connection *child_server_data = malloc(sizeof(Connection));
    if (!child_server_data)
    {
        close(client_fd);
        return -1;
    }

    if (InitServer(child_server_data, 0) < 0)
    {
        close(client_fd);
        free(child_server_data);
        perror("child_server init error");
        return -1;
    }

    if (SendEndpoint(client_fd, child_server_data) < 0)
    {
        close(client_fd);
        close(child_server_data->fd);
        free(child_server_data);
        return -1;
    }

    close(client_fd);
    pthread_t thread = {0};
    if (pthread_create(&thread, NULL, HandleCLient, child_server_data) != 0)
    {
        close(child_server_data->fd);
        free(child_server_data);
        return -1;
    }
    // HandleCLient(child_server_data);

    return 0;
}

int main()
{

    // Получаем pid для простоты завершения процесса если что-то пойдёт не так
    printf("PID: %d\n", getpid());
    printf("Simple parallel | SERVER!\n");

    // Обработка завершения программы по ctrl+c через sigaction
    struct sigaction sa;
    sa.sa_handler = HandleSignal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    // Создаём главный сервер
    Connection liseten_server_data = {0};
    if (InitServer(&liseten_server_data, 1) < 0)
    {
        close(liseten_server_data.fd);
        perror("liseten_server init error");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        Connection client_info = {0};
        socklen_t client_addr_size = sizeof(client_info.addr);
        // Принимаем входящие заявки от клиентов
        client_info.fd = accept(liseten_server_data.fd, (struct sockaddr *)&client_info.addr, &client_addr_size);
        if (client_info.fd < 0)
        {
            // free(client_info);
            if (errno == EINTR)
                break;
            else
                continue;
        }

        // Готовим дочерний сервер и передаём ему задачу на обработку клиента
        if (PrepareChildServer(client_info.fd) < 0)
        {
            continue;
        }
    }

    close(liseten_server_data.fd);
    printf("\nServer sutting down!\n");
    exit(EXIT_SUCCESS);
}
