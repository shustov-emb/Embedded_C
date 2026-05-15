#16.4.4 Ethernet заголовок

RAW Клиентом отправляем сообщение серверу в ручную поменяв `Ethernet`, `IP` и `UDP` заголовки  (сервер я взял из `homework_16/16_1_AF_FAMILY/AF_INET_SOC_DGRAM`), и RAW сокетом слушаем нужный порт, получаем изменённое сообщенеие от сервера, выводим на экран.

Для тестов разворачивал два докер контейнера. Примерный списко команд:
```bash
# Создаем изолированнуею Docker сеть для связи сервера и клиента
docker network create raw-net

# Запускаем контейнер-сервер (Ubuntu) в сети 'raw-net' с полными правами на сеть (RAW-сокеты) и монтируем код в /app
docker run --name raw-server --network raw-net --cap-add=NET_ADMIN --cap-add=NET_RAW -v /home/user/embedded_c/homework_16/16_4_RAW_Socket/4_Manual_Ethernet:/app -it ubuntu

# Запускаем контейнер-клиент с теми же правами и общим кодом в той же сети для тестов
docker run --name raw-client --network raw-net --cap-add=NET_ADMIN --cap-add=NET_RAW -v /home/user/embedded_c/homework_16/16_4_RAW_Socket/4_Manual_Ethernet:/app -it ubuntu


# raw-client:
# Устанавливаем пакет для получения информации о сети
apt install iproute2
# Узнаём айпи и макс адрес контейнера
ip addr
cd app/bin
# Запускаем после сервера
./cLient

# raw-server:
# Тут делаем тоже самое, и после настроек запускам сервер
apt install iproute2
ip addr
cd app/bin
./server

```

Демонстрация работы. Верхний терминал - клиент, нижний - сервер.
![Демонстрация работы](Ubuntu_24.04.3_ltsc_-_VMware_Workstation_(14.May.2026)(867).png)

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