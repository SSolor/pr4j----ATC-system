#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

static SOCKET server_socket = INVALID_SOCKET;

extern "C" {
    __declspec(dllexport) int init_winsock() {
        WSADATA wsaData;
        return WSAStartup(MAKEWORD(2, 2), &wsaData);
    }

    __declspec(dllexport) void cleanup_winsock() {
        WSACleanup();
    }

    __declspec(dllexport) int send_message(const char* server_ip, int port, char* response_buffer, int buffer_size) {
        struct sockaddr_in server_addr;
        SOCKET client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (client_socket == INVALID_SOCKET) {
            return -1;
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) != 1) {
            closesocket(client_socket);
            return -2;
        }

        if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
            closesocket(client_socket);
            return -2;
        }

        const char* message = "hello server";
        if (send(client_socket, message, (int)strlen(message), 0) == SOCKET_ERROR) {
            closesocket(client_socket);
            return -3;
        }

        int bytes_received = recv(client_socket, response_buffer, buffer_size - 1, 0);
        if (bytes_received <= 0) {
            closesocket(client_socket);
            return -4;
        }

        response_buffer[bytes_received] = '\0';
        closesocket(client_socket);
        return bytes_received;
    }

    __declspec(dllexport) int start_server(int port) {
        struct sockaddr_in server_addr;
        
        // Create socket
        server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (server_socket == INVALID_SOCKET) {
            return -1;
        }

        // Set socket to non-blocking mode
        u_long mode = 1;
        ioctlsocket(server_socket, FIONBIO, &mode);

        // Setup server address
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port);

        // Bind socket
        if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
            closesocket(server_socket);
            server_socket = INVALID_SOCKET;
            return -2;
        }

        // Listen for connections
        if (listen(server_socket, 5) == SOCKET_ERROR) {
            closesocket(server_socket);
            server_socket = INVALID_SOCKET;
            return -3;
        }

        return 0;
    }

    __declspec(dllexport) int check_for_client(char* client_ip_buffer, int buffer_size) {
        struct sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);
        char buffer[1024];

        // Try to accept a connection (non-blocking)
        SOCKET client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        
        if (client_socket == INVALID_SOCKET) {
            int error = WSAGetLastError();
            if (error == WSAEWOULDBLOCK) {
                return 0; // No pending connections
            }
            return -1; // Error
        }

        // Get client IP
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip_buffer, buffer_size);

        // Set client socket to blocking mode for recv/send
        u_long mode = 0;
        ioctlsocket(client_socket, FIONBIO, &mode);

        // Receive message
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            
            // Send response
            const char* response = "hello client";
            send(client_socket, response, (int)strlen(response), 0);
        }

        closesocket(client_socket);
        return 1; // Client handled successfully
    }

    __declspec(dllexport) void stop_server() {
        if (server_socket != INVALID_SOCKET) {
            closesocket(server_socket);
            server_socket = INVALID_SOCKET;
        }
    }

    __declspec(dllexport) int is_server_running() {
        return (server_socket != INVALID_SOCKET) ? 1 : 0;
    }
}