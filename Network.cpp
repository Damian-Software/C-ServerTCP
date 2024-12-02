#include "Network.h"
#include "Serialize.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include "SendData.h" // Pro funkci sendData
#include <stdexcept>
#include <sys/epoll.h>
#include <thread>
#include <functional>
#include <vector>

#define MAX_EVENTS 1000
#define WORKER_THREADS 4
#define BUFFER_SIZE 4096

Network::Network(int server_port)
    : server_port(server_port), server_fd(-1), epoll_fd(-1)
{
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) 
    {
        std::cerr << "Failed to create epoll file descriptor." << std::endl;
        exit(EXIT_FAILURE);
    }

    // Vytvoøení socketu serveru
    server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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


    int flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK); // Non-blocking socket


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
    //epoll_fd = epoll_create1(0);
    //if (epoll_fd < 0) 
    //{
    //    std::cerr << "Failed to create epoll instance." << std::endl;
    //    close(server_fd);
    //    exit(EXIT_FAILURE);
    //}

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
    worker_threads.emplace_back(&Network::workerLoop, this); // Spuštìní vlákna pro správu pøipojení

    std::cout << "Server started on port " << server_port << std::endl;
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

void Network::sendPacket(const PacketBase& packet, int client_socket)
{
    // Serializace paketu
    std::vector<uint8_t> serialized_data;
    Serialize::serialize(packet, serialized_data);

    // Kontrola velikosti serializovaných dat
    if (serialized_data.size() > static_cast<size_t>(INT_MAX))
    {
        std::cerr << "Serialized data length exceeds allowed size." << std::endl;
        return;
    }

    // Získání zámku, aby se zabránilo konkurenènímu pøístupu ke klientské mapì
    std::lock_guard<std::mutex> lock(client_mutex);

    auto it = client_sessions.find(client_socket);
    if (it != client_sessions.end())
    {
        ssize_t bytes_sent = send(client_socket, reinterpret_cast<const char*>(serialized_data.data()), static_cast<int>(serialized_data.size()), 0);

        if (bytes_sent < 0)
        {
            std::cerr << "Failed to send packet to client socket " << client_socket << ": " << strerror(errno) << std::endl;
        }
        else
        {
            std::cout << "Sent packet of type: " << static_cast<int>(packet.getPacketType()) << " to client socket " << client_socket << ", bytes sent: " << bytes_sent << std::endl;
        }
    }
    else
    {
        std::cerr << "Client socket " << client_socket << " not found." << std::endl;
    }

}

std::vector<int> Network::getClientSockets()
{
    std::lock_guard<std::mutex> lock(client_mutex);
    std::vector<int> client_sockets;
    for (const auto& pair : client_sessions)
    {
        client_sockets.push_back(pair.first);
    }
    return client_sockets;
}

// Pracovní vlákno, které spravuje epoll události
void Network::workerLoop() 
{
    //const int MAX_EVENTS = 10;
    epoll_event events[MAX_EVENTS];

    while (true) {
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

        for (int i = 0; i < num_events; ++i) {
            if (events[i].data.fd == server_fd) {
                handleNewConnection();
            }
            else {
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
    client_sessions[client_socket] = std::make_shared<ClientSession>(client_socket); // Vytvoøení nové session pro klienta

    std::cout << "New client connected, socket: " << client_socket << std::endl;

}

// Zpracování pøijatých dat od klienta
void Network::handleClient(int client_socket)
{
    auto it = client_sessions.find(client_socket);
    if (it == client_sessions.end())
    {
        std::cerr << "Client session not found for socket " << client_socket << std::endl;
        closeClient(client_socket);
        return;
    }

    ClientSession& session = *(it->second);
    uint8_t buffer[BUFFER_SIZE];
    ssize_t bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);

    if (bytes_read <= 0)
    {
        closeClient(client_socket);
        return;
    }

    // Pøidání pøijatých dat do bufferu klientské session
    session.addToBuffer(std::vector<uint8_t>(buffer, buffer + bytes_read));

    std::vector<uint8_t>& client_buffer = session.getBuffer();

    while (client_buffer.size() >= 8) // Minimální velikost: 4 bajty typ + 4 bajty délka
    {
        // Získání typu paketu a délky (prvních 8 bajtù)
        PacketType packet_type = static_cast<PacketType>((client_buffer[0] << 24) | (client_buffer[1] << 16) | (client_buffer[2] << 8) | client_buffer[3]);
        size_t packet_length = (client_buffer[4] << 24) | (client_buffer[5] << 16) | (client_buffer[6] << 8) | client_buffer[7];
        size_t total_packet_length = 8 + packet_length;

        // Zkontrolujte, zda máme dostatek dat pro kompletní paket
        if (client_buffer.size() < total_packet_length) {
            // Poèkejte na další data, protože paket není kompletní
            break;
        }

        // Pokud máme kompletní paket, zpracujeme ho
        try
        {
            // Vytvoøte ukazatel na deserializovaný paket
            std::shared_ptr<PacketBase> packet = Serialize::deserialize(client_buffer.data(), total_packet_length);

            // Výpis zpracovaných dat pro ladìní
            std::cout << "Processed raw data for packet type: ";
            for (size_t i = 0; i < total_packet_length; ++i) {
                std::cout << std::hex << static_cast<int>(client_buffer[i]) << " ";
            }
            std::cout << std::dec << std::endl;

            // Zpracujte paket podle jeho typu
            switch (packet->getPacketType())
            {
            case PacketType::LOGIN:
            {
                auto login_packet = std::static_pointer_cast<LoginPacket>(packet);
                std::cout << "Received login ID: " << login_packet->account_id << std::endl;
                break;
            }
            case PacketType::LOGINPWD:
            {
                auto login_packet_pwd = std::static_pointer_cast<LoginPacketPWD>(packet);
                std::cout << "Received login password: " << login_packet_pwd->password << std::endl;
                break;
            }
            case PacketType::MESSAGE:
            {
                auto message_packet = std::static_pointer_cast<MessagePacket>(packet);
                std::cout << "Received message: " << message_packet->message << std::endl;

                if (message_packet->message.find("POZDRAV") != std::string::npos)
                {
                    MessagePacket response_packet;
                    response_packet.message = "Hello, tady server.";
                    sendPacket(response_packet, client_socket); // Odeslání odpovìdi konkrétnímu klientovi
                    std::cout << "Sent response to client " << client_socket << ": " << response_packet.message << std::endl;

                }



                break;
            }
            default:
                std::cerr << "Unknown packet type received." << std::endl;
                break;
            }

            // Odstraòte kompletnì zpracovaný paket z bufferu
            client_buffer.erase(client_buffer.begin(), client_buffer.begin() + total_packet_length);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Failed to deserialize packet: " << e.what() << std::endl;
            client_buffer.clear(); // Vyèistìte buffer pøi kritické chybì
            return;
        }

        // Výpis zbývajících dat po vymazání z bufferu
        if (!client_buffer.empty()) {
            std::cout << "Remaining data in buffer after erasing: ";
            for (auto b : client_buffer) {
                std::cout << std::hex << static_cast<int>(b) << " ";
            }
            std::cout << std::dec << std::endl;
        }
    }
}

// Uzavøení pøipojení a odstranìní session
void Network::closeClient(int client_socket)
{
    std::lock_guard<std::mutex> lock(client_mutex);
    client_sessions.erase(client_socket);
    close(client_socket);

    std::cout << "Client socket " << client_socket << " closed." << std::endl;
}

