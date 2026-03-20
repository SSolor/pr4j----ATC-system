#pragma once
#include <winsock2.h>
#include <string>
/**
 * @class ClientEngine
 * @brief Manages the network connection and communication with the weather server.
 * * This class handles the lifecycle of a WinSock socket, allowing for 
 * connection, disconnection, and data retrieval for weather-specific queries.
 */
class ClientEngine {
public:
/**
     * @brief Construct a new Client Engine object.
     * * Initializes internal members and prepares the engine for a connection.
     */
    ClientEngine();
    /**
     * @brief Destroy the Client Engine object.
     * * Ensures that any active socket connections are closed before destruction.
     */
    ~ClientEngine();
    /**
     * @brief Establishes a connection to the weather server.
     * * @param ip The IPv4 address of the server (e.g., "127.0.0.1").
     * @param port The port number the server is listening on.
     * @return true If the connection was successfully established.
     * @return false If the connection failed or the socket could not be initialized.
     */

    bool connectToServer(const std::string& ip, unsigned short port);

    /**
     * @brief Gracefully closes the connection to the server.
     */

    void disconnect();

    /**
     * @brief Sends a request for weather data for a specific location.
     * * @param location The name of the city or region (e.g., "London").
     * @return std::string The raw weather data response from the server, 
     * or an error message if the request fails.
     */

    std::string requestWeather(const std::string& location);

private:
    SOCKET sock; ///< The WinSock socket handle.
    unsigned int clientID; ///< Unique identifier assigned by the server or system.
};