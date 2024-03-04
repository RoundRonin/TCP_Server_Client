
#pragma once

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include <filesystem>
#include <fstream>
#include <json/json.h>
#include <string>

#include <csignal>
#include <any>
#include <mutex>
#include <condition_variable>
#include <queue>

enum ERRORCODE
{
    SUCCESS,
    ERROR,
    INVALID_FILE,
};

struct settings
{
    int maxThreads;
    int port;
    int maxFileSize;
    std::string savePath;

    void print()
    {
        std::cout << "Max threads: " << maxThreads << std::endl;
        std::cout << "Port: " << port << std::endl;
        std::cout << "Max file size: " << maxFileSize << " KB" << std::endl;
        std::cout << "Save path: " << savePath << std::endl;
    }
};

class TcpServer
{

private:
    settings undefined;
    settings defaultSettings;
    settings currentSettings;

    int serverSocket;
    bool exit;

public:
    TcpServer()
    {
        defaultSettings = currentSettings = undefined = {
            .maxThreads = -1,
            .port = -1,
            .maxFileSize = -1,
            .savePath = "",
        };

        serverSocket = -1;
    }
    TcpServer(settings defaultSettings) : TcpServer() { this->defaultSettings = defaultSettings; }

    ERRORCODE acceptConnections()
    {
        std::vector<std::thread> threadPool;
        for (int i = 0; i < currentSettings.maxThreads; ++i)
        {
            threadPool.emplace_back(&TcpServer::workerThread, this);
        }

        while (!exit)
        {
            // Accept a client connection
            sockaddr_in clientAddress{};
            socklen_t clientAddressSize = sizeof(clientAddress);

            int clientSocket = accept(serverSocket,
                                      reinterpret_cast<sockaddr *>(&clientAddress),
                                      &clientAddressSize);
            if (clientSocket == -1)
            {
                std::cerr << "Failed to accept a client connection" << std::endl;
                return ERRORCODE::ERROR;
            }
            else
            {
                // Add the client socket to the work queue
                std::unique_lock<std::mutex> lock(workQueueMutex);
                workQueue.push(clientSocket);
                workQueueCondition.notify_one();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        for (std::thread &thread : threadPool)
        {
            thread.join();
        }

        return ERRORCODE::SUCCESS;
    }

    ERRORCODE startServer()
    {
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == -1)
        {
            std::cerr << "Error creating socket" << std::endl;
            return ERRORCODE::ERROR;
        }

        sockaddr_in serverAddress{};
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = INADDR_ANY;
        serverAddress.sin_port = htons(currentSettings.port);

        if (bind(serverSocket, reinterpret_cast<sockaddr *>(&serverAddress),
                 sizeof(serverAddress)) == -1)
        {
            std::cerr << "Failed to bind the socket" << std::endl;
            return ERRORCODE::ERROR;
        }

        if (listen(serverSocket, currentSettings.maxThreads) == -1)
        {
            std::cerr << "Failed to listen for connections" << std::endl;
            return ERRORCODE::ERROR;
        }

        std::cout << "Server listening on port " << currentSettings.port << std::endl;

        return ERRORCODE::SUCCESS;
    }

    void stopServer()
    {
        close(serverSocket);
        exit = true;
    }

    ERRORCODE parseSettings(int argc, char *argv[])
    {
        if (argc < 2 || argc > 9)
            return ERRORCODE::ERROR;

        settings out = {
            .maxThreads = undefined.maxThreads,
            .port = undefined.port,
            .maxFileSize = undefined.maxFileSize,
            .savePath = undefined.savePath,
        };

        if (argv[1][0] == '-')
        {
            // loop doesn't check the last arg to avoid segfault if only the flag is
            // passed
            for (int i = 1; i < argc - 1; i++)
            {

                if (std::string(argv[i]) == "-t")
                {
                    int arg_number = i + 1;
                    if (argv[arg_number][0] != '-')
                        out.maxThreads = atoi(argv[arg_number]);
                }
                else if (std::string(argv[i]) == "-p")
                {
                    int arg_number = i + 1;
                    if (argv[arg_number][0] != '-')
                        out.port = atoi(argv[arg_number]);
                }
                else if (std::string(argv[i]) == "-s")
                {
                    int arg_number = i + 1;
                    if (argv[arg_number][0] != '-')
                        out.maxFileSize = atoi(argv[arg_number]);
                }
                else if (std::string(argv[i]) == "-f")
                {
                    int arg_number = i + 1;
                    if (argv[arg_number][0] != '-')
                        out.savePath = argv[arg_number];
                }
            }

            if (argc % 2 == 0)
                return ERRORCODE::ERROR;
        }
        else
        {
            // Read the file
            const std::string configPath = argv[1];

            std::ifstream file(configPath);
            if (!file.is_open())
                return ERRORCODE::INVALID_FILE;

            Json::Value root;
            file >> root;

            if (root.empty())
                return ERRORCODE::ERROR;

            if (root.isMember("maxThreads") && root["maxThreads"].isInt())
                out.maxThreads = root["maxThreads"].asInt();
            if (root.isMember("port") && root["port"].isInt())
                out.port = root["port"].asInt();
            if (root.isMember("maxFileSize") && root["maxFileSize"].isInt())
                out.maxFileSize = root["maxFileSize"].asInt();
            if (root.isMember("savePath") && root["savePath"].isString())
                out.savePath = root["savePath"].asString();
        }

        if (out.maxThreads == undefined.maxThreads && out.port == undefined.port &&
            out.maxFileSize == undefined.maxFileSize &&
            out.savePath == undefined.savePath)
            return ERRORCODE::ERROR;

        if (out.maxThreads == undefined.maxThreads)
            out.maxThreads = defaultSettings.maxThreads;
        if (out.port == undefined.port)
            out.port = defaultSettings.port;
        if (out.maxFileSize == undefined.maxFileSize)
            out.maxFileSize = defaultSettings.maxFileSize;
        if (out.savePath == undefined.savePath)
            out.savePath = defaultSettings.savePath;

        currentSettings = out;

        return ERRORCODE::SUCCESS;
    }

    void printSettings()
    {
        currentSettings.print();
    }

private:
    std::mutex workQueueMutex;
    std::condition_variable workQueueCondition;
    std::queue<int> workQueue;

    // Worker thread function
    void workerThread()
    {
        while (true)
        {
            std::unique_lock<std::mutex> lock(workQueueMutex);
            workQueueCondition.wait(lock, [&]()
                                    { return !workQueue.empty() || exit; });

            if (exit)
            {
                break; // Exit the worker thread
            }

            int clientSocket = workQueue.front();
            workQueue.pop();
            lock.unlock();

            // Handle the connection in a separate thread
            // std::thread connectionThread(&TcpServer::handleConnection, this, clientSocket);
            // connectionThread.detach();
            handleConnection(clientSocket);
        }
    }

    void handleConnection(int clientSocket)
    {
        // std::this_thread::sleep_for(std::chrono::milliseconds(4000));
        // Receive file size, bytes
        size_t fileSize;
        recv(clientSocket, reinterpret_cast<char *>(&fileSize), sizeof(fileSize), 0);

        int status = -1;

        if ((int)fileSize / 1024 > currentSettings.maxFileSize)
        {
            std::cout << "[ERROR]" << std::endl;
            std::cout << "  File exceeds max size limit: " << std::endl;
            std::cout << "    Maxixum size is " << currentSettings.maxFileSize << " KB" << std::endl;
            std::cout << "    Client tried sending " << (int)fileSize / 1024 << " KB" << std::endl;

            status = -1;
        }
        else
        {
            std::filesystem::path outputFile = handleFileName(clientSocket);

            char buffer[4096];
            size_t totalReceived = 0;
            std::ofstream outputFileStream(outputFile.string(), std::ios::binary);

            while (totalReceived < fileSize)
            {
                ssize_t received = recv(clientSocket, buffer, sizeof(buffer), 0);
                if (received <= 0)
                {
                    std::cerr << "Error receiving file" << std::endl;
                    break;
                }
                totalReceived += received;
                outputFileStream.write(buffer, received);
            }

            std::cout << "[INFO]" << std::endl;
            std::cout << "  Successfully received file" << std::endl;

            status = 0;
        }

        send(clientSocket, reinterpret_cast<const char *>(&status), sizeof(status), 0);
        send(clientSocket, reinterpret_cast<const char *>(&currentSettings.maxFileSize), sizeof(currentSettings.maxFileSize), 0);

        close(clientSocket);
    }

    std::filesystem::path handleFileName(int clientSocket)
    {
        size_t fileNameLength;
        recv(clientSocket, reinterpret_cast<char *>(&fileNameLength), sizeof(fileNameLength), 0);

        std::cout << fileNameLength << std::endl;

        char fileNameBuffer[fileNameLength];
        recv(clientSocket, fileNameBuffer, sizeof(fileNameBuffer), 0);
        std::string fileName(fileNameBuffer, fileNameLength);

        std::filesystem::path savePathWithFileName = std::filesystem::path(currentSettings.savePath) / fileName;

        std::filesystem::path outputDirectory = savePathWithFileName.parent_path();
        if (!std::filesystem::exists(outputDirectory))
        {
            std::filesystem::create_directories(outputDirectory);
        }

        std::filesystem::path outputFile = savePathWithFileName;
        int fileCounter = 1;
        while (std::filesystem::exists(outputFile))
        {
            std::string newFileName = fileName.substr(0, fileName.find_last_of('.')) + '(' + std::to_string(fileCounter) + ')' +
                                      fileName.substr(fileName.find_last_of('.'));
            outputFile = std::filesystem::path(currentSettings.savePath) / newFileName;
            fileCounter++;
        }

        return outputFile;
    }
};
