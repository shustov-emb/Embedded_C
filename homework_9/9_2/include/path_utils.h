/**
 * @file path_utils.h
 * @author Шустов Александр
 * @brief Впомогательные методы в основном для формирования путей и списка каталогов/файлов в выбранной директории
 * @version 0.1
 * @date 2026-02-25
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include <stdlib.h>
#include <ncurses.h>
#include "panel.h"
#include "file_data.h"

#ifndef PATH_UTILS_H    
#define PATH_UTILS_H

/**
 * @brief 
 * 
 * @param [in] path Путь для проверки
 * @return true - Является корневой директорией
 * @return false - Не является корневой директорией
 */
bool IsRootPath(const char *path);

/**
* @brief Функция формирующая полный путь путем объединения базового пути и имени объекта.
*
* @param [in] base Базовый путь (родительский каталог)
* @param [in] name Имя добавляемого файла или подкаталога
* @param [out] out Буфер, в который будет записан результирующий путь
* @param [in] out_size Размер выходного буфера
*/
void BuildPath(const char *base, const char *name, char *out, size_t out_size);

/**
* @brief Функция извлекающая путь к родительскому каталогу из заданного строкой пути.
*
* @param [in] path Исходный полный путь
* @param [out] out Буфер, в который будет записан результирующий путь
* @param [in] out_size Размер выходного буфера
*/
void BuildParentPath(const char *path, char *out, size_t out_size);

/**
 * @brief Функция формирующая данные о всех объектах в выбранной директории path
 * 
 * @param [in] path Исходный полный путь
 * @param [out] records Массив структур в которые будут записанны данные по всем объектам выбранной директории path 
 * @return int - Количество объектов выбранной директориии path
 */
int GetDirInfo(char *path, FileData **records);

/**
 * @brief Функция обновляющая данные панели, по новому пути объекта
 * 
 * @param [out] panel Панель для обновления данных
 * @param [in] new_path Новый путь объекта
 */
void RenewDirData(Panel *panel, const char *new_path);

#endif //PATH_UTILS_H