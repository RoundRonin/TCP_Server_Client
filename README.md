# Description

A Tcp Server-Client aaplication that provides the functionality of sending files one-by-one from client to server, where they are processed via thread-safe system.

Server accepts multiple clients and provides a multithreaded processor.

Maximum number of threads and some other parametrs can be preconfigured at the start of the server.

# Building

To build this app use

```sh 
make build
```

at the root directory.

If you want to rebuild the app use

```sh 
make rebuild
```

To clean upp all the building use

```sh 
make clean
```

Building process also generates symlinks to the executables at the project's root, so you don't need to manage them.

# Usage

After building the app you can start the server and specify some parameters.

To start the server use 

```sh 
./Tcp_Server
```

at the root of the project

after running this command you will get a prompt of its usage

To send a file use

```sh 
./Tcp_Client
```

at the root of the project.
