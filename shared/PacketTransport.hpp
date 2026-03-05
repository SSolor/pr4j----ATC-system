#pragma once
#include "Packet.hpp"

class PacketTransport {
public:
    // Sends one full Packet over TCP
    static bool sendPacket(int sock, const Packet& packet);

    // Receives one full Packet over TCP
    static bool receivePacket(int sock, Packet& outPacket);

private:
    // Ensures ALL bytes are sent
    static bool sendAll(int sock, const char* data, int size);

    // Ensures EXACTLY size bytes are received
    static bool recvAll(int sock, char* buffer, int size);
};