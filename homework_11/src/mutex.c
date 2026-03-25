/**
 * @file mutex.c
 * @author Шустов Александр
 * @brief
 * @version 0.1
 * @date 2026-03-25
 *
 * @copyright Copyright (c) 2026
 *
 */

#define _POSIX_C_SOURCE 200112L // без этой строчки не определялся pthread_spinlock_t
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/**
 * @brief Определяем основные значения
 * STORE_COUNT - Количество магазинов
 * CUSTOMER_COUNT - Количетсво поситителей
 * LOADER_SUPPLY_AMOUNT - Размер поставок погрузчика
 */
#define STORE_COUNT 5
#define CUSTOMER_COUNT 3
#define LOADER_SUPPLY_AMOUNT 200

/**
 * @brief Определяем mutex и потоки
 * status - статус звершения потоков
 * locks - mutex замки
 * threads - количество потоков
 * thread_id - id потоков
 */
void *status;
pthread_mutex_t locks[STORE_COUNT];
pthread_t threads[4];
int thread_id[4] = {0, 1, 2, 3};

/**
 * @brief Данные для инициализации
 */
int store_min = 900;           // Минимальное значение запасов в магазине на старте
int store_max = 1100;          // Максимальное количество запасов в магазине на старте
int store_supply[STORE_COUNT]; // Массив с запасами магазинов

int customer_min = 9900;             // Минимальное значение запасов покупателя на старте
int customer_max = 10100;            // Максимальное количество запасов покупателя на старте
int customer_demand[CUSTOMER_COUNT]; // Массив с потербностями покупателей

/**
 * @brief Поточная функция скупки товаров из магазинов
 *
 * @param [in] arg Параметры потока, в нашем случае число - id потока
 * @return void*
 */
void *BuyGoods(void *arg)
{

    int thread_id = *(int *)arg; // Получем идентификатор потока назначенный при создании
    while (1)
    {
        // Цикл для прохода покупателя по магазинам, по очерёдно
        for (size_t i = 0; i < STORE_COUNT; i++)
        {

            // Пытаемся заблокировать магазин
            if (!pthread_mutex_trylock(&locks[i]))
            {
                printf("Customer %d (%d)\t| store %ld (%d)\t\t| -> ENTER\n", thread_id, customer_demand[thread_id], i, store_supply[i]);

                // Проверяем если в магазине нет припасов, то выходим из него, предварительно разблокировав
                if (store_supply[i] <= 0)
                {
                    printf("Customer %d (%d)\t| store %ld (%d)\t\t| <- LEAVE, STORE EMPTY\n", thread_id, customer_demand[thread_id], i, store_supply[i]);
                    pthread_mutex_unlock(&locks[i]);
                    // printf("Customer %d\t\t| GOING TO SLEEP zzzZZZ\n", thread_id);
                    // sleep(2);
                    continue;
                }
                else
                {
                    /*Если в магазине есть какой нибудь товар, то супаем его/
                     *Сделал циклом, а не просто вычитанием, хотя бы какую-то полезную нагрузку дать потоку */
                    int temp = store_supply[i];
                    for (int j = 0; j < temp; j++)
                    {
                        // Если потребности покупателя удовлетворены, мы разблокируем магазин и выходим из функции
                        if (customer_demand[thread_id] <= 0)
                        {
                            pthread_mutex_unlock(&locks[i]);
                            return 0;
                        }

                        customer_demand[thread_id]--;
                        store_supply[i]--;
                    }

                    // Если потребности клиента не были утолены, то разблокируем магазин и идём в другой
                    printf("Customer %d (%d)\t| store %ld (%d)\t\t| + BUY\n", thread_id, customer_demand[thread_id], i, store_supply[i]);
                    pthread_mutex_unlock(&locks[i]);
                }
            }
            // Если магазин занят, то так и пишем
            else
            {
                printf("Customer %d\t\t| store %ld\t\t| x STORE BUSY\n", thread_id, i);
            }

            // printf("Customer %d\t\t| GOING TO SLEEP zzzZZZ\n", thread_id);
            // sleep(2);
        }
    }
}

/**
 * @brief Вспомогательная функция для разблокировки мьютекса
 * Нужна для pthread_cleanup_push
 * @param [in] arg Параметр с pthread_mutex_t
 */
void cleanup_mutex_unlock(void *arg)
{
    pthread_mutex_unlock((pthread_mutex_t *)arg);
}

/**
 * @brief Функция погрузчика для загрузки товаров в магазины
 *
 * @return void*
 */
void *LoadShops()
{
    while (1)
    {
        // Выбираем рандомно в какой магазин поток будет грузить припасы
        int store_to_supply = rand() % 5;

        // Пытаемася зайти в магазин
        if (!pthread_mutex_trylock(&locks[store_to_supply]))
        {
            /*Данный поток прерывается извне, и ближайшее прерывание происходит на pritnf
            Мы просто не успеем дойти до pthread_mutex_unlock и выходим до освобождения мютекса,
            и именно поэтому мы пользуемся pthread_cleanup_push.
            Этот макрос позволяет нам гарантированно выполнить все что находиться от pthread_cleanup_push и до pthread_cleanup_pop
            и вызвать функцию разблокировки мьютекса которую мы передали*/
            pthread_cleanup_push(cleanup_mutex_unlock, &locks[store_to_supply]);
            store_supply[store_to_supply] += LOADER_SUPPLY_AMOUNT;
            printf("Loader\t\t\t| store %d (%d)\t\t| ++ LOAD SUCCESS\n", store_to_supply, store_supply[store_to_supply]);
            pthread_cleanup_pop(1);
            // pthread_mutex_unlock(&locks[store_to_supply]);
        }
        // Если зайти в магазин не получилось, пишем что он занят
        else
        {
            printf("Loader\t\t\t| store %d\t| -- LOAD FAIL, STORE BUSY\n", store_to_supply);
        }
        //
        sleep(1);
    }
}

/**
 * @brief Функция начальной инициализации. Отвечает за начальную генерацию количества товаро в магазине,
 * за количество потребностей покупателей
 * за генерацию потоков покупателей и погрузчика
 */
void Init()
{
    srand(time(NULL));

    // Рандомим количество товаров в каждом магазине исоздаём мьютексы
    for (size_t i = 0; i < STORE_COUNT; i++)
    {
        store_supply[i] = (rand() % (store_max - store_min + 1)) + store_min;
        pthread_mutex_init(&locks[i], NULL);
    }

    // Рандомим количество потребностей у покупателей
    for (size_t i = 0; i < CUSTOMER_COUNT; i++)
        customer_demand[i] = (rand() % (customer_max - customer_min + 1)) + customer_min;

    // Создаём потоки покупателей
    for (size_t i = 0; i < CUSTOMER_COUNT; i++)
        pthread_create(&threads[i], NULL, BuyGoods, &thread_id[i]);

    // Создаём поток погрузчика
    pthread_create(&threads[CUSTOMER_COUNT], NULL, LoadShops, &thread_id[CUSTOMER_COUNT]);
}

int main(void)
{

    Init();

    // Ждём когда потребности покупателей утолятся
    for (size_t j = 0; j < CUSTOMER_COUNT; j++)
    {
        pthread_join(threads[j], &status);
        printf("Customer %ld\t\t| FINISHED SHOPPING. Exit status %ld\n", j, (long)status);
    }

    // Когда дождались завершившихся потоков с покупателями, выходим из потока погрузчика
    pthread_cancel(threads[CUSTOMER_COUNT]);
    pthread_join(threads[CUSTOMER_COUNT], &status);
    printf("LOADING STOPPED\t\t| Loader stopped working.\n");

    // Высвобождаем память
    for (size_t k = 0; k < CUSTOMER_COUNT + 1; k++)
        pthread_mutex_destroy(&locks[k]);

    printf("\n\nMUTEX\n\n");

    return 0;
}