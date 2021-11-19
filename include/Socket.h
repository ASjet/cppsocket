#ifndef SOCKET_H
#define SOCKET_H

#include <cstdio>
#include <memory>
#include <string>
using std::string;
////////////////////////////////////////////////////////////////////////////////
namespace CppSocket {

using byte = uint8_t;
using port_t = uint16_t;
using sock_t = int;

class Socket;

enum class ip_v { IPv4, IPv6 };
enum class proto_t { TCP, UDP, SCTP, RAW };
enum class sock_err { CREATE_SOCKET_ERROR };

const string NULL_ADDRESS("0.0.0.0");
constexpr port_t NULL_PORT(0);
constexpr sock_t MAIN_SOCK(0);
constexpr sock_t NULL_SOCK(-1);

struct addr_t {
  addr_t() : ipaddr(NULL_ADDRESS), port(NULL_PORT) {}
  addr_t(string _Address, port_t _Port) : ipaddr(_Address), port(_Port) {}
  string ipaddr;
  port_t port;
};
////////////////////////////////////////////////////////////////////////////////
class Socket {
public:
  Socket(const ip_v _IPVersion, const proto_t _Protocol);
  ~Socket();

  /*
   * Disconnect specified connection
   @param _SockDesc descriptor of connection socket
   */
  void disconnect(const sock_t _SockDesc);

  /*
   * Disconnect all connections and close sockets
   */
  void close(void);

  /*
   @param _Port specify bind port
   */
  void bind(const port_t _Port) const;

  /*
   @param _LinkCount max number of concurrent tcp connection
   */
  void listen(const std::size_t _LinkCount) const;

  /*
   @return descriptor of accepted connection socket; SOCKD_ERR in error
   */
  sock_t accept(void);

  /*
   @param _HostName target hostname in domain name or IP address
   @param _Port port on target host
   */
  bool connect(const addr_t &_HostAddr);

  /*
   * Send data to MAIN_SOCK
   @param _Buffer Pointer of buffer containing data to send
   @param _Length Length of data in bytes
   @return Number of bytes successfully sent
   */
  inline std::size_t send(const byte *buf, const std::size_t len) const {
    return send(buf, len, MAIN_SOCK);
  }

  /*
   @param _Buffer pointer of buffer containing data to send
   @param _Length length of data in bytes
   @param _SockDesc (Optional)descriptor of socket; default MAIN_SOCK
   @return number of bytes successfully sent
   */
  std::size_t send(const byte *_Buffer, const std::size_t _Length,
                   const sock_t _SockDesc) const;

  /*
   * Send data to target host
   @param _Buffer Pointer of buffer containing data to send
   @param _Length Length of data in bytes
   @param _HostName Target hostname in domain name or IP address
   @param _Port Port on target host
   @return Number of bytes successfully sent
   */
  inline std::size_t sendto(const byte *_Buffer, const std::size_t _Length,
                            const addr_t &_HostAddr) const;

  /*
   * Receive data from MAIN_SOCK
   @param _Buffer Pointer of buffer to save received data
   @param _Size Size of buffer
   @return Number of bytes actually received
   */
  inline std::size_t recv(byte *buf, const std::size_t size) const {
    return recv(buf, size, MAIN_SOCK);
  }

  /*
   * Receive data from specfied socket
   @param _Buffer Pointer of buffer to save received data
   @param _Size Size of buffer
   @param _SockDesc Descriptor of socket
   @return Number of bytes actually received
   */
  std::size_t recv(byte *_Buffer, const std::size_t _Size,
                   const sock_t _SockDesc) const;

  /*
   * Receive data from target host
   @param _Buffer Pointer of buffer to save received data
   @param _Size Size of buffer
   @param _HostAddr Target host address in addr_t
   @return Number of bytes actually received
   */
  std::size_t recvfrom(byte *_Buffer, const std::size_t _Size,
                       const addr_t &_HostAddr) const;

  /*
   @return Address of MAIN_SOCK in addr_t
   */
  inline addr_t &addr() const { return addr(MAIN_SOCK); }

  /*
   @param _SockDesc Descriptor of connection socket
   @return Address of specfied socket in addr_t
   */
  addr_t &addr(const sock_t _SockDesc) const;

  /*
   @return true if MAIN_SOCK is available else false
   */
  inline bool isConnecting() const { return isConnecting(MAIN_SOCK); }

  /*
   @param _SockDesc Descriptor of socket
   @return true if socket is available else false
   */
  bool isConnecting(const sock_t _SockDesc) const;

  /*
   @return true if MAIN_SOCK is available else false
   */
  inline bool avaliable() const { return avaliable(MAIN_SOCK); }

  /*
   @param _SockDesc Descriptor of socket
   @return true if socket is available else false
   */
  bool avaliable(const sock_t _SockDesc) const;

  /*
   @param _SockDesc Descriptor of socket
   @return Number of connections
   */
  std::size_t connCnt(void) const;

  /*
   * Set MAIN_SOCK to unblock mode
   @return true if succeed else false
   */
  inline bool setub() const { return setub(MAIN_SOCK); }

  /*
   * Set specfied socket to unblock mode
   @param _SockDesc Descriptor of socket
   @return true if succeed else false
   */
  bool setub(const sock_t _SockDesc) const;

  /*
   @return error message of error code
   */
  static const char *strerr(const int _ErrorCode);

private:
  struct Impl;
  struct conn_t;
  std::shared_ptr<Impl> pImpl_;
};

} // namespace CppSocket

#endif