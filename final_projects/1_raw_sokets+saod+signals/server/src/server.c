/**
 * @file server.c
 * @author Шустов Александр
 * @brief Точка входа серверного приложения
 * @version 0.1
 * @date 2026-06-10
 *
 * @copyright Copyright (c) 2026
 *
 */
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "server_utils.h"

int main(void)
{
    printf("\nECHO SERVER (port: %d)\n", SERVER_PORT);
    raw_fd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (raw_fd < 0)
    {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    if (InitSignalHandling() == -1)
    {
        perror("InitSignalHandling error");
        close(raw_fd);
        exit(EXIT_FAILURE);
    }

    // Выделяем память под клиентский массив
    if (InitClientList() == -1)
    {
        perror("InitClientList error");
        close(raw_fd);
        exit(EXIT_FAILURE);
    }

    // Создаём поток для обработки пришедших клиентских сообщений
    pthread_t console_thread = {0};
    if (pthread_create(&console_thread, NULL, HandleRecive, NULL) != 0)
    {
        perror("pthread_create error");
        // Освобождаем ранее выделенную память при ошибке создания потока
        FreeClientList();
        close(raw_fd);
        exit(EXIT_FAILURE);
    }

    char command[4096] = {0};
    // По сути просто ждём символа q в консоль чтобы завершить программу
    while (!stop_requested && fgets(command, sizeof(command), stdin))
    {
        if (command[0] == 'q' && (command[1] == '\n' || command[1] == '\0'))
            break;
    }

    // Закрываем дескриптор
    if (raw_fd >= 0)
        close(raw_fd);
    // Поток с обработкой сообщений увидит что дескриптор закрыт - завершится. Тут мы его дождёмся и выйдем
    pthread_join(console_thread, NULL);
    // Освобождаем память массива клиентов
    FreeClientList();

    exit(EXIT_SUCCESS);
}
