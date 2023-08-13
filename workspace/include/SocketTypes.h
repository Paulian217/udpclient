#ifndef WORKSPACE_INCLUDE_SOCKETTYPES_H
#define WORKSPACE_INCLUDE_SOCKETTYPES_H

#include <cstdint>
#include <vector>

using SocketFD = int;
using SocketSize = size_t;
using SocketAddress = struct sockaddr_in;
using ByteBuffer = std::vector<unsigned char>;

constexpr const int INVALID_SOCKET = -1;
constexpr const int SOCKET_ERROR = -1;
constexpr const int SOCKET_EOF = 0;
constexpr const int SOCKET_NO_OPTION = 0;
constexpr const SocketSize MAX_BUFFER = 65535;

enum class SocketReturn { SUCCESS, FAILED, EOF, MAX };
enum class SocketStatus { IDLE, CONNECTING, CONNECTED, RUNNING, DISCONNECTING, MAX };

#endif  // WORKSPACE_INCLUDE_SOCKETTYPES_H