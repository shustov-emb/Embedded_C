/**
 * @file server.c
 * @author Шустов Александр
 * @brief Серверная часть разделяемой памяти на System V
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
#include <fcntl.h>
#include <string.h>

#define PATH "/tmp/shm_system_v"
#define SIZE 1024
#define MODE IPC_CREAT | IPC_EXCL | 0666

//Структура данных, которая будет лежать в разделяемой памяти
typedef struct SharedData
{
    char text[SIZE]; //Буфер для сообщения
} SharedData;

//Для semctl с SETVAL нужен union semun
union semun
{
    int val;
};

int main(void)
{
    //Открываем файл, чтобы по нему получить ключ для shmget
    int fd = open(PATH, O_CREAT | O_RDWR, 0666);
    if (fd != -1)
        close(fd);

    //Получаем ключ для сегмента разделяемой памяти
    key_t token = ftok(PATH, 'a');
    if (token == -1)
    {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    //Создаём сегмент разделяемой памяти
    int shm_id = shmget(token, sizeof(SharedData), MODE);
    if (shm_id == -1)
    {
        //Если старый сегмент остался, удаляем его и создаём заново
        shm_id = shmget(token, sizeof(SharedData), 0666);
        if (shm_id != -1)
            shmctl(shm_id, IPC_RMID, NULL);

        shm_id = shmget(token, sizeof(SharedData), MODE);
        if (shm_id == -1)
        {
            perror("shmget");
            unlink(PATH);
            exit(EXIT_FAILURE);
        }
    }

    //Создаём два семафора: 0 - сервер записал, 1 - клиент ответил
    int sem_id = semget(token, 2, MODE);
    if (sem_id == -1)
    {
        sem_id = semget(token, 2, 0666);
        if (sem_id != -1)
            semctl(sem_id, 0, IPC_RMID);

        sem_id = semget(token, 2, MODE);
        if (sem_id == -1)
        {
            perror("semget");
            shmctl(shm_id, IPC_RMID, NULL);
            unlink(PATH);
            exit(EXIT_FAILURE);
        }
    }

    //Оба семафора изначально равны нулю
    union semun arg;
    arg.val = 0;
    semctl(sem_id, 0, SETVAL, arg);
    semctl(sem_id, 1, SETVAL, arg);

    //Подключаем сегмент в адресное пространство процесса
    SharedData *data = shmat(shm_id, NULL, 0);
    if (data == (void *)-1)
    {
        perror("shmat");
        shmctl(shm_id, IPC_RMID, NULL);
        unlink(PATH);
        exit(EXIT_FAILURE);
    }

    printf("\n");

    //Записываем сообщение для клиента
    memset(data, 0, sizeof(SharedData));
    memcpy(data->text, "Hi!", 4);

    //Сообщаем клиенту, что сообщение готово
    struct sembuf op_post = {0, 1, 0};
    semop(sem_id, &op_post, 1);

    //Ждём, пока клиент запишет ответ
    struct sembuf op_wait = {1, -1, 0};
    semop(sem_id, &op_wait, 1);

    printf("client: %s\n\n", data->text);

    //Отключаем и удаляем разделяемую память
    shmdt(data);
    shmctl(shm_id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID);
    unlink(PATH);

    exit(EXIT_SUCCESS);
}
