#include <stdio.h>
#include <dlfcn.h>

//Вспомогательная функция для получения пользовательского ввода
int get_input(int *a, int *b)
{
    printf("Enter two values:\n");
    if (scanf("%d %d", a, b) != 2)
    {
        printf("Wrong input!\n");
        while (getchar() != '\n')
            ;
        return 0;
    }

    return 1;
}


int main()
{

    //Получаем дескриптор библиотеки
    void* handle = dlopen("libcalc.so",RTLD_LAZY);
    if(!handle){
        char *error = dlerror();
        printf("Error: %s\n", error);
        return 1;
    } 

    //Объявляем указатели на функции
    int (*addPtr)(int, int);
    int (*subPtr)(int, int);
    int (*mulPtr)(int, int);
    int (*divPtr)(int, int);

    //Присваиваем функции из библиотеки в указатели
    addPtr = dlsym(handle, "add");
    subPtr = dlsym(handle, "sub");
    mulPtr = dlsym(handle, "mul");
    divPtr = dlsym(handle, "div");

    int choice = 0;
    int a;
    int b;
    int running = 1;

    while (running)
    {
        printf("\n1. Addition\n2. Subtraction\n3. Multiplication\n4. Division\n5. EXIT\n");

        printf("Enter value: "); 
        scanf("%d", &choice);
        switch (choice)
        {
        case 1:
            if (get_input(&a, &b) && addPtr)
                printf("%d + %d = %d\n", a, b, addPtr(a, b));
            break;
        case 2:
            if (get_input(&a, &b) && subPtr)
                printf("%d - %d = %d\n", a, b, subPtr(a, b));
            break;
        case 3:
            if (get_input(&a, &b) && mulPtr)
                printf("%d * %d = %d\n", a, b, mulPtr(a, b));
            break;
        case 4:
            if (get_input(&a, &b) && divPtr)
            {
                //Проверка деления на ноль
                if (b == 0)
                {
                    printf("Cannot divide by zero\n");
                    break;
                }
                printf("%d / %d = %d\n", a, b, divPtr(a, b));
            }
            break;
        case 5:
            running = 0;
            break;
        default:
            printf("Wrong option!\n");
        }
    }

    dlclose(handle);

    return 0;
}