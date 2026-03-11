
/**
 * @file main.c
 * @author Шустов Александр
 * @brief Реализовать программу, которая порождает процесс. 
Родительский процесс и дочерний выводят на экран свой pid, ppid. 
Затем родительский ждет завершения дочернего и выводит его статус завершения
 * @version 0.1
 * @date 2026-03-11
 *
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdio.h>
#include <unistd.h>
//#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(void)
{

	//Объявляем процесс и интовый статус
	pid_t child_pid;
	int status;
    printf("\n");
	
	//Разветвляем процесс и через условия говорим кому что делать!
	child_pid = fork();
	if (child_pid == 0) {
		printf("Child pid = %d | Child ppid = %d\n\n", getpid(), getppid());
		//Завершаем дочерний процесс
		exit(5);
    } else {
        printf("Parent pid = %d | Parent ppid = %d\n\n", getpid(), getppid());
		//Дожидаемся статуса завершения дочернего процесса и выводим его на экран
		wait(&status);
		printf("status = %d\n\n", WEXITSTATUS(status));
	}
	return 0;
}