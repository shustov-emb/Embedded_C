/**
 * @file memory_cleaner.h
 * @author Шустов Александры
 * @brief Функции для работы с очисткой памяти
 * @version 0.1
 * @date 2026-03-30
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <types.h>

#ifndef MEMORY_CLEANER_H
#define MEMORY_CLEANER_H

/**
 * @brief Функция для чистки памяти
 * 
 * @param [in] user_data - указатель, на строку с пользовательским вводом
 * @param [in] command - указатель на стурктуру с командами
 * @return int - сейча возвращает только ноль, но в целом можно возвращать - 1 в случае ошибки
 */
int FreeMem(char *user_data, Command **command);

#endif //MEMORY_CLEANER_H