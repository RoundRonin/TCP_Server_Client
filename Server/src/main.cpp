#include <arpa/inet.h> #include <cstdlib>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

// Максимальное количество потоков, порт, максимальный размер файла, а также
// путь для сохранения файлов должны указываться в аргументах запуска сервера
// или конфигурационном файле

enum ERRORCODE {
  SUCCESS,
  ERROR,
};

struct settingsServer {
  int maxThreads;
  int port;
  int maxFileSize;
  std::string savePath;
  ERRORCODE ERROR;

  void print() {
    std::cout << "Max threads: " << maxThreads << std::endl;
    std::cout << "Port: " << port << std::endl;
    std::cout << "Max file size: " << maxFileSize << std::endl;
    std::cout << "Save path: " << savePath << std::endl;
  }
};

struct connection {
  int serverSocket;
  ERRORCODE ERROR;
};

settingsServer parseSettings(int argc, char *argv[],
                             settingsServer defaultSettings);
void handleConnection(int clientSocket);

int main(int argc, char *argv[]) {

  settingsServer defaultSettings = {
      .maxThreads = 4,
      .port = 1234,
      .maxFileSize = 8096,
      .savePath = "./",
      .ERROR = ERRORCODE::ERROR,
  };

  settingsServer sets = parseSettings(argc, argv, defaultSettings);
  if (sets.ERROR != ERRORCODE::SUCCESS)
    return 1;
  sets.print();
  // Create socket
  int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket == -1) {
    std::cerr << "Error creating socket" << std::endl;
    return 1;
  }

  // Set server information
  sockaddr_in serverAddress{};
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = INADDR_ANY;
  serverAddress.sin_port = htons(serverPort);

  // Bind the socket to the server address
  if (bind(serverSocket, reinterpret_cast<sockaddr *>(&serverAddress),
           sizeof(serverAddress)) == -1) {
    std::cerr << "Failed to bind the socket" << std::endl;
    return 1;
  }

  // Listen for connections
  if (listen(serverSocket, 5) == -1) {
    std::cerr << "Failed to listen for connections" << std::endl;
    return 1;
  }

  std::cout << "Server listening on port " << serverPort << std::endl;

  while (true) {
    // Accept a client connection
    sockaddr_in clientAddress{};
    socklen_t clientAddressSize = sizeof(clientAddress);

    int clientSocket =
        accept(serverSocket, reinterpret_cast<sockaddr *>(&clientAddress),
               &clientAddressSize);
    if (clientSocket == -1) {
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

settingsServer parseSettings(int argc, char *argv[],
                             settingsServer defaultSettings) {
  if (argc < 2 || argc > 9) {
    std::cout << "Usage: ./Tcp_Server <configFile>" << std::endl
              << "or: ./Tcp_Server -t <maxThreads> -p <port> -s <maxFileSize> "
                 "-f <savePath>";
    return defaultSettings;
  }

  settingsServer undefined = {
      .maxThreads = -1,
      .port = -1,
      .maxFileSize = -1,
      .savePath = "./",
      .ERROR = ERRORCODE::ERROR,
  };

  settingsServer out = {
      .maxThreads = undefined.maxThreads,
      .port = undefined.port,
      .maxFileSize = undefined.maxFileSize,
      .savePath = undefined.savePath,
      .ERROR = undefined.ERROR,
  };

  if (argv[1][0] == '-') {
    // loop doesn't check the last arg to avoid segfault if only the flag is
    // passed
    for (int i = 1; i < argc - 1; i++) {
      if (std::string(argv[i]) == "-t") {
        int arg_number = i + 1;
        if (argv[arg_number][0] != '-')
          out.maxThreads = atoi(argv[arg_number]);
      } else if (std::string(argv[i]) == "-p") {
        int arg_number = i + 1;
        if (argv[arg_number][0] != '-')
          out.port = atoi(argv[arg_number]);
      } else if (std::string(argv[i]) == "-s") {
        int arg_number = i + 1;
        if (argv[arg_number][0] != '-')
          out.maxFileSize = atoi(argv[arg_number]);
      } else if (std::string(argv[i]) == "-f") {
        int arg_number = i + 1;
        if (argv[arg_number][0] != '-')
          out.savePath = argv[arg_number];
      }
    }

    if ((out.maxThreads == undefined.maxThreads && out.port == undefined.port &&
         out.maxFileSize == undefined.maxFileSize &&
         out.savePath == undefined.savePath) ||
        argc % 2 == 0) {
      std::cout
          << "Usage: ./Tcp_Server <configFile>" << std::endl
          << "or: ./Tcp_Server -t <maxThreads> -p <port> -s <maxFileSize> "
             "-f <savePath>";
      return defaultSettings;
    }
  } else {
    // Read the file
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

void handleConnection(int clientSocket) {
  // Receive file size
  size_t fileSize;
  recv(clientSocket, reinterpret_cast<char *>(&fileSize), sizeof(fileSize), 0);

  // Receive file content
  std::vector<char> buffer(4096);
  size_t totalReceived = 0;
  while (totalReceived < fileSize) {
    ssize_t received = recv(clientSocket, buffer.data(), buffer.size(), 0);
    if (received <= 0) {
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
