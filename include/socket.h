#ifndef SOCKET_H
#define SOCKET_H

#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////////////

#define NULL_ADDRESS "0.0.0.0"
#define NULL_PORT 0

typedef uint16_t port_t;
typedef unsigned char byte;

class Socket;
extern std::vector<Socket> socket_list;
enum ipv_t
{
    IPv4,
    IPv6
};
enum conn_proto_t
{
    TCP,
    UDP
};
struct sock_info
{
    std::string address;
    port_t port;
};

#include "uni_socketIO.h"
////////////////////////////////////////////////////////////////////////////////
class Socket
{
public:
    Socket() = default;
    Socket(ipv_t _IPVersion, conn_proto_t _ConnectType);
    ~Socket();
    void disconnect(void);
    void closeSocket(void);
    int bindTo(port_t _Port);
    int listenOn(int _LinkCount);
    int acceptFrom(void);
    sock_info &peerAddr(void);
    int connectTo(std::string _HostName, port_t _Port);
    ssize_t sendData(const void *_Buffer, int _Size);
    ssize_t sendTo(const void *_Buffer, int _Size, std::string _HostName, port_t _Port);
    ssize_t recvData(void *_Buffer, int _Size);
    bool isConnecting(void);

private:
    std::string hostname;
    port_t port;
    std::string ip;
    ipv_t ipv;
    conn_proto_t type;
    sock_info peer_info;
    sockfd_t sock_fd = FD_NULL;
    sockfd_t conn_fd = FD_NULL;
};

#endif