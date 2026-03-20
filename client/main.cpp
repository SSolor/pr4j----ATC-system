/**
 * @file main.cpp
 * @brief Entry point for the Weather Client application.
 * * This file handles command-line argument parsing, WinSock initialization,
 * and the main input loop for user commands (WEATHER, QUIT).
 */
#include "ClientEngine.hpp"
#include <winsock2.h>
#include <iostream>
#include <string>
#include <cstdlib>

/** * @brief Link with the WinSock 2.2 library.
 */

#pragma comment(lib, "ws2_32.lib")
/**
 * @brief Main execution loop for the Weather Client.
 * * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments. 
 * argv[1]: Server IP (default: 127.0.0.1)
 * argv[2]: Server Port (default: 54000)
 * @return int Exit status (0 for success, 1 for failure).
 * * @details The function performs the following steps:
 * 1. Parses IP and Port from arguments.
 * 2. Initializes WinSock via WSAStartup.
 * 3. Connects to the server using ClientEngine.
 * 4. Enters a REPL (Read-Eval-Print Loop) to process 'WEATHER' or 'QUIT' commands.
 * 5. Cleans up resources on exit.
 */
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

        /** @note Handles graceful shutdown request from user. */
        if (cmd == "QUIT") {
            std::cout << "BYE\n";
            std::cout.flush();
            break;
        }

        if (cmd == "WEATHER") {

            /** @note Fetches weather data for a specified city. */
            std::string location;
            std::cin >> location;

            std::string result = client.requestWeather(location);
            std::cout << "WEATHER_RESPONSE " << result << "\n";
            std::cout.flush();
            continue;
        }

        // --- Error Handling for Unknown Commands ---
        std::string rest;
        std::getline(std::cin, rest);
        std::cout << "ERROR Unknown command\n";
        std::cout.flush();
    }

    // --- Cleanup ---
    client.disconnect();
    WSACleanup();
    return 0;
}