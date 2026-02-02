/**
 * @file io.c
 * @author Шустов Александр
 * @brief Исходный код реализации вспомогательных методов пользовательского ввода/вывода 
 * @version 0.1
 * @date 2026-02-02
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <malloc.h> 
#include <stdio.h>
#include <stdlib.h>
#include "io.h"

char *ReadString()
{
    size_t size = 64;
    size_t len = 0;
    char *buf = malloc(size);
    int ch;

    //Если память не выделилась, освобождаем выделенную память и возвращаем NULL
    if (!buf)
        return NULL;

    //Посимвольно считываем ввод пользователя
    while ((ch = getchar()) != '\n' && ch != EOF)
    {
        buf[len] = (char)ch;
        len++;

        /*Если длина строки превышет объём выделенной память
          то увеличиваем размер в два раза и realloc'ом 
          перевыделяем память
        */
        if (len + 1 >= size)
        {
            size *= 2;
            char *tmp = realloc(buf, size);
            
            /*Если память по какой то причине выделить не получилось
              освобождаем выделенную память и возвращаем NULL
            */
            if (!tmp)
            {
                free(buf);
                return NULL;
            }
            buf = tmp;
        }
    }

    //Еесли пользователь ничего не ввёл то освобождаем выделенную память и возвращаем NULL
    if(len == 0){
        free(buf);
        return NULL;
    }
    
    //Ставим терминирующий ноль в концце строки и возвращем строку
    buf[len] = '\0';
        return buf;
}

size_t GetNumber(const char *message){
    char *input;
    size_t val = 0;
    while (1) {
        
        if (message) printf("%s", message);
        input = ReadString(); 
        
        //Если введённые данные не NULL 
        if (input) {
            //Проверяем есть ли в ведённых пользовательем данных число size_t
            int items = sscanf(input, "%zu", &val);
            
            //Освобождаем память вводимых данных, они больше не понадобятся
            free(input);

            //Если есть совпадение, возвращаем значение 
            if (items == 1) return val;
        }
        printf("Invalid input. Try again!\n");
    }
}
