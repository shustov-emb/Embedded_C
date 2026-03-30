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
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

// Решил тут дефайнами определить путь и режимы
#define PATH "/tmp/test_buffer"
#define MODE S_IRUSR | S_IRGRP | S_IROTH

int main(void) {

    // Буфер
    char msg[4];

    // Открываем дескриптор по укаанному пути, и с указанными режимами
    int fd = open(PATH, O_RDONLY, MODE);
    
    // Читаем в буфер
    read(fd, msg, 4);

    printf("\n\n(Client) Other process says: %s\n\n", msg);
    
    // Закрываем дескриптор
    close(fd);

    return 0;
}