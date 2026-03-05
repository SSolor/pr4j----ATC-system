#include "ServerEngine.hpp"
#include <winsock2.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        std::cout << "WSAStartup failed\n";
        return 1;
    }

    ServerEngine server;
    if (!server.start(54000)) {
        WSACleanup();
        return 1;
    }

    server.run();

    WSACleanup();
    return 0;
}