/**
 * @file utils.c
 * @author Шустов Александр
 * @brief Реализация вспомогательных функций приложения
 * @version 0.1
 * @date 2026-06-10
 *
 * @copyright Copyright (c) 2026
 *
 */

#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "utils.h"

int keep_running = 1;
DriverManager dm = {0};

int ReadString(char *buffer, size_t size)
{
    size_t len = 0;
    int ch = 0;

    if (size == 0)
        return -1;

    while ((ch = getchar()) != '\n' && ch != EOF)
    {
        if (len + 1 >= size)
            return -1;

        buffer[len] = (char)ch;
        len++;
    }

    if (len == 0)
        return -1;

    buffer[len] = '\0';
    return 0;
}

int WriteMessage(int fd, const char *message)
{
    size_t message_len = strlen(message) + 1;

    if (write(fd, message, message_len) == -1)
        return -1;

    return 0;
}

// Очень сильно путало, кому какой дескриптор закрывать!)
int CreateDriver(Driver *driver)
{
    if (pipe(driver->host_to_driver) == -1)
        return -1;

    if (pipe(driver->driver_to_host) == -1)
    {
        close(driver->host_to_driver[0]);
        close(driver->host_to_driver[1]);
        return -1;
    }

    // Создаём дочерний процесс драйвера
    driver->pid = fork();
    if (driver->pid == -1)
    {
        close(driver->host_to_driver[0]);
        close(driver->host_to_driver[1]);
        close(driver->driver_to_host[0]);
        close(driver->driver_to_host[1]);
        return -1;
    }

    if (driver->pid == 0)
    {
        // В дочернем процессе закрываем неиспользуемые дескрипторы и посылаем процесс работать
        close(driver->host_to_driver[1]);
        close(driver->driver_to_host[0]);
        PretendToWork(driver);
        exit(EXIT_SUCCESS);
    }

    // В родительском процессе закрываем неиспользуемые дескрипторы, получается крест на крест с дочерним
    close(driver->host_to_driver[0]);
    close(driver->driver_to_host[1]);
    printf("\ndriver (pid: %d) created\n\n", driver->pid);
    return 0;
}

void PretendToWork(Driver *driver)
{
    struct pollfd poll_fd = {0};
    char read_buffer[DRIVER_MESSAGE_SIZE] = {0};
    char write_buffer[DRIVER_MESSAGE_SIZE] = {0};
    int poll_status = 0;
    int time_left = 0;
    int scan_results = 0;
    time_t end_time = 0;

    poll_fd.fd = driver->host_to_driver[0];
    poll_fd.events = POLLIN;

    while (1)
    {
        // Ожидаем поступление команды от родительского процесса
        poll_status = poll(&poll_fd, 1, -1);
        if (poll_status <= 0)
            continue;

        time_left = (int)(end_time - time(NULL));

        memset(read_buffer, 0, sizeof(read_buffer));
        // Считываем команду из входящего дескриптора драйвера
        if (read(driver->host_to_driver[0], read_buffer, sizeof(read_buffer) - 1) <= 0)
            continue;

        char command[DRIVER_MESSAGE_SIZE] = {0};
        int seconds = 0;
        scan_results = sscanf(read_buffer, "%63s %d", command, &seconds);

        if (scan_results == 1)
        {
            // Команда exit завершает дочерний процесс
            if (strcmp(command, "exit") == 0)
                break;

            if (strcmp(command, "status") == 0)
            {
                memset(write_buffer, 0, sizeof(write_buffer));
                // По оставшемуся времени высчитываем состояние драйвера
                if (time_left > 0)
                    snprintf(write_buffer, sizeof(write_buffer), "BUSY %d", time_left);
                else
                    snprintf(write_buffer, sizeof(write_buffer), "AVALIBLE");

                // Пишем состояние в родительский дескриптор
                if (WriteMessage(driver->driver_to_host[1], write_buffer) == -1)
                    perror("PretendToWork -> WriteMessage");
            }

            continue;
        }

        if (scan_results == 2)
        {
            // Если предыдущая задача не завершена, возвращаем состояние занятости
            if (time_left > 0)
            {
                snprintf(write_buffer, sizeof(write_buffer), "BUSY %d", time_left);
                if (WriteMessage(driver->driver_to_host[1], write_buffer) == -1)
                    perror("PretendToWork -> WriteMessage");
                continue;
            }

            // Имитируем занятость и запоминаем момент завершения новой задачи
            end_time = time(NULL) + seconds;
        }
    }
}

void SendTask(int pid, int seconds)
{
    for (size_t i = 0; i < dm.count; i++)
    {
        if (dm.drivers[i].pid != pid)
            continue;

        char write_buffer[DRIVER_MESSAGE_SIZE] = {0};
        // Формируем команду задачи и отправляем выбранному драйверу
        snprintf(write_buffer, sizeof(write_buffer), "set %d", seconds);
        if (WriteMessage(dm.drivers[i].host_to_driver[1], write_buffer) == -1)
            perror("SendTask -> WriteMessage");
        return;
    }

    printf("\ndriver (pid: %d) not found\n\n", pid);
}

void HandleSendTaskCommand(void)
{
    char user_input[100] = {0};
    int pid = 0;
    int seconds = 0;

    printf("Enter - <pid> <seconds>: ");
    fflush(stdout);

    if (ReadString(user_input, sizeof(user_input)) == -1)
        return;

    if (sscanf(user_input, "%d %d", &pid, &seconds) == 2)
        SendTask(pid, seconds);
}

int GetStatus(int pid)
{
    for (size_t i = 0; i < dm.count; i++)
    {
        if (dm.drivers[i].pid != pid)
            continue;

        if (WriteMessage(dm.drivers[i].host_to_driver[1], "status") == -1)
            perror("GetStatus -> WriteMessage");
        return 1;
    }

    printf("\ndriver (pid: %d) not found\n\n", pid);
    return 0;
}

int HandleGetStatusCommand(void)
{
    char user_input[100] = {0};
    int pid = 0;

    printf("Enter pid: ");
    fflush(stdout);

    if (ReadString(user_input, sizeof(user_input)) == -1)
        return 0;

    if (sscanf(user_input, "%d", &pid) == 1)
        return GetStatus(pid);

    return 0;
}

int GetDrivers(void)
{
    for (size_t i = 0; i < dm.count; i++)
    {
        if (WriteMessage(dm.drivers[i].host_to_driver[1], "status") == -1)
            perror("GetDrivers -> WriteMessage");
    }

    return (int)dm.count;
}

void ExitDrivers(void)
{
    for (size_t i = 0; i < dm.count; i++)
    {
        if (WriteMessage(dm.drivers[i].host_to_driver[1], "exit") == -1)
            perror("ExitDrivers -> WriteMessage");
    }
}

void PrintMenu(void)
{
    printf("1. Create driver\n");
    printf("2. Send task (pid seconds)\n");
    printf("3. Check status (pid)\n");
    printf("4. Check all drivers\n");
    printf("5. Exit\n");
    printf("Enter value: ");
    fflush(stdout);
}

int ProcessCommand(int chosen_option)
{
    switch (chosen_option)
    {
    case 1:
        // Создаём новый драйвер и сохраняем его в массиве
        if (dm.count >= MAX_DRIVERS_COUNT)
        {
            printf("\ndrivers limit reached\n\n");
            break;
        }

        if (CreateDriver(&dm.drivers[dm.count]) == 0)
            dm.count++;
        break;
    case 2:
        // Отправляем задачу выбранному драйверу
        HandleSendTaskCommand();
        break;
    case 3:
        printf("\n");
        // Запрашиваем статус отдельного драйвера
        return HandleGetStatusCommand();
    case 4:
        printf("\n");
        // Получить статус всех драйверов
        return GetDrivers();
    case 5:
        // Завершить все дочерние процессы и основной поток
        ExitDrivers();
        keep_running = 0;
        break;
    default:
        break;
    }

    return 0;
}
