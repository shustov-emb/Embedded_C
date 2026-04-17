/**
 * @file server.c
 * @author Шустов Александр
 * @brief Серверная часть очередей сообщений на system5
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
#include <fcntl.h>
#include <string.h>


#define PATH "/tmp/msg_queue"   //Буфер очереди 
#define SIZE 100                //Размер сообщения
#define MODE IPC_CREAT | IPC_EXCL | 0666 //Режим создания очереди

//Структура сообщения
typedef struct MSG {
    long mtype;      //Тип для приоритета
    char text[SIZE]; //Буфер для сообщения
} MSG;

int main (void) {

    //Массив сообщений, одно для чтения, второе для отправки
    MSG msg[1]; 
    msg[0].mtype = 1; //Задём приоритет отправки
    //Копируем сообщение для отправки в текст структуры
    memcpy(msg[0].text, "Hi!", 4);

    //Открываем файл, получаем дескриптор
    int fd = open(PATH, O_CREAT | O_RDWR, 0666);
    if (fd != -1) close(fd);

    //получаем токен очереди
    key_t token = ftok(PATH, 'a');

    //Создаём очередь
    int msg_id = msgget(token, MODE);
    if (msg_id == -1) {
        //msgget(token, IPC_CREAT | 0666);
        //msgctl(token, msg_id, NULL);
        //Если очередь есть, удаляем её и создаём по новой! 
        msgctl(token, IPC_RMID, NULL);
        msg_id = msgget(token, MODE);
    }

    printf("\n");
    //Отправляем сообщение в очередь с приоритетос
    msgsnd(msg_id, &msg[0], sizeof(msg[0].text), 0);
    
    //Ждём сообщения от клиента с приоритетом 2 (Блокирующий вызов)
    msgrcv(msg_id, &msg[1], sizeof(msg[1].text), 2, 0);
    printf("client: %s\n\n", msg[1].text);

    //Удаляем очередь
    msgctl(msg_id, IPC_RMID, NULL);

    //Удаляем файл
    unlink(PATH);
    
    exit(EXIT_SUCCESS);
}