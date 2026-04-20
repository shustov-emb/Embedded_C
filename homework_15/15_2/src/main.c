/**
 * @file main.c
 * @author Шустов Александр
 * @brief Блокировка сигналов через sigprocmask
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

int main(void)
{

    //Выводим pid для удобства
    printf("\nPID: %d\n", getpid());
    sigset_t set;
    
    //Обнуляем набор сигналов
    sigemptyset(&set);
    //Добавляем сигнал который будем блокировать
    sigaddset(&set, SIGINT);

    //Устанавливаем "фильтр", и говорми что блокируем сигналы из набора
    sigprocmask(SIG_BLOCK, &set, NULL);

    //Сигнал прерывания заблокирован 
    int i = 0;
    while (i != 15)
    {
        sleep(1);
        i++;
    }

    printf("Now its ublocked\n");
    //Разблокируем сигнал, досрочно прервать программу можно будет другим сигналом
    // Например тем же SIGUSR1
    sigprocmask(SIG_UNBLOCK, &set, NULL);
    while (1)
        sleep(1);

    exit(EXIT_SUCCESS);
}