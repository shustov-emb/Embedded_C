/**
 * @file hw_4.h
 * @author Шустов Александр
 * @brief Заголовочный файл к домашнему заданию №4
 * @version 0.1
 * @date 2025-12-10
 */

#ifndef HOMEWORK_4_H
#define HOMEWORK_4_H
#define ARRAY_SIZE 100 ///< Задаём размер массива содержащую структуру Subscriber
#define STRING_SIZE 12 ///< Задаём размер символьных массивов в паолях структуры Subscriber

/**
 * @brief Структура содержащая данные абоннета
 * 
 */
struct Subscriber
{
    char not_empty; ///< Используется для определения заполнености структуры
    char name[STRING_SIZE]; ///< Имя абонента
    char second_name[STRING_SIZE]; ///< Фамилия абонента
    char phone_number[STRING_SIZE]; ///< Номер телефона абонента
};

/**
 * @brief Вспомогательная функция которая затирает массив символов нулями
 * 
 * @param[in,out] char_array Указатель на массив символов который надо обнулить
 */
void EraseCharArray(char *char_array);

/**
 * @brief Вспомогательная функция которая копирует массив символов из src в dst
 * 
 * @param[in,out] src Массив источника
 * @param[in,out] dst Массив получателя
 */
void CopyArray(char *src, char *dst);

/**
 * @brief Вспомогательна функция которая затирает структуру нулями
 * 
 * @param[in,out] subscriber Указатель на структуру которую надо обнулить
 */
void EraseStructure(struct Subscriber *subscriber);

/**
 * @brief Вспомогательная функция которая копирует структуру src в dst
 * 
 * @param[in,out] src Структура источника
 * @param[in,out] dst Структура получателя
 */
void CopyStructure(struct Subscriber *src, struct Subscriber *dst);

/**
 * @brief Вспомогательна функция которая позволяет получить и обработать вводимую пользователем строку (массив символов)
 * 
 * @param[in] message Строка сообщение для информирования пользователя о том какие данные требуются
 * @param[out] buffer Указатель на строку в которую мы получаем вводимые данные пользователя
 */
void GetString(const char *message, char *buffer);

/**
 * @brief Вспомогательная функция сравнивающая два массива символов
 * 
 * @param[in] str1 Первая строка для сравнения
 * @param[in] str2 Вторая строка для сравнения
 * @return возвращает 1 если строки одинаковы, и 0 если различаются
 */
int  StrCompare(char *str1, char *str2);

/**
 * @brief Функция добавляющая абонента в массив с абонентами
 * 
 * @param[in,out] array Массив содержащий список абонентов
 */
void AddSubscriber(struct Subscriber array[]);

/**
 * @brief Функция удаляющая абонента с указанным номером из массива с абонентами
 * 
 * @param[in,out] array Массив содержащий список абонентов
 */
void DeleteSubscriber(struct Subscriber array[]);

/**
 * @brief Выводит спиок непустых абонентов, если указан параметр name_ot_find - выводит абонентов схожих с указаной строкой
 * 
 * @param[in] array Массив содержащий список абонентов в котором производится поиск
 * @param[in] name_ot_find Параметр по которому осуществляется поиск абннента по имени
 */
void PrintSubscribers(struct Subscriber array[], char *name_ot_find);

#endif