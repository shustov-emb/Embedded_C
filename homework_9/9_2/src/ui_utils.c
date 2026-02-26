#include <string.h>
#include "path_utils.h"
#include "ui_utils.h"

void DrawPanel(struct Panel *panel) {

    //Перерисовка родительской рамки и пути
    werase(panel->parent_window);
    box(panel->parent_window, 0, 0);

    int parent_x = getmaxx(panel->parent_window);
    int path_max = parent_x - 10;
    if (path_max < 4) path_max = 4;

    //Отрисовываем заголовок родительского окна в котором будут отображаться путь
    char title[1024];
    if ((int)strlen(panel->current_dir) > path_max) {
        snprintf(title, sizeof(title), "...%s", panel->current_dir + strlen(panel->current_dir) - (size_t)(path_max - 3));
    } else {
        snprintf(title, sizeof(title), "%s", panel->current_dir);
    }

    //Если панель активна в данный момент, то инвертируем стиль 
    if (panel->is_active) {
        wattron(panel->parent_window, A_REVERSE | A_BOLD);
    }
    mvwprintw(panel->parent_window, 0, 2, " %s ", title);
    if (panel->is_active) {
        wattroff(panel->parent_window, A_REVERSE | A_BOLD);
    }

    //Затираем дочернее окно для надёжности
    werase(panel->child_window);
    
    //Получаем значение размеров дочернего окна в котором будем все выводить
    int lines, cols;
    getmaxyx(panel->child_window, lines, cols);

    //Рассчитываем размеры под колонки 
    int name_size,dir_size;
    name_size = cols/2;
    dir_size = (cols-name_size)/4;

    //Скролл без пустой последней строки
    int visible_rows = (lines > 1) ? (lines - 1) : 0;

    //Высчитываем индекс самого верхнего элемента
    if (panel->files_count > 0) {
        if (panel->selected_index < panel->top_index) {
            panel->top_index = panel->selected_index;
        } else if (visible_rows > 0 && panel->selected_index >= panel->top_index + visible_rows) {
            panel->top_index = panel->selected_index - visible_rows + 1;
        }
    } else {
        panel->selected_index = 0;
        panel->top_index = 0;
    }

    //Задаём цветовую схему для заголовка
    wattron(panel->child_window, COLOR_PAIR(2) | A_BOLD);
    /*Равномерно закрашиваем строку заголовка, чтобы не было темного хвоста справа.
     * Возможно так делать неправильно, но как победить недокрашенный фон в конце строки заголовка, я не придумал*/
    wmove(panel->child_window, 0, 0);
    whline(panel->child_window, ' ', cols);
    //Выводим на экран
    mvwprintw(panel->child_window, 0, 1, "%-*s  | %-*s | %-*s", 
            name_size, "Name", 
            dir_size, "Type", 
            dir_size, "Size"); 
    wattroff(panel->child_window, COLOR_PAIR(2)| A_BOLD);

    char* dir_file;

    //Основной цикл вывода списка файлов/папок
    for (int i = 0; i < (lines - 1); i++)
    {
        size_t idx = panel->top_index + i;
        if (idx >= panel->files_count) {
            break;
        }

        //Если выбранный индекс совпадает с индексом из списка, то красим его в другую цветовую палитру, через wattrset, что крайне удобно
        if(idx == panel->selected_index && panel->is_active) {
            wattrset(panel->child_window, COLOR_PAIR(3) | A_BOLD);
        } else{
            wattrset(panel->child_window, COLOR_PAIR(1));
        }

        //Определяем является ли объект папкой или файлом
        dir_file = (panel->dir_data[idx].is_dir == 1) ? "/":" ";

        wmove(panel->child_window, (int)i + 1, 0);
        //Для нижней строки child не трогаем последний столбец, чтобы не было артефакта в углу
        int row = i + 1;
        int line_len = (row == lines - 1 && cols > 0) ? (cols - 1) : cols;
        whline(panel->child_window, ' ', line_len);
        //Ну и выводим имя файла на экран
        mvwprintw(panel->child_window, i + 1, 1, "%s%-*s | %-*s | %-*ld",
            dir_file,
            name_size, panel->dir_data[idx].name, 
            dir_size, (panel->dir_data[idx].is_dir == 1) ? "DIR":"FILE" , 
            dir_size, panel->dir_data[idx].size);
    }

    //Помечаем окно родительское как грязное
    touchwin(panel->parent_window);
    //Обновляем
    wnoutrefresh(panel->parent_window);
    wnoutrefresh(panel->child_window);
}
