#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#define WIN32_LEAN_AND_MEAN
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <signal.h>
#include "Socket.h"
#include "uni_socketIO.h"
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")
////////////////////////////////////////////////////////////////////////////////
#define AF(ipv) ((IPv4 == (ipv)) ? AF_INET : AF_INET6)
int PROTO(type_t type)
{
    switch (type)
    {
    TCP:
        return IPPROTO_TCP;
    UDP:
        return IPPROTO_UDP;
    SCTP:
        return IPPROTO_SCTP;
    default:
        return 0;
    }
}
int SOCK_TYPE(type_t type)
{
    switch (type)
    {
    TCP:
        return SOCK_STREAM;
    UDP:
        return SOCK_DGRAM;
    SCTP:
        return SOCK_SEQPACKET;
    RAW:
        return SOCK_RAW;
    default:
        return 0;
    }
}
////////////////////////////////////////////////////////////////////////////////
void sigint_handler(int sig)
{
    for (auto s = socket_list.begin(); s != socket_list.end(); ++s)
        s->closeSocket();
    WSACleanup();
    exit(sig);
}
int ns(const char *host,
       const char *port,
       struct addrinfo **result,
       ipv_t ipv, type_t type)
{
    int iResult;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF(ipv);
    hints.ai_socktype = SOCK_TYPE(type);
    hints.ai_protocol = PROTO(type);
    if (nullptr == host)
        hints.ai_flags = AI_PASSIVE;
    if (0 != (iResult = getaddrinfo(host, port, &hints, result)))
    {
        fprintf(stderr,
                "ns: getaddrinfo(%d): %s:%s",
                iResult,
                ((nullptr == host) ? NULL_ADDRESS : host),
                ((nullptr == port) ? "*" : port));
        return -1;
    }
    return 0;
}
int getPeerAddr(struct sockaddr_in *addr, addr_t &peer, ipv_t ipv)
{
    char addr_buf[ADDRESS_BUFFER_SIZE];
    memset(addr_buf, 0, ADDRESS_BUFFER_SIZE);
    if (NULL == inet_ntop(AF(ipv), &(addr->sin_addr), addr_buf, ADDRESS_BUFFER_SIZE))
    {
        fprintf(stderr,
                "inet_ntop(%d)",
                WSAGetLastError());
        peer.addr = NULL_ADDRESS;
        peer.port = NULL_PORT;
        return -1;
    }
    peer.addr = std::string(addr_buf);
    peer.port = ntohs(addr->sin_port);
    return 0;
}
////////////////////////////////////////////////////////////////////////////////
void installSigIntHandler(void)
{
    signal(SIGINT, sigint_handler);
}
int initlock(lock_t *_Lock)
{
    return 0;
}
int lock(lock_t *_Lock)
{
    return 0;
}
int unlock(lock_t *_Lock)
{
    return 0;
}
int destroylock(lock_t *_Lock)
{
    return 0;
}
int initrwlock(rwlock_t *_RWlock)
{
    return 0;
}
int rlock(rwlock_t *_RWlock)
{
    return 0;
}
int wlock(rwlock_t *_RWlock)
{
    return 0;
}
int unrwlock(rwlock_t *_RWlock)
{
    return 0;
}
int destroyrwlock(rwlock_t *_RWlock)
{
    return 0;
}
int uni_setub(sockfd_t sock_fd)
{
    return 0;
}
////////////////////////////////////////////////////////////////////////////////
sockfd_t uni_socket(ipv_t ipv, type_t type)
{
    int iResult;
    WSADATA wsaData;

    sockfd_t sockfd = FD_NULL;

    // Initialize Winsock
    if (0 != (iResult = WSAStartup(MAKEWORD(2, 2), &wsaData)))
    {
        fprintf(stderr,
                "socketIOwin: uni_socket: WSAStartup(%d)\n",
                iResult);
        return FD_NULL;
    }

    // Create a SOCKET
    if (INVALID_SOCKET == (sockfd = socket(AF(ipv), SOCK_TYPE(type), PROTO(type))))
    {
        fprintf(stderr,
                "socketIOwin: uni_socket: socket(%d)\n",
                WSAGetLastError());
        return FD_NULL;
    }

    BOOL bOptVal = FALSE;
    int bOptLen = sizeof(BOOL);
    iResult = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&bOptVal, bOptLen);
    if (iResult == SOCKET_ERROR)
    {
        wprintf(L"setsockopt for SO_KEEPALIVE failed with error: %u\n", WSAGetLastError());
    }

    return sockfd;
}

bool is_open(sockfd_t fd)
{
    return (FD_NULL != fd);
}

void uni_close(sockfd_t fd)
{
    if (is_open(fd))
        closesocket(fd);
}

int uni_bind(sockfd_t sock_fd, port_t port, ipv_t ipv, type_t type)
{
    struct addrinfo *result = nullptr;
    if (-1 == ns(nullptr, std::to_string(port).c_str(), &result, ipv, type))
    {
        fprintf(stderr, " in socketIOwin: uni_bind\n");
        return -1;
    }

    if (SOCKET_ERROR == bind(sock_fd, result->ai_addr, (int)result->ai_addrlen))
    {
        fprintf(stderr,
                "socketIOwin: uni_bind: bind(%d)\n",
                WSAGetLastError());
        freeaddrinfo(result);
        return -1;
    }

    freeaddrinfo(result);
    return 0;
}

int uni_listen(sockfd_t sock_fd, int cnt)
{
    if (SOCKET_ERROR == listen(sock_fd, cnt))
    {
        fprintf(stderr,
                "socketIOwin: uni_listen: listen(%d)\n",
                WSAGetLastError());
        return -1;
    }
    return 0;
}

sockfd_t uni_accept(sockfd_t sock_fd, addr_t &peer, ipv_t ipv)
{
    sockfd_t conn_fd;
    byte addr[SOCKADDR_BUFFER_SIZE];
    memset(addr, 0, SOCKADDR_BUFFER_SIZE);
    int len = SOCKADDR_BUFFER_SIZE;

    if (INVALID_SOCKET == (conn_fd = accept(sock_fd, (struct sockaddr *)addr, &len)))
    {
        if (WSAEINTR != WSAGetLastError())
            fprintf(stderr,
                    "socketIOwin: uni_accept: accept(%d)\n",
                    WSAGetLastError());
        return FD_NULL;
    }

    if (SOCKADDR_BUFFER_SIZE == len)
    {
        fprintf(stderr,
                "socketIOwin: uni_accept: buffer is too small to held entile sockaddr(byte[%d])\n",
                SOCKADDR_BUFFER_SIZE);
        addr[SOCKADDR_BUFFER_SIZE - 1] = '\0';
    }
    if (-1 == getPeerAddr((struct sockaddr_in *)addr, peer, ipv))
        fprintf(stderr, " in socketIOwin: uni_accept\n");
    return conn_fd;
}

int uni_connect(sockfd_t sock_fd,
                std::string host,
                port_t port,
                addr_t &peer,
                ipv_t ipv,
                type_t type)
{
    struct addrinfo *result = nullptr, *p = nullptr;
    if (0 == ns(host.c_str(), std::to_string(port).c_str(), &result, ipv, type))
    {
        for (p = result; nullptr != p; p = p->ai_next)
        {
            if (SOCKET_ERROR == connect(sock_fd, p->ai_addr, (int)p->ai_addrlen))
                continue;
            else
            {
                if (-1 == getPeerAddr((struct sockaddr_in *)p->ai_addr, peer, ipv))
                    fprintf(stderr, " in socketIOwin: uni_connect\n");
                freeaddrinfo(result);
                return 0;
            }
        }
        freeaddrinfo(result);
        fprintf(stderr,
                "socketIOwin: uni_connect: connect: unable to connect to %s:%hu\n",
                host.c_str(),
                port);
    }
    else
        fprintf(stderr, " in socketIOwin: uni_connect\n");
    peer.addr = NULL_ADDRESS;
    peer.port = NULL_PORT;
    return -1;
}

size_t uni_send(sockfd_t sock_fd, const void *buf, size_t length)
{
    int cnt = send(sock_fd, (const char *)buf, length, 0);
    if (SOCKET_ERROR == cnt)
    {
        fprintf(stderr,
                "socketIOwin: uni_send: send(%d): unable to send data\n",
                WSAGetLastError());
        return 0;
    }
    return cnt;
}

size_t uni_sendto(sockfd_t sock_fd,
                  const void *buf,
                  size_t length,
                  std::string host,
                  port_t port,
                  addr_t &peer,
                  ipv_t ipv,
                  type_t type)
{
    int cnt;
    struct addrinfo *result = nullptr, *p = nullptr;
    if (0 == ns(host.c_str(), std::to_string(port).c_str(), &result, ipv, type))
    {
        for (p = result; nullptr != p; p = p->ai_next)
        {
            if (SOCKET_ERROR == (cnt = sendto(sock_fd, (const char *)buf, length, 0, p->ai_addr, (int)p->ai_addrlen)))
                continue;
            else
            {
                if (-1 == getPeerAddr((struct sockaddr_in *)p->ai_addr, peer, ipv))
                    fprintf(stderr, " in socketIOwin: uni_sendto\n");
                freeaddrinfo(result);
                return cnt;
            }
        }
        freeaddrinfo(result);
        fprintf(stderr,
                "socketIOwin: uni_sendto: send(%d): unable to send data to %s:%hu\n",
                cnt,
                host.c_str(),
                port);
    }
    else
        fprintf(stderr, " in socketIOwin: uni_sendto\n");
    peer.addr = NULL_ADDRESS;
    peer.port = NULL_PORT;
    return 0;
}

size_t uni_recv(sockfd_t sock_fd, void *buf, size_t size)
{
    int cnt;
    if (SOCKET_ERROR == (cnt = recv(sock_fd, (char *)buf, size, 0)))
    {
        fprintf(stderr,
                "socketIOwin: uni_recv: recv(%d): unable to receive data\n",
                cnt);
        return 0;
    }
    return cnt;
}

size_t uni_recvfrom(sockfd_t sock_fd,
                    void *buf,
                    size_t size,
                    std::string host,
                    port_t port,
                    addr_t &peer,
                    ipv_t ipv,
                    type_t type)
{
    int cnt;
    struct addrinfo *result = nullptr, *p = nullptr;
    if (0 == ns(host.c_str(), std::to_string(port).c_str(), &result, ipv, type))
    {
        for (p = result; nullptr != p; p = p->ai_next)
        {
            if (SOCKET_ERROR == (cnt = recvfrom(sock_fd, (char *)buf, size, 0, p->ai_addr, (int *)&p->ai_addrlen)))
                continue;
            else
            {
                if (-1 == getPeerAddr((struct sockaddr_in *)p->ai_addr, peer, ipv))
                    fprintf(stderr, " in socketIOwin: uni_recvfrom\n");
                freeaddrinfo(result);
                return cnt;
            }
        }
        freeaddrinfo(result);
        fprintf(stderr,
                "socketIOwin: uni_recv: recvfrom(%d): unable to receive data from %s:%hu\n",
                cnt,
                host.c_str(),
                port);
    }
    else
        fprintf(stderr, " in socketIOwin: uni_recvfrom\n");
    peer.addr = NULL_ADDRESS;
    peer.port = NULL_PORT;
    return 0;
}

bool uni_isConnecting(sockfd_t sock_fd)
{
    return (FD_NULL != sock_fd);
}

#endif