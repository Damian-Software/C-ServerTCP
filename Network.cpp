#include "Network.h"
#include "Serialize.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>

#define MAX_EVENTS 1000
#define WORKER_THREADS 4
#define BUFFER_SIZE 1024

Network::Network(int server_port)
    : server_port(server_port), server_fd(-1), epoll_fd(-1)
{
    // Vytvoøení socketu serveru
    server_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    if (server_fd < 0) 
    {
        std::cerr << "Failed to create server socket." << std::endl;
        exit(EXIT_FAILURE);
    }

    if (server_port < 0 || server_port > 65535) 
    {
        std::cerr << "Invalid port number." << std::endl;
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0x0)
    {
        std::cerr << "Setsockopt" << std::endl;
        exit(EXIT_FAILURE);

    }


    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    //server_addr.sin_port = htons(server_port);
    server_addr.sin_port = htons(static_cast<uint16_t>(server_port));

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) 
    {
        std::cerr << "Binding failed." << std::endl;
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, SOMAXCONN) < 0) 
    {
        std::cerr << "Listening failed." << std::endl;
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Vytvoøení epoll instance
    epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) 
    {
        std::cerr << "Failed to create epoll instance." << std::endl;
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    struct epoll_event event {};
    event.data.fd = server_fd;
    event.events = EPOLLIN | EPOLLET;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) < 0) 
    {
        std::cerr << "Failed to add server socket to epoll." << std::endl;
        close(server_fd);
        close(epoll_fd);
        exit(EXIT_FAILURE);
    }

}

Network::~Network() 
{
    stopServer();
}

void Network::startServer()
{
    // Spustíme vlákna pro zpracování epoll událostí
    for (int i = 0; i < WORKER_THREADS; ++i) 
    {
        worker_threads.emplace_back(&Network::workerLoop, this);
    }

    for (auto& thread : worker_threads) 
    {
        thread.join();
    }
}

void Network::stopServer() 
{
    if (server_fd >= 0) 
    {
        close(server_fd);
    }
    if (epoll_fd >= 0) 
    {
        close(epoll_fd);
    }
    for (auto& thread : worker_threads) 
    {
        if (thread.joinable()) 
        {
            thread.join();
        }
    }
}

void Network::workerLoop() 
{
    epoll_event events[MAX_EVENTS];

    while (true) 
    {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (n < 0) 
        {
            std::cerr << "epoll_wait failed." << std::endl;
            break;
        }

        for (int i = 0; i < n; ++i) 
        {
            if (events[i].data.fd == server_fd) 
            {
                handleNewConnection();
            }
            else 
            {
                handleClient(events[i].data.fd);
            }
        }
    }
}

void Network::handleNewConnection() 
{
    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);
    int client_socket = accept4(server_fd, (struct sockaddr*)&client_addr, &client_len, SOCK_NONBLOCK);

    if (client_socket < 0) 
    {
        std::cerr << "Failed to accept new connection." << std::endl;
        return;
    }

    epoll_event event{};
    event.data.fd = client_socket;
    event.events = EPOLLIN | EPOLLET;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &event) < 0) 
    {
        std::cerr << "Failed to add client socket to epoll." << std::endl;
        close(client_socket);
    }

    std::lock_guard<std::mutex> lock(client_mutex);
    client_data[client_socket] = nullptr; // Inicializace dat pro klienta
}

void Network::handleClient(int client_socket) 
{
    uint8_t buffer[BUFFER_SIZE];
    int bytes_read = read(client_socket, reinterpret_cast<char*>(buffer), static_cast<int>(sizeof(buffer)));

    if (bytes_read <= 0) 
    {
        closeClient(client_socket);
        return;
    }

    try
    {
        std::shared_ptr<PacketBase> packet = Serialize::deserialize(buffer, bytes_read);

        switch (packet->getPacketType()) 
        {
        case PacketType::LOGIN: 
        {
            auto login_packet = std::static_pointer_cast<LoginPacket>(packet);
            std::cout << "Received login packet with account ID: " << login_packet->account_id << std::endl;
            break;
        }
        case PacketType::LOGINPWD:
        {
            auto login_packet_pwd = std::static_pointer_cast<LoginPacketPWD>(packet);
            std::cout << "Recived login packet with account password: " << login_packet_pwd->password << std::endl;
            break;
        }
        case PacketType::MESSAGE: 
        {
            auto message_packet = std::static_pointer_cast<MessagePacket>(packet);
            std::cout << "Received message: " << message_packet->message << std::endl;
            break;
        }
        case PacketType::DATA:
        {
            auto data_packet = std::static_pointer_cast<DataPacket>(packet);
            std::cout << "Received data: " << std::string(data_packet->data.begin(), data_packet->data.end()) << std::endl;
            std::cout << "Received data_as_string: " << data_packet->data_as_string << std::endl;
            break;
        }
        case PacketType::TEST:
        {
            auto test_packet = std::static_pointer_cast<TestPacket>(packet);
            std::cout << "Received test string: " << test_packet->test_string << std::endl;
            std::cout << "Received test vector: ";
            for (int value : test_packet->test_vector)
            {
                std::cout << value << " ";
            }
            std::cout << std::endl;
            break;
        }
        // Další typy paketù...
        default:
            std::cerr << "Unknown packet type received." << std::endl;
            break;
        }
    }
    catch (const std::exception& e) 
    {
        std::cerr << "Failed to deserialize packet: " << e.what() << std::endl;
    }
}

void Network::closeClient(int client_socket) 
{
    std::lock_guard<std::mutex> lock(client_mutex);
    client_data.erase(client_socket);
    close(client_socket);
}
