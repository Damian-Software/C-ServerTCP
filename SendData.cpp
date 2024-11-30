#include "SendData.h"
#include <stdexcept>
#include <limits>
#include <sys/socket.h> // Nahrazení winsock2.h
#include <unistd.h>     // Potøeba pro close()
#include <climits>

size_t sendData(int socket, const uint8_t* buffer, size_t length) 
{
    if (length > static_cast<size_t>(INT_MAX)) 
    {
        throw std::runtime_error("Data length exceeds maximum value for send function");
    }

    ssize_t bytes_sent = send(socket, reinterpret_cast<const char*>(buffer), static_cast<int>(length), 0);
    if (bytes_sent < 0) 
    {
        throw std::runtime_error("Failed to send data to socket");
    }

    return static_cast<size_t>(bytes_sent);
}
