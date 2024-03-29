# Описание

Tcp Server-Client -- приложение, предоставляющее функцию отправки файлов по одному с клиента на сервер, где происходит их обработка с помощью многопоточной системыю

Сервер способен принять несколько клиентов и предоставить многопоточный обработчик.

Максимальное число потоков и некоторые другие параметры могут быть настроены при старте сервера.

# Построение

Для начала скопируйте репозиторий и переместитесь в него с помощью

```sh 
git clone https://github.com/RoundRonin/TCP_Server_Client.git
cd TCP_Server_Client
```

Чтобы построить приложение используйте (находясь в корне проекта):

```sh 
make build
```

Если вы хотите перестроить приложение используйте:

```sh 
make rebuild
```

Чтобы удалить результат построения используйте:

```sh 
make clean
```

Построение также генерирует симлинки к исполняемым файлам в корне проекта, поэтому вам не придется заниматься их перемещением.

# Использование

После построения приложения вы можете запустить сервер с определением некоторых параметров.

After building the app you can start the server and specify some parameters.

## Server

Чтобы запустить сервер используйте (в корне проекта):

```sh 
./Tcp_Server
```

после исполненяи этой комманды вы получите шаблон ее использования:

```sh
Usage: ./Tcp_Server <configFile>
or: ./Tcp_Server -t <maxThreads> -p <port> -s <maxFileSize> -f <savePath>
```

Как указано в шаблоне, вы можете либо предоставить конфигурационный файл:

Конфиг -- Json файл, который предоставляет параметры. Достаточно предложить только параметры, которые выхотите изменить. Структура файла конфигурации:

|параметр|опсиание|по-умолчанию|
|---|---|---|
|maxThreads|максимальное количество одновременных потоков, которые могут обрабатывать запросы клиентов|4|
|port|порт сервера|1234|
|maxFileSize|максимальный размер файла, который сервер обработает, определяется в килобайтах (КБ). Сервер откажет каждой попытке отправить файл большего размера|8096|
|savePath|локация сохранения файла|./Output|

Пример конфигурационного файла:

```json
{
    "maxThreads": 10,
    "maxFileSize": 10000,
    "savePath": "~/TcpServerFiles"
}
```

или вы можете использовать флаги. Вам нет необходимости подавать все флаги, достаточно отправить только изменяющие значения, которые вы хотите переопределить, тогда для остальных настроек будут использованы описанные в коде настройки (определены в main.cpp сервера). Описание флагов:

|параметр|опсиание|по-умолчанию|
|---|---|---|
|-t|максимальное количество одновременных потоков, которые могут обрабатывать запросы клиентов|4|
|-p|порт сервера|1234|
|-s|максимальный размер файла, который сервер обработает, определяется в килобайтах (КБ). Сервер откажет каждой попытке отправить файл большего размера|8096|
|-f|локация сохранения файла|./Output|

## Client

Чтобы отправить файл используйте (находясь в корне проекта):

```sh 
./Tcp_Client
```

Шаблон клиента:

```sh 
Usage: ./Tcp_Client <filename> -i <ip_address> -p <port>
```

Имя файла должны быть предоставлено при каждом использовании клиента. Правила использования флагов аналогичны серверным. Вы можете подать любое количество флаговю Стандартные настройки определны в коде (в main.cpp клиента). Описание флагов:

|флаг|описание|по-умолчанию|
|---|---|---|
|-i|ip адрес сервера|127.0.0.1|
|-p|порт сервера|1234|


