#include <memory_cleaner.h>
#include <types.h>
#include <globals.h>


/**
 * @brief Функция для чистки памяти
 * 
 * @param [in] user_data - указатель на пользовательскую строку
 * @param [in] command - указатель на тип Commands
 * @return int 
 */
int FreeMem(char *user_data, Command **command)
{

    for (size_t i = 0; i < pipeline.cmd_count; i++)
    {

        if (pipeline.commands[i]->args){
            free(pipeline.commands[i]->args);
            pipeline.commands[i]->argc = 0;
        }

        if (pipeline.commands[i])
            free(pipeline.commands[i]);
    }

    if(user_data)
        free(user_data);

    if(command)
        free(command);
    // free(pipeline.commands);

    return 0;
}