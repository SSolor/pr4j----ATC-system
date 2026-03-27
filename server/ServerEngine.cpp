/**
 * @file ServerEngine.cpp
 * @brief Implementation of the ServerEngine class.
 * * Provides the logic for managing a TCP server, handling incoming connections,
 * and processing weather-related packets using a custom transport protocol.
 */

#include "ServerEngine.hpp"
#include "../shared/PacketTransport.hpp"
#include "../shared/packet.h"
#include "WeatherService.hpp"
#include <iostream>
#include <string>

/**
 * @brief Default constructor.
 * * Initializes the listening socket to INVALID_SOCKET and sets the running state to false.
 */
ServerEngine::ServerEngine() : listenSock(INVALID_SOCKET), running(false) {}

/**
 * @brief Destructor.
 * * Ensures the server is stopped and socket resources are released upon destruction.
 */
ServerEngine::~ServerEngine() {
    stop();
}

/**
 * @brief Initializes and starts the TCP server.
 * * @param port The port number to bind the server to.
 * @return true If the socket was successfully created, bound, and set to listen.
 * @return false If any step of the socket setup fails.
 * * @details This method performs the standard WinSock setup:
 * 1. Creates a TCP/IP socket.
 * 2. Binds the socket to all available interfaces (INADDR_ANY).
 * 3. Sets the socket to a listening state with a maximum backlog.
 */
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

/**
 * @brief The main server loop that accepts new client connections.
 * * @details This is a blocking call that runs as long as the 'running' flag is true.
 * For every successful connection, it calls handleClient() to process requests.
 * Currently, this server processes one client at a time (synchronous).
 */
void ServerEngine::run() {
    while (running) {
        sockaddr_in clientAddr{};
        int len = sizeof(clientAddr);

        // Blocks until a client connects
        SOCKET clientSock = accept(listenSock, (sockaddr*)&clientAddr, &len);
        if (clientSock == INVALID_SOCKET) {
            if (running) {
                std::cout << "accept() failed\n";
            }
            continue;
        }

        std::cout << "Client connected!\n";

        handleClient(clientSock);

        // Clean up the client socket after handleClient returns
        closesocket(clientSock);
        std::cout << "Client disconnected.\n";
    }
}

/**
 * @brief Stops the server and closes the listening socket.
 */
void ServerEngine::stop() {
    running = false;

    if (listenSock != INVALID_SOCKET) {
        closesocket(listenSock);
        listenSock = INVALID_SOCKET;
    }
}

/**
 * @brief Processes incoming packets from a specific client.
 * * @param clientSock The socket associated with the connected client.
 * * @details This method implements the communication protocol:
 * 1. Receives a packet via PacketTransport.
 * 2. If it's a WEATHER_REQUEST, it queries WeatherService and sends a WEATHER_RESPONSE.
 * 3. If the packet type is unknown, it sends an ERROR_MSG.
 * 4. The loop breaks if the client disconnects or a transport error occurs.
 */
void ServerEngine::handleClient(SOCKET clientSock) {
    while (true) {
        packet req;

        if (!PacketTransport::receivePacket((int)clientSock, req)) {
            break;
        }

        unsigned char type = req.getPKType();
        unsigned char clientID = req.getCLID();

        if (type == pkt_req) {
            std::string location(req.getData(), req.getPloadLength());

            std::cout << "REQUEST RECEIVED: " << location << "\n";
            std::cout << "Client ID: " << (int)clientID << "\n";

            std::string weather = WeatherService::getWeather(location);

            packet resp;
            resp.PopulPacket((char*)weather.c_str(), (int)weather.size(), clientID, pkt_dat);

            std::cout << "DATA: ";
            std::cout.write(resp.getData(), resp.getPloadLength());
            std::cout << std::endl;

            // just for me to debug
            std::cout << "CRC: " << resp.calcCRC() << std::endl;
            std::cout << "TYPE: " << (int)resp.getPKType() << std::endl;
            std::cout << "FLAG: " << (int)resp.getTFlag() << std::endl;
            std::cout << "CLIENT ID: " << (int)resp.getCLID() << std::endl;
            std::cout << "LENGTH: " << (int)resp.getPloadLength() << std::endl;

            if (!PacketTransport::sendPacket((int)clientSock, resp)) {
                break;
            }
        }
        else {
            std::string msg = "Unknown request type";

            packet err;
            err.PopulPacket((char*)msg.c_str(), (int)msg.size(), clientID, pkt_empty);

            if (!PacketTransport::sendPacket((int)clientSock, err)) {
                break;
            }
        }
    }
}