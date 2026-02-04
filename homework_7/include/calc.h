/**
 * @file calc.h
 * @author Шустов Александр
 * @brief 
 * @version 0.1
 * @date 2026-02-04
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef CALC_H
#define CALC_H

/**
 * @brief СЛожение двух чисел
 * 
 * @param[in] a первый операд
 * @param[in] b второй операнд
 * @return результат сложения
 */
int add(int a, int b);

/**
 * @brief Вычитание двух чисел
 * 
 * @param[in] a первый операд
 * @param[in] b второй операнд
 * @return результат вычитания
 */
int div(int a, int b);

/**
 * @brief Умножение двух чисел
 * 
 * @param[in] a первый операд
 * @param[in] b второй операнд
 * @return результат умножения
 */
int mul(int a, int b);

/**
 * @brief Деление двух чисел
 * 
 * @param[in] a первый операд
 * @param[in] b второй операнд
 * @return результат деления
 */
int sub(int a, int b);

#endif //CALC_H