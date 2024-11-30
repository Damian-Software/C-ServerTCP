#pragma once
#ifndef SEND_DATA_H
#define SEND_DATA_H
#include <cstdint>
#include <sys/socket.h> // Nahrazení winsock2.h
#include <unistd.h>     // Potøeba pro close()

size_t sendData(int socket, const uint8_t* buffer, size_t length);

#endif // SEND_DATA_H
