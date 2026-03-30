/**
 * @file types.h
 * @author Шустов Александр
 * @brief Структуры для работы с командами 
 * @version 0.1
 * @date 2026-03-30
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdlib.h>

#ifndef TYPES_H
#define TYPES_H

/**
 * @brief Структура одной команды
 * **args - массив со строковыми токенами команд и их параметрами
 * capacity - максимальная емкость массива
 * argc - текущее количество аргументов
 */
typedef struct Command
{
    char **args;
    size_t capacity;
    size_t argc;
} Command;

/**
 * @brief Структура списка команд
 * **commands - строковый массив со списком команд
 * capacity - максимальная ёмкость массива
 * cmd_count - текуще количество команж
 */
typedef struct Pipeline
{
    Command **commands;
    size_t capacity;
    size_t cmd_count;
} Pipeline;

#endif //TYPES_H