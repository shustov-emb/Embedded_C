/**
 * @file main.c
 * @author Шустов Александр
 * @brief Сигналы через sigaction, обработка приходящего сигнала SIGUSR1
 * @version 0.1
 * @date 2026-04-17
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

/**
 * @brief Обработчик сигнала
 * 
 * @param sig_num Номер сигнала
 * @param info Информация о сигнале
 * @param args Параметры сигнала?
 */
void sig_handler(int sig_num, siginfo_t *info, void *args)
{
    (void)args;
    printf("Signal recieved: %d | Sender PID: %d\n", sig_num, info->si_pid);
}

int main(void)
{

    // Выводим pid для удобства
    printf("\nPID: %d\n", getpid());

    struct sigaction sig_struct = {0};
    sigset_t set;
    
    //Обнуляем набор
    sigemptyset(&set);
    //Добавляем сигнал SIGUSR1 в набор
    sigaddset(&set, SIGUSR1);

    //Инициализируем структуру
    sig_struct.sa_mask = set;
    sig_struct.sa_sigaction = sig_handler;
    sig_struct.sa_flags = SA_SIGINFO;

    //Регестрируем, поведение процесса при получении сигнала
    int error = sigaction(SIGUSR1, &sig_struct, NULL);
    if (error < 0)
    {
        perror("sigaction error");
        exit(EXIT_FAILURE);
    }

    /* Бесконечный цикл, во время которого ловим сигнал 
     * Проверить можно утилитой kill -10 <PID>
     */
    while (1)
        sleep(1);

    exit(EXIT_SUCCESS);
}