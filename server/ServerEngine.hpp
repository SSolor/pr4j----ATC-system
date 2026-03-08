#pragma once
#include <winsock2.h>

class ServerEngine {
public:
    ServerEngine();
    ~ServerEngine();

    bool start(unsigned short port);
    void run();               
    void stop();

private:
    SOCKET listenSock;
    bool running;

    void handleClient(SOCKET clientSock);
};