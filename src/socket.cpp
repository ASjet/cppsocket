#include <cstdio>
#include <vector>
#include "Socket.h"
#include "uni_socketIO.h"
std::vector<Socket> socket_list;
////////////////////////////////////////////////////////////////////////////////
void Socket::disconnect(void)
{
    if (is_open(conn_fd))
        uni_close(conn_fd);
    conn_fd = FD_NULL;
}
void Socket::closeSocket()
{
    disconnect();
    if (is_open(sock_fd))
        uni_close(sock_fd);
    sock_fd = FD_NULL;
}

Socket::Socket(ipv_t _IPVersion, conn_proto_t _ConnectType)
{
    type = _ConnectType;
    ipv = _IPVersion;
    sock_fd = uni_socket(ipv, type);
    if (!is_open(sock_fd))
    {
        closeSocket();
        exit(-1);
    }

    // Install signal handler for SIGINT
    installSigHandler();
    socket_list.push_back(*this);
}
Socket::~Socket()
{
    closeSocket();
}

int Socket::bindTo(port_t port)
{
    return uni_bind(sock_fd, port, ipv);
}

int Socket::listenOn(int cnt)
{
    return uni_listen(sock_fd, cnt);
}

int Socket::acceptFrom(void)
{
    conn_fd = uni_accept(sock_fd, &peer_info, ipv);
    if (is_open(conn_fd))
        return 0;
    else
        return -1;
}

int Socket::connectTo(std::string host, port_t port)
{
    return uni_connect(sock_fd, host, port, &peer_info, ipv);
}

ssize_t Socket::sendData(const void *buf, int size)
{
    int sd = is_open(conn_fd) ? conn_fd : sock_fd;
    int cnt = uni_send(sd, buf, size);

    if (cnt < size)
        fprintf(stderr, "Socket: sendData: send: Only %d/%d byte(s) data is sent.\n", cnt, size);

    return cnt;
}

ssize_t Socket::sendTo(const void *buf, int size, std::string host, port_t port)
{
    ssize_t cnt = uni_sendto(sock_fd, buf, size, host, port);

    if (cnt != size)
        fprintf(stderr, "Socket: sendTo: only sent %d/%d Byte(s)\n", cnt, size);

    return cnt;
}

ssize_t Socket::recvData(void *buf, int size)
{
    int sd = is_open(conn_fd) ? conn_fd : sock_fd;
    return uni_recv(sd, buf, size, type, &peer_info, ipv);
}

sock_info &Socket::peerAddr(void)
{
    return peer_info;
}

bool Socket::isConnecting(void)
{
    int sd = is_open(conn_fd) ? conn_fd : sock_fd;
    return uni_isConnecting(sd);
}