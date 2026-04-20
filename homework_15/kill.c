/**
 * @file kill.c
 * @author Шустов Александр
 * @brief Утилита для послания сигнала процессу
 * @version 0.1
 * @date 2026-04-20
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    //Берём 2 аргумента, 1 - номер сигнала, 2 PID процесса
    if (argc != 3)
        exit(EXIT_FAILURE);

    //Базовые проверки на целые числа
    int signal = atoi(argv[1]);
    pid_t pid = (pid_t)atoi(argv[2]);

    if (pid < 0 || signal < 0)
        exit(EXIT_FAILURE);

    //Применяем сигнал к процессу 
    if (kill(pid, signal) != 0)
    {
        perror("signal not sent\n");
        exit(EXIT_FAILURE);
    }

    printf("Signal sent successfully\n");
    exit(EXIT_SUCCESS);
}