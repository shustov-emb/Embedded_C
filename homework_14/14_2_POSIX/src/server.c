/**
 * @file server.c
 * @author Шустов Александр
 * @brief Серверная часть разделяемой памяти POSIX
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
#include <sys/stat.h>
#include <semaphore.h>
#include <string.h>

#define PATH "/14_2_POSIX_SHM"
#define SEM_SERVER "/14_2_sem_server"
#define SEM_CLIENT "/14_2_sem_client"
#define SIZE 1024
#define MODE S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH

//Структура данных в разделяемой памяти
typedef struct SharedData
{
    char text[SIZE]; //Буфер для сообщения
} SharedData;

int main(void)
{
    //На случай прошлого аварийного завершения удаляем старый объект
    shm_unlink(PATH);
    sem_unlink(SEM_SERVER);
    sem_unlink(SEM_CLIENT);

    //Создаём объект разделяемой памяти
    int shm_fd = shm_open(PATH, O_CREAT | O_EXCL | O_RDWR, MODE);
    if (shm_fd == -1)
    {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    //Создаём два семафора: один для клиента, второй для сервера
    sem_t *sem_server = sem_open(SEM_SERVER, O_CREAT | O_EXCL, MODE, 0);
    if (sem_server == SEM_FAILED)
    {
        perror("sem_open");
        close(shm_fd);
        shm_unlink(PATH);
        exit(EXIT_FAILURE);
    }

    sem_t *sem_client = sem_open(SEM_CLIENT, O_CREAT | O_EXCL, MODE, 0);
    if (sem_client == SEM_FAILED)
    {
        perror("sem_open");
        sem_close(sem_server);
        sem_unlink(SEM_SERVER);
        close(shm_fd);
        shm_unlink(PATH);
        exit(EXIT_FAILURE);
    }

    //Задаём размер объекта разделяемой памяти
    if (ftruncate(shm_fd, sizeof(SharedData)) == -1)
    {
        perror("ftruncate");
        close(shm_fd);
        shm_unlink(PATH);
        exit(EXIT_FAILURE);
    }

    //Отображаем объект разделяемой памяти в адресное пространство процесса
    SharedData *data = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (data == MAP_FAILED)
    {
        perror("mmap");
        close(shm_fd);
        shm_unlink(PATH);
        exit(EXIT_FAILURE);
    }

    printf("\n");

    //Записываем сообщение для клиента
    memset(data, 0, sizeof(SharedData));
    memcpy(data->text, "Hi!", 4);

    //Сообщаем клиенту, что сообщение готово
    sem_post(sem_server);

    //Ждём ответ клиента
    sem_wait(sem_client);

    printf("client: %s\n\n", data->text);

    //Освобождаем ресурсы
    munmap(data, sizeof(SharedData));
    sem_close(sem_client);
    sem_close(sem_server);
    sem_unlink(SEM_CLIENT);
    sem_unlink(SEM_SERVER);
    close(shm_fd);
    shm_unlink(PATH);

    exit(EXIT_SUCCESS);
}
