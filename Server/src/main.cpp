#include "TcpServer.hpp"
#include <iterator>

void fatalErrorHandler();
void signalHandler(int signalNumber);

int main(int argc, char *argv[])
{
    bool terminateServer = false;

    std::signal(SIGTERM, signalHandler);
    std::signal(SIGHUP, signalHandler);

    TcpServer server(
        {
            .maxThreads = 4,
            .port = 1234,
            .maxFileSize = 8096,
            .savePath = "./Output",
        });

    if (server.parseSettings(argc, argv))
        fatalErrorHandler();

    server.printSettings();

    if (server.startServer() != SUCCESS)
        exit(1);

    if (server.acceptConnections())
        exit(1);

    server.stopServer();

    return 0;
}

void fatalErrorHandler()
{
    std::cout << "Usage: ./Tcp_Server <configFile>" << std::endl
              << "or: ./Tcp_Server -t <maxThreads> -p <port> -s <maxFileSize> "
                 "-f <savePath>" << std::endl;
    exit(1);
}

void signalHandler(int signalNumber)
{
    if (signalNumber == SIGTERM || signalNumber == SIGHUP)
    {
        std::cout << "Recieved interupt signal: " << signalNumber << "\nExiting..." << std::endl;
        exit(signalNumber);
    }
}
