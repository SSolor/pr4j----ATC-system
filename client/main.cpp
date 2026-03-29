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
        std::string input = "";
        

        // STARTUP
        if (cmd == "QUIT") {
            std::cout << "BYE\n";
            std::cout.flush();
            break;
        }

        // EMERGENCY
        if (cmd == "EMERGENCY") {
            std::string result = client.pkRequest(input, pkt_emgcy);
            std::cout << "EMERGENCY_RESPONSE: " << result << "\n";
            std::cout.flush();
            continue;
        }

        //AUTH
        if (cmd == "Login"){
            std::string Username;
            std::cout << "Please enter Username: ";
            std::cin >> Username;
            std::string Password;
            std::cout << "Please enter Password: ";
            std::cin >> Password;
            std::string info = Username+","+Password;
            
            std::string result = client.pkRequest(info, pkt_auth);
            std::cout << "LoginAuth: " << result << "\n";
            std::cout.flush();
            continue;

        }
        
        //PRE_FLIGHT
        if (cmd == "WEATHER") {
            std::string input;
            std::cout << "Please enter location name: ";
            std::cin >> input;

            std::string result = client.dataRequest(input, req_weather);
            std::cout << "WEATHER_RESPONSE: " << result << "\n";
            std::cout.flush();
            continue;
        }
        if (cmd == "FLIGHT") {
            std::string input;
            std::cout << "Please enter Flight ID: ";
            std::cin >> input;

            std::string result = client.dataRequest(input, req_fplan);
            std::cout << "Flight Plan: " << result << "\n";
            std::cout.flush();
            continue;
        }
        if (cmd == "TAXI") {
            std::string result = client.dataRequest(input, req_taxi);
            std::cout << "Taxi Response: " << result << "\n";
            std::cout.flush();
            continue;
        }

        // ACTIVE_AIRSPACE
        if (cmd == "LAND") {
            std::string result = client.dataRequest(input, req_taxi);
            std::cout << "Clearance_Request: " << result << "\n";
            std::cout.flush();
            continue;
        }
        if (cmd == "TELEM") {
            std::string result = client.dataRequest(input, req_telemetry);
            std::cout << "Telemetry Response: " << result << "\n";
            std::cout.flush();
            continue;
        }
        if (cmd == "TRAFFIC") {
            std::string result = client.dataRequest(input, req_traffic);
            std::cout << "Air Traffic Response: " << result << "\n";
            std::cout.flush();
            continue;
        }

        //DATA_TRANSFER
        if (cmd == "MANUAL") {
            std::string result = client.dataRequest(input, req_file);
            std::cout << "Manual Response: " << result << "\n";
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