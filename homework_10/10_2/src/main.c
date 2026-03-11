 /**
 * @file main.c
 * @author Шустов Александр
 * @brief Реализовать программу, которая порождает процесс1 и процесс2, ждет завершения дочерних процессов. 
Процесс1 в свою очередь порождает процесс3 и процесс4 и ждет их завершения. 
Процесс2 порождает процесс5 и ждет его завершения. 
Все процессы выводят на экран свой pid, ppid
 * @version 0.1
 * @date 2026-03-11
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(void)
{
    //Объявляем переменные под процессы
    pid_t pid_1;
    pid_t pid_2;
    pid_t pid_3;
    pid_t pid_4;
    pid_t pid_5;

    //Объявляем переменные пол их статусы
    int pid_1_status;
    int pid_2_status;
    int pid_3_status;
    int pid_4_status;
    int pid_5_status;

    printf("Parent: pid = %d | ppid = %d\n", getpid(), getppid());  //fflush(NULL);

    //Создаём первый процесс
    pid_1 = fork();

    //Если первый процесс не порождённый а основной, тогда порождаем ещё один процесс
    if(pid_1 != 0){
        pid_2 = fork();
    }

    //Условия выполнения для порождённых первого и второго процессов
    if (pid_1 == 0) {
        
        printf("pid_1: pid = %d | ppid = %d\n", getpid(), getppid()); 
        
        pid_3 = fork();

        //Аналогично тому как мы порождали первый и второй процессы, порождаем третий и четвёртый.
        //Только тут в качестве родителя выступает первый процесс
        if(pid_3 != 0){
            pid_4 = fork();
        }

        //Условия выполнения первого, третьего и четвёртого процессов
        if (pid_3 == 0)
        {
             printf("pid_3: pid = %d | ppid = %d\n", getpid(), getppid());
             //Завершаем третий процесс даём ему статус 3
             exit(3); 
        } else if (pid_4 == 0) {
             printf("pid_4: pid = %d | ppid = %d\n", getpid(), getppid()); 
             //Завершаем четвертый процесс даём ему статус 4
             exit(4);
        }

        //Ждём завершения дочерних третьего и четвёртого процессов
        else
        {
            wait(&pid_3_status);
            printf("exit status = %d\n", WEXITSTATUS(pid_3_status));
            wait(&pid_4_status);
            printf("exit status = %d\n", WEXITSTATUS(pid_4_status));    
        }
        
        //Завершаем первый процесс даём ему статус 1
        exit(1);

    } else if (pid_2 == 0) {

        printf("pid_2: pid = %d | ppid = %d\n", getpid(), getppid());         

        //Порождаем пятый процесс во втором процессе
        pid_5 = fork();

        //Условия выполнения пятого и второго процессов
        if (pid_5 == 0)
        {
            printf("pid_5: pid = %d | ppid = %d\n", getpid(), getppid());
            //Завершаем пятый процесс даём ему статус 5
            exit(5); 
        }
        //Завершаем первый процесс даём ему статус 1
        else
        {
            wait(&pid_5_status);
            printf("exit status = %d\n",WEXITSTATUS(pid_5_status));
        }

        //Ждём завершения дочернего пятого процесса
        exit(2);

    //Ждём завершения дочерних первого и второго процессов
    } else {
        wait(&pid_1_status);
        printf("exit status = %d\n", WEXITSTATUS(pid_1_status));
        wait(&pid_2_status);
        printf("exit status = %d\n\n", WEXITSTATUS(pid_2_status));       
    }

    return 0;
}