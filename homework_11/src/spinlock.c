#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define STORE_COUNT 5
#define CUSTOMER_COUNT 3
#define LOADER_SUPPLY_AMOUNT 200

void *status;

pthread_spinlock_t locks[STORE_COUNT];
pthread_t threads[4];
int thread_id[4] = {0, 1, 2, 3};

int store_min = 900;
int store_max = 1100;
int store_supply[STORE_COUNT];

int customer_min = 9900;
int customer_max = 10100;
int customer_demand[CUSTOMER_COUNT];

void *BuyGoods(void *arg)
{
    int thread_id = *(int *)arg;
    while (1)
    {
       // Цикл для прохода покупателя по магазинам, по очерёдно
        for (size_t i = 0; i < STORE_COUNT; i++)
        {

            // Пытаемся заблокировать магазин
            if (!pthread_spin_unlock(&locks[i]))
            {
                printf("Customer %d (%d)\t| store %ld (%d)\t\t| -> ENTER\n", thread_id, customer_demand[thread_id], i, store_supply[i]);

                // Проверяем если в магазине нет припасов, то выходим из него, предварительно разблокировав
                if (store_supply[i] <= 0)
                {
                    printf("Customer %d (%d)\t| store %ld (%d)\t\t| <- LEAVE, STORE EMPTY\n", thread_id, customer_demand[thread_id], i, store_supply[i]);
                    pthread_spin_unlock(&locks[i]);
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
                            pthread_spin_unlock(&locks[i]);
                            return 0;
                        }

                        customer_demand[thread_id]--;
                        store_supply[i]--;
                    }

                    // Если потребности клиента не были утолены, то разблокируем магазин и идём в другой
                    printf("Customer %d (%d)\t| store %ld (%d)\t\t| + BUY\n", thread_id, customer_demand[thread_id], i, store_supply[i]);
                    pthread_spin_unlock(&locks[i]);
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

void cleanup_spin_unlock(void *arg)
{
    pthread_spin_unlock((pthread_spinlock_t *)arg);
}

void *LoadShops()
{
    while (1)
    {
        int store_to_supply = rand() % 5;
        
        if (!pthread_spin_trylock(&locks[store_to_supply])) 
        {
            pthread_cleanup_push(cleanup_spin_unlock, (void *)&locks[store_to_supply]); 
            store_supply[store_to_supply] += LOADER_SUPPLY_AMOUNT;
            printf("Loader\t\t\t| store %d (%d)\t\t| ++ LOAD SUCCESS\n", store_to_supply, store_supply[store_to_supply]);
            pthread_cleanup_pop(1);
        }
        else
        {
            printf("Loader\t\t\t| store %d\t| -- LOAD FAIL, STORE BUSY\n", store_to_supply);
        }
        sleep(1);
    }
}

void Init()
{
    srand(time(NULL));

    for (size_t i = 0; i < STORE_COUNT; i++)
    {
        store_supply[i] = (rand() % (store_max - store_min + 1)) + store_min;
        pthread_spin_init(&locks[i], PTHREAD_PROCESS_PRIVATE); 
    }
    
    for (size_t i = 0; i < CUSTOMER_COUNT; i++)
        customer_demand[i] = (rand() % (customer_max - customer_min + 1)) + customer_min;
    
    for (size_t i = 0; i < CUSTOMER_COUNT; i++)
        pthread_create(&threads[i], NULL, BuyGoods, &thread_id[i]);

    pthread_create(&threads[CUSTOMER_COUNT], NULL, LoadShops, &thread_id[CUSTOMER_COUNT]);
}

int main(void)
{

    Init();

    for (size_t j = 0; j < CUSTOMER_COUNT; j++)
    {
        pthread_join(threads[j], &status);
        printf("Customer %ld\t\t| FINISHED SHOPPING. Exit status %ld\n", j, (long)status);
    }

    pthread_cancel(threads[CUSTOMER_COUNT]);
    pthread_join(threads[CUSTOMER_COUNT], &status);
    printf("LOADING STOPPED\t\t| Loader stopped working.\n");

    for (size_t k = 0; k < CUSTOMER_COUNT + 1; k++)
        pthread_spin_destroy(&locks[k]); 
                                         
    printf("\n\nSPINLOCK\n\n");

    return 0;
}