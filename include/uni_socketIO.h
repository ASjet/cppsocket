#ifndef UNI_SOCKETIO_H
#define UNI_SOCKETIO_H

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>

using sockd_t = SOCKET;
using tcb_t = void;
const sockd_t NULL_SOCKD(INVALID_SOCKET);

#ifdef _WIN64
// define something for Windows (64-bit only)
#else
// define something for Windows (32-bit only)
#endif

#elif __APPLE__
#error "Unsupported platform: macOS"
//#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR
// iOS Simulator
#elif TARGET_OS_IPHONE
// iOS device
#elif TARGET_OS_MAC
// Other kinds of Mac OS
#else
#error "Unknown Apple platform"
#endif

#elif defined(__linux__) || defined(__unix__)

#include <netinet/tcp.h>
using sockd_t = int;
using tcb_t = tcp_info;
constexpr sockd_t NULL_SOCKD(-1);

#else
#error "Unknown compiler"
#endif

#include "socket.h"

namespace socket {

sockd_t uni_socket(const ip_v _IPVersion, const proto_t _Protocol);
void uni_close(const sockd_t _Socket);
void uni_bind(const sockd_t _Socket, const port_t _Port, const ip_v _IPVersion,
              const proto_t _Protocol);
void uni_listen(const sockd_t _Socket, const std::size_t _ListenCount);
sockd_t uni_accept(const sockd_t _Socket, addr_t &_OppoAddr,
                   const ip_v _IPversion);
bool uni_connect(const sockd_t _Socket, const addr_t &_HostAddr,
                 addr_t &_OppoAddr, const ip_v _IPVersion,
                 const proto_t _Protocol);
std::size_t uni_send(const sockd_t _Socket, const byte *_DataBuffer,
                     const std::size_t _Length);
std::size_t uni_sendto(const sockd_t _Socket, const byte *_DataBuffer,
                       const std::size_t _Length, const addr_t &_HostAddr,
                       addr_t &_OppoAddr, const ip_v _IPVersion,
                       const proto_t _Protocol);
std::size_t uni_recv(const sockd_t _Socket, byte *_DataBuffer,
                     const std::size_t _BufferSize);
std::size_t uni_recvfrom(const sockd_t _Socket, byte *_DataBuffer,
                         const std::size_t _BufferSize, const addr_t &_HostAddr,
                         addr_t &_OppoAddr, const ip_v _IPVersion,
                         const proto_t _Protocol);
bool uni_isConnecting(const sockd_t sd);
bool uni_setub(const sockd_t _Socket);
const char *uni_strerr(const int _ErrorCode);

}


#endif