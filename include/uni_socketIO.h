#ifndef UNI_SOCKETIO_H
#define UNI_SOCKETIO_H

#include "Socket.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
//define something for Windows (32-bit and 64-bit, this part is common)
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

#elif __linux__ || __unix__

#define SOCKET_BUFFER_SIZE 2048
#define ADDRESS_BUFFER_SIZE 16
#define SOCKADDR_BUFFER_SIZE 128
#define FD_NULL -1

typedef int sockfd_t;

void installSigHandler(void);
bool is_open(sockfd_t sock_fd);

#else
#error "Unknown compiler"
#endif

int uni_close(sockfd_t fd);
sockfd_t uni_socket(ipv_t ipv, conn_proto_t type);
int uni_bind(sockfd_t sock_fd, port_t port, ipv_t ipv);
int uni_listen(sockfd_t sock_fd, int cnt);
sockfd_t uni_accept(sockfd_t sock_fd, sock_info *peer, ipv_t ipv);
int uni_connect(sockfd_t sock_fd, std::string host, port_t port, sock_info *peer, ipv_t ipv);
int uni_send(sockfd_t sock_fd, const void *buf, int length);
int uni_sendto(sockfd_t sock_fd, const void *buf, int length, std::string host, port_t port);
int uni_recv(sockfd_t sock_fd, void *buf, int size, conn_proto_t type, sock_info *peer, ipv_t ipv);
bool uni_isConnecting(sockfd_t sock_fd);

#endif