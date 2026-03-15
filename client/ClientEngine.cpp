/**
 * @file ClientEngine.cpp
 * @brief Implementation of the ClientEngine class for handling server connectivity and weather requests.
 */

#include "ClientEngine.hpp"
#include "../shared/PacketTransport.hpp"
#include "../shared/Packet.hpp"
#include <iostream>
#include <vector>

/**
 * @brief Construct a new Client Engine object.
 * Initializes the socket to an invalid state and sets a default client ID.
 */

ClientEngine::ClientEngine() : sock(INVALID_SOCKET), clientID(1) {}

/**
 * @brief Destroy the Client Engine object.
 * Ensures the connection is closed before the object is destroyed.
 */
ClientEngine::~ClientEngine() {
    disconnect();
}

/**
 * @brief Establishes a TCP connection to the specified server.
 * * @param ip The IPv4 address of the server as a string.
 * @param port The port number to connect to.
 * @return true if the connection was successful.
 * @return false if socket creation or connection failed.
 */

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

/**
 * @brief Gracefully closes the active socket connection.
 */

void ClientEngine::disconnect() {
    if (sock != INVALID_SOCKET) {
        closesocket(sock);
        sock = INVALID_SOCKET;
    }
}

/**
 * @brief Sends a weather data request to the server and awaits a response.
 * * This method encapsulates the location string into a Packet, sends it via 
 * PacketTransport, and handles the resulting server response or error.
 * * @param location The name of the city or region for the weather query.
 * @return std::string The weather data, a server error message, or a transport error description.
 */

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