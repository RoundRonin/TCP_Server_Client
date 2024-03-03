#include <iostream>
#include <fstream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <filesystem>

namespace fs = std::filesystem;

enum ERRORCODE
{
    SUCCESS,
    ERROR,
};

struct settings
{
    std::string filename;
    std::string serverIP;
    int port;
    ERRORCODE ERROR;

    void print()
    {
        std::cout << "Filename: " << filename << std::endl;
        std::cout << "IP Address: " << serverIP << std::endl;
        std::cout << "Port: " << port << std::endl;
    }
};

struct connection
{
    int serverSocket;
    ERRORCODE ERROR;
};

void sendFile(const std::string &filename, int serverSocket);
settings parseSettings(int argc, char *argv[], settings defaultSettings);
connection openConnection(settings);

int main(int argc, char *argv[])
{
    settings defaultSettings = {
        .filename = "Default.md",
        .serverIP = "127.0.0.1",
        .port = 1234,
        .ERROR = ERRORCODE::ERROR,
    };

    settings sets = parseSettings(argc, argv, defaultSettings);
    if (sets.ERROR != ERRORCODE::SUCCESS)
        return 1;
    sets.print();

    connection conn = openConnection(sets);
    if (conn.ERROR != ERRORCODE::SUCCESS)
        return 1;

    sendFile(sets.filename, conn.serverSocket);
    close(conn.serverSocket);

    return 0;
}

connection openConnection(settings settings)
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        std::cerr << "Error creating socket" << std::endl;
        return {serverSocket, ERRORCODE::ERROR};
    }

    // Set server information
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(settings.port);

    if (inet_pton(AF_INET, settings.serverIP.c_str(), &(serverAddress.sin_addr)) <= 0)
    {
        std::cerr << "Invalid server address" << std::endl;
        return {serverSocket, ERRORCODE::ERROR};
    }

    // Connect to the server
    if (connect(serverSocket, reinterpret_cast<sockaddr *>(&serverAddress), sizeof(serverAddress)) == -1)
    {
        std::cerr << "Failed to connect to the server" << std::endl;
        return {serverSocket, ERRORCODE::ERROR};
    }

    return {serverSocket, ERRORCODE::SUCCESS};
}

settings parseSettings(int argc, char *argv[], settings defaultSettings)
{
    if (argc < 2 || argc > 6)
    {
        std::cout << "Usage: ./Tcp_Client <filename> -i <ip_address> -p <port>" << std::endl;
        return defaultSettings;
    }

    settings undefined = {
        .filename = "",
        .serverIP = "",
        .port = -1,
        .ERROR = ERRORCODE::ERROR,
    };

    settings out = {
        .filename = argv[1],
        .serverIP = undefined.serverIP,
        .port = undefined.port,
        .ERROR = undefined.ERROR,
    };

    if (argc > 2)
    {
        // loop doesn't check the last arg to avoid segfault if only the flag is passed
        for (int i = 1; i < argc - 1; i++)
        {
            if (std::string(argv[i]) == "-i")
            {
                int arg_number = i + 1;
                if (argv[arg_number][0] != '-')
                    out.serverIP = argv[arg_number];
            }
            else if (std::string(argv[i]) == "-p")
            {
                int arg_number = i + 1;
                if (argv[arg_number][0] != '-')
                    out.port = atoi(argv[arg_number]);
            }
        }

        if ((out.serverIP == undefined.serverIP && out.port == undefined.port) || argc % 2 == 1)
        {
            std::cout << "Use -i to indicate <ip_adress>\nUse -p to indicate <port>" << std::endl;
            return defaultSettings;
        }
    }

    if (out.serverIP == undefined.serverIP)
        out.serverIP = defaultSettings.serverIP;
    if (out.port == undefined.port)
        out.port = defaultSettings.port;

    out.ERROR = ERRORCODE::SUCCESS;
    return out;
}

void sendFile(const std::string &filename, int serverSocket)
{
    std::string extractedFilename = fs::path(filename).filename();
    size_t filenameSize = extractedFilename.size();

    std::ifstream file(filename, std::ios::binary);
    if (!file)
    {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    // Get file size
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    send(serverSocket, reinterpret_cast<const char *>(&fileSize), sizeof(fileSize), 0);
    send(serverSocket, reinterpret_cast<const char *>(&filenameSize), sizeof(filenameSize), 0);
    send(serverSocket, extractedFilename.c_str(), extractedFilename.size(), 0);

    // Send file content
    char buffer[4096];
    while (file)
    {
        file.read(buffer, sizeof(buffer));
        send(serverSocket, buffer, file.gcount(), 0);
    }
    int status = -2;
    recv(serverSocket, reinterpret_cast<char *>(&status), sizeof(status), 0);
    int additional_info;
    recv(serverSocket, reinterpret_cast<char *>(&additional_info), sizeof(additional_info), 0);

    switch (status)
    {
    case -1:
        std::cout << "[ERROR]" << std::endl;
        std::cout << "  Failed to send file, the file is too big!" << std::endl;
        std::cout << "    Appropriate file size is " << additional_info << " KB" << std::endl;
        std::cout << "    While you've tried sending " << (int)fileSize / 1024 << " KB!" << std::endl;
        break;

    default:
        std::cout << "[INFO]" << std::endl;
        std::cout << "  Succesful file sending" << std::endl;
        break;
    }

    file.close();
}
