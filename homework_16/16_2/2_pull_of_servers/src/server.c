/**
 * @file server.c
 * @author Шустов Александр
 * @brief При подключении клиента слушающий сервер создаёт пулл серверов с дочерними серверами
 * Мониторит статус серверов через семафор, и отправляет эндпоинт свободного сервера клиенту.
 * @version 0.1
 * @date 2026-05-04
 *
 * @copyright Copyright (c) 2026
 *
 */

#define _POSIX_C_SOURCE 200809L
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <semaphore.h>
#include <time.h>

#define MAIN_SERVER_PORT 9898
#define CHILD_SERVER_COUNT 5
#define SHM_NAME "/pull_of_servers_shm"

volatile sig_atomic_t keep_running = 1;

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
 * @brief Структура для хранения данных о соединении
 */
typedef struct Connection
{
    int fd;
    struct sockaddr_in addr;
} Connection;

/**
 * @brief Данные дочернего сервера
 *
 */
typedef struct ChildServerState
{
    int id;
    int busy;
    int port;
    pid_t pid;
} ChildServerState;

/**
 * @brief Структура под данные разделяемой памяти
 *
 */
typedef struct SharedData
{
    sem_t semaphore;
    ChildServerState child_servers[CHILD_SERVER_COUNT];
} SharedData;

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
 * @param port Можно указать явный порт
 * @param listen_count Сколько подключений слушать
 * @return int
 */
int InitServer(Connection *connection, int port, int listen_count)
{
    connection->addr.sin_family = AF_INET;
    connection->addr.sin_port = htons(port);
    if (inet_pton(AF_INET, "127.0.0.1", &connection->addr.sin_addr) <= 0)
    {
        return -1;
    }

    connection->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (connection->fd < 0)
    {
        return -1;
    }

    // Задаём для сокета настройку, которая позволяет использовать порт сразу после закрытия соединения
    // Нужна чисто для того чтобы не выжидать пока порт снова станет доступен, а запусить сервер сразу после закрытия
    int opt = 1;
    if (setsockopt(connection->fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        close(connection->fd);
        return -1;
    }

    socklen_t server_addr_size = sizeof(connection->addr);
    if (bind(connection->fd, (struct sockaddr *)&connection->addr, server_addr_size) < 0)
    {
        close(connection->fd);
        return -1;
    }

    // Переделал логику немного, теперь булевый параметр main_server
    // Мы просто передаём порт, если порт пустой, значит сервер дочерний
    // Значит через getsockname получаем его порт
    if (port == 0)
    {
        if (getsockname(connection->fd, (struct sockaddr *)&connection->addr, &server_addr_size) < 0)
        {
            close(connection->fd);
            return -1;
        }
    }

    // Ну и количество подключений мы теперь можем явно указать
    if (listen(connection->fd, listen_count) < 0)
    {
        close(connection->fd);
        return -1;
    }

    return 0;
}

/**
 * @brief Отправляет клиенту эндпоинт дочернего сервера в виде строки
 * Немного упростил функцию
 * @param client_fd Дескриптор клиента
 * @param port Порт дочернего сервера
 * @return int 0 - Успех, -1 - Ошибка
 */
int SendEndpoint(int client_fd, int port)
{
    char endpoint[32] = {0};
    if (snprintf(endpoint, sizeof(endpoint), "127.0.0.1:%d", port) >= (int)sizeof(endpoint))
    {
        return -1;
    }

    return send(client_fd, endpoint, strlen(endpoint) + 1, 0);
}

/**
 * @brief Инициализируем разделяемую память
 *
 * @return SharedData*
 */
SharedData *InitSharedData(void)
{
    // Получаем дескриптор
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd < 0)
    {
        return NULL;
    }

    // Устанавливаем размер сегмента памяти
    if (ftruncate(shm_fd, sizeof(SharedData)) < 0)
    {
        close(shm_fd);
        shm_unlink(SHM_NAME);
        return NULL;
    }

    // Получаем доступ к разделяемой памяти через указатель
    SharedData *shared_data = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    close(shm_fd);
    if (shared_data == MAP_FAILED)
    {
        shm_unlink(SHM_NAME);
        return NULL;
    }

    // Инициализируем память
    memset(shared_data, 0, sizeof(SharedData));

    // Создаём межпроцессорный семафор
    if (sem_init(&shared_data->semaphore, 1, 1) < 0)
    {
        munmap(shared_data, sizeof(SharedData));
        shm_unlink(SHM_NAME);
        return NULL;
    }

    return shared_data;
}

/**
 * @brief Удаляем раздеяеме данные
 *
 * @param shared_data Ссылка на разделяеме данные
 */
void DestroySharedData(SharedData *shared_data)
{
    sem_destroy(&shared_data->semaphore);
    munmap(shared_data, sizeof(SharedData));
    shm_unlink(SHM_NAME);
}

/**
 * @brief Устанавливаем статус сервера, занят или нет
 *
 * @param shared_data Разделяемые данные
 * @param index id/индекс сервера
 * @param busy Статус
 */
void SetChildServerStatus(SharedData *shared_data, int index, int busy)
{
    sem_wait(&shared_data->semaphore);
    shared_data->child_servers[index].busy = busy;
    sem_post(&shared_data->semaphore);
}

/**
 * @brief Получить свободный сервер
 *
 * @param shared_data Разделяемые данные
 * @return int Индекс свободного сервера, -1 - Ошибка
 */
int GetFreeChildServer(SharedData *shared_data)
{
    int result = -1;

    sem_wait(&shared_data->semaphore);
    for (int i = 0; i < CHILD_SERVER_COUNT; i++)
    {
        if (!shared_data->child_servers[i].busy)
        {
            shared_data->child_servers[i].busy = 1;
            result = i;
            break;
        }
    }
    sem_post(&shared_data->semaphore);

    return result;
}

/**
 * @brief Обработка входящего клиентского подключения
 *
 * @param child_server Сетевые данные дочернего сервера
 * @param child_server_state Состояние дочернего сервера
 * Порт вынесен сюда же для удобства,
 * чтобы главный процесс мог быстро получить эндпоинт сервера из общей памяти
 * @param shared_data Разделяемые данные
 */
void HandleChildServer(Connection *child_server, ChildServerState *child_server_state, SharedData *shared_data)
{
    while (keep_running)
    {
        Connection client_info = {0};
        socklen_t client_addr_size = sizeof(client_info.addr);
        // Принимаем соединение
        client_info.fd = accept(child_server->fd, (struct sockaddr *)&client_info.addr, &client_addr_size);
        if (client_info.fd < 0)
        {
            if (errno == EINTR)
                continue;
            else
                continue;
        }

        // Получаем время и отправляем обратно клиенту
        char buffer[100] = {0};
        if (GetTime(buffer, sizeof(buffer)) == 0)
        {
            if (send(client_info.fd, buffer, strlen(buffer), 0) < 0)
            {
                perror("send error");
            }
        }

        close(client_info.fd);
        SetChildServerStatus(shared_data, child_server_state->id, 0);
    }
}

/**
 * @brief Создаёт дочерний сервер
 *
 * @param child_server Сетевые данные дочернего сервера
 * @param child_server_state Состояние дочернего сервера
 * @param shared_data Разделяемые данные
 * @return int 0 - Успех, -1 - Ошибка
 */
int CreateChildServer(Connection *child_server, ChildServerState *child_server_state, SharedData *shared_data)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        return -1;
    }

    if (pid == 0)
    {
        HandleChildServer(child_server, child_server_state, shared_data);
        close(child_server->fd);
        exit(EXIT_SUCCESS);
    }

    // Сохраняем PID дочернего процесса, на всякий случай
    child_server_state->pid = pid;
    close(child_server->fd);
    return 0;
}

/**
 * @brief Инициализирует дочерние сервера
 *
 * @param child_server Сетевые данные дочернего сервера
 * @param shared_data Разделяемые данные
 * @param count Количество серверов
 * @return int 0 - Успех, -1 - Ошибка
 */
int InitChildServers(Connection *child_servers, SharedData *shared_data, int count)
{
    for (int i = 0; i < count; i++)
    {
        if (InitServer(&child_servers[i], 0, 1) < 0)
        {
            return -1;
        }

        shared_data->child_servers[i].id = i;
        shared_data->child_servers[i].busy = 0;
        shared_data->child_servers[i].port = ntohs(child_servers[i].addr.sin_port);
        shared_data->child_servers[i].pid = 0;

        if (CreateChildServer(&child_servers[i], &shared_data->child_servers[i], shared_data) < 0)
        {
            return -1;
        }
    }

    return 0;
}

/**
 * @brief Закрыть соеденения дочерних серверов
 *
 * @param child_servers Список серверов
 * @param count Количество серверов
 */
void CloseConnections(Connection *child_servers, int count)
{
    for (int i = 0; i < count; i++)
    {
        if (child_servers[i].fd > 0)
        {
            close(child_servers[i].fd);
        }
    }
}

/**
 * @brief Выводит статусы завершения работы дочерних серверов в консоль
 *
 */
void PrintChildProcessStatus(void)
{
    int status = 0;
    pid_t pid = 0;

    while ((pid = wait(&status)) > 0)
    {
        if (WIFEXITED(status))
        {
            printf("Child process %d exited with status %d\n", pid, WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status))
        {
            printf("Child process %d terminated by signal %d\n", pid, WTERMSIG(status));
        }
    }
}

int main()
{
    printf("PID: %d\n", getpid());
    printf("Pull of servers | SERVER!\n");

    // Обработка завершения программы по ctrl+c через sigaction
    struct sigaction sa;
    sa.sa_handler = HandleSignal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    // Инициализируем разделяемые данные
    SharedData *shared_data = InitSharedData();
    if (!shared_data)
    {
        perror("shared data init error");
        exit(EXIT_FAILURE);
    }

    // Инициализируем дочерние сервера
    Connection child_servers[CHILD_SERVER_COUNT] = {0};
    if (InitChildServers(child_servers, shared_data, CHILD_SERVER_COUNT) < 0)
    {
        CloseConnections(child_servers, CHILD_SERVER_COUNT);
        DestroySharedData(shared_data);
        perror("child_servers init error");
        exit(EXIT_FAILURE);
    }

    // Инициализируем главный сервер
    Connection listen_server_data = {0};
    if (InitServer(&listen_server_data, MAIN_SERVER_PORT, 10) < 0)
    {
        CloseConnections(child_servers, CHILD_SERVER_COUNT);
        DestroySharedData(shared_data);
        perror("listen_server init error");
        exit(EXIT_FAILURE);
    }

    while (keep_running)
    {
        // Принимаем входящие заявки от клиентов
        Connection client_info = {0};
        socklen_t client_addr_size = sizeof(client_info.addr);
        client_info.fd = accept(listen_server_data.fd, (struct sockaddr *)&client_info.addr, &client_addr_size);
        if (client_info.fd < 0)
        {
            if (errno == EINTR)
                break;
            else
                continue;
        }

        // Получаем индекс свободного сервера, если таких нет отправляем клиенту "BUSY"
        int free_child_server = GetFreeChildServer(shared_data);
        if (free_child_server < 0)
        {
            send(client_info.fd, "BUSY", 5, 0);
            close(client_info.fd);
            continue;
        }

        // Отправляем эндпоинт сервера клиенту
        if (SendEndpoint(client_info.fd, shared_data->child_servers[free_child_server].port) < 0)
        {
            // Меняем статус сервера
            SetChildServerStatus(shared_data, free_child_server, 0);
        }

        close(client_info.fd);
    }

    close(listen_server_data.fd);

    // Завершаем дочерние процессы
    for (int i = 0; i < CHILD_SERVER_COUNT; i++)
    {
        if (shared_data->child_servers[i].pid > 0)
        {
            kill(shared_data->child_servers[i].pid, SIGINT);
        }
    }

    PrintChildProcessStatus();

    DestroySharedData(shared_data);
    printf("\nServer shutting down!\n");
    exit(EXIT_SUCCESS);
}
