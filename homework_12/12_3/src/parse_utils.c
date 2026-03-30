#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

#include <types.h>
#include <parse_utils.h>
#include <globals.h>

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

int ParseCommand(Command *cmd, char *input)
{
    char *token = strtok(input, " ");

    size_t i = 0;
    while (token != NULL)
    {
        // Увеличиваем capacity в два раза по достижению лимитов, перевыдыелям память
        if (i + 1 == cmd->capacity)
        {
            cmd->capacity *= 2;
            cmd->args = realloc(cmd->args, sizeof(cmd->args) * cmd->capacity);
            if (!cmd->args)
            {
                free(cmd->args);
                return -1;
            }

            //TODO: ЧИСТКА ПОСЛЕ REALOC. муторно и дорого, просто буду обнулять элемент после последнего записанного аргумента
        }
        else
        {
            // Пишем токен в список аргументов, увеличиваем количество аргументов в команде
            cmd->args[i] = token;
            cmd->argc++;
            i++;
        }
        token = strtok(NULL, " ");
    }

    // Обнуляем последнее значение в конце всех аргументов, чтобы execvp не ругался на мусор в памяти
    cmd->args[i + 1] = NULL;

    return 0;
}

Command *InitCommand()
{
    //Выделяем память под команду, в случае неудачи возвращаем NULL
    Command *cmd = malloc(sizeof(Command));
    if (!cmd)
        return NULL;
    cmd->capacity = 8;
    cmd->args = calloc(cmd->capacity, sizeof(char *));
    if (!cmd->args)
    {
        free(cmd);
        return NULL;
    }

    return cmd;
}

int ParsePipeLine(char *input)
{
     // Увеличиваем capacity в два раза по достижению лимитов, перевыдыелям память
    if(pipeline.capacity + 1 == pipeline.cmd_count){
        pipeline.capacity *=2;
        Command *temp = realloc(pipeline.commands,sizeof(Command)* pipeline.capacity);
        if (!temp)
            return -1;
        pipeline.commands = &temp;

         //TODO: ЧИСТКА ПОСЛЕ REALOC. муторно и дорого, просто буду обнулять элемент после последней записанной команды
    }

    // Обнуляем последнее значение в конце всех аргументов, чтобы execvp не ругался на мусор в памяти
    pipeline.commands[pipeline.cmd_count+1] = NULL;

    //Инициализируем команду
    Command *cmd = InitCommand();
    if (!cmd)
        return -1;

    // Пытаемся распарсить команду на токены
    if (ParseCommand(cmd, input) == -1)
        return -1;
    // else if (ParseCommand(cmd, input) == -2)
    //     return -2;

    pipeline.commands[pipeline.cmd_count] = cmd;
    pipeline.cmd_count++;

    return 0;
}

int ParseStringToCommand(char *string)
{

    // Парсим входящую от пользователя строку по '|'  - разделителю
    char *cmd_ptr = string;
    int size = strlen(string) + 1;
    for (size_t i = 0; (int)i < size; i++)
    {
        //
        if (string[i] == '|' && string[i] != '\0')
        {
            string[i] = '\0';
            if (ParsePipeLine(cmd_ptr) == -1)
                return -1;
            cmd_ptr = &string[i + 1];
        }
        else if (string[i] == '\0')
        {
            if (ParsePipeLine(cmd_ptr) == -1)
                return -1;
        }
    }
    return 0;
}
