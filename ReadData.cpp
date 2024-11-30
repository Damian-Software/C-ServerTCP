#include "ReadData.h"
#include <stdexcept>
#include <limits>
#include <sys/socket.h> // Nahrazení winsock2.h
#include <unistd.h>     // Potøeba pro close()
#include <climits>

size_t readData(int socket, uint8_t* buffer, size_t length) 
{
    if (length > static_cast<size_t>(INT_MAX)) 
    {
        throw std::runtime_error("Data length exceeds maximum value for recv function");
    }

    ssize_t bytes_received = recv(socket, reinterpret_cast<char*>(buffer), static_cast<int>(length), 0);

    if (bytes_received < 0) 
    {
        throw std::runtime_error("Failed to receive data from socket");
    }

    return static_cast<size_t>(bytes_received);
}
