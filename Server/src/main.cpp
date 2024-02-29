#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>

void handleConnection(int clientSocket)
{
    // Receive file size
    size_t fileSize;
    recv(clientSocket, reinterpret_cast<char *>(&fileSize), sizeof(fileSize), 0);

    // Receive file content
    std::vector<char> buffer(4096);
    size_t totalReceived = 0;
    while (totalReceived < fileSize)
    {
        ssize_t received = recv(clientSocket, buffer.data(), buffer.size(), 0);
        if (received <= 0)
        {
            std::cerr << "Error receiving file" << std::endl;
            break;
        }
        totalReceived += received;
        // Process received data as needed
        // ...
    }

    // Close the client socket
    close(clientSocket);
}

int main()
{
    int serverPort = 1234;

    // Create socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    // Set server information
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(serverPort);

    // Bind the socket to the server address
    if (bind(serverSocket, reinterpret_cast<sockaddr *>(&serverAddress), sizeof(serverAddress)) == -1)
    {
        std::cerr << "Failed to bind the socket" << std::endl;
        return 1;
    }

    // Listen for connections
    if (listen(serverSocket, 5) == -1)
    {
        std::cerr << "Failed to listen for connections" << std::endl;
        return 1;
    }

    std::cout << "Server listening on port " << serverPort << std::endl;

    while (true)
    {
        // Accept a client connection
        sockaddr_in clientAddress{};
        socklen_t clientAddressSize = sizeof(clientAddress);

        int clientSocket = accept(serverSocket, reinterpret_cast<sockaddr *>(&clientAddress), &clientAddressSize);
        if (clientSocket == -1)
        {
            std::cerr << "Failed to accept a client connection" << std::endl;
            return 1;
        }

        // Handle the connection in a separate thread
        std::thread connectionThread(handleConnection, clientSocket);
        connectionThread.detach();
    }

    // Close the server socket
    close(serverSocket);

    return 0;
}

// void parser();

// int main(int argc, char **argv)
// {
// }

// void parser()
// {

//     // Parse the console input and write it to an array
// }