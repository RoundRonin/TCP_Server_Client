#include <iostream>
#include <fstream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

void sendFile(const std::string &filename, int serverSocket)
{
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

    // Send file size
    send(serverSocket, reinterpret_cast<const char *>(&fileSize), sizeof(fileSize), 0);

    // Send file content
    char buffer[4096];
    while (file)
    {
        file.read(buffer, sizeof(buffer));
        send(serverSocket, buffer, file.gcount(), 0);
    }

    file.close();
}

int main()
{
    std::string serverIP = "127.0.0.1";
    int serverPort = 1234;

    // Create socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1)
    {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    // Set server information
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);

    if (inet_pton(AF_INET, serverIP.c_str(), &(serverAddress.sin_addr)) <= 0)
    {
        std::cerr << "Invalid server address" << std::endl;
        return 1;
    }

    // Connect to the server
    if (connect(clientSocket, reinterpret_cast<sockaddr *>(&serverAddress), sizeof(serverAddress)) == -1)
    {
        std::cerr << "Failed to connect to the server" << std::endl;
        return 1;
    }

    // Send file to server
    std::string filename;
    std::cout << "Enter file name: ";
    std::cin >> filename;

    sendFile(filename, clientSocket);

    // Close the socket
    close(clientSocket);

    return 0;
}