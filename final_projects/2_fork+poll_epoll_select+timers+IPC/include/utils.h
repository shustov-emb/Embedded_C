/**
 * @file utils.h
 * @author Шустов Александр
 * @brief Заголовочный файл со вспомогательными функциями приложения
 * @version 0.1
 * @date 2026-06-10
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

#define MAX_DRIVERS_COUNT 100
#define DRIVER_MESSAGE_SIZE 64

typedef struct Driver
{
    int pid;
    int host_to_driver[2];
    int driver_to_host[2];
} Driver;

typedef struct DriverManager
{
    Driver drivers[MAX_DRIVERS_COUNT];
    size_t count;
} DriverManager;

extern int keep_running;
extern DriverManager dm;

/**
 * @brief Считывает строку из стандартного ввода
 *
 * @param buffer Буфер для считанной строки
 * @param size Размер буфера
 * @return int 0 - Успех, -1 - Ошибка
 */
int ReadString(char *buffer, size_t size);

/**
 * @brief Отправлает сообщение на указанный дескриптор
 *
 * @param fd Файловый дескриптор
 * @param message Сообщение для записи
 * @return int 0 - Успех, -1 - Ошибка
 */
int WriteMessage(int fd, const char *message);

/**
 * @brief Создаёт дочерний процесс драйвера и каналы обмена
 *
 * @param driver Структура драйвера для заполнения
 * @return int 0 - Успех, -1 - Ошибка
 */
int CreateDriver(Driver *driver);

/**
 * @brief Имитирует выполнение задач дочерним процессом
 *
 * @param driver Данные драйвера
 */
void PretendToWork(Driver *driver);

/**
 * @brief Отправляет задание выбранному драйверу
 *
 * @param pid Идентификатор процесса драйвера
 * @param seconds Длительность задачи в секундах
 */
void SendTask(int pid, int seconds);

/**
 * @brief Считывает введённые пользователем pid и секунды 
*/
void HandleSendTaskCommand(void);

/**
 * @brief Запрашивает состояние выбранного драйвера
 *
 * @param pid pid драйвера
 * @return int 1 - Запрос отправлен, 0 - Драйвер не найден
 */
int GetStatus(int pid);

/**
 * @brief Считывает введённый пользователем pid и запрашивает состояние выбранного драйвера
 * @return int 1 - Запрос отправлен, 0 - Драйвер не найден
 */
int HandleGetStatusCommand(void);

/**
 * @brief Запрашивает состояние всех драйверов
 * @return int Количество драйверов, которым отправлен запрос
 */
int GetDrivers(void);

/**
 * @brief Отправляет всем драйверам команду завершения
 */
void ExitDrivers(void);

/**
 * @brief Выводит меню
 */
void PrintMenu(void);

/**
 * @brief Обрабатывает команду пользователя из главного меню
 *
 * @param chosen_option Номер выбранного пункта меню
 * @return int Количество ожидаемых ответов от драйверов
 */
int ProcessCommand(int chosen_option);

#endif // UTILS_H
