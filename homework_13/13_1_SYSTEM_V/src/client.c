/**
 * @file client.c
 * @author Шустов Александр
 * @brief Клиентская часть очередей сообщений на system5
 * @version 0.1
 * @date 2026-04-02
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

#define PATH "/tmp/msg_queue"   //Буфер очереди 
#define SIZE 100                //Размер сообщения

//Структура сообщения
typedef struct MSG {
    long mtype;      //Тип для приоритета
    char text[SIZE]; //Буфер для сообщения
} MSG;

int main (void) {

    MSG msg[1]; 
    msg[0].mtype = 2;  //Задём приоритет отправки
    memcpy(msg[0].text, "Hello from client!", 19); //Копируем сообщение для отправки в текст структуры

    //Получаем токен и пытаемся подключится
    key_t token = ftok(PATH, 'a');
    int msg_id = msgget(token, 0666);
    //Если не получилось - выходим
    if (msg_id == -1) {
        printf("error:%d",msg_id);
        exit(EXIT_FAILURE);
    }

    printf("\n");
    //Ждём сообщения от сервера с приоритетом 1 (Блокирующй вызов)
    msgrcv(msg_id, &msg[1], sizeof(msg[1].text), 1, 0);
    printf("server: %s\n\n", msg[1].text);

    //Отправляем сообщение в ответ с приоритетом 2
    msgsnd(msg_id, &msg[0], sizeof(msg[0].text), 0);
    
    exit(EXIT_SUCCESS);
}