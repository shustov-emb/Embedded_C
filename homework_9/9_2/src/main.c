#include <string.h>
#include "path_utils.h"
#include "ui_utils.h"
#include "file_data.h"
#include "panel.h"

//Создаём переменные под окна
WINDOW *win_left, *win_right;
WINDOW *sub_left, *sub_right;

//Переменные для панелей с данными, и так же указатель на активную панель
Panel left_panel, right_panel;
Panel *active_panel;

//Вспомогательная функция, можно конечно было создать отдельную функцию, но решено было оставить так
void DestroyWindows(void) {
    if (sub_right) { delwin(sub_right); sub_right = NULL; }
    if (sub_left)  { delwin(sub_left);  sub_left  = NULL; }
    if (win_right) { delwin(win_right); win_right = NULL; }
    if (win_left)  { delwin(win_left);  win_left  = NULL; }
}

//Тут инициализируем окна, задаём фон, привязываем окна в панели
void InitWindows(void) {
    int half = COLS / 2;

    win_left = newwin(LINES - 2, half, 0, 0);
    win_right = newwin(LINES - 2, COLS - half, 0, half);

    wbkgd(win_left, COLOR_PAIR(1));
    wbkgd(win_right, COLOR_PAIR(1));

    sub_left = derwin(win_left, LINES - 4, half - 2, 1, 1);
    sub_right = derwin(win_right, LINES - 4, COLS - half - 2, 1, 1);

    wbkgd(sub_left, COLOR_PAIR(1));
    wbkgd(sub_right, COLOR_PAIR(1));

    left_panel.parent_window = win_left;
    left_panel.child_window = sub_left;
    right_panel.parent_window = win_right;
    right_panel.child_window = sub_right;
}

//Инициализируем панели оставшимися даными - списко файлов, количество, индексы выбранный и индекс верхнего объекта
void InitPanels(Panel *panel, const char *path) {
    panel->files_count = 0;
    panel->dir_data = NULL;
    panel->top_index = 0;
    panel->selected_index = 0;
    panel->is_active = 1;
    strncpy(panel->current_dir, path, sizeof(panel->current_dir) - 1);
    panel->current_dir[sizeof(panel->current_dir) - 1] = '\0';
}

int main()
{
    //Задаём начальный путь, объекты которого будут отображены
    char path[2] = "/";      

    //Инциализация ncurses
    initscr();
    refresh();

    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(FALSE);
    start_color();
    use_default_colors();
    
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    init_pair(2, COLOR_YELLOW, COLOR_BLUE);
    init_pair(3, COLOR_BLACK, COLOR_WHITE);

    InitWindows();
    InitPanels(&left_panel, path);
    InitPanels(&right_panel, path);
   
    RenewDirData(&left_panel, left_panel.current_dir);
    RenewDirData(&right_panel, right_panel.current_dir);

    //Рисуем панели, внутри функции помечаем их на обновление
    DrawPanel(&left_panel);
    DrawPanel(&right_panel);

    //Обновляем все панели разом
    doupdate(); 

    //Запоминаем активную панель
    active_panel = &left_panel;
    int key;
    while ((key = getch()) != 'q')
    {
        switch (key)
        {
            //Отслеживаем нажатие, если ввех то выбранный индекс у активной панели уменьшаем
            case KEY_UP:
               if (active_panel->files_count > 0 && active_panel->selected_index > 0) {
                    active_panel->selected_index--;
                    DrawPanel(active_panel);
                    doupdate();
                }
                break;

            //Вниз - выбранный индекс у активной панели уменьшаем
            case KEY_DOWN:  
                if (active_panel->files_count > 0 && active_panel->selected_index + 1 < active_panel->files_count) {
                    active_panel->selected_index++;
                    DrawPanel(active_panel);
                    doupdate();
                }
                break;

            /*По enter проверяем если выбранный файл это папка, то формируем путь внутрь этой папки. 
             Если это "..",  то формируем путь на каталог выше*/
            case KEY_ENTER:
            case '\n':
            case '\r':
                if (active_panel->files_count > 0) {
                    FileData *selected = &active_panel->dir_data[active_panel->selected_index];
                    if (selected->is_dir) {
                        char nextPath[1024];
                        
                        if (strcmp(selected->name, "..") == 0) {
                            BuildParentPath(active_panel->current_dir, nextPath, sizeof(nextPath));
                        } else {
                            BuildPath(active_panel->current_dir, selected->name, nextPath, sizeof(nextPath));
                        }
                        
                        RenewDirData(active_panel, nextPath);
                        DrawPanel(active_panel);
                        doupdate();
                    }
                }
                break;

            case '\t':
                //По нажатию tab переключаем панели, активной панелью помечаем ту что была неактивной
                active_panel->is_active = 0;
                active_panel = (active_panel == &left_panel)? &right_panel : &left_panel;
                active_panel->is_active = 1;
                
                //Отрисовываем сразу обе панели, для того чтобы убрать выделение выбранного индекса с теперь уже неактивной панеи
                DrawPanel(&left_panel);
                DrawPanel(&right_panel);
                doupdate();
                break;

            /*По методичке которую сбрасывали, через resizeterm  не особо получилось реагировать на изменение размера.
              Нагуглил, что слушать событие resize проще и предпочтительней*/
            case KEY_RESIZE:

                /*Мы по новой отрисовываем окна по новым размерам, может это и не очень эфективно, но я и так делаю это задание уже больше недели.
                  Было бы чуть больше свободного времени может придумал бы что-нибудь получше, но пока так.*/
                clear();
                DestroyWindows();
                InitWindows();

                DrawPanel(&left_panel);
                DrawPanel(&right_panel);
                doupdate();
                break;

            default:
                break;
        }
    }


    //Освобождаем выделенную память, под масси структур с данными об объектах
    free(left_panel.dir_data);
    free(right_panel.dir_data);

    //Освбождаем память под оокна
    DestroyWindows();

    //Завершаем программу
    endwin();

    exit(EXIT_SUCCESS);
    
}
