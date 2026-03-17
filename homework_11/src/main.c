#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define STORE_COUNT 5
#define CUSTOMER_COUNT 3

// pthread_once_t key_init = PTHREAD_ONCE_INIT;
// pthread_cond_t awake_condition = PTHREAD_COND_INITIALIZER;
// pthread_key_t key;

void *status;
pthread_mutex_t locks[STORE_COUNT]; //= PTHREAD_MUTEX_INITIALIZER;
pthread_t threads[4];
int thread_id[4] = {0, 1, 2, 3};

int store_min = 900;
int store_max = 1100;
int store_supply[STORE_COUNT];

int customer_min = 9900;
int customer_max = 10100;
int customer_demand[CUSTOMER_COUNT];

// void dest(void *buff){
// 	free(buff);
// }

// void key_create(){

// 	pthread_key_create(&key, dest);
// }

// char buff;
// pthread_once(&once, key_create);

// buff = pthread_getspicific(key);
// if (buff == NULL){
//     buff = malloc(255);
//     pthread_setspecific(key, buff);
// }

// pthread_mutex_lock(&m1);
// pthread_cond_wait(&cond, &m1);

// pthread_cond_signal(&cond1);

void *BuyGoods(void *arg)
{
    int thread_id = *(int *)arg;
    while (1)
    {

        for (size_t i = 0; i < STORE_COUNT; i++)
        {

            if (!pthread_mutex_trylock(&locks[i]))
            {
                printf("Customer %d (%d)\t| store %ld (%d)\t\t| ENTER\n", thread_id, customer_demand[thread_id], i, store_supply[i]);

                if (store_supply[i] <= 0)
                {
                    printf("Customer %d (%d)\t| store %ld (%d)\t\t| LEAVE, STORE EMPTY\n", thread_id, customer_demand[thread_id], i, store_supply[i]);
                    pthread_mutex_unlock(&locks[i]);
                    continue;
                }

                if (customer_demand[thread_id] >= store_supply[i])
                {
                    customer_demand[thread_id] -= store_supply[i];
                    store_supply[i] = 0;
                    printf("Customer %d (%d)\t| store %ld (%d)\t\t| ___BUY\n", thread_id, customer_demand[thread_id], i, store_supply[i]);
                    // printf("\t thread %d, BUYS store %ld (demand %d, supply %d)\n", thread_id, i, customer_demand[thread_id], store_supply[i]);
                    //  printf("\tBought: Customer %d - demand %d | store %ld - supply %d\n",
                    //     thread_id, customer_demand[thread_id], i, store_supply[i]);
                    //  fflush(stdout);
                    printf("Customer %d (%d)\t| store %ld (%d)\t\t| LEAVE STORE\n", thread_id, customer_demand[thread_id], i, store_supply[i]);
                    // printf("\t thread %d, UNLOCKS store %ld (demand %d, supply %d)\n", thread_id, i, customer_demand[thread_id], store_supply[i]);
                    pthread_mutex_unlock(&locks[i]);
                }
                else
                {
                    store_supply[i] -= customer_demand[thread_id];
                    customer_demand[thread_id] = 0;
                    printf("Customer %d (%d)\t| store %ld (%d)\t\t| ___BUY\n", thread_id, customer_demand[thread_id], i, store_supply[i]);
                    // printf("\t thread %d, BUYS store %ld (demand %d, supply %d)\n", thread_id, i, customer_demand[thread_id], store_supply[i]);
                    //  printf("\tBought: Customer %d - demand %d | store %ld - supply %d\n",
                    //     thread_id, customer_demand[thread_id], i, store_supply[i]);
                    //  fflush(stdout);
                    printf("Customer %d (%d)\t| store %ld (%d)\t\t| DONE SHOPPING\n", thread_id, customer_demand[thread_id], i, store_supply[i]);
                    // printf("\t\t thread %d, EXIT store %ld (demand %d, supply %d)\n", thread_id, i, customer_demand[thread_id], store_supply[i]);
                    pthread_mutex_unlock(&locks[i]);
                    return 0;
                }
            }
            else
            {
                // lock
                printf("Customer %d\t\t| store %ld\t\t| x STORE BUSY\n", thread_id, i);
            }
        }

        sleep(2);
    }
}

void cleanup_unlock(void *arg) //TODO 
{
    pthread_mutex_unlock((pthread_mutex_t *)arg);
}

void *LoadShops()
{
    while (1)
    {
        int store_to_supply = rand() % 5;
        if (!pthread_mutex_trylock(&locks[store_to_supply]))
        {
            pthread_cleanup_push(cleanup_unlock, &locks[store_to_supply]); //TODO 
            store_supply[store_to_supply] += 2200;
            printf("Loader\t\t\t| store %d (%d)\t\t| LOAD SUCCESS\n", store_to_supply, store_supply[store_to_supply]);
            // fflush(stdout);
            pthread_cleanup_pop(1); //TODO 
           // pthread_mutex_unlock(&locks[store_to_supply]);
        }
        else
        {
            printf("Loader\t\t\t| store %d\t| LOAD FAIL, STORE BUSY\n", store_to_supply);
            // fflush(stdout);
        }
        sleep(1);
    }
}

void Init()
{
    // Рандомим магазинам начальные запасы
    for (size_t i = 0; i < STORE_COUNT; i++)
    {
        store_supply[i] = (rand() % (store_max - store_min + 1)) + store_min;
        pthread_mutex_init(&locks[i], NULL);
    }

    // Рандомим потребности покупателей
    for (size_t i = 0; i < CUSTOMER_COUNT; i++)
        customer_demand[i] = (rand() % (customer_max - customer_min + 1)) + customer_min;
}

int main(void)
{
    srand(time(NULL));

    Init();

    for (size_t i = 0; i < CUSTOMER_COUNT; i++)
        pthread_create(&threads[i], NULL, BuyGoods, &thread_id[i]);

    pthread_create(&threads[CUSTOMER_COUNT], NULL, LoadShops, &thread_id[CUSTOMER_COUNT]);

    for (size_t j = 0; j < CUSTOMER_COUNT; j++)
    {
        pthread_join(threads[j], &status);
        printf("Customer %ld\t\t| FINISHED SHOPPING. Exit status %ld\n", j, (long)status);
        // fflush(stdout);
    }

    pthread_cancel(threads[CUSTOMER_COUNT]);
    pthread_join(threads[CUSTOMER_COUNT], &status);
    printf("LOADING STOPPED\t\t| Loader stopped working.\n");
    // fflush(stdout);

    for (size_t k = 0; k < CUSTOMER_COUNT + 1; k++)
        pthread_mutex_destroy(&locks[k]);

    return 0;
}