/**
 * @file main.c
 * @author Шустов Александр
 * @brief Точка входа приложения управления драйверами
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
#include <unistd.h>
#include "utils.h"

int main(void)
{
    char user_input[100] = {0};
    struct pollfd poll_fds[MAX_DRIVERS_COUNT] = {0};
    int chosen_option = 0;
    int pending_replies = 0;

    while (keep_running)
    {
        // Печатаем меню
        PrintMenu();

        memset(user_input, 0, sizeof(user_input));
        // Считываем данные введённый пользователем
        if (ReadString(user_input, sizeof(user_input)) == -1)
            continue;

        if (sscanf(user_input, "%d", &chosen_option) != 1)
            continue;

        // Обрабатываем команду и получаем количество ожидаемых ответов от драйверов
        pending_replies = ProcessCommand(chosen_option);

        // Пока ожидаются ответы - опрашиваем все каналы драйверов через poll
        while (keep_running && pending_replies > 0)
        {
            memset(poll_fds, 0, sizeof(poll_fds));

            // Подготавливаем массив дескрипторов для ожидания данных от каждого драйвера
            for (size_t i = 0; i < dm.count; i++)
            {
                poll_fds[i].fd = dm.drivers[i].driver_to_host[0];
                poll_fds[i].events = POLLIN;
            }

            // Ждём появления данных хотя бы в одном канале
            if (poll(poll_fds, dm.count, -1) <= 0)
                continue;

            for (size_t i = 0; i < dm.count; i++)
            {
                if ((poll_fds[i].revents & POLLIN) == 0)
                    continue;

                memset(user_input, 0, sizeof(user_input));
                // Считываем ответ из канала конкретного драйвера
                if (read(dm.drivers[i].driver_to_host[0], user_input, sizeof(user_input) - 1) <= 0)
                    continue;

                // Выводим полученный ответ и уменьшаем количество ожидаемых сообщений
                printf("driver (pid: %d): %s\n", dm.drivers[i].pid, user_input);
                pending_replies--;

                if (pending_replies == 0)
                {
                    printf("\n");
                    break;
                }
            }
        }
    }

    exit(EXIT_SUCCESS);
}
