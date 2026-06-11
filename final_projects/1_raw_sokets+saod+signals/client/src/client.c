/**
 * @file client.c
 * @author Шустов Александр
 * @brief Точка входа клиентского приложения
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
#include "client_utils.h"

int main(void)
{
    if (InitApp() == -1)
    {
        perror("InitApp error");
        exit(EXIT_FAILURE);
    }

    if (InitSignalHandling() == -1)
    {
        perror("InitSignalHandling error");
        close(raw_fd);
        exit(EXIT_FAILURE);
    }

    printf("\nECHO CLIENT (port: %d)\n", LOCAL_PORT);

    // Создаём поток для получения сообщений от сервера
    pthread_t console_thread = {0};
    if (pthread_create(&console_thread, NULL, HandleReceive, NULL) != 0)
    {
        perror("HandleConsole thread create error");
        exit(EXIT_FAILURE);
    }

    char message[4096] = {0};

    // Считываем данные пользователя
    while (!stop_requested && fgets(message, sizeof(message), stdin))
    {
        if (message[0] == 'q' && (message[1] == '\n' || message[1] == '\0'))
        {
            // Если пользователь введёт q - то отправляем серверу команду exit
            if (SendMessage("exit") < 0)
                perror("sendto error");
            break;
        }

        if (message[0] == '\n')
            continue;

        size_t message_len = strlen(message);
        if (message_len > 0 && message[message_len - 1] == '\n')
            message[message_len - 1] = '\0';

        // если сообщение не терменирующий нуль - отправляем серверу
        if (SendMessage(message) < 0)
            perror("sendto error");
    }

    if (stop_requested)
    {
        if (SendMessage("exit") < 0)
            perror("sendto error");
    }

    // Закрываем дескриптор
    if (raw_fd >= 0)
        close(raw_fd);
    // Поток с обработкой сообщений увидит что дескриптор закрыт - завершится. Тут мы его дождёмся и выйдем
    pthread_join(console_thread, NULL);

    exit(EXIT_SUCCESS);
}
