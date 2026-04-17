#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "client_ui.h"

//Печать строки с обрезкой по ширине окна
static void PrintShort(WINDOW *win, int y, int x, int width, const char *text)
{
    char buffer[CHAT_TEXT_SIZE + CHAT_QUEUE_SIZE + 4];

    //Если места нет, печатать нечего
    if (width <= 0)
        return;

    snprintf(buffer, sizeof(buffer), "%s", text);

    //Непечатаемые символы заменяются пробелами, чтобы окно не ломалось
    for (int i = 0; buffer[i] != '\0'; i++)
    {
        if (!isprint((unsigned char)buffer[i]))
            buffer[i] = ' ';
    }

    //Длинная строка обрезается по ширине окна
    if ((int)strlen(buffer) > width)
    {
        if (width > 3)
            snprintf(buffer + width - 3, sizeof(buffer) - (size_t)(width - 3), "...");
        else
            buffer[width] = '\0';
    }

    mvwprintw(win, y, x, "%-*s", width, buffer);
}

//Инициализация цветовых пар ncurses
static void InitColor(void)
{
    start_color();
    use_default_colors();
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    init_pair(2, COLOR_YELLOW, COLOR_BLUE);
    init_pair(3, COLOR_BLACK, COLOR_WHITE);
    init_pair(4, COLOR_CYAN, COLOR_BLUE);
}

void DestroyUi(ClientUi *ui)
{
    //Окна удаляются в обратном порядке: сначала дочерние, потом родительские
    if (ui->chat_sub) { delwin(ui->chat_sub); ui->chat_sub = NULL; }
    if (ui->clients_sub) { delwin(ui->clients_sub); ui->clients_sub = NULL; }
    if (ui->input_win) { delwin(ui->input_win); ui->input_win = NULL; }
    if (ui->chat_win) { delwin(ui->chat_win); ui->chat_win = NULL; }
    if (ui->clients_win) { delwin(ui->clients_win); ui->clients_win = NULL; }
}

void RecreateUi(ClientUi *ui)
{
    int clients_width = COLS / 4;
    int input_height = 3;
    int work_height = LINES - input_height;
    int chat_width;

    //Справа располагается список клиентов, слева окно сообщений
    if (clients_width < 18)
        clients_width = 18;
    if (clients_width > COLS / 2)
        clients_width = COLS / 2;

    chat_width = COLS - clients_width;

    DestroyUi(ui);
    clear();

    //Сверху создаются два окна: сообщения и пользователи
    ui->chat_win = newwin(work_height, chat_width, 0, 0);
    ui->clients_win = newwin(work_height, clients_width, 0, chat_width);

    //Снизу на всю ширину терминала создаётся поле ввода команд
    ui->input_win = newwin(input_height, COLS, work_height, 0);

    wbkgd(ui->clients_win, COLOR_PAIR(1));
    wbkgd(ui->chat_win, COLOR_PAIR(1));
    wbkgd(ui->input_win, COLOR_PAIR(1));

    //Внутренние окна нужны, чтобы текст не попадал на рамку
    ui->chat_sub = derwin(ui->chat_win, work_height - 2, chat_width - 2, 1, 1);
    ui->clients_sub = derwin(ui->clients_win, work_height - 2, clients_width - 2, 1, 1);

    wbkgd(ui->clients_sub, COLOR_PAIR(1));
    wbkgd(ui->chat_sub, COLOR_PAIR(1));
}

void InitUi(ClientUi *ui)
{
    memset(ui, 0, sizeof(*ui));

    //Базовая инициализация ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    //getch не должен блокировать главный цикл, потому что параллельно надо обновлять чат
    nodelay(stdscr, TRUE);
    curs_set(TRUE);
    InitColor();
    RecreateUi(ui);
}

//Отрисовка правого окна со списком клиентов
static void DrawClients(ClientUi *ui, char clients[][CHAT_QUEUE_SIZE], size_t clients_count)
{
    int lines, cols;

    //Перерисовка рамки и внутреннего окна со списком клиентов
    werase(ui->clients_win);
    box(ui->clients_win, 0, 0);
    wattron(ui->clients_win, COLOR_PAIR(2) | A_BOLD);
    mvwprintw(ui->clients_win, 0, 2, " Clients ");
    wattroff(ui->clients_win, COLOR_PAIR(2) | A_BOLD);

    werase(ui->clients_sub);
    getmaxyx(ui->clients_sub, lines, cols);

    //Выводим только тех клиентов, которые помещаются в окно
    for (int i = 0; i < lines && (size_t)i < clients_count; i++)
    {
        wattron(ui->clients_sub, COLOR_PAIR(4) | A_BOLD);
        mvwprintw(ui->clients_sub, i, 0, "* ");
        wattroff(ui->clients_sub, COLOR_PAIR(4) | A_BOLD);
        PrintShort(ui->clients_sub, i, 2, cols - 2, clients[i]);
    }

    wnoutrefresh(ui->clients_win);
    wnoutrefresh(ui->clients_sub);
}

//Отрисовка левого окна с сообщениями чата
static void DrawChat(ClientUi *ui, ChatMessage *messages, size_t messages_count)
{
    int lines, cols;
    size_t first = 0;

    werase(ui->chat_win);
    box(ui->chat_win, 0, 0);
    wattron(ui->chat_win, COLOR_PAIR(2) | A_BOLD);
    mvwprintw(ui->chat_win, 0, 2, " Chat ");
    wattroff(ui->chat_win, COLOR_PAIR(2) | A_BOLD);

    werase(ui->chat_sub);
    getmaxyx(ui->chat_sub, lines, cols);

    //В окне чата показывается нижняя часть истории сообщений
    if (messages_count > (size_t)lines)
        first = messages_count - (size_t)lines;

    for (int i = 0; i < lines && first + (size_t)i < messages_count; i++)
    {
        ChatMessage *msg = &messages[first + (size_t)i];
        char line[CHAT_TEXT_SIZE + CHAT_QUEUE_SIZE + 4];

        //Системные сообщения выводятся без имени отправителя
        if (msg->msg_type == MSG_SYSTEM)
            snprintf(line, sizeof(line), "* %s", msg->text);
        else
            snprintf(line, sizeof(line), "%s: %s", msg->client_queue, msg->text);

        PrintShort(ui->chat_sub, i, 0, cols, line);
    }

    wnoutrefresh(ui->chat_win);
    wnoutrefresh(ui->chat_sub);
}

//Отрисовка нижнего окна ввода команд
static void DrawInput(ClientUi *ui, const char *input)
{
    int cols;

    //Строка ввода команд всегда остаётся снизу
    werase(ui->input_win);
    box(ui->input_win, 0, 0);
    cols = getmaxx(ui->input_win);

    wattron(ui->input_win, COLOR_PAIR(2) | A_BOLD);
    wattroff(ui->input_win, COLOR_PAIR(2) | A_BOLD);
    mvwprintw(ui->input_win, 1, 1, "> ");
    PrintShort(ui->input_win, 1, 3, cols - 4, input);

    //Окно помечается к обновлению, а реальный вывод делает doupdate
    wnoutrefresh(ui->input_win);
}

void DrawUi(ClientUi *ui,
            ChatMessage *messages,
            size_t messages_count,
            char clients[][CHAT_QUEUE_SIZE],
            size_t clients_count,
            const char *input)
{
    //Все окна обновляются одной пачкой, чтобы меньше мигало
    DrawClients(ui, clients, clients_count);
    DrawChat(ui, messages, messages_count);
    DrawInput(ui, input);
    doupdate();
}
