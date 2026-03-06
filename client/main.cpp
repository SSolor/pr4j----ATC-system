#include "ClientEngine.hpp"
#include <winsock2.h>
#include <iostream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cout << "WSAStartup failed\n";
        return 1;
    }

    // Fixed connection settings (no command args)
    std::string ip = "127.0.0.1";
    unsigned short port = 54000;

    // Read location from stdin (sent by Python GUI)
    std::string location;
    std::getline(std::cin, location);

    if (location.empty()) {
        std::cout << "No location provided\n";
        WSACleanup();
        return 1;
    }

    ClientEngine client;
    if (!client.connectToServer(ip, port)) {
        std::cout << "connect() failed\n";
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server!\n";

    std::string result = client.requestWeather(location);
    std::cout << result << "\n";

    client.disconnect();
    WSACleanup();
    return 0;
}