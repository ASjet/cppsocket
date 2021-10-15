#ifndef UNI_SOCKETIO_H
#define UNI_SOCKETIO_H

#include <cstdio>
#include "Socket.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
//define something for Windows (32-bit and 64-bit, this part is common)

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>

#define ADDRESS_BUFFER_SIZE 32
#define SOCKADDR_BUFFER_SIZE 128
#define FD_NULL INVALID_SOCKET

typedef long ssize_t;
typedef SOCKET sockfd_t;

#ifdef _WIN64
//define something for Windows (64-bit only)
#else
//define something for Windows (32-bit only)
#endif

#elif __APPLE__
#include <TargetConditionals.h>
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

#include <arpa/inet.h>
#define SOCKET_BUFFER_SIZE 2048
#define ADDRESS_BUFFER_SIZE 32
#define SOCKADDR_BUFFER_SIZE 128
#define FD_NULL -1

typedef int sockfd_t;


#else
#error "Unknown compiler"
#endif

void installSigIntHandler(void);
sockfd_t uni_socket(ipv_t ipv, conn_proto_t type);
bool is_open(sockfd_t sock_fd);
void uni_close(sockfd_t fd);
int uni_bind(sockfd_t sock_fd, port_t port, ipv_t ipv, conn_proto_t type);
int uni_listen(sockfd_t sock_fd, int cnt);
sockfd_t uni_accept(sockfd_t sock_fd, addr_t *peer, ipv_t ipv);
int uni_connect(sockfd_t sock_fd, std::string host, port_t port, addr_t *peer, ipv_t ipv, conn_proto_t type);
ssize_t uni_send(sockfd_t sock_fd, const void *buf, ssize_t length);
ssize_t uni_sendto(sockfd_t sock_fd, const void *buf, ssize_t length, std::string host, port_t port, addr_t *peer, ipv_t ipv, conn_proto_t type);
ssize_t uni_recv(sockfd_t sock_fd, void *buf, ssize_t size);
ssize_t uni_recvfrom(sockfd_t sock_fd, void *buf, ssize_t size, std::string host, port_t port, addr_t *peer, ipv_t ipv, conn_proto_t type);
bool uni_isConnecting(sockfd_t sock_fd);

#endif