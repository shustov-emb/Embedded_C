/**
 * @file globals.h
 * @author Шустов Александр
 * @brief Решил попробовать сделать глобальный модуль для глобальных переменных, 
 * не уверен что так делают, но я попробовал, не уверен что буду дальше такое делать.
 * @version 0.1
 * @date 2026-03-30
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <types.h>

#ifndef GLOBALS_H
#define GLOBALS_H

/**
 * @brief Переменная для выхода из программы
 */
extern int exit_program;

/**
 * @brief Переменная для работы со списком команд см. /include/types.h
 */
extern Pipeline pipeline;

#endif //GLOBALS_H