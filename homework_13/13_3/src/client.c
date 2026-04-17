#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "client_ui.h"
#include "transport.h"

#define INPUT_SIZE 512

/**
 * @brief Клиентские данные 
 */
typedef struct ClientData
{
    ChatMessage messages[MAX_HISTORY];          // История сообщений
    size_t messages_count;                      // Количество сообщений
    char clients[MAX_CLIENTS][CHAT_QUEUE_SIZE]; // Список клиенов
    size_t clients_count;                       // Количество клиентов
    pthread_mutex_t lock;                       // Мьютекс
} ClientData;

/**
 * @brief Аргументы для потока
 */
typedef struct ThreadArgs
{
    TransportQueue *queue; // Очередь
    ClientData *data;      // Клиентские данные
} ThreadArgs;

static volatile sig_atomic_t keep_running = 1;
static char client_queue_path[CHAT_QUEUE_SIZE];

//Обработчик сигнала просто меняет флаг работы
static void HandleSignal(int sig)
{
    (void)sig;
    //Флаг проверяется в основном цикле и в потоке приёма
    keep_running = 0;
}

static void CopyString(char *dst, size_t dst_size, const char *src)
{
    //strncpy сам не всегда ставит '\0', поэтому последний байт задаётся вручную
    strncpy(dst, src, dst_size - 1);
    dst[dst_size - 1] = '\0';
}

static void AddMessage(ClientData *data, const ChatMessage *message)
{
    //При заполнении истории удаляем самое старое сообщение
    if (data->messages_count >= MAX_HISTORY)
    {
        /*memmove сдвигает сообщения на одну позицию влево:
          messages[1] становится messages[0], messages[2] становится messages[1] и так далее.
          Используется именно memmove, потому что копирование идёт внутри одного массива
          и области памяти пересекаются.
        */
        memmove(&data->messages[0],
                &data->messages[1],
                sizeof(ChatMessage) * (MAX_HISTORY - 1));

        //После сдвига последнее место считается свободным под новое сообщение
        data->messages_count = MAX_HISTORY - 1;
    }

    memcpy(&data->messages[data->messages_count], message, sizeof(*message));
    data->messages_count++;
}

static void ParseClients(ClientData *data, const char *text)
{
    char buffer[CHAT_TEXT_SIZE];
    char *save_ptr = NULL;
    char *name;

    //Сервер присылает список клиентов одной строкой с разделителем '\n'
    data->clients_count = 0;
    CopyString(buffer, sizeof(buffer), text);

    name = strtok_r(buffer, "\n", &save_ptr);
    while (name && data->clients_count < MAX_CLIENTS)
    {
        CopyString(data->clients[data->clients_count],
                   sizeof(data->clients[data->clients_count]),
                   name);
        data->clients_count++;
        name = strtok_r(NULL, "\n", &save_ptr);
    }
}

static void *ReceiveMessages(void *arg)
{
    ThreadArgs *thread_args = arg;

    while (keep_running)
    {
        ChatMessage message = {0};

        //Ожидаем сообщение из личной очереди клиента
        if (TransportReceive(thread_args->queue, &message) == -1)
            continue;

        //Поток только принимает данные и складывает их в общее состояние
        pthread_mutex_lock(&thread_args->data->lock);
        if (message.msg_type == MSG_USERS)
            ParseClients(thread_args->data, message.text);
        else
            AddMessage(thread_args->data, &message);
        pthread_mutex_unlock(&thread_args->data->lock);
    }

    return NULL;
}

static ChatMessage BuildMessage(MessageType type, const char *text)
{
    ChatMessage message = {0};

    message.msg_type = type;
    CopyString(message.client_queue, sizeof(message.client_queue), client_queue_path);
    CopyString(message.text, sizeof(message.text), text);

    return message;
}

static int SendClientMessage(TransportQueue *server_queue, MessageType type, const char *text)
{
    ChatMessage message = BuildMessage(type, text);

    //Все сообщения клиента уходят в общую очередь сервера
    return TransportSend(server_queue, &message);
}

static void PrepareClientQueueName(void)
{
    //Имя очереди строится через pid процесса, оно же дальше будет именем клиента
    snprintf(client_queue_path, sizeof(client_queue_path), "/%d_Client", getpid());
}

static void CopyDataForDraw(ClientData *data,
                            ChatMessage *messages,
                            size_t *messages_count,
                            char clients[][CHAT_QUEUE_SIZE],
                            size_t *clients_count)
{
    //Для отрисовки берётся снимок данных, чтобы ncurses работал в основном потоке
    pthread_mutex_lock(&data->lock);

    *messages_count = data->messages_count;
    memcpy(messages, data->messages, sizeof(ChatMessage) * data->messages_count);

    *clients_count = data->clients_count;
    memcpy(clients, data->clients, sizeof(data->clients));

    pthread_mutex_unlock(&data->lock);
}

static int IsExitCommand(const char *input)
{
    return strcmp(input, "exit") == 0;
}

static void HandleInputKey(int key,
                           char *input,
                           size_t *input_len,
                           TransportQueue *server_queue)
{
    if (key == ERR)
        return;

    //Удаление последнего символа в строке ввода
    if (key == KEY_BACKSPACE || key == 127 || key == '\b')
    {
        if (*input_len > 0)
        {
            (*input_len)--;
            input[*input_len] = '\0';
        }
        return;
    }

    if (key == '\n' || key == '\r')
    {
        if (*input_len == 0)
            return;

        //Enter отправляет сообщение или завершает клиент
        if (IsExitCommand(input))
        {
            SendClientMessage(server_queue, MSG_CLOSE, "left chat");
            keep_running = 0;
        }
        else
        {
            SendClientMessage(server_queue, MSG_TEXT, input);
        }

        *input_len = 0;
        input[0] = '\0';
        return;
    }

    if (key >= 0 && key < 256 && isprint(key) && *input_len + 1 < INPUT_SIZE)
    {
        //Обычные печатные символы добавляются в буфер ввода
        input[*input_len] = (char)key;
        (*input_len)++;
        input[*input_len] = '\0';
    }
}

int main(void)
{
    TransportQueue server_queue = {0};
    TransportQueue client_queue = {0};
    ClientUi ui;
    ClientData data = {0};
    ThreadArgs thread_args;
    pthread_t thread;
    struct sigaction sa;
    char input[INPUT_SIZE] = {0};
    size_t input_len = 0;
    ChatMessage messages[MAX_HISTORY];
    char clients[MAX_CLIENTS][CHAT_QUEUE_SIZE];
    size_t messages_count;
    size_t clients_count;

    PrepareClientQueueName();

    //Клиент сначала создаёт личную очередь для входящих сообщений
    if (TransportCreateReadQueue(&client_queue, client_queue_path) == -1)
        exit(EXIT_FAILURE);

    //Потом открывает серверную очередь для исходящих сообщений
    if (TransportOpenWriteQueue(&server_queue, SERVER_QUEUE) == -1)
    {
        TransportClose(&client_queue);
        TransportUnlink(client_queue_path);
        exit(EXIT_FAILURE);
    }

    pthread_mutex_init(&data.lock, NULL);

    //Поток приёма нужен, чтобы сообщения приходили независимо от ввода
    thread_args.queue = &client_queue;
    thread_args.data = &data;
    pthread_create(&thread, NULL, ReceiveMessages, &thread_args);

    sa.sa_handler = HandleSignal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    //После подключения сервер пришлёт историю и список клиентов
    SendClientMessage(&server_queue, MSG_OPEN, "joined chat");
    InitUi(&ui);

    //Основной цикл обрабатывает клавиатуру и перерисовывает интерфейс
    while (keep_running)
    {
        int key = getch();

        //При изменении размера терминала окна создаются заново
        if (key == KEY_RESIZE)
            RecreateUi(&ui);
        else
            HandleInputKey(key, input, &input_len, &server_queue);

        CopyDataForDraw(&data, messages, &messages_count, clients, &clients_count);
        DrawUi(&ui, messages, messages_count, clients, clients_count, input);

        //napms делает паузу в миллисекундах, чтобы цикл не грузил процессор постоянно
        napms(40);
    }

    //Перед закрытием клиент сообщает серверу о выходе
    SendClientMessage(&server_queue, MSG_CLOSE, "left chat");
    pthread_cancel(thread);
    pthread_join(thread, NULL);

    DestroyUi(&ui);
    endwin();

    pthread_mutex_destroy(&data.lock);
    TransportClose(&server_queue);
    TransportClose(&client_queue);
    TransportUnlink(client_queue_path);

    exit(EXIT_SUCCESS);
}
