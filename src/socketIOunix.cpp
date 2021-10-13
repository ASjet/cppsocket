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
#include <pthread.h>
#include <netinet/tcp.h>
#include "Socket.h"
#include "uni_socketIO.h"
////////////////////////////////////////////////////////////////////////////////
#define DOMAIN(ipv) (IPv4 == ipv) ? AF_INET : AF_INET6
#define SOCK_TYPE(type) (TCP == type) ? SOCK_STREAM : SOCK_DGRAM
//////////////////////////////////////////////////////////////////////////////////
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
int ns(std::string host, std::vector<struct sockaddr_in> &lst)
{
    struct addrinfo *listp, *p, hints;
    char addr_buf[ADDRESS_BUFFER_SIZE];
    int errcode;
    memset(&hints, 0, sizeof(hints));
    memset(addr_buf, 0, ADDRESS_BUFFER_SIZE);
    if (0 != (errcode = getaddrinfo(host.c_str(), nullptr, &hints, &listp)))
    {
        fprintf(stderr, "socketIOunix: ns: getaddrinfo(%d): %s\n", errcode, gai_strerror(errcode));
        return errcode;
    }
    for (p = listp; p; p = p->ai_next)
        lst.push_back(*(struct sockaddr_in *)p->ai_addr);
    freeaddrinfo(listp);
    return 0;
}

int getPeerAddr(byte *addr, sock_info *peer, ipv_t ipv)
{
    int domain = DOMAIN(ipv);
    char addr_buf[ADDRESS_BUFFER_SIZE];
    memset(addr_buf, 0, ADDRESS_BUFFER_SIZE);
    if (NULL == inet_ntop(domain, &((struct sockaddr_in *)addr)->sin_addr, addr_buf, ADDRESS_BUFFER_SIZE))
    {
        fprintf(stderr, "socketIOunix: getPeerAddr: inet_ntop(%d): %s\n", errno, strerror(errno));
        peer->port = NULL_PORT;
        peer->address = std::string(NULL_ADDRESS);
        return -1;
    }
    peer->port = ntohs(((struct sockaddr_in *)addr)->sin_port);
    peer->address = std::string(addr_buf);
    return 0;
}
////////////////////////////////////////////////////////////////////////////////
void installSigHandler(void)
{
    static pthread_once_t once = PTHREAD_ONCE_INIT;
    pthread_once(&once, install_sig_handle);
}
bool is_open(sockfd_t sock_fd)
{
    return (sock_fd > 0);
}
////////////////////////////////////////////////////////////////////////////////
int uni_close(sockfd_t fd)
{
    return close(fd);
}

sockfd_t uni_socket(ipv_t ipv, conn_proto_t type)
{
    int proto = (IPv4 == ipv) ? IPPROTO_IP : IPPROTO_IPV6;
    int sock_t = SOCK_TYPE(type);
    int domain = DOMAIN(ipv);
    int fd;
    if (0 > (fd = socket(domain, sock_t, proto)))
    {
        fprintf(stderr, "socketIOunix: uni_socket: socket(%d): %s\n", errno, strerror(errno));
        return -1;
    }
    return fd;
}

int uni_bind(sockfd_t sock_fd, port_t port, ipv_t ipv)
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = DOMAIN(ipv);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    if (-1 == bind(sock_fd, (struct sockaddr *)&addr, sizeof(addr)))
    {
        fprintf(stderr, "socketIOunix: uni_bind: bind(%d): %s\n", errno, strerror(errno));
        return -1;
    }
    return 0;
}

int uni_listen(sockfd_t sock_fd, int cnt)
{
    if (-1 == listen(sock_fd, cnt))
    {
        fprintf(stderr, "socketIOunix: uni_listen: listen(%d): %s\n", errno, strerror(errno));
        return -1;
    }
    return 0;
}

sockfd_t uni_accept(sockfd_t sock_fd, sock_info *peer, ipv_t ipv)
{
    int conn_fd;
    byte addr[SOCKADDR_BUFFER_SIZE];
    memset(addr, 0, sizeof(addr));
    socklen_t len;

    if (-1 == (conn_fd = accept(sock_fd, (struct sockaddr *)addr, &len)))
    {
        fprintf(stderr, "socketIOunix: uni_accept: accept(%d): %s\n", errno, strerror(errno));
        return -1;
    }

    if (len == SOCKADDR_BUFFER_SIZE)
    {
        fprintf(stderr, "socketIOunix: uni_accept: sockaddr is too large to be held by addr(char[%d]).\n", SOCKADDR_BUFFER_SIZE);
        addr[SOCKADDR_BUFFER_SIZE - 1] = '\0';
    }
    getPeerAddr(addr, peer, ipv);
    return conn_fd;
}

int uni_connect(sockfd_t sock_fd, std::string host, port_t port, sock_info *peer, ipv_t ipv)
{
    int errcode;
    ssize_t cnt;
    int domain = DOMAIN(ipv);
    byte *addr;
    std::vector<struct sockaddr_in> addr_list;
    if (0 != (errcode = ns(host, addr_list)))
        return errcode;

    for (auto i = addr_list.begin(); i != addr_list.end(); ++i)
    {
        i->sin_port = htons(port);
        i->sin_family = domain;
        addr = (byte *)&*i;
        cnt = sizeof(*i);
        if (-1 == connect(sock_fd, (struct sockaddr *)addr, cnt))
            fprintf(stderr, "socketIOunix: uni_connect: connect(%d): %s\nRetrying...\n", errno, strerror(errno));
        else
            return getPeerAddr(addr, peer, ipv);
    }
    fprintf(stderr, "socketIOunix: uni_connect: cannot establish connection to %s:%d\n", host.c_str(), port);
    return -1;
}

int uni_send(sockfd_t sock_fd, const void *buf, int length)
{
    return send(sock_fd, buf, length, 0);
}

int uni_sendto(sockfd_t sock_fd, const void *buf, int length, std::string host, port_t port)
{
    int errcode;
    ssize_t cnt;
    std::vector<struct sockaddr_in> addr_list;
    if (0 != (errcode = ns(host, addr_list)))
        return errcode;

    for (auto p = addr_list.begin(); p != addr_list.end(); ++p)
    {
        p->sin_port = htons(port);
        if (0 == (cnt = sendto(sock_fd, buf, length, 0, (struct sockaddr *)&*p, sizeof(*p))))
            fprintf(stderr, "socketIOunix: uni_sendto: sendto(%d): %s\nRetrying...\n", errno, strerror(errno));
        else
            break;
    }

    if (cnt == 0)
        fprintf(stderr, "socketIOunix: uni_sendto: unable to send data to %s:%d\n", host.c_str(), port);

    return cnt;
}

int uni_recv(sockfd_t sock_fd, void *buf, int size, conn_proto_t type, sock_info *peer, ipv_t ipv)
{
    int sock_type = SOCK_TYPE(type);
    socklen_t len;
    byte addr[SOCKADDR_BUFFER_SIZE];
    memset(buf, 0, size);
    memset(addr, 0, SOCKADDR_BUFFER_SIZE);
    int cnt = (sock_type == SOCK_STREAM) ? recv(sock_fd, buf, size - 1, 0) : recvfrom(sock_fd, buf, size - 1, 0, (struct sockaddr *)addr, &len);
    if (-1 == cnt)
    {
        if (errno == EINTR)
            return 0;
        else
        {
            fprintf(stderr, "socketIOunix: uni_recv: recv(-1): unable to receive data\n");
            return -1;
        }
    }
    getPeerAddr(addr, peer, ipv);
    return cnt;
}

bool uni_isConnecting(sockfd_t sock_fd)
{
    struct tcp_info info;
    memset(&info, 0, sizeof(info));
    int len = sizeof(info);
    getsockopt(sock_fd, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len);
    return (info.tcpi_state == TCP_ESTABLISHED);
}