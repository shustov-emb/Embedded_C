/**
 * @file file_data.h
 * @author Шустов Александр
 * @brief Файл описания структуры данных файла
 * @version 0.1
 * @date 2026-02-25
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef FILE_DATA_H    
#define FILE_DATA_H

/**
 * @brief Определение структуры файла
 * 
 */
typedef struct FileData{
    char name[1024];    // Имя файла/папки
    long size;          // Размер файла/папки
    bool is_dir;        // is_dir == 1 - папка, is_dir == 0 - файл
} FileData;

#endif //FILE_DATA_H