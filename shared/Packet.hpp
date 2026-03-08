#pragma once
#include <vector>
#include <string>
#include <stdexcept>
#include <cstring>

class Packet {
public:

    // Packet Types
    enum Type {
        AUTH_REQUEST = 1,
        AUTH_RESPONSE,
        WEATHER_REQUEST,
        WEATHER_RESPONSE,
        TELEMETRY,
        FILE_REQUEST,
        FILE_CHUNK,
        FILE_COMPLETE,
        ERROR_MSG
    };

    // Packet Flags
    enum Flags {
        FLAG_NONE = 0,
        FLAG_MORE = 1
    };

    // Packet Header
    struct Header {
        unsigned int magic;
        unsigned int type;
        unsigned int clientID;
        unsigned int payloadSize;
        unsigned int flags;
        unsigned int sequence;
    };

    // Packet Tail
    struct Tail {
        unsigned int checksum;
    };

    // Constructors
    Packet() {
        header.magic = MAGIC;
        header.type = 0;
        header.clientID = 0;
        header.payloadSize = 0;
        header.flags = 0;
        header.sequence = 0;
        tail.checksum = 0;
    }

    Packet(int type,
           unsigned int clientID,
           const std::vector<char>& data,
           unsigned int flags = FLAG_NONE,
           unsigned int sequence = 0)
    {
        header.magic = MAGIC;
        header.type = type;
        header.clientID = clientID;
        header.payloadSize = (unsigned int)data.size();
        header.flags = flags;
        header.sequence = sequence;

        body = data;

        tail.checksum = calculateChecksum();
    }

    // Serialize Packet
    std::vector<char> serialize() const {

        int totalSize =
            (int)sizeof(Header)
            + (int)body.size()
            + (int)sizeof(Tail);

        std::vector<char> buffer;
        buffer.resize(totalSize);

        int offset = 0;

        // Copy Header
        std::memcpy(
            buffer.data() + offset,
            &header,
            sizeof(Header)
        );
        offset += (int)sizeof(Header);

        // Copy Body
        if (!body.empty()) {
            std::memcpy(
                buffer.data() + offset,
                body.data(),
                body.size()
            );
            offset += (int)body.size();
        }

        // Copy Tail
        std::memcpy(
            buffer.data() + offset,
            &tail,
            sizeof(Tail)
        );

        return buffer;
    }

    // Deserialize Packet
    static Packet deserialize(const std::vector<char>& buffer) {

        if (buffer.size() < sizeof(Header) + sizeof(Tail))
            throw std::runtime_error("Buffer too small");

        Packet p;
        int offset = 0;

        // Read Header
        std::memcpy(
            &p.header,
            buffer.data() + offset,
            sizeof(Header)
        );
        offset += (int)sizeof(Header);

        if (p.header.magic != MAGIC)
            throw std::runtime_error("Invalid packet magic");

        int expectedSize =
            (int)sizeof(Header)
            + (int)p.header.payloadSize
            + (int)sizeof(Tail);

        if ((int)buffer.size() != expectedSize)
            throw std::runtime_error("Packet size mismatch");

        // Read Body
        p.body.resize(p.header.payloadSize);

        if (p.header.payloadSize > 0) {
            std::memcpy(
                p.body.data(),
                buffer.data() + offset,
                p.header.payloadSize
            );
            offset += (int)p.header.payloadSize;
        }

        // Read Tail
        std::memcpy(
            &p.tail,
            buffer.data() + offset,
            sizeof(Tail)
        );

        // Validate checksum
        unsigned int calculated = p.calculateChecksum();

        if (calculated != p.tail.checksum)
            throw std::runtime_error("Checksum failed");

        return p;
    }

    // Accessors
    const Header& getHeader() const {
        return header;
    }

    const std::vector<char>& getBody() const {
        return body;
    }

    std::string getBodyAsString() const {
        return std::string(body.begin(), body.end());
    }

private:

    Header header;
    std::vector<char> body;
    Tail tail;

    static const unsigned int MAGIC = 0x12345678;

    // Checksum Calculation
    unsigned int calculateChecksum() const {

        unsigned int sum = 0;

        // Header bytes
        const char* headerPtr = (const char*)&header;
        for (int i = 0; i < (int)sizeof(Header); i++) {
            sum += (unsigned char)headerPtr[i];
        }

        // Body bytes
        for (char c : body) {
            sum += (unsigned char)c;
        }

        return sum;
    }
};