#include <cstdio>
#include <cstring>
#include <vector>
#include <cstddef>
#include "Socket.h"
#include "uni_socketIO.h"
std::vector<Socket> socket_list;
////////////////////////////////////////////////////////////////////////////////
void Socket::disconnect(sockd_t sd)
{
    if(is_open(socks[sd].fd))
        uni_close(socks[sd].fd);
    socks.erase(sd);
    for(auto i = conns.begin(); i != conns.end(); ++i)
        if(*i == sd)
        {
            conns.erase(i);
            break;
        }
}
void Socket::closeSocket()
{
    for(auto i = socks.begin(); i != socks.end(); ++i)
    {
        if(is_open(i->second.fd))
            uni_close(i->second.fd);
    }
    socks.clear();
    conns.clear();
}

Socket::Socket(ipv_t _IPVersion, conn_proto_t _ConnectType)
{
    type = _ConnectType;
    ipv = _IPVersion;
    socks.clear();
    conns.clear();
    sockfd_t sock_fd = uni_socket(ipv, type);
    conn_t ms;
    if (is_open(sock_fd))
    {
        ms.fd = sock_fd;
        socks[MAIN_SOCKD] = ms;
        cur_sd = MAIN_SOCKD;
        // Install signal handler for SIGINT
        installSigIntHandler();
        socket_list.push_back(*this);
    }
    else
        closeSocket();
}
Socket::~Socket()
{
    closeSocket();
}

int Socket::bindTo(port_t port)
{
    return uni_bind(socks[MAIN_SOCKD].fd, port, ipv, type);
}

int Socket::listenOn(int cnt)
{
    return uni_listen(socks[MAIN_SOCKD].fd, cnt);
}

sockd_t Socket::acceptFrom(void)
{
    conn_t conn;
    sockfd_t conn_fd = uni_accept(socks[MAIN_SOCKD].fd, conn.addr, ipv);
    if (is_open(conn_fd))
    {
        conn.fd = conn_fd;
        ++cur_sd;
        socks[cur_sd] = conn;
        conns.push_back(cur_sd);
        return cur_sd;
    }
    else
        return SOCKD_ERR;
}

int Socket::connectTo(std::string host, port_t port)
{
    conn_t &ms = socks[MAIN_SOCKD];
    int rc = uni_connect(ms.fd, host, port, ms.addr, ipv, type);
    if(-1 == rc)
    {
        uni_close(ms.fd);
        ms.fd = uni_socket(ipv, type);
        if(!is_open(ms.fd))
            closeSocket();
        return -1;
    }
    return 0;
}

size_t Socket::sendData(const void *buf, size_t size, sockd_t sd)
{
    size_t cnt = uni_send(socks[sd].fd, buf, size);
    if (cnt < size)
        fprintf(stderr,
                "Socket: sendData: Only %zd/%zd byte(s) data is sent.\n",
                cnt,
                size);
    return cnt;
}

size_t Socket::sendTo(const void *buf, size_t size, std::string host, port_t port)
{
    conn_t &ms = socks[MAIN_SOCKD];
    size_t cnt = uni_sendto(ms.fd, buf, size, host, port, ms.addr, ipv, type);
    if (cnt < size)
        fprintf(stderr,
                "Socket: sendTo: only sent %zd/%zd Byte(s)\n",
                cnt,
                size);
    return cnt;
}

size_t Socket::recvData(void *buf, size_t size, sockd_t sd)
{
    memset(buf, 0, size);
    return uni_recv(socks[sd].fd, buf, size);
}

size_t Socket::recvFrom(void *buf, size_t size, std::string host, port_t port)
{
    memset(buf, 0, size);
    conn_t &ms = socks[MAIN_SOCKD];
    return uni_recvfrom(ms.fd, buf, size, host, port, ms.addr, ipv, type);
}

addr_t &Socket::addr(sockd_t sd)
{
    return socks[sd].addr;
}

bool Socket::isConnecting(sockd_t sd)
{
    return uni_isConnecting(socks[sd].fd);
}

bool Socket::avaliable(sockd_t sd)
{
    return (FD_NULL != socks[sd].fd);
}

std::size_t Socket::connCnt(void)
{
    return conns.size();
}