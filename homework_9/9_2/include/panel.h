/**
 * @file panel.h
 * @author Шустов Александр
 * @brief Описание панели файлового проводника
 * @version 0.1
 * @date 2026-02-25
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdlib.h>
#include <ncurses.h>
#include "file_data.h"

#ifndef PANEL_H    
#define PANEL_H

/**
 * @brief Панель хранящая информацию об окнах проводника и полную информацию об указанном в currentDir каталоге
 * 
 */
typedef struct Panel{
    WINDOW *parent_window;   // Окно родитель, просто рамка с указанием текущей директории
    WINDOW *child_window;    // Окно потомок, в него и будут выводиться список папок/файлов
    FileData *dir_data;      // Список файлов/папок в текущей директории
    size_t top_index;       // Индекс вернего элемента списка
    size_t selected_index;  // Индекс выбранного элемента
    size_t files_count;     // Количество файлов в выбранной директории
    char current_dir[1024];  // Текущая директория
    bool is_active;          // Флаг является ли панель активной в данный момент
} Panel;

#endif //PANEL_H
