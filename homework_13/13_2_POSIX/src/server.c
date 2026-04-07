/**
 * @file server.c
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
//Размер сообщения и режимы
#define SIZE 1024
#define MODE S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH

//Структура сообщений, тут понял что можно в целмом то одной структурой обойтись! 
typedef struct MSG
{
    char text[SIZE];
} MSG;

//структура в которую будем читать атрибуты приходящих сообщений для получения их размера
struct mq_attr attr;

int main(void)
{

    //Тут будут и отправляемые сообщения и получаемые
    MSG message;

    //Копируем сообщение для отправки
    memcpy(message.text, "Hello form server!\0", 19);

    //Создаём очередь и получаем дескриптор для первой очереди
    mqd_t mq_desc_server = mq_open(PATH_SERVER, O_CREAT | O_EXCL | O_WRONLY, MODE, NULL);
    //При ошибке пытаемся удалить очередь и выходим, мало ли сколько там процессов в этой очереди сидят, ждать пока они все позакрываются не вариант
    if (mq_desc_server == -1)
    {
        perror("Error mq_desc");
        mq_close(mq_desc_server);
        mq_unlink(PATH_SERVER);
        exit(EXIT_FAILURE);
        // mq_desc_server = mq_open(mq_desc_server, O_CREAT | O_EXCL, MODE, NULL);
    }

    printf("\n");
    //Отправляем сообщение с приоритетом 2, у клиента 1
    mq_send(mq_desc_server, (char *)&message, sizeof(MSG), 2);

    //Создаём очередь и получаем дескриптор для второй очереди
    mqd_t mq_desc_client = mq_open(PATH_CLIENT, O_CREAT | O_EXCL | O_RDONLY, MODE, NULL);
    //Тоже самое что и с первой очередью
    if (mq_desc_client == -1)
    {
        perror("Error mq_desc");
        mq_close(mq_desc_client);
        mq_unlink(PATH_CLIENT);
        exit(EXIT_FAILURE);
        //mq_desc_client = mq_open(mq_desc_client, O_CREAT | O_EXCL, MODE, NULL);
    }

    //Сюда получаем приоритет входящего сообщения, нигде не используем но для информации 
    unsigned int msg_priority;
    //Получаем аттрибуты входящего сообщения, для того чтобы размер определить
    mq_getattr(mq_desc_server, &attr);
    //Получаем сообщение, размеры мы из аттрибутов входящего сообщения получаем 
    mq_receive(mq_desc_client, (char *)&message, attr.mq_msgsize, &msg_priority);
    printf("server: %s\n\n", message.text);

    //Закрываем дескрипторы
    mq_close(mq_desc_client);
    mq_close(mq_desc_server);

    //Удаляем файлы
    mq_unlink(PATH_CLIENT);
    mq_unlink(PATH_SERVER);

    exit(EXIT_SUCCESS);
}
