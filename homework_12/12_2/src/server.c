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

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// Определяем переменные, сообщения путь и режимы открытия
#define PATH "/tmp/test_buffer"
#define MODE S_IRUSR |S_IWUSR | S_IRGRP | S_IROTH
#define MSG "Hi!"

int main (void) {

    // Создаём именованный канал
    mkfifo(PATH, MODE);

    // Открываем дескриптор по указанному пути
    int fd = open(PATH, O_WRONLY, MODE);

    // Пишем через дескриптор
    write(fd, MSG, 4);

    // Закрываем дескриптор
    close(fd);

    // Удаляем временный файл? как я понял, unlink лучше делать там же где мы создавали канал
    unlink(PATH);
   
    return 0;
}