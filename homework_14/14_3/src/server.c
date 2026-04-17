#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "server_data.h"
#include "transport.h"

static volatile sig_atomic_t keep_running = 1;

static void HandleSignal(int sig)
{
    (void)sig;
    //После SIGINT сервер выйдет из основного цикла
    keep_running = 0;
}

static void CopyString(char *dst, size_t dst_size, const char *src)
{
    //Копирование строки с гарантированным завершающим нулём
    strncpy(dst, src, dst_size - 1);
    dst[dst_size - 1] = '\0';
}

static void AddToHistory(MessageList *msg_list, const ChatMessage *message)
{
    //История хранит последние MAX_HISTORY сообщений
    if (msg_list->count >= MAX_HISTORY)
    {
        /*memmove сдвигает историю на одну позицию влево:
          самое старое сообщение затирается, а в конце освобождается место.
          Для такого сдвига нужен memmove, потому что источник и получатель
          находятся внутри одного массива.
        */
        memmove(&msg_list->messages[0],
                &msg_list->messages[1],
                sizeof(ChatMessage) * (MAX_HISTORY - 1));

        //После сдвига в истории остаётся MAX_HISTORY - 1 сообщений
        msg_list->count = MAX_HISTORY - 1;
    }

    memcpy(&msg_list->messages[msg_list->count], message, sizeof(*message));
    msg_list->count++;
}

static int FindClient(ConnectionList *con_list, const char *queue)
{
    //Поиск клиента выполняется по имени его личного объекта разделяемой памяти
    for (size_t i = 0; i < con_list->count; i++)
    {
        if (strcmp(con_list->clients[i].queue, queue) == 0)
            return (int)i;
    }

    return -1;
}

//Добавление клиента в массив подключений
static int AddClient(ConnectionList *con_list, const ChatMessage *message)
{
    //Каждый клиент определяется именем своего объекта разделяемой памяти
    if (con_list->count >= MAX_CLIENTS)
        return -1;

    if (FindClient(con_list, message->client_queue) != -1)
        return 0;

    CopyString(con_list->clients[con_list->count].queue,
               sizeof(con_list->clients[con_list->count].queue),
               message->client_queue);
    con_list->count++;

    return 0;
}

//Удаление клиента из массива подключений
static int CloseClientConnection(ConnectionList *con_list, const char *queue)
{
    int index = FindClient(con_list, queue);

    //Если клиента уже нет в списке, повторный close игнорируется
    if (index == -1)
        return -1;

    //После удаления хвост массива сдвигается влево
    for (size_t i = (size_t)index; i + 1 < con_list->count; i++)
        con_list->clients[i] = con_list->clients[i + 1];

    con_list->count--;
    return 0;
}

//Отправка одного сообщения одному клиенту
static void SendToClient(const char *queue, const ChatMessage *message)
{
    TransportQueue client_queue = {0};

    //Объект разделяемой памяти клиента открывается только на время отправки
    if (TransportOpenWriteQueue(&client_queue, queue) == -1)
        return;

    TransportSend(&client_queue, message);
    TransportClose(&client_queue);
}

//Рассылка сообщения всем подключенным клиентам
static void Broadcast(ConnectionList *con_list, const ChatMessage *message)
{
    //Рассылка идёт всем клиентам из текущего списка
    for (size_t i = 0; i < con_list->count; i++)
        SendToClient(con_list->clients[i].queue, message);
}

//Отправка всей истории одному клиенту
static void SendAllMessages(MessageList *msg_list, const char *queue)
{
    //При входе новый клиент получает всю накопленную историю
    for (size_t i = 0; i < msg_list->count; i++)
        SendToClient(queue, &msg_list->messages[i]);
}

//Рассылка полного списка клиентов всем подключенным клиентам
static void BroadcastClients(ConnectionList *con_list)
{
    ChatMessage message = {0};
    size_t len = 0;

    message.msg_type = MSG_USERS;

    //Список клиентов отправляется отдельным служебным сообщением
    for (size_t i = 0; i < con_list->count; i++)
    {
        int written = snprintf(message.text + len,
                               sizeof(message.text) - len,
                               "%s\n",
                               con_list->clients[i].queue);

        if (written < 0 || (size_t)written >= sizeof(message.text) - len)
            break;
        len += (size_t)written;
    }

    Broadcast(con_list, &message);
}

//Формирование служебного сообщения для истории и рассылки
static ChatMessage BuildSystemMessage(const char *client_queue, const char *text)
{
    ChatMessage message = {0};

    //Системное сообщение хранится в той же истории, что и обычный текст
    message.msg_type = MSG_SYSTEM;
    CopyString(message.client_queue, sizeof(message.client_queue), client_queue);
    snprintf(message.text, sizeof(message.text), "%s %s", client_queue, text);

    return message;
}

//Обработка подключения нового клиента
static void ProcessOpen(ConnectionList *con_list, MessageList *msg_list, ChatMessage *message)
{
    ChatMessage system_message;

    //Новый клиент сначала добавляется в список подключений
    if (AddClient(con_list, message) == -1)
    {
        system_message = BuildSystemMessage("server", "client limit reached");
        SendToClient(message->client_queue, &system_message);
        return;
    }

    SendAllMessages(msg_list, message->client_queue);

    //После отправки истории всем сообщается о новом клиенте
    system_message = BuildSystemMessage(message->client_queue, "joined chat");
    AddToHistory(msg_list, &system_message);
    Broadcast(con_list, &system_message);
    BroadcastClients(con_list);
}

//Обработка отключения клиента
static void ProcessClose(ConnectionList *con_list, MessageList *msg_list, ChatMessage *message)
{
    ChatMessage system_message = BuildSystemMessage(message->client_queue, "left chat");

    //Клиент удаляется до рассылки, чтобы сообщение не ушло в закрытый объект памяти
    if (CloseClientConnection(con_list, message->client_queue) == -1)
        return;

    AddToHistory(msg_list, &system_message);
    Broadcast(con_list, &system_message);
    BroadcastClients(con_list);
}

//Обработка обычного сообщения в чат
static void ProcessText(ConnectionList *con_list, MessageList *msg_list, ChatMessage *message)
{
    //Обычное сообщение пишется в историю и рассылается всем участникам
    message->msg_type = MSG_TEXT;
    AddToHistory(msg_list, message);
    Broadcast(con_list, message);
}

int main(void)
{
    TransportQueue server_queue = {0};
    ConnectionList con_list = {0};
    MessageList msg_list = {0};
    struct sigaction sa;

    printf("PID: %d\n", getpid());

    //Сервер создаёт общий объект разделяемой памяти, куда клиенты будут писать сообщения
    if (TransportCreateReadQueue(&server_queue, SERVER_QUEUE) == -1)
        exit(EXIT_FAILURE);

    sa.sa_handler = HandleSignal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    while (keep_running)
    {
        ChatMessage message = {0};

        //Сервер ждёт сообщения в общем объекте разделяемой памяти и обрабатывает их по типу
        if (TransportReceive(&server_queue, &message) == -1)
            continue;

        printf("%s\t: %s\n", message.client_queue, message.text);

        if (message.msg_type == MSG_OPEN)
            ProcessOpen(&con_list, &msg_list, &message);
        else if (message.msg_type == MSG_CLOSE)
            ProcessClose(&con_list, &msg_list, &message);
        else if (message.msg_type == MSG_TEXT)
            ProcessText(&con_list, &msg_list, &message);
    }

    printf("Server shutting down...\n");
    TransportClose(&server_queue);
    TransportUnlink(SERVER_QUEUE);

    exit(EXIT_SUCCESS);
}
