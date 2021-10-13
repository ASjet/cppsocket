#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <list>
#include <netdb.h>
#include <signal.h>
#include <pthread.h>
#include <netinet/tcp.h>
#include "Socket.h"

std::list<Socket> socket_list;
////////////////////////////////////////////////////////////////////////////////
void closeAllSockets(int sig)
{
    for (auto s = socket_list.begin(); s != socket_list.end(); ++s)
    {
        s->closeSocket();
    }
    exit(sig);
}
static void install_sig_handle(void)
{
    signal(SIGINT, closeAllSockets);
}
void Socket::disconnect(void)
{
    if (conn_fd > 0)
    {
        shutdown(conn_fd, SHUT_RDWR);
        close(conn_fd);
    }
    conn_fd = -1;
}
void Socket::closeSocket()
{
    disconnect();
    if (sock_fd > 0)
    {
        shutdown(sock_fd, SHUT_RDWR);
        close(sock_fd);
    }
    sock_fd = -1;
}
int ns(std::string host, std::list<struct sockaddr_in> &lst)
{
    struct addrinfo *listp, *p, hints;
    char addr_buf[ADDRESS_BUFFER_SIZE];
    int errcode;
    bzero(&hints, sizeof(hints));
    if (0 != (errcode = getaddrinfo(host.c_str(), nullptr, &hints, &listp)))
    {
        return errcode;
    }
    for (p = listp; p; p = p->ai_next)
        lst.push_back(*(struct sockaddr_in *)p->ai_addr);
    freeaddrinfo(listp);
    return 0;
}

Socket::Socket(ipv_t _IPVersion, conn_proto_t _ConnectType)
{
    int proto = (IPv4 == _IPVersion) ? IPPROTO_IP : IPPROTO_IPV6;
    int type = (TCP == _ConnectType) ? SOCK_STREAM : SOCK_DGRAM;
    domain = (IPv4 == _IPVersion) ? AF_INET : AF_INET6;
    protocol = _ConnectType;
    if (0 > (sock_fd = socket(domain, type, proto)))
    {
        fprintf(stderr, "Socket: Socket: socket(%d): %s\n", errno, strerror(errno));
        exit(-1);
    }
    // Install signal handler for SIGINT
    static pthread_once_t once = PTHREAD_ONCE_INIT;
    pthread_once(&once, install_sig_handle);
    socket_list.push_back(*this);
}
Socket::~Socket()
{
    closeSocket();
}

int Socket::bindTo(port_t port)
{
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = domain;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    if (-1 == bind(sock_fd, (struct sockaddr *)&addr, sizeof(addr)))
    {
        fprintf(stderr, "Socket: bindTo: bind(%d): %s\n", errno, strerror(errno));
        return -1;
    }
    return 0;
}

int Socket::listenOn(int cnt)
{
    if (-1 == listen(sock_fd, cnt))
    {
        fprintf(stderr, "Socket: listenOn: listen(%d): %s\n", errno, strerror(errno));
        return -1;
    }
    return 0;
}

int Socket::acceptFrom(void)
{
    bzero(addr, SOCKADDR_BUFFER_SIZE);
    socklen_t len;
    if (-1 == (conn_fd = accept(sock_fd, (struct sockaddr *)addr, &len)))
    {
        fprintf(stderr, "Socket: acceptFrom: accept(%d): %s\n", errno, strerror(errno));
        return -1;
    }
    if (len == SOCKADDR_BUFFER_SIZE)
    {
        fprintf(stderr, "Socket: acceptFrom: sockaddr is too large to be held by addr(char[%d]).\n", SOCKADDR_BUFFER_SIZE);
        addr[SOCKADDR_BUFFER_SIZE - 1] = '\0';
    }
    return 0;
}

int Socket::connectTo(std::string host, port_t port)
{
    std::list<struct sockaddr_in> addr_list;
    int errcode, cnt;
    byte * p, * add = addr;
    if (0 != (errcode = ns(host, addr_list)))
    {
        fprintf(stderr, "Socket: connectTo: ns(%d): %s\n", errcode, gai_strerror(errcode));
        return errcode;
    }
    for (auto i = addr_list.begin(); i != addr_list.end(); ++i)
    {
        i->sin_port = htons(port);
        i->sin_family = domain;
        p = (byte *)&*i;
        cnt = sizeof(*i);
        if (-1 == connect(sock_fd, (struct sockaddr *)p, cnt))
        {
            fprintf(stderr, "Socket: connectTo: connect(%d): %s\nRetrying...\n", errno, strerror(errno));
        }
        else
        {
            bzero(addr, ADDRESS_BUFFER_SIZE);
            while(cnt--)
                *add++ = *p++;
            return 0;
        }
    }
    fprintf(stderr, "Socket: connectTo: cannot establish connection to %s:%d\n", host.c_str(), port);
    return -1;
}

ssize_t Socket::sendData(const void *buf, int size)
{
    int sd = (conn_fd > 0) ? conn_fd : sock_fd;
    int cnt = send(sd, buf, size, 0);
    if(cnt < size)
    {
        fprintf(stderr, "Socket: sendData: send: Only %d/%d byte(s) data is sent.\n", cnt, size);
    }
    return cnt;
}

ssize_t Socket::sendTo(const void *buf, int size, std::string host, port_t port)
{
    std::list<struct sockaddr_in> addr_list;
    int errcode;
    ssize_t cnt;
    if (0 != (errcode = ns(host, addr_list)))
    {
        fprintf(stderr, "Socket: sendTo: ns(%d): %s\n", errcode, gai_strerror(errcode));
        return 0;
    }
    for (auto p = addr_list.begin(); p != addr_list.end(); ++p)
    {
        p->sin_port = htons(port);
        if (0 == (cnt = sendto(sock_fd, buf, size, 0, (struct sockaddr *)&*p, sizeof(*p))))
        {
            fprintf(stderr, "Socket: sendTo: sendto(%d): %s\nRetrying...\n", errno, strerror(errno));
        }
        else
        {
            break;
        }
    }
    if (cnt == 0)
        fprintf(stderr, "Socket: sendTo: cannot send data to %s:%d\n", host.c_str(), port);
    if(cnt != size)
        fprintf(stderr, "Socket: sendTo: only %d/%d Byte(s) sent.\n", cnt, size);
    return cnt;
}

ssize_t Socket::recvData(void *buf, int size)
{
    bzero(addr, SOCKADDR_BUFFER_SIZE);
    socklen_t len;
    int sd = (conn_fd > 0) ? conn_fd : sock_fd;
    int cnt = (protocol == SOCK_STREAM) ? recv(sd, buf, size - 1, 0) : recvfrom(sd, buf, size - 1, 0, (struct sockaddr *)addr, &len);
    if (-1 == cnt)
    {
        if (errno == EINTR)
            return cnt;
        else
        {
            fprintf(stderr, "Socket: recvData: recvfrom(-1): error\n");
            disconnect();
        }
    }
    return cnt;
}

sock_info Socket::peerAddr(void)
{
    char addr_buf[ADDRESS_BUFFER_SIZE];
    if (NULL == inet_ntop(domain, &((struct sockaddr_in *)addr)->sin_addr, addr_buf, ADDRESS_BUFFER_SIZE))
    {
        fprintf(stderr, "Socket: info: inet_ntop(%d): %s\n", errno, strerror(errno));
        return {"", 0};
    }
    port_t port = ntohs(((struct sockaddr_in *)addr)->sin_port);
    return {std::string(addr_buf), port};
}

bool Socket::isConnecting(void)
{
    struct tcp_info info;
    int len = sizeof(info);
    int sd = (conn_fd > 0) ? conn_fd : sock_fd;
    getsockopt(sd, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len);
    return (info.tcpi_state == TCP_ESTABLISHED);
}

int Socket::socketnum(void)
{
    return sock_fd;
}