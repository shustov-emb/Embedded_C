/*Написать программу, которая создает файл с именем output.txt,
записывает в него строку “String from file”, затем считывает ее из файла
с конца и выводит на экран*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

int main(){
    
    //Открываем файл на запись, если его нет, то файл создастся сам, получаем дескриптор
    int fd = open("./output.txt", O_RDWR | O_CREAT ,0666);
    
    char *buf = "String from file";
    //Высчитываем размер строки
    size_t size = sizeof(char)*strlen(buf);
    //Записываем
    write(fd, buf, size);   
    //Обязательно закрываем дескриптор
    close(fd);

    //Открываем файл на чтение
    fd = open("./output.txt", O_RDONLY);
    
    //Находим конец файла + 1 для терминирующего нуля
    int file_size = lseek(fd, 0, SEEK_END)+1;
    char buffer[file_size];

    //Итерируемся задом наперёд, и по одному считываем в буффер
    for (size_t i = file_size; i > 0; i--)
    {
        lseek(fd, file_size-i, SEEK_SET);
        read(fd, buffer+(i-2), 1);
    }
    
    //Терминируем нулём конец строки
    buffer[file_size-1] = '\0';
    //Обязательно закрываем дескриптор
    close(fd);

    printf("\nFile backwards: %s\n\n", buffer);

    return 0;
}