#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include <types.h>
#include <parse_utils.h>
#include <globals.h>
#include <memory_cleaner.h>


int main(void) {

    //Инициалируем список команд
    pipeline.cmd_count = 0; 
    pipeline.capacity = 8;
    pipeline.commands = calloc(pipeline.capacity, sizeof(Command));
    if (!pipeline.commands)
        return -1;

    char *user_data;

    while (1)
    {
        printf("username:");
        fflush(0);
        user_data = ReadString();

        // Если данные невведены продолжаем
        if (!user_data)
        {
            continue;
        } 
        // Если в строке есть токен "exit" - выходим, попутно высвобождая память
        else if (strstr(user_data, "exit") != NULL)
        {
             FreeMem(user_data, pipeline.commands);
             break;
        }
        

        // Парсим команды, если успешно то пишем все в переменную pipeline, если нет то чистим память и продолжаем
        if (ParseStringToCommand(user_data) == -1)
        {
            FreeMem(user_data,NULL);
            continue;
        }

        // Тут начинается работа с каналами
        int prev_read_fd = -1;
        int pipe_fds[2];
        for (size_t i = 0; i < pipeline.cmd_count; i++)
        {
            // Создаем канал, если это не последняя команда
            if (i < pipeline.cmd_count - 1)
            {
                if (pipe(pipe_fds) == -1)
                {
                    perror("Error (pipe) ");
                    FreeMem(user_data,pipeline.commands);
                    return -1;
                }
            }

            // Создаём дочерний процесс
            pid_t pid = fork();
            if (pid == 0)
            {
                // Если не первая команда — берем вход из предыдущей трубы
                if (i > 0)
                {
                    dup2(prev_read_fd, STDIN_FILENO);
                    close(prev_read_fd);
                }

                // Если не последняя команда — направляем выход в текущую трубу
                if (i < pipeline.cmd_count - 1)
                {
                    close(pipe_fds[0]); // Чтение нам тут не нужно
                    dup2(pipe_fds[1], STDOUT_FILENO);
                    close(pipe_fds[1]);
                }

                // Выполняем команду
                execvp(pipeline.commands[i]->args[0], pipeline.commands[i]->args);
                perror("Error (execvp)");
                FreeMem(user_data, pipeline.commands);
                exit(EXIT_FAILURE);
            }
            else if (pid > 0)
            {
                // Больше не нужен вход от предыдущего шага
                if (i > 0)
                {
                    close(prev_read_fd);
                }

                // Если не последняя команда, сохраняем выход трубы для следующего шага
                if (i < pipeline.cmd_count - 1)
                {
                    close(pipe_fds[1]);         // Писать в неё родитель не будет
                    prev_read_fd = pipe_fds[0];
                }
            }
        }

        // В конце обязательно закрываем последний болтающийся дескриптор, если он был
        if (prev_read_fd != -1)
            close(prev_read_fd);

        // Ожидание всех детей
        for (size_t i = 0; i < pipeline.cmd_count; i++)
        {
            pipeline.commands[i]->argc = 0;
            wait(NULL);
        }

        // Чистим память
        FreeMem(user_data,NULL);
        pipeline.cmd_count = 0;
    }
}
