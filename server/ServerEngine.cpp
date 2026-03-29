#include "ServerEngine.hpp"
#include "../shared/PacketTransport.hpp"
#include "../shared/packet.h"
#include "../shared/Request.h"
#include "WeatherService.hpp"
#include "FileTransferManager.hpp"
#include "ClientSession.hpp"
#include "StateMachine.hpp"
#include <iostream>
#include <string>

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
            if (running) {
                std::cout << "accept() failed\n";
            }
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
    StateMachine stateMachine;

    while (true) {
        packet rxPkt;

        if (!PacketTransport::receivePacket((int)clientSock, rxPkt)) {
            break;
        }

        unsigned char type = rxPkt.getPKType();
        unsigned char clientID = rxPkt.getCLID();

        if (rxPkt.calcCRC() != rxPkt.GetCRC()){
            std::string msg = "CRC Checksum Failed";

            packet err;
            err.PopulPacket((char*)msg.c_str(), (int)msg.size(), clientID, pkt_empty);

            if (!PacketTransport::sendPacket((int)clientSock, err)) {
                break;
            }
        }

        if (type == pkt_req) {
            Request rxReq(rxPkt.getData(), rxPkt.getPloadLength());
            std::string data;
            reqtyp reqType = static_cast<reqtyp>(rxReq.getType());

            if (!stateMachine.canHandle(reqType)) {
                data = "Request denied in state: " + stateMachine.getStateName();
            }
            else if (reqType == req_weather) {
                std::string location(rxReq.getBody(), rxReq.getBsize());

                std::cout << "REQUEST RECEIVED: " << location << "\n";
                std::cout << "Client ID: " << (int)clientID << "\n";

                data = WeatherService::getWeather(location);
                stateMachine.onRequestHandled(reqType);
            }
            else if (reqType == req_telemetry) {
                std::string telemetry(rxReq.getBody(), rxReq.getBsize());
                data = FileTransferManager::logTelemetry(telemetry, clientID);
                stateMachine.onRequestHandled(reqType);
            }
            else if (reqType == req_file) {
                data = FileTransferManager::getManualInfo();
                stateMachine.onRequestHandled(reqType);
            }
            else if (reqType == req_taxi) {
                data = FileTransferManager::getTaxiClearance();
                stateMachine.onRequestHandled(reqType);
            }
            else if (reqType == req_fplan) {
                std::string flightId(rxReq.getBody(), rxReq.getBsize());
                data = FileTransferManager::getFlightPlan(flightId);
                stateMachine.onRequestHandled(reqType);
            }
            else if (reqType == req_traffic) {
                std::string flightId(rxReq.getBody(), rxReq.getBsize());
                data = FileTransferManager::getTraffic(flightId);
                stateMachine.onRequestHandled(reqType);
            }
            else {
                data = "Unknown request type";
            }

            packet resp;
            resp.PopulPacket((char*)data.c_str(), (int)data.size(), clientID, pkt_dat);

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
            continue;
        }

        if (type == pkt_auth) {
            std::string credentials(rxPkt.getData(), rxPkt.getPloadLength());
            std::cout << "AUTH ATTEMPT: " << credentials << "\n";
            std::cout << "Client ID: " << (int)clientID << "\n";

            std::string response = ClientSession::authenticate(credentials);
            if (response.find("Authentication Successful") != std::string::npos) {
                stateMachine.onAuthSuccess();
            }

            packet authResp;
            authResp.PopulPacket((char*)response.c_str(), (int)response.size(), clientID, pkt_auth);

            if (!PacketTransport::sendPacket((int)clientSock, authResp)) {
                break;
            }
            continue;
        }

        {
            std::string msg = "Unknown packet type";

            packet err;
            err.PopulPacket((char*)msg.c_str(), (int)msg.size(), clientID, pkt_empty);

            if (!PacketTransport::sendPacket((int)clientSock, err)) {
                break;
            }
        }
    }
}