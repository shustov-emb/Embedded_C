/**
 * @file client.c
 * @author Шустов Александр
 * @brief Решил сделать с двумя очередями, потому что пробовал с одной, 
 * и там получалось что сервер и отправляет сообщение и сразу его читает
 * 
 * @version 0.1
 * @date 2026-04-02
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <sys/stat.h>       
#include <mqueue.h>
#include <fcntl.h>
#include <string.h>

//ИДентификаторы для очередей
#define PATH_SERVER "/132Server"
#define PATH_CLIENT "/132Client"

//Размер сообщений
#define SIZE 1024

//Структура сообщений, тут понял что можно в целмом то одной структурой обойтись!
typedef struct MSG {
    char text[SIZE];
} MSG;

//структура в которую будем читать атрибуты приходящих сообщений для получения их размера
struct mq_attr attr;

int main (void) {

    //Тут будут и отправляемые сообщения и получаемые
    MSG msg_snd;
 
    //Пытаемся подключится к очереди и получаем дескриптор для первой очереди
    mqd_t mq_desc_server = mq_open(PATH_SERVER, O_RDONLY | O_EXCL);
    //Не получилось, выходим
    if (mq_desc_server == -1) {
        perror("Error mq_desc: ");
        exit(EXIT_FAILURE);
    }
    
    printf("\n");
    //Сюда получаем приоритет входящего сообщения, нигде не используем но для информации 
    unsigned int msg_priority;
    //Получаем аттрибуты входящего сообщения, для того чтобы размер определить
    mq_getattr(mq_desc_server, &attr);
    //Получаем сообщение, размеры мы из аттрибутов входящего сообщения получаем 
    mq_receive(mq_desc_server, (char *)&msg_snd, attr.mq_msgsize, &msg_priority);
    printf("client: %s\n\n", msg_snd.text);

    //Пытаемся подключится к очереди и получаем дескриптор для второй очереди
    mqd_t mq_desc_client = mq_open(PATH_CLIENT, O_WRONLY | O_EXCL);
    //Не получилось, выходим
    if (mq_desc_client == -1){
        perror("Error mq_desc: ");
        exit(EXIT_FAILURE);
    }

    //Копируем сообщение для отправки
    memcpy(msg_snd.text, "Hello from client!", 18);
    //Отправляем сообщение от клиента с приоритетом 1
    mq_send(mq_desc_client, (char *)&msg_snd, sizeof(MSG), 1);

    //Закрываем дескрипторы, сервер файлы эти удалит
    mq_close(mq_desc_client);
    mq_close(mq_desc_server);
    
    exit(EXIT_SUCCESS);
}