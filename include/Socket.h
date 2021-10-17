#ifndef SOCKET_H
#define SOCKET_H

#include <string>
#include <vector>
#include <map>
#include <cstdio>
////////////////////////////////////////////////////////////////////////////////

#define NULL_ADDRESS "0.0.0.0"
#define NULL_PORT 0
#define MAIN_SOCKD 0
#define SOCKD_ERR -1

typedef uint16_t port_t;
typedef unsigned char byte;
typedef int sockd_t;

class Socket;
extern std::vector<Socket> socket_list;

enum ipv_t
{
    IPv4,
    IPv6
};
enum type_t
{
    TCP,
    UDP,
    SCTP,
    RAW
};
struct addr_t
{
    std::string addr = NULL_ADDRESS;
    port_t port = NULL_PORT;
};

#include "uni_socketIO.h"

struct conn_t
{
    sockfd_t fd = FD_NULL;
    addr_t addr;
};
////////////////////////////////////////////////////////////////////////////////
class Socket
{
public:
    Socket() = default;
    Socket(ipv_t _IPVersion, type_t _ConnectType);
    ~Socket();

    /*
     * disconnect specified connection
     @param _SockDesc descriptor of connection socket
     */
    void disconnect(sockd_t _SockDesc);

    /*
     * disconnect all connections and close sockets
     */
    void closeSocket(void);

    /*
     @param _Port specify bind port
     @return 0 if bind successfully; -1 in error
     */
    int bindTo(port_t _Port);

    /*
     @param _LinkCount max number of concurrent tcp connection
     @return 0 if listen successfully; -1 in error
     */
    int listenOn(int _LinkCount);

    /*
     @return descriptor of accepted connection socket; SOCKD_ERR in error
     */
    sockd_t acceptFrom(void);

    /*
     @param _HostName target hostname in domain name or IP address
     @param _Port port on target host
     @return 0 if successful connected to host; -1 in error
     */
    int connectTo(std::string _HostName, port_t _Port);

    /*
     @param _Buffer pointer of buffer containing data to send
     @param _Length length of data in bytes
     @param _SockDesc (Optional)descriptor of socket; default MAIN_SOCKD
     @return number of bytes successfully sent
     */
    size_t sendData(const void *_Buffer, size_t _Length, sockd_t _SockDesc = MAIN_SOCKD);

    /*
     @param _Buffer pointer of buffer containing data to send
     @param _Length length of data in bytes
     @param _HostName target hostname in domain name or IP address
     @param _Port port on target host
     @return number of bytes successfully sent
     */
    size_t sendTo(const void *_Buffer, size_t _Length, std::string _HostName, port_t _Port);

    /*
     * this function will initialize each byte of the buffer with 0x0
     @param _Buffer pointer of buffer to save received data
     @param _Size size of buffer
     @param _SockDesc (Optional)descriptor of socket; default MAIN_SOCKD
     @return number of bytes actually received
     */
    size_t recvData(void *_Buffer, size_t _Size, sockd_t _SockDesc = MAIN_SOCKD);

    /*
     * this function will initialize each byte of the buffer with 0x0
     @param _Buffer pointer of buffer to save received data
     @param _Size size of buffer
     @param _HostName target hostname in domain name or IP address
     @param _Port port on target host
     @return number of bytes actually received
     */
    size_t recvFrom(void * _Buffer, size_t _Size, std::string _HostName, port_t _Port);

    /*
     @param (Optional)_SockDesc descriptor of connection socket; default MAIN_SOCKD
     @return address of specfied socket in addr_t
     */
    addr_t &addr(sockd_t _SockDesc = MAIN_SOCKD);

    /*
     @param _SockDesc (Optional)descriptor of socket; default MAIN_SOCKD
     @return true if socket is available else false
     */
    bool isConnecting(sockd_t _SockDesc = MAIN_SOCKD);

    /*
     @param _SockDesc descriptor of socket
     @return true if socket is available else false
     */
    bool avaliable(sockd_t _SockDesc = MAIN_SOCKD);

    /*
     @param _SockDesc descriptor of socket
     @return number of connections
     */
    size_t connCnt(void);

    int setub(sockd_t _SockDesc = MAIN_SOCKD);


private:
    std::string hostname = NULL_ADDRESS;
    port_t port = NULL_PORT;
    std::string ip = NULL_ADDRESS;
    ipv_t ipv = IPv4;
    type_t type = TCP;
    rwlock_t rwlock;
    sockd_t cur_sd = MAIN_SOCKD;
    std::map<sockd_t, conn_t> socks;
    std::vector<sockd_t> conns;
};

#endif