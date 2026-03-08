#include "PacketTransport.hpp"
#include <winsock2.h>
#include <vector>
#include <cstring>   

bool PacketTransport::sendPacket(int sock, const Packet& packet)
{
    std::vector<char> bytes = packet.serialize();

    // send full packet buffer
    return sendAll(sock, bytes.data(), (int)bytes.size());
}

bool PacketTransport::receivePacket(int sock, Packet& outPacket)
{
    // Read header first
    Packet::Header header;
    if (!recvAll(sock, (char*)&header, (int)sizeof(Packet::Header)))
        return false;

    // Basic validation early
    if (header.magic != 0x12345678)
        return false;

    // Create full buffer: [Header][Body][Tail]
    int totalSize = (int)sizeof(Packet::Header)
                  + (int)header.payloadSize
                  + (int)sizeof(Packet::Tail);

    std::vector<char> fullBuffer;
    fullBuffer.resize(totalSize);

    // Copy header into fullBuffer
    int offset = 0;
    memcpy(fullBuffer.data() + offset, &header, sizeof(Packet::Header));
    offset += (int)sizeof(Packet::Header);

    // Read body + tail 
    int remaining = (int)header.payloadSize + (int)sizeof(Packet::Tail);

    if (remaining > 0) {
        if (!recvAll(sock, fullBuffer.data() + offset, remaining))
            return false;
    }

    // Deserialize to Packet
    try {
        outPacket = Packet::deserialize(fullBuffer);
    }
    catch (...) {
        return false;
    }

    return true;
}

bool PacketTransport::sendAll(int sock, const char* data, int size)
{
    int totalSent = 0;

    while (totalSent < size) {
        int sent = send(sock, data + totalSent, size - totalSent, 0);
        if (sent <= 0) return false;
        totalSent += sent;
    }

    return true;
}

bool PacketTransport::recvAll(int sock, char* buffer, int size)
{
    int totalReceived = 0;

    while (totalReceived < size)
    {
        int received = recv(sock, buffer + totalReceived, size - totalReceived, 0);

        if (received == 0) {
            // connection closed cleanly
            return false;
        }

        if (received < 0) {
            // actual socket error
            return false;
        }

        totalReceived += received;
    }

    return true;
}