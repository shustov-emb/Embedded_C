/**
 * @file main.c
 * @author  Шустов Александр
 * @brief Реализовать аналог командного интерпретатора bash.
При запуске программы пользователю предлагается ввести имя программы и опции запуска программы.
Программа порождает процесс и в нем выполняет введенную программу с заданными опциями, ждет завершения дочернего процесса.
Снова возвращается к вводу следующей программы. Выход из интерпретатора по команде exit.
 * @version 0.1
 * @date 2026-03-11
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

/**
 * @brief 
 * 
 */
typedef struct Command
{
    char **argv;
    size_t capacity;
} Command;

/**
 * @brief 
 * 
 * @return char* 
 */
char *ReadString()
{
    size_t size = 64;
    size_t len = 0;
    char *buf = malloc(size);
    int ch;

    if (buf == NULL)
        return NULL;

    while ((ch = getchar()) != '\n' && ch != EOF)
    {
        buf[len] = (char)ch;
        len++;

        if (len + 1 >= size)
        {
            size *= 2;
            char *tmp = realloc(buf, size);
            if (!tmp)
            {
                free(buf);
                return NULL;
            }
            buf = tmp;
        }
    }

    if (len == 0)
    {
        free(buf);
        return NULL;
    }

    buf[len] = '\0';
    return buf;
}

/**
 * @brief Get the Command object
 * 
 * @param command 
 * @param user_data 
 * @return int 
 */
int GetCommand(Command *command, char *user_data)
{

    //Парсим первый аргумент
    command->argv[0] = strtok(user_data, " ");

    //Если он null пустой значит пришла пустая строка, возвращаем -1
    if (!command->argv[0])
        return -1;

    //Цикл в котором парсим оставшиеся аргументы
    size_t i = 1;
    do
    {

        //Если количество аргументов больше чем выделенная память, то мы добавляем в два раза больше памяти 
        if (i == command->capacity)
        {
            
            command->capacity *= 2;
            command = realloc(command, sizeof(command) * command->capacity);
            if (!command)
            {
                free(command);
                return -1;
            }
        }
        //Парсим аргументы в буфер
        else
        {
            command->argv[i] = strtok(NULL, " ");
            i++;
        }

    //Если последний аргумент null - значит все распарсилось.
    } while (command->argv[i - 1] != NULL);

    //Попытка сжимать буфер под аргументы, но потом мне показалось что это избыточно, будет лишний раз нагружать процесор
    // void *temp;
    // if (buffer_size > i)
    // {

    //     temp = realloc(command, sizeof(command) * i);
    //     if (!temp)
    //     {
    //         free(command);
    //         return NULL;
    //     }
    //     else
    //         command = temp;
    // }

    return 0;
}

int main()
{

    char *user_data;
    int sub_proc_status;
    pid_t sub_proc;

    //Инициализируем команду, будем переиспользовать эту структуру, это лучше чем пересоздавать новую каждый раз
    Command *command = malloc(sizeof(Command));
    if (!command)
        return -1;
   
    //Количество аргументов доступных изначально!
    command->capacity = 64;
   
    //Память под аргументы
    command->argv = malloc(sizeof(char *) * command->capacity);
    if (!command->argv)
        return -1;
    


    //Главный цикл в котором мы запускаем програмы и опрашиваем пользователя
    while (1) {
        printf("username:");
        fflush(NULL); //поскольку новую строку не ставим, флашим вывод
        
        //Берём данные у пользователя
        user_data = ReadString();
        if(!user_data){
            continue;
        }

        //Если пользователь ввёл exit то выходим
        if (strcmp(command->argv[0], "exit") == 0){
            free(user_data);
            break;
        }

        //Получаем уже распаршенную команду
        if (GetCommand(command, user_data) == -1){
            free(user_data);    
            continue;
        }
            
        //Создаём процесс, потому что exec подменит бинарную программу
        sub_proc = fork();
        if (sub_proc == 0)
        {
            //Запускаем процесс через execvp, первым аргументом он принимает название программы
            //А вторым массив аргументов и название программы, что крайне странно, хотя мы уже и передали название программы первым параметром
            if (execvp(command->argv[0], command->argv) == -1)
            {
                //В случае ошибки высвобождаем память
                perror("Execution error");
                free(command->argv);
                free(command);
                free(user_data);
                exit(-1);
            }
        }

        wait(&sub_proc_status);
        //Статус нам вовзаращает та программа которую запускает exec
        // printf("satatus: %d\n",WEXITSTATUS(sub_proc_status));

        //Высвобождаем память
        free(user_data);
    } 

    //Высвобождаем все остальное
    free(command->argv);
    free(command);

    return 0;
}