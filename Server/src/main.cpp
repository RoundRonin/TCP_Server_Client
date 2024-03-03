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

namespace fs = std::filesystem;

enum ERRORCODE
{
    SUCCESS,
    ERROR,
    INVALID_FILE,
};

struct settingsServer
{
    int maxThreads;
    int port;
    int maxFileSize;
    std::string savePath;
    ERRORCODE ERROR;

    void print()
    {
        std::cout << "Max threads: " << maxThreads << std::endl;
        std::cout << "Port: " << port << std::endl;
        std::cout << "Max file size: " << maxFileSize << " KB" << std::endl;
        std::cout << "Save path: " << savePath << std::endl;
    }
};

struct connection
{
    int serverSocket;
    ERRORCODE ERROR;
};

connection startServer(settingsServer settings);
settingsServer parseSettings(int argc, char *argv[],
                             settingsServer defaultSettings);
void handleConnection(int clientSocket, settingsServer settings);
void signalHandler(int signalNumber);
fs::path handleFileName(int clientSocket, settingsServer settings);

int main(int argc, char *argv[])
{
    bool terminateServer = false;

    std::signal(SIGTERM, signalHandler);
    std::signal(SIGHUP, signalHandler);

    settingsServer defaultSettings = {
        .maxThreads = 4,
        .port = 1234,
        .maxFileSize = 8096,
        .savePath = "./Output",
        .ERROR = ERRORCODE::ERROR,
    };

    settingsServer sets = parseSettings(argc, argv, defaultSettings);
    if (sets.ERROR != ERRORCODE::SUCCESS)
    {
        std::cout << "Usage: ./Tcp_Server <configFile>" << std::endl
                  << "or: ./Tcp_Server -t <maxThreads> -p <port> -s <maxFileSize> "
                     "-f <savePath>";
        return 1;
    }
    sets.print();

    connection connection = startServer(sets);
    if (connection.ERROR != ERRORCODE::SUCCESS)
        return 1;

    while (true)
    {
        // Accept a client connection
        sockaddr_in clientAddress{};
        socklen_t clientAddressSize = sizeof(clientAddress);

        int clientSocket = accept(connection.serverSocket,
                                  reinterpret_cast<sockaddr *>(&clientAddress),
                                  &clientAddressSize);
        if (clientSocket == -1)
        {
            std::cerr << "Failed to accept a client connection" << std::endl;
            return 1;
        }

        // Handle the connection in a separate thread
        std::thread connectionThread(handleConnection, clientSocket, sets);
        connectionThread.detach();

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    close(connection.serverSocket);

    return 0;
}

void handleConnection(int clientSocket, settingsServer settings)
{
    // Receive file size, bytes
    size_t fileSize;
    recv(clientSocket, reinterpret_cast<char *>(&fileSize), sizeof(fileSize), 0);

    int status = -1;

    if ((int)fileSize / 1024 > settings.maxFileSize)
    {
        std::cout << "[ERROR]" << std::endl;
        std::cout << "  File exceeds max size limit: " << std::endl;
        std::cout << "    Maxixum size is " << settings.maxFileSize << " KB" << std::endl;
        std::cout << "    Client tried sending " << (int)fileSize / 1024 << " KB" << std::endl;

        status = -1;
    }
    else
    {
        fs::path outputFile = handleFileName(clientSocket, settings);

        char buffer[4096];
        size_t totalReceived = 0;
        std::ofstream outputFileStream(outputFile.string(), std::ios::binary); // Open the output file in binary mode

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

            std::cout << totalReceived << std::endl;
        }

        std::cout << "[INFO]" << std::endl;
        std::cout << "  Successfully received file" << std::endl;

        status = 0;
    }

    send(clientSocket, reinterpret_cast<const char *>(&status), sizeof(status), 0);
    send(clientSocket, reinterpret_cast<const char *>(&settings.maxFileSize), sizeof(settings.maxFileSize), 0);

    close(clientSocket);
}

fs::path handleFileName(int clientSocket, settingsServer settings)
{
    // Receive file name length
    size_t fileNameLength;
    recv(clientSocket, reinterpret_cast<char *>(&fileNameLength), sizeof(fileNameLength), 0);

    std::cout << fileNameLength << std::endl;

    // Receive file name
    char fileNameBuffer[fileNameLength];
    recv(clientSocket, fileNameBuffer, sizeof(fileNameBuffer), 0);
    std::string fileName(fileNameBuffer, fileNameLength);

    // Append the received file name to the savePath for the output file
    fs::path savePathWithFileName = fs::path(settings.savePath) / fileName;

    // Check if the output directory exists, create it if it doesn't
    fs::path outputDirectory = savePathWithFileName.parent_path();
    if (!fs::exists(outputDirectory))
    {
        fs::create_directories(outputDirectory);
    }

    // Check if the output file exists, create a new file name if it does
    fs::path outputFile = savePathWithFileName;
    int fileCounter = 1;
    while (fs::exists(outputFile))
    {
        std::string newFileName = fileName.substr(0, fileName.find_last_of('.')) + '(' + std::to_string(fileCounter) + ')' +
                                  fileName.substr(fileName.find_last_of('.'));
        outputFile = fs::path(settings.savePath) / newFileName;
        fileCounter++;
    }

    return outputFile;
}

connection startServer(settingsServer settings)
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        std::cerr << "Error creating socket" << std::endl;
        return {serverSocket, ERRORCODE::ERROR};
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(settings.port);

    if (bind(serverSocket, reinterpret_cast<sockaddr *>(&serverAddress),
             sizeof(serverAddress)) == -1)
    {
        std::cerr << "Failed to bind the socket" << std::endl;
        return {serverSocket, ERRORCODE::ERROR};
    }

    // Listen for connections
    if (listen(serverSocket, settings.maxThreads) == -1)
    {
        std::cerr << "Failed to listen for connections" << std::endl;
        return {serverSocket, ERRORCODE::ERROR};
    }

    std::cout << "Server listening on port " << settings.port << std::endl;

    return {serverSocket, ERRORCODE::SUCCESS};
}

settingsServer parseSettings(int argc, char *argv[],
                             settingsServer defaultSettings)
{
    if (argc < 2 || argc > 9)
    {
        return defaultSettings;
    }

    settingsServer undefined = {
        .maxThreads = -1,
        .port = -1,
        .maxFileSize = -1,
        .savePath = "",
        .ERROR = ERRORCODE::ERROR,
    };

    settingsServer out = {
        .maxThreads = undefined.maxThreads,
        .port = undefined.port,
        .maxFileSize = undefined.maxFileSize,
        .savePath = undefined.savePath,
        .ERROR = undefined.ERROR,
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
        {
            out.ERROR = ERRORCODE::ERROR;
            return out;
        }
    }
    else
    {
        // Read the file
        const std::string configPath = argv[1];

        std::ifstream file(configPath);
        if (!file.is_open())
        {
            out.ERROR = ERRORCODE::INVALID_FILE;
            return out;
        }

        Json::Value root;
        file >> root;

        if (root.empty())
        {
            return defaultSettings;
        }

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
    {
        out.ERROR = ERRORCODE::ERROR;
        return defaultSettings;
    }

    if (out.maxThreads == undefined.maxThreads)
        out.maxThreads = defaultSettings.maxThreads;
    if (out.port == undefined.port)
        out.port = defaultSettings.port;
    if (out.maxFileSize == undefined.maxFileSize)
        out.maxFileSize = defaultSettings.maxFileSize;
    if (out.savePath == undefined.savePath)
        out.savePath = defaultSettings.savePath;

    out.ERROR = ERRORCODE::SUCCESS;
    return out;
}

void signalHandler(int signalNumber)
{
    if (signalNumber == SIGTERM || signalNumber == SIGHUP)
    {
        std::cout << "Recieved interupt signal: " << signalNumber << "\nExiting..." << std::endl;
        exit(signalNumber);
    }
}