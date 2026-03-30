/**
 * @file parse_utils.h
 * @author Шустов Александр
 * @brief Функции для парсинга команд на токены
 * @version 0.1
 * @date 2026-03-30
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <types.h>

#ifndef PARSE_UTILS_H
#define PARSE_UTILS_H

/**
 * @brief Функция принимающая пользовательский ввод
 * 
 * @return char* - указатель на введённые пользователем данные
 */
char *ReadString();

/**
 * @brief Функция парсящая строку на строки с командами.
 * Делит строки по токену '|'
 * @param [in] string - строка введённоая пользователем
 * @return int - 0 в случае успеха, -1 в случае ошибки
 */
int ParseStringToCommand(char *string);

/**
 * @brief Функция парсящая строки с командами на структуру команды, так же сохраняет данные о количестве команд.
 * 
 * @param [in] input - строка с командой полученная из функции ParseStringToCommand
 * @return int - 0 в случае успеха, -1 в случае ошибки
 */
int ParsePipeLine(char *input);

/**
 * @brief Функция парсящая команду на токены 
 * 
 * @param [out] cmd - структура куда будут сохранены данные о команде с токенами и количествам токенов
 * @param [in] input - входящая строка с командами
 * @return int 
 */
int ParseCommand(Command *cmd, char *input);

/**
 * @brief Функция инициализирующая тип Command, выделяя память для полей данной структуры
 * 
 * @return Command* - указатель на тип Command
 */
Command *InitCommand();



#endif