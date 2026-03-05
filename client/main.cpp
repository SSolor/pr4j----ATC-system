#include "ClientEngine.hpp"
#include <winsock2.h>
#include <iostream>
#include <string>
#include <cstdlib>

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char* argv[]) {
    std::string ip = "127.0.0.1";
    unsigned short port = 54000;

    if (argc >= 2) ip = argv[1];
    if (argc >= 3) {
        int p = std::atoi(argv[2]);
        if (p > 0 && p <= 65535) port = (unsigned short)p;
    }

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        std::cout << "WSAStartup failed\n";
        return 1;
    }

    ClientEngine client;
    if (!client.connectToServer(ip, port)) {
        std::cout << "CONNECT_FAILED\n";
        WSACleanup();
        return 1;
    }

    std::cout << "CONNECTED\n";
    std::cout.flush();

    // Expected input:
    // WEATHER <location>
    std::string cmd;
    while (std::cin >> cmd) {

        if (cmd == "QUIT") {
            std::cout << "BYE\n";
            std::cout.flush();
            break;
        }

        if (cmd == "WEATHER") {
            std::string location;
            std::cin >> location;

            std::string result = client.requestWeather(location);
            std::cout << "WEATHER_RESPONSE " << result << "\n";
            std::cout.flush();
            continue;
        }

        std::string rest;
        std::getline(std::cin, rest);
        std::cout << "ERROR Unknown command\n";
        std::cout.flush();
    }

    client.disconnect();
    WSACleanup();
    return 0;
}