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

class Network
{
public:
    Network(int server_port);
    ~Network();

    void startServer();
    void stopServer();

private:
    int epoll_fd;
    int server_port;
    int server_fd;
    std::vector<std::thread> worker_threads;
    std::unordered_map<int, std::shared_ptr<PacketBase>> client_data;
    std::mutex client_mutex;

    void workerLoop();
    void handleNewConnection();
    void handleClient(int client_socket);
    void closeClient(int client_socket);
};

#endif // !NETWORK_H
