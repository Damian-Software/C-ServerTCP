#pragma once
#ifndef NETWORK_H
#define NETWORK_H
#include <string>
#include <vector>
#include <memory>
#include <sys/epoll.h>
#include <thread>
#include <unordered_map>
#include <mutex>
#include "Packet.h"
#include "ReadData.h"
#include "SendData.h"

class ClientSession
{
public:
    ClientSession(int socket_fd) : socket_fd(socket_fd) {}

    int getSocket() const { return socket_fd; }

    void addToBuffer(const std::vector<uint8_t>& data) {
        std::lock_guard<std::mutex> lock(buffer_mutex);
        buffer.insert(buffer.end(), data.begin(), data.end());
    }

    std::vector<uint8_t>& getBuffer() {
        std::lock_guard<std::mutex> lock(buffer_mutex);
        return buffer;
    }

    void clearBuffer(size_t length) {
        std::lock_guard<std::mutex> lock(buffer_mutex);
        buffer.erase(buffer.begin(), buffer.begin() + length);
    }

private:
    int socket_fd;
    std::vector<uint8_t> buffer;
    std::mutex buffer_mutex; // Synchronizace bufferu klientské session
};


class Network
{
public:
    Network(int server_port);
    ~Network();

    void startServer();
    void stopServer();
    void sendPacket(const PacketBase& packet, int client_socket);

    // Getter pro získání seznamu klientských socketù (bezpeèná kopie)
    std::vector<int> getClientSockets();

private:
    int epoll_fd;
    int server_port;
    int server_fd;
    std::vector<std::thread> worker_threads;
    std::unordered_map<int, std::shared_ptr<ClientSession>> client_sessions;
    std::mutex client_mutex;

    void workerLoop();
    void handleNewConnection();
    void handleClient(int client_socket);
    void closeClient(int client_socket);
};

#endif // !NETWORK_H
