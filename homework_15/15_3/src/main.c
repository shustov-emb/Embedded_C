/**
 * @file main.c
 * @author Шустов Александр
 * @brief Ожидания прихода сигнала через sigwait
 * @version 0.1
 * @date 2026-04-17
 * 
 * @copyright Copyright (c) 2026
 * 
 */


#define _POSIX_C_SOURCE 200809L
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

int main (void) {
    sigset_t set;
    int sig_num;

    //Обнуляем набор
    sigemptyset(&set);
    //Добавляем сигнал который будем обрабатывать
    sigaddset(&set, SIGUSR1);

    //Блокируем сигнал, чтобы пришедший сигнал SIGUSR1 
    //не выкинул нас из программы своим дефолтным поведением
    if(sigprocmask(SIG_BLOCK, &set, NULL) != 0){
        perror("sigprocmask error");
        exit(EXIT_FAILURE);
    }
    
    printf("PID:\n%d\n", getpid());

    while (1)
    {
        //Ждём SIGUSR1 и реагируем на него!
        sigwait(&set, &sig_num);
        printf("I got SIGUSR1 signal (%d)\n", sig_num);
    }

    exit(EXIT_SUCCESS);
}