/**
 * @file client.c
 * @author Шустов Александр
 * @brief Клиентская часть разделяемой памяти на System V
 * @version 0.1
 * @date 2026-04-13
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>

#define PATH "/tmp/shm_system_v"
#define SIZE 1024

//Структура данных, которая будет лежать в разделяемой памяти
typedef struct SharedData
{
    //Буфер для сообщения
    char text[SIZE]; 
} SharedData;

int main(void)
{
    //Получаем ключ и пытаемся найти уже созданный сервером сегмент
    key_t token = ftok(PATH, 'a');
    if (token == -1)
    {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    //Создаём сегмент разделяемой памяти
    int shm_id = shmget(token, sizeof(SharedData), 0666);
    if (shm_id == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    //Подключаемся к набору семафоров, созданному сервером
    int sem_id = semget(token, 2, 0666);
    if (sem_id == -1)
    {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    //Подключаем разделяемую память. shmat при ошибке возвращает не NULL, а значение (void *)-1
    SharedData *data = shmat(shm_id, NULL, 0);
    if (data == (void *)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    //Ждём, пока сервер запишет сообщение
    struct sembuf op_wait = {0, -1, 0};
    semop(sem_id, &op_wait, 1);

    printf("\nclient: %s\n\n", data->text);

    //Записываем ответ серверу
    memcpy(data->text, "Hello from client!", 19);

    //Сообщаем серверу, что ответ готов
    struct sembuf op_post = {1, 1, 0};
    semop(sem_id, &op_post, 1);

    shmdt(data);

    exit(EXIT_SUCCESS);
}
