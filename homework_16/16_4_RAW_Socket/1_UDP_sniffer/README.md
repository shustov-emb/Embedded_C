#16_4.1 RAW Socket 

Простой снифер upd пакетов на raw socket

>Совет! 
>Запускать программу через `make client`.<br>
>Во втором терминале можно воспользоваться командой: `echo "Some data" | nc -u 127.0.0.1 777` для проверки основной программы

---

Для управления проектом используйте следующие команды `make`:

*   **`make`**                      — Сборка проекта. Компилирует объектные файлы и собирает исполняемый файл `run`.
*   **`make run`**                  — Компиляция и автоматический запуск c server.
*   **`make client/server`**        — Компиляция и автоматический запуск програму client/server. 
    (пример `make server`)
*   **`make valgrind prog=client/server`** — Запускает программу client/server с valgrind 
    (пример `make valgrind prog=client`).
*   **`make helgrind prog=client/server`** — Запускает программу client/server с helgrind. 
    (пример `make helgrind prog=server`)
*   **`make clean`**    — Очистка: удаляет директории `obj/`, `bin/` вместе с исполняемым файлом `bin/run`

## Структура проекта

*   `src/server.c` — Серверная часть.
*   `src/client.c` — Клиентская часть.
*   `obj/`         — Временные объектные файлы.
*   `bin/`         — Исполняемые файлы `server` и `client`.