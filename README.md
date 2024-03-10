# Description

A Tcp Server-Client aplication that provides functionality of sending files one-by-one from client to server, where they are processed via thread-safe system.

Server accepts multiple clients and provides a multithreaded processor.

Maximum number of threads and some other parametrs can be preconfigured at the start of the server.

# Building

First, clone repo and cd in it via

```sh 
git clone https://github.com/RoundRonin/TCP_Server_Client.git
cd TCP_Server_Client
```

To build this app use (at the root directory):

```sh 
make build
```

If you want to rebuild the app use:

```sh 
make rebuild
```

To clean upp all the building use:

```sh 
make clean
```

Building process also generates symlinks to the executables at the project's root, so you don't need to manage them.

# Usage

After building the app you can start the server and specify some parameters.

## Server

To start the server use (at the root of the project):

```sh 
./Tcp_Server
```

after running this command you will get a prompt of its usage:

```sh 
Usage: ./Tcp_Server <configFile>
or: ./Tcp_Server -t <maxThreads> -p <port> -s <maxFileSize> -f <savePath>
```

As stated in the usage prompt, you can either supply a config file

Config is a Json file that supplies parametrs. You can supply only paramets you want to change. Config file structure:

|parametr|description|default|
|---|---|---|
|maxThreads|maximum amount of simultaneous threads that can process client's requests|4|
|port|port of the server|1234|
|maxFileSize|maximum size of file that the server will process, dedined in kilobytes (KB). Server will reject every attempt to send a bigger file|8096|
|savePath|location to safe file|./Output|

Config example:

```json
{
    "maxThreads": 10,
    "maxFileSize": 10000,
    "savePath": "~/TcpServerFiles"
}
```

or use flags. You don't have to supply all the flags, it is sufficent to send only ones you want to change, then for other settings hardcoded defaults (defined at main.cpp) will be provided. Flag description:

|flag|description|default|
|---|---|---|
|-t|maximum amount of simultaneous threads that can process client's requests|4|
|-p|port of the server|1234|
|-s|maximum size of file that the server will process, dedined in kilobytes (KB). Server will reject every attempt to send a bigger file|8096|
|-f|location to safe file|./Output|

## Client

To send a file use (at the root of the project):

```sh 
./Tcp_Client
```

Client's prompt is

```sh 
Usage: ./Tcp_Client <filename> -i <ip_address> -p <port>
```

Filename should be supplied at all times. Flag rules are the same as with the server. You can provide any amount of flags, defailts are hardcoded (in client's main.cpp). Flag description:

|flag|description|default|
|---|---|---|
|-i|ip adress of the server|127.0.0.1|
|-p|server port|1234|


