#pragma once
#include "packet.h"

class PacketTransport {
public:
    // sends one fully serialized packet over TCP
    static bool sendPacket(int sock, packet& pak);

    // receives one full packet over TCP and rebuilds it
    static bool receivePacket(int sock, packet& outpak);

private:
    // makes sure all bytes are sent
    static bool sendAll(int sock, const char* data, int size);

    // makes sure exactly size bytes are received
    static bool recvAll(int sock, char* buffer, int size);
};