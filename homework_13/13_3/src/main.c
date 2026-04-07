#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <ncurses.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#define SIZE 1024
#define SERVER_PATH "/133_Server"
// #define

typedef enum MessageType
{
    OPEN,
    CLOSE,
    TEXT
} MessageType;

typedef struct Message
{
    MessageType msg_type;
    char text[SIZE];
    char client_name[100];
} Message;

typedef struct MessageList
{
    Message *messages;
    int count;
} MessageList;

typedef struct ConnectionList
{
    char *clients[SIZE];
    int count;
} ConnectionList;

struct mq_attr queue_attr;

volatile int keep_running;
int is_server;
Message msg_recieved;
Message msg_send;

void handle_signal(int sig)
{
    (void)sig;
    keep_running = 0;
}

int ServerSendLastMessage(ConnectionList *con_list, char *message_text)
{
    int i = 0;

    while (i < con_list->count)
    {
        mqd_t desc = mq_open(con_list->clients[i], O_WRONLY | O_EXCL);
        if (desc == -1)
        {
            perror("Error in ServerSendLastMessage");
            return -1;
        }

        Message send_message;
        strcpy(send_message.client_name, con_list->clients[i]);
        strcpy(send_message.text, message_text);
        send_message.msg_type = TEXT;
        mq_send(desc, (char *)&send_message, sizeof(Message), 2);
    }

    return 0;
}

int ServerSendAllMessages(MessageList *msg_list, char *client_name)
{

    int i = 0;
    mqd_t desc = mq_open(client_name, O_WRONLY | O_EXCL);

    while (i < msg_list->count)
    {
        if (desc == -1)
        {
            perror("Error in ServerSendAllMessages");
            return -1;
        }

        Message send_message;
        strcpy(send_message.client_name, SERVER_PATH);
        strcpy(send_message.text, msg_list->messages[i].text);
        send_message.msg_type = TEXT;
        mq_send(desc, (char *)&send_message, sizeof(Message), 2);
    }

    return 0;
}

int CloseCLientConnection(ConnectionList *con_list, char *client_name)
{

    for (size_t i = 0; i < (size_t)con_list->count; i++)
    {
        if (strcmp(con_list->clients[i], client_name) == 0)
        {
            for (size_t j = i; j < (size_t)con_list->count; j++)
            {
                // TODO
                con_list->clients[i] = con_list->clients[i + 1];
            }
            con_list->count--;
            return 0;
        }
    }

    return -1;
}

int main(int argc, char *argv[])
{

    if (argc == 2 && strcmp(argv[1], "s") == 0)
        is_server = 1;
    else
        is_server = 0;

    mqd_t queue;
    keep_running = 1;

    signal(SIGINT, handle_signal);

    if (is_server)
    {
        MessageList msg_list = {0};
        msg_list.messages = malloc(sizeof(Message) * 1024);
        ConnectionList con_list = {0};

        queue = mq_open(SERVER_PATH, O_CREAT | O_EXCL | O_WRONLY, 0664);
        if (queue == -1)
        {
            perror("Server queue error");
            mq_close(queue);
            mq_unlink(SERVER_PATH);
            exit(EXIT_FAILURE);
        }

        while (keep_running)
        {

            mq_getattr(queue, &queue_attr);
            ssize_t message_error = mq_receive(queue, (char *)&msg_recieved, queue_attr.mq_msgsize, NULL);
            if (message_error == -1)
            {
                // TODO:
            }

            if (msg_recieved.msg_type == OPEN)
            {
                //memcpy(&msg_list.messages[msg_list.count], &msg_recieved, sizeof(Message));
                strcpy(msg_list.messages[msg_list.count], msg_recieved.text);
                msg_list.count++;
                ServerSendAllMessages(&msg_list, msg_recieved.client_name);
                ServerSendLastMessage(&con_list, msg_recieved.text);
            }
            else if (msg_recieved.msg_type == TEXT)
            {
                strcpy(msg_list.messages[msg_list.count], msg_recieved.text);
                msg_list.count++;
                ServerSendLastMessage(&con_list, msg_recieved.text);
            }
            else if (msg_recieved.msg_type == CLOSE)
            {
                strcpy(msg_list.messages[msg_list.count], msg_recieved.text);
                msg_list.count++;
                CloseCLientConnection(&con_list, msg_recieved.client_name);
                ServerSendLastMessage(&con_list, msg_recieved.text);
            }

            if (errno == EINTR)
            {
                printf("server shuting down");
                mq_close(queue);
                mq_unlink(SERVER_PATH);
                break;
            }
        }
    }
    else
    {
        queue = mq_open(SERVER_PATH, O_CREAT | O_EXCL | O_RDWR, 0664);
    }

    exit(EXIT_SUCCESS);
}