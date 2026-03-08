#pragma once
#include <winsock2.h>
#include <string>

class ClientEngine {
public:
    ClientEngine();
    ~ClientEngine();

    bool connectToServer(const std::string& ip, unsigned short port);
    void disconnect();

    std::string requestWeather(const std::string& location);

private:
    SOCKET sock;
    unsigned int clientID;
};