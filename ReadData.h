#pragma once
#pragma once
#ifndef READ_DATA_H
#define READ_DATA_H
#include <cstdint>
#include <sys/socket.h> // Nahrazen� winsock2.h
#include <unistd.h>     // Pot�eba pro close()

size_t readData(int socket, uint8_t* buffer, size_t length);

#endif // READ_DATA_H

