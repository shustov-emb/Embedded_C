/**
 * @file client_utils.h
 * @author Шустов Александр
 * @brief Заголовочный файл, для работы с клиентской частью приложения
 * @version 0.1
 * @date 2026-06-10
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef CLIENT_UTILS_H
#define CLIENT_UTILS_H

#include <signal.h>
#include <netinet/in.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9000

/**
 * @brief Порт клиентского приложения, определяется во время выполнения
 */
extern int LOCAL_PORT;

/**
 * @brief Сырой файловый дескриптор
 */
extern int raw_fd;

/**
 * @brief Флаг завершения приложения по сигналу SIGINT
 */
extern volatile sig_atomic_t stop_requested;
extern struct sockaddr_in serv_addr;

/**
 * @brief Формирует udp заголовок и отправляет сообщение клиенту
 * 
 * @param message Сообщение для отправки
 * @return int 0 - Успех, -1 - Ошибка
 */
int SendMessage(char *message);

/**
 * @brief Инициализирует глобальные переменные приложения
 * @return int 0 - Успех, -1 - Ошибка
 */
int InitApp();

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
 * @brief Поточная функция, отвечает за получение и обработку сообщений от сервера
 * 
 * @param arg 
 * @return void* 
 */
void *HandleReceive(void *arg);


#endif //CLIENT_UTILS_H
