#ifndef SOCKET_H
#define SOCKET_H

#include <string>

////////////////////////////////////////////////////////////////////////////////
#define SOCKET_BUFFER_SIZE 2048
#define ADDRESS_BUFFER_SIZE 16
#define SOCKADDR_BUFFER_SIZE 128

/////////////////////////////////////////////////////////////////////////////////

typedef uint16_t port_t;
enum ipv_t {
    IPv4,
    IPv6
};
enum conn_proto_t {
    TCP,
    UDP
};

struct sock_info {
    std::string address;
    port_t port;
};

////////////////////////////////////////////////////////////////////////////////
class Socket{
    public:
    Socket() = default;
    Socket(ipv_t _IPVersion, conn_proto_t _ConnectType);
    ~Socket();
    void disconnect(void);
    void closeSocket(void);
    int bindTo(port_t _Port);
    int listenOn(int _LinkCount);
    int acceptFrom(void);
    sock_info peerAddr(void);
    int connectTo(std::string _HostName, port_t _Port);
    ssize_t sendData(const void * _Buffer, int _Size);
    ssize_t sendTo(const void * _Buffer, int _Size, std::string _HostName, port_t _Port);
    ssize_t recvData(void * _Buffer, int _Size);
    bool isConnecting(void);
    int socketnum();

    private:
    std::string hostname;
    port_t port;
    std::string ip;
    int domain;
    int protocol;
    int sock_fd = -1;
    int conn_fd = -1;
    bool is_conn_est;
    char addr[SOCKADDR_BUFFER_SIZE];
};



#endif