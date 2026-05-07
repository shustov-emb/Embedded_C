# 16.3.1 Broadcast

Реализовать программу:
1. Сервер - посылающий `broadcast` рассылку, с некоторым сообщением
2. Клиент - принимает сообщение из рассылки

> **Совет!** 
<br>Сначала запустить клиент через `make client`

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