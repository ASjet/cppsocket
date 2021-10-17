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
typedef int lock_t;
typedef int rwlock_t;

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
#include <pthread.h>
#define SOCKET_BUFFER_SIZE 2048
#define ADDRESS_BUFFER_SIZE 32
#define SOCKADDR_BUFFER_SIZE 128
#define FD_NULL -1

typedef int sockfd_t;
typedef pthread_mutex_t lock_t;
typedef pthread_rwlock_t rwlock_t;


#else
#error "Unknown compiler"
#endif

int initlock(lock_t *_Lock);
int lock(lock_t *_Lock);
int unlock(lock_t *_Lock);
int destroylock(lock_t *_Lock);
int initrwlock(rwlock_t * _RWlock);
int rlock(rwlock_t * _RWlock);
int wlock(rwlock_t * _RWlock);
int unrwlock(rwlock_t * _RWlock);
int destroyrwlock(rwlock_t * _RWlock);
void installSigIntHandler(void);
sockfd_t uni_socket(ipv_t ipv, type_t type);
bool is_open(sockfd_t sock_fd);
void uni_close(sockfd_t fd);
int uni_bind(sockfd_t sock_fd, port_t port, ipv_t ipv, type_t type);
int uni_listen(sockfd_t sock_fd, int cnt);
sockfd_t uni_accept(sockfd_t sock_fd, addr_t &peer, ipv_t ipv);
int uni_connect(sockfd_t sock_fd, std::string host, port_t port, addr_t &peer, ipv_t ipv, type_t type);
size_t uni_send(sockfd_t sock_fd, const void *buf, size_t length);
size_t uni_sendto(sockfd_t sock_fd, const void *buf, size_t length, std::string host, port_t port, addr_t &peer, ipv_t ipv, type_t type);
size_t uni_recv(sockfd_t sock_fd, void *buf, size_t size);
size_t uni_recvfrom(sockfd_t sock_fd, void *buf, size_t size, std::string host, port_t port, addr_t &peer, ipv_t ipv, type_t type);
bool uni_isConnecting(sockfd_t sock_fd);
int uni_setub(sockfd_t sock_fd);

#endif