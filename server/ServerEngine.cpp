#include "ServerEngine.hpp"
#include "../shared/PacketTransport.hpp"
#include "../shared/Packet.hpp"
#include "WeatherService.hpp"
#include <iostream>

ServerEngine::ServerEngine() : listenSock(INVALID_SOCKET), running(false) {}

ServerEngine::~ServerEngine() {
    stop();
}

bool ServerEngine::start(unsigned short port) {
    listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSock == INVALID_SOCKET) {
        std::cout << "socket() failed\n";
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(listenSock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cout << "bind() failed\n";
        closesocket(listenSock);
        listenSock = INVALID_SOCKET;
        return false;
    }

    if (listen(listenSock, SOMAXCONN) == SOCKET_ERROR) {
        std::cout << "listen() failed\n";
        closesocket(listenSock);
        listenSock = INVALID_SOCKET;
        return false;
    }

    running = true;
    std::cout << "Server listening on port " << port << "\n";
    return true;
}

void ServerEngine::run() {
    while (running) {
        sockaddr_in clientAddr{};
        int len = sizeof(clientAddr);

        SOCKET clientSock = accept(listenSock, (sockaddr*)&clientAddr, &len);
        if (clientSock == INVALID_SOCKET) {
            if (running) std::cout << "accept() failed\n";
            continue;
        }

        std::cout << "Client connected!\n";

        handleClient(clientSock);

        closesocket(clientSock);
        std::cout << "Client disconnected.\n";
    }
}

void ServerEngine::stop() {
    running = false;
    if (listenSock != INVALID_SOCKET) {
        closesocket(listenSock);
        listenSock = INVALID_SOCKET;
    }
}

void ServerEngine::handleClient(SOCKET clientSock) {
    while (true) {
        Packet req;
        if (!PacketTransport::receivePacket((int)clientSock, req)) {
            break;
        }

        unsigned int type = req.getHeader().type;

        if (type == Packet::WEATHER_REQUEST) {
            std::string location = req.getBodyAsString();
            std::cout << "WEATHER_REQUEST: " << location << "\n";
            std::cout << "Client ID: " << req.getHeader().clientID << "\n"; 

            // Get weather from file (or placeholder for now)
            std::string weather = WeatherService::getWeather(location);

            Packet resp(Packet::WEATHER_RESPONSE,
                        req.getHeader().clientID,
                        std::vector<char>(weather.begin(), weather.end()));

            PacketTransport::sendPacket((int)clientSock, resp);
        }
        else {
            std::string msg = "Unknown request type";
            Packet err(Packet::ERROR_MSG,
                       req.getHeader().clientID,
                       std::vector<char>(msg.begin(), msg.end()));
            PacketTransport::sendPacket((int)clientSock, err);
        }
    }
}