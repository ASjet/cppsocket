#include <cstdio>
#include <cstring>
#include <vector>
#include <cstddef>
#include "Socket.h"
#include "uni_socketIO.h"
std::vector<Socket> socket_list;
addr_t NULL_ADDR;
////////////////////////////////////////////////////////////////////////////////
void Socket::disconnect(sockd_t sd)
{
    if(socks.find(sd) == socks.end())
    {
        if(-1 == wlock(&rwlock))
            return;
        if(is_open(socks[sd].fd))
            uni_close(socks[sd].fd);
        socks.erase(sd);

        for(auto i = conns.begin(); i != conns.end(); ++i)
            if(*i == sd)
            {
                conns.erase(i);
                break;
            }
        unrwlock(&rwlock);
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
    cur_sd = MAIN_SOCKD;
    destroyrwlock(&rwlock);
}

Socket::Socket(ipv_t _IPVersion, conn_proto_t _ConnectType)
{
    type = _ConnectType;
    ipv = _IPVersion;
    socks.clear();
    conns.clear();
    initrwlock(&rwlock);
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
    int status;
    if(-1 == rlock(&rwlock))
        return -1;
    status = uni_bind(socks[MAIN_SOCKD].fd, port, ipv, type);
    unrwlock(&rwlock);
    return status;
}

int Socket::listenOn(int cnt)
{
    int status;
    if(-1 == rlock(&rwlock))
        return -1;
    status = uni_listen(socks[MAIN_SOCKD].fd, cnt);
    unrwlock(&rwlock);
    return status;
}

sockd_t Socket::acceptFrom(void)
{
    conn_t conn;
    if(-1 == rlock(&rwlock))
        return SOCKD_ERR;
    sockfd_t conn_fd = uni_accept(socks[MAIN_SOCKD].fd, conn.addr, ipv);
    unrwlock(&rwlock);

    if (is_open(conn_fd))
    {
        conn.fd = conn_fd;
        if(-1 == wlock(&rwlock))
            return SOCKD_ERR;
        ++cur_sd;
        socks[cur_sd] = conn;
        conns.push_back(cur_sd);
        unrwlock(&rwlock);
        return cur_sd;
    }
    else
        return SOCKD_ERR;
}

int Socket::connectTo(std::string host, port_t port)
{
    if(-1 == rlock(&rwlock))
        return -1;
    conn_t &ms = socks[MAIN_SOCKD];
    int rc = uni_connect(ms.fd, host, port, ms.addr, ipv, type);
    unrwlock(&rwlock);
    if(-1 == rc)
    {
        if(-1 == wlock(&rwlock))
            return -1;
        uni_close(ms.fd);
        ms.fd = uni_socket(ipv, type);
        if(!is_open(ms.fd))
        {
            unrwlock(&rwlock);
            closeSocket();
        }
        unrwlock(&rwlock);
        return -1;
    }
    return 0;
}

size_t Socket::sendData(const void *buf, size_t size, sockd_t sd)
{
    if(-1 == rlock(&rwlock))
        return 0;
    size_t cnt = uni_send(socks[sd].fd, buf, size);
    unrwlock(&rwlock);
    if (cnt < size)
        fprintf(stderr,
                "Socket: sendData: Only %zd/%zd byte(s) data is sent.\n",
                cnt,
                size);
    return cnt;
}

size_t Socket::sendTo(const void *buf, size_t size, std::string host, port_t port)
{
    if(-1 == rlock(&rwlock))
        return 0;
    conn_t &ms = socks[MAIN_SOCKD];
    size_t cnt = uni_sendto(ms.fd, buf, size, host, port, ms.addr, ipv, type);
    unrwlock(&rwlock);
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
    if(-1 == rlock(&rwlock))
        return 0;
    size_t cnt = uni_recv(socks[sd].fd, buf, size);
    unrwlock(&rwlock);
    return cnt;
}

size_t Socket::recvFrom(void *buf, size_t size, std::string host, port_t port)
{
    memset(buf, 0, size);
    if(-1 == rlock(&rwlock))
        return 0;
    conn_t &ms = socks[MAIN_SOCKD];
    size_t cnt = uni_recvfrom(ms.fd, buf, size, host, port, ms.addr, ipv, type);
    unrwlock(&rwlock);
    return cnt;
}

addr_t &Socket::addr(sockd_t sd)
{
    if(-1 == rlock(&rwlock))
        return NULL_ADDR;
    addr_t &a = socks[sd].addr;
    unrwlock(&rwlock);
    return a;
}

bool Socket::isConnecting(sockd_t sd)
{
    return uni_isConnecting(socks[sd].fd);
}

bool Socket::avaliable(sockd_t sd)
{
    return (FD_NULL != socks[sd].fd);
}

size_t Socket::connCnt(void)
{
    if(-1 == rlock(&rwlock))
        return 0;
    size_t cnt = conns.size();
    unrwlock(&rwlock);
    return cnt;
}