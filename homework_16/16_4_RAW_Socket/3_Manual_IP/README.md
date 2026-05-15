#16.4.3 IP Заголовок

RAW Клиентом отправляем сообщение серверу в ручную поменяв `IP` и `UDP` заголовки  (сервер я взял из `homework_16/16_1_AF_FAMILY/AF_INET_SOC_DGRAM`), и RAW сокетом слушаем нужный порт, получаем изменённое сообщенеие от сервера, выводим на экран.

>Совет! 
>Запускать сначала `make server` а потом `make client`.<br>
>Клиента запускать через `sudo`<br>

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