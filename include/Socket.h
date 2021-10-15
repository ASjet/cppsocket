#ifndef SOCKET_H
#define SOCKET_H

#include <string>
#include <vector>
#include <cstdio>

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
struct addr_t
{
    std::string addr;
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

    // disconnect established connection
    void disconnect(void);

    // close socket
    void closeSocket(void);

    /*
     @param _Port specify bind port
     @return 0 if bind successfully otherwise -1
     */
    int bindTo(port_t _Port);

    /*
     @param _LinkCount max number of concurrent tcp connection
     @return 0 if listen successfully otherwise -1
     */
    int listenOn(int _LinkCount);

    /*
     @return socket_fd of new established connection; -1 in error
     */
    int acceptFrom(void);

    /*
     @return information of connected peer in addr_t
     */
    addr_t &peerAddr(void);

    /*
     @param _HostName target hostname in domain name or IP address
     @param _Port port on target host
     @return 0 if connection established otherwise -1
     */
    int connectTo(std::string _HostName, port_t _Port);

    /*
     @param _Buffer pointer of buffer containing data to send
     @param _Length length of data in bytes
     @return number of bytes successfully sent
     */
    ssize_t sendData(const void *_Buffer, ssize_t _Length);

    /*
     @param _Buffer pointer of buffer containing data to send
     @param _Length length of data in bytes
     @param _HostName target hostname in domain name or IP address
     @param _Port port on target host
     @return number of bytes successfully sent
     */
    ssize_t sendTo(const void *_Buffer, ssize_t _Length, std::string _HostName, port_t _Port);

    /*
     @param _Buffer pointer of buffer to save received data
     @param _Size size of buffer
     @return number of bytes actually received
     */
    ssize_t recvData(void *_Buffer, ssize_t _Size);

    /*
     @param _Buffer pointer of buffer to save received data
     @param _Size size of buffer
     @param _HostName target hostname in domain name or IP address
     @param _Port port on target host
     @return number of bytes actually received
     */
    ssize_t recvFrom(void * _Buffer, ssize_t _Size, std::string _HostName, port_t _Port);

    /*
     @return true if connection is available else false
     */
    bool isConnecting(void);

    /*
     @return true if socket is available else false
     */
    bool avaliable(void);

private:
    std::string hostname;
    port_t port;
    std::string ip;
    ipv_t ipv;
    conn_proto_t type;
    addr_t peer_info;
    sockfd_t sock_fd = FD_NULL;
    sockfd_t conn_fd = FD_NULL;
};

#endif