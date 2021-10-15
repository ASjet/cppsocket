#if defined(__linux__) || defined(__unix__)

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <netinet/tcp.h>
#include "Socket.h"
#include "uni_socketIO.h"
////////////////////////////////////////////////////////////////////////////////
#define AF(ipv) ((IPv4 == (ipv)) ? AF_INET : AF_INET6)
#define PROTO(type) ((TCP == (type)) ? IPPROTO_TCP : IPPROTO_UDP)
#define SOCK_TYPE(type) ((TCP == (type)) ? SOCK_STREAM : SOCK_DGRAM)
//////////////////////////////////////////////////////////////////////////////////
void sigint_handler(int sig)
{
    for (auto s = socket_list.begin(); s != socket_list.end(); ++s)
        s->closeSocket();
    exit(sig);
}

int ns(const char *host,
       const char *port,
       struct addrinfo **result,
       ipv_t ipv, conn_proto_t type)
{
    int errcode;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF(ipv);
    hints.ai_socktype = SOCK_TYPE(type);
    hints.ai_protocol = PROTO(type);
    if (nullptr == host)
        hints.ai_flags = AI_PASSIVE;
    if (0 != (errcode = getaddrinfo(host, port, &hints, result)))
    {
        fprintf(stderr,
                "ns: getaddrinfo(%d): %s: \"%s:%s\"",
                errcode,
                gai_strerror(errcode),
                ((nullptr == host) ? NULL_ADDRESS : host),
                ((nullptr == port) ? "*" : port));
        return -1;
    }
    return 0;
}

int getPeerAddr(struct sockaddr_in *addr, addr_t *peer, ipv_t ipv)
{
    char addr_buf[ADDRESS_BUFFER_SIZE];
    memset(addr_buf, 0, ADDRESS_BUFFER_SIZE);
    if (NULL == inet_ntop(AF(ipv), &(addr->sin_addr), addr_buf, ADDRESS_BUFFER_SIZE))
    {
        fprintf(stderr,
                "socketIOunix: getPeerAddr: inet_ntop(%d): %s",
                errno,
                strerror(errno));
        peer->port = NULL_PORT;
        peer->addr = std::string(NULL_ADDRESS);
        return -1;
    }
    peer->addr = std::string(addr_buf);
    peer->port = ntohs(addr->sin_port);
    return 0;
}
////////////////////////////////////////////////////////////////////////////////
void installSigIntHandler(void)
{
    signal(SIGINT, sigint_handler);
}
////////////////////////////////////////////////////////////////////////////////
sockfd_t uni_socket(ipv_t ipv, conn_proto_t type)
{
    int reuse = 1;
    sockfd_t fd;
    if (0 > (fd = socket(AF(ipv), SOCK_TYPE(type), PROTO(type))))
    {
        fprintf(stderr,
                "socketIOunix: uni_socket: socket(%d): %s\n",
                errno,
                strerror(errno));
        return FD_NULL;
    }
    if(0 > setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)))
    {
        close(fd);
        fprintf(stderr,
                "socketIOunix: uni_socket: setsockopt(%d): %s\n",
                errno,
                strerror(errno));
        return FD_NULL;
    }
    return fd;
}

bool is_open(sockfd_t fd)
{
    return (0 < fd);
}

void uni_close(sockfd_t fd)
{
    if (is_open(fd))
        close(fd);
}

int uni_bind(sockfd_t sock_fd, port_t port, ipv_t ipv, conn_proto_t type)
{
    int errcode;
    struct addrinfo *result = nullptr;
    if (-1 == ns(nullptr, std::to_string(port).c_str(), &result, ipv, type))
    {
        fprintf(stderr, " in socketIOwin: uni_bind\n");
        return -1;
    }

    if (-1 == bind(sock_fd, result->ai_addr, result->ai_addrlen))
    {
        fprintf(stderr,
                "socketIOunix: uni_bind: bind(%d): %s\n",
                errno,
                strerror(errno));
        freeaddrinfo(result);
        return -1;
    }
    freeaddrinfo(result);
    return 0;
}

int uni_listen(sockfd_t sock_fd, int cnt)
{
    if (-1 == listen(sock_fd, cnt))
    {
        fprintf(stderr,
                "socketIOunix: uni_listen: listen(%d): %s\n",
                errno,
                strerror(errno));
        return -1;
    }
    return 0;
}

sockfd_t uni_accept(sockfd_t sock_fd, addr_t *peer, ipv_t ipv)
{
    sockfd_t conn_fd;
    byte addr[SOCKADDR_BUFFER_SIZE];
    memset(addr, 0, SOCKADDR_BUFFER_SIZE);
    socklen_t len = SOCKADDR_BUFFER_SIZE;

    if (-1 == (conn_fd = accept(sock_fd, (struct sockaddr *)addr, &len)))
    {
        fprintf(stderr,
                "socketIOuniafx: uni_accept: accept(%d): %s\n",
                errno,
                strerror(errno));
        return FD_NULL;
    }

    if (SOCKADDR_BUFFER_SIZE == len)
    {
        fprintf(stderr,
                "socketIOunix: uni_accept: buffer is too small to held entile sockaddr(byte[%d])\n",
                SOCKADDR_BUFFER_SIZE);
        addr[SOCKADDR_BUFFER_SIZE - 1] = '\0';
    }
    if (-1 == getPeerAddr((struct sockaddr_in *)addr, peer, ipv))
        fprintf(stderr, " in socketIOunix: uni_accept\n");
    return conn_fd;
}

int uni_connect(sockfd_t sock_fd,
                std::string host,
                port_t port,
                addr_t *peer,
                ipv_t ipv,
                conn_proto_t type)
{
    struct addrinfo *result = nullptr, *p = nullptr;
    if (0 == ns(host.c_str(), std::to_string(port).c_str(), &result, ipv, type))
    {
        for (p = result; nullptr != p; p = p->ai_next)
        {
            if (-1 == connect(sock_fd, p->ai_addr, p->ai_addrlen))
                continue;
            else
            {
                if (-1 == getPeerAddr((struct sockaddr_in *)p->ai_addr, peer, ipv))
                    fprintf(stderr, " in socketIOunix: uni_connect\n");
                freeaddrinfo(result);
                return 0;
            }
            freeaddrinfo(result);
        }
        fprintf(stderr,
                "socketIOunix: uni_connect: connect: unable to connect to %s:%hu\n",
                host.c_str(),
                port);
    }
    else
        fprintf(stderr, " in socketIOunix: uni_connect\n");
    peer->addr = NULL_ADDRESS;
    peer->port = NULL_PORT;
    return -1;
}

ssize_t uni_send(sockfd_t sock_fd, const void *buf, ssize_t length)
{
    ssize_t cnt = send(sock_fd, buf, length, 0);
    if (-1 == cnt)
    {
        fprintf(stderr,
                "socketIOunix: uni_send: send(%d): %s\n",
                errno,
                strerror(errno));
        return 0;
    }
    return cnt;
}

ssize_t uni_sendto(sockfd_t sock_fd,
                   const void *buf,
                   ssize_t length,
                   std::string host,
                   port_t port,
                   addr_t *peer,
                   ipv_t ipv,
                   conn_proto_t type)
{
    ssize_t cnt;
    struct addrinfo *result = nullptr, *p = nullptr;
    if (0 == ns(host.c_str(), std::to_string(port).c_str(), &result, ipv, type))
    {
        for (p = result; nullptr != p; p = p->ai_next)
        {
            if (-1 == (cnt = sendto(sock_fd, (const char *)buf, length, 0, p->ai_addr, p->ai_addrlen)))
                continue;
            else
            {
                if (-1 == getPeerAddr((struct sockaddr_in *)p->ai_addr, peer, ipv))
                    fprintf(stderr, " in socketIOunix: uni_sendto\n");
                freeaddrinfo(result);
                return cnt;
            }
        }
        freeaddrinfo(result);
        fprintf(stderr,
                "socketIOunix: uni_sendto: send(%d): unable to send data to %s:%hu\n",
                cnt,
                host.c_str(),
                port);
    }
    else
        fprintf(stderr, " in socketIOunix: uni_sendto\n");
    peer->addr = NULL_ADDRESS;
    peer->port = NULL_PORT;
    return 0;
}

ssize_t uni_recv(sockfd_t sock_fd, void *buf, ssize_t size)
{
    ssize_t cnt;
    if (-1 == (cnt = recv(sock_fd, (char *)buf, size, 0)))
    {
        fprintf(stderr,
                "socketIOunix: uni_recv: recv(%d): unable to receive data\n",
                cnt);
        return 0;
    }
    return cnt;
}

ssize_t uni_recvfrom(sockfd_t sock_fd,
                     void *buf,
                     ssize_t size,
                     std::string host,
                     port_t port,
                     addr_t *peer,
                     ipv_t ipv,
                     conn_proto_t type)
{
    ssize_t cnt;
    struct addrinfo *result = nullptr, *p = nullptr;
    if (0 == ns(host.c_str(), std::to_string(port).c_str(), &result, ipv, type))
    {
        memset(buf, 0, size);
        for (p = result; nullptr != p; p = p->ai_next)
        {
            if (-1 == (cnt = recvfrom(sock_fd, buf, size, 0, p->ai_addr, &p->ai_addrlen)))
                continue;
            else
            {
                if (-1 == getPeerAddr((struct sockaddr_in *)p->ai_addr, peer, ipv))
                    fprintf(stderr, " in socketIOunix: uni_recvfrom\n");
                freeaddrinfo(result);
                return cnt;
            }
        }
        freeaddrinfo(result);
        fprintf(stderr,
                "socketIOunix: uni_recvfrom: recvfrom(%d): unable to receive data from %s:%hu\n",
                cnt,
                host.c_str(),
                port);
    }
    else
        fprintf(stderr, " in socketIOunix: uni_recvfrom\n");
    peer->addr = NULL_ADDRESS;
    peer->port = NULL_PORT;
    return 0;
}

bool uni_isConnecting(sockfd_t sock_fd)
{
    struct tcp_info info;
    memset(&info, 0, sizeof(info));
    size_t len = sizeof(info);
    getsockopt(sock_fd, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len);
    return (info.tcpi_state == TCP_ESTABLISHED);
}

#endif
