#ifndef SOCKET_H
#define SOCKET_H

#include <cstdio>
#include <memory>
#include <string>
#include "connection.h"
using std::string;

using byte = uint8_t;
using port_t = uint16_t;
using sock_t = int;
////////////////////////////////////////////////////////////////////////////////
namespace socket {

class Socket;
class Connection;

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
const char *strerr(const int _ErrorCode);
////////////////////////////////////////////////////////////////////////////////
class Socket {
public:
  Socket(const ip_v _IPVersion, const proto_t _Protocol);
  ~Socket();

  /*
   * Disconnect specified connection
   @param _SockDesc descriptor of connection socket
   */
  // void disconnect(const sock_t _SockDesc);

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
  Connection* accept(void);

  /*
   @param _HostName target hostname in domain name or IP address
   @param _Port port on target host
   */
  Connection* connect(const addr_t &_HostAddr);


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

private:
  void makeSocket();
  sockd_t sockd = NULL_SOCKD;
  ip_v ipv;
  proto_t protocol;
  addr_t haddr;
  addr_t oaddr;
};

} // namespace Socket

#endif