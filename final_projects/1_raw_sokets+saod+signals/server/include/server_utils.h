/**
 * @file server_utils.h
 * @author Шустов Александр
 * @brief Заголовочный файл для работы с серверной частью приложения
 * @version 0.1
 * @date 2026-06-10
 *
 * @copyright Copyright (c) 2026
 *
 */

#include <arpa/inet.h>
#include <signal.h>

#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9000
#define MAX_CLIENTS 64

/**
 * @brief Структура содержащая данные подключённого клиента
 */
typedef struct ClientInfo
{
    int is_used;
    uint32_t client_ip;
    uint16_t client_port;
    uint32_t msg_count;
} ClientInfo;

/**
 * @brief Список подключённых клиентов
 */
extern ClientInfo client_list[MAX_CLIENTS];
extern int raw_fd;
/**
 * @brief Флаг завершения приложения по сигналу SIGINT
 */
extern volatile sig_atomic_t stop_requested;

/**
 * @brief Отправляет сообщения клиенту
 * @param raw_fd Дескриптор
 * @param client Информация о клиенте которому будем отправлять сообщение
 * @param message Сообщение для отправки
 * @return int 0 - Успех, -1 - Ошибка
 */
int SendMessage(int raw_fd, struct ClientInfo *client, char *message);

/**
 * @brief Получает индекс клиента из массива, ищет клиента по порту и ip адресу в массиве
 *
 * @param client_ip Клиентский ip адресс
 * @param client_port Клиентский порт
 * @return int -1 - Ошибка в противном случае возвращаем индекс найденного клиента в массиве
 */
int GetClientIndex(uint32_t client_ip, uint16_t client_port);

/**
 * @brief Добавляет клиента в клиентский массив
 *
 * @param client_ip Клиентский ip адресс
 * @param client_port Клиентский порт
 * @return int 0 - Успех, -1 - Ошибка
 */
int AddNewClient(uint32_t client_ip, uint16_t client_port);

/**
 * @brief Удаляет клиента из клиентского массива
 *
 * @param client_ip Клиентский ip адресс
 * @param client_port Клиентский порт
 * @return int 0 - Успех, -1 - Ошибка
 */
int RemoveClient(uint32_t client_ip, uint16_t client_port);

/**
 * @brief Инициализирует обработку сигнала SIGINT
 * @return int 0 - Успех, -1 - Ошибка
 */
int InitSignalHandling();

/**
 * @brief Обработчик сигнала SIGINT
 * @param signal_number Номер полученного сигнала
 */
void HandleSigint(int signal_number);

/**
 * @brief Поточная функция, отвечает за получение сообщений от клиентов, обработку и отправку сообщения обратно
 *
 * @param arg
 * @return void*
 */
void *HandleRecive(void *arg);

#endif // SERVER_UTILS_H
