/**
 * @file ClientEngine.cpp
 * @brief Implementation of the ClientEngine class for handling server connectivity and weather requests.
 */

#include "ClientEngine.hpp"
//#include "../shared/PacketTransport.hpp"
#include "../shared/Packet.h"
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
    
    // Create packet body (e.g., location string)
    packet req;

    int bodySize = req.PopulPacket(
        (char*)location.data(), 
        (int)location.size(), 
        (char)clientID, 
        pkt_req     // PKTYPE defines any Request packet as pkt_req
    );

    // Serialize and Send request packet
    int serializedSize;
    char* serializedReq = req.Serialize(&serializedSize);

    int bytesSent = send(sock, serializedReq, serializedSize, 0);
    if (bytesSent == SOCKET_ERROR) {
        //std::cout << "Failed to send packet\n";
        return "Failed to send WEATHER_REQUEST";
    }

    // Receive response packet
    char buffer[MAX_PKTSIZE];
    int received = recv(sock, buffer, sizeof(buffer), MSG_WAITALL);
    if (received == SOCKET_ERROR || received == 0) { 
        //std::cout << "Failed to receive packet\n";
        return "Failed to receive response";
    }

    // Deserialize response packet
    packet resp(buffer); 
    
    //read header to determine if response is valid and what type it is
    if (resp.getTFlag() == PKT_TRNSMT_INCOMP) {
        //std::cout << "Received incomplete packet\n";
        return "Received incomplete response";
    }
    if (resp.getPKType() == pkt_dat) { 
        return std::string(resp.getData()); 
    }

    return "Unexpected response type";
}