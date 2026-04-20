/**
 * @file client.c
 * @author Шустов Александр
 * @brief Клиентская часть разделяемой памяти POSIX
 * @version 0.1
 * @date 2026-04-13
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <string.h>

#define PATH "/14_2_POSIX_SHM"
#define SEM_SERVER "/14_2_sem_server"
#define SEM_CLIENT "/14_2_sem_client"
#define SIZE 1024

//Структура данных в разделяемой памяти
typedef struct SharedData
{
    char text[SIZE]; //Буфер для сообщения
} SharedData;

int main(void)
{
    //Открываем объект разделяемой памяти, созданный сервером
    int shm_fd = shm_open(PATH, O_RDWR, 0666);
    if (shm_fd == -1)
    {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    //Открываем семафоры, созданные сервером
    sem_t *sem_server = sem_open(SEM_SERVER, 0);
    if (sem_server == SEM_FAILED)
    {
        perror("sem_open");
        close(shm_fd);
        exit(EXIT_FAILURE);
    }

    sem_t *sem_client = sem_open(SEM_CLIENT, 0);
    if (sem_client == SEM_FAILED)
    {
        perror("sem_open");
        sem_close(sem_server);
        close(shm_fd);
        exit(EXIT_FAILURE);
    }

    //Отображаем объект разделяемой памяти в память процесса
    SharedData *data = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (data == MAP_FAILED)
    {
        perror("mmap");
        close(shm_fd);
        exit(EXIT_FAILURE);
    }

    //Ждём сообщение от сервера
    sem_wait(sem_server);

    printf("\nclient: %s\n\n", data->text);

    //Записываем ответ серверу
    memcpy(data->text, "Hello from client!", 19);

    //Сообщаем серверу, что ответ готов
    sem_post(sem_client);

    munmap(data, sizeof(SharedData));
    sem_close(sem_client);
    sem_close(sem_server);
    close(shm_fd);

    exit(EXIT_SUCCESS);
}
