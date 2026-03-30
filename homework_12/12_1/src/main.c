/**
 * @file main.c
 * @author Шустов Александр
 * @brief 
 * @version 0.1
 * @date 2026-03-30
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

// Создаём интовый массив под дескрипторы
int fields[2];

int main(void)
{
    int status;

    // Создаём каналы
    if (pipe(fields) == -1)
        return -1;

    // Создаём дочерний процесс
    pid_t pid = fork();
    if (pid == 0)
    {
        // Закрываем дескриптор на запись
        close(fields[1]);
        // Создаём локальный буфер в который будем читать
        char child_buffer[4];
        // Читаем через дескриптор в наш буфер
        read(fields[0], child_buffer, 4);
        // Закрываем дескриптор после чтения
        close(fields[0]);
        printf("\n(Child) Parent says: %s\n", child_buffer);
        exit(1);
    }

    // Закрываем дескриптор для чтения в родительском процессе
    close(fields[0]);

    // Локальный буфер для передачи дочернему процессу
    char parent_buffer[4] = "Hi!";
    // Пишем из буфера
    write(fields[1], parent_buffer, 4);
    close(fields[1]);
    wait(&status);
    printf("\n\n");
    //printf("Exit status: %d\n\n", WEXITSTATUS(status));
    return 0;
}