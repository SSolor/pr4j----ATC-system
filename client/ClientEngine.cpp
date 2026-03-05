#include "ClientEngine.hpp"
#include "../shared/PacketTransport.hpp"
#include "../shared/Packet.hpp"
#include <iostream>
#include <vector>

ClientEngine::ClientEngine() : sock(INVALID_SOCKET), clientID(1) {}

ClientEngine::~ClientEngine() {
    disconnect();
}

bool ClientEngine::connectToServer(const std::string& ip, unsigned short port) {
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        std::cout << "socket() failed\n";
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cout << "connect() failed\n";
        closesocket(sock);
        sock = INVALID_SOCKET;
        return false;
    }

    std::cout << "Connected to server!\n";
    return true;
}

void ClientEngine::disconnect() {
    if (sock != INVALID_SOCKET) {
        closesocket(sock);
        sock = INVALID_SOCKET;
    }
}

std::string ClientEngine::requestWeather(const std::string& location) {
    std::vector<char> body(location.begin(), location.end());

    Packet req(Packet::WEATHER_REQUEST, clientID, body);
    if (!PacketTransport::sendPacket((int)sock, req)) {
        return "Failed to send WEATHER_REQUEST";
    }

    Packet resp;
    if (!PacketTransport::receivePacket((int)sock, resp)) {
        return "Failed to receive response";
    }

    if (resp.getHeader().type == Packet::WEATHER_RESPONSE) {
        return resp.getBodyAsString();
    }

    if (resp.getHeader().type == Packet::ERROR_MSG) {
        return "Server error: " + resp.getBodyAsString();
    }

    return "Unexpected response type";
}