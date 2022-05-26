#ifndef SOCKET_H
#define SOCKET_H

#include <string>
////////////////////////////////////////////////////////////////////////////////
namespace cppsocket {

using byte = uint8_t;
using port_t = uint16_t;
using sock_t = int;

struct Socket_impl;
struct Connection_impl;
class Socket;
class Connection;

enum class ip_v { IPv4, IPv6 };
enum class proto_t { TCP, UDP, SCTP, RAW };
enum class sock_err { CREATE_SOCKET_ERROR };

const std::string NULL_ADDRESS("0.0.0.0");
constexpr port_t NULL_PORT(0);
constexpr sock_t MAIN_SOCK(0);
constexpr sock_t NULL_SOCK(-1);

/*
  @return error message of error code
*/
const char *strerr(const int _ErrorCode);
////////////////////////////////////////////////////////////////////////////////
class addr_t {
  public:
    addr_t() : ipaddr(NULL_ADDRESS), port(NULL_PORT) {}
    addr_t(const addr_t& addr): ipaddr(addr.ipaddr), port(addr.port) {}
    addr_t(const std::string _Address, const port_t _Port) : ipaddr(_Address), port(_Port) {}
    addr_t& operator=(const addr_t& rhs) {
      ipaddr = rhs.ipaddr;
      port = rhs.port;
      return *this;
    }

    std::string ipaddr;
    port_t port;
};
////////////////////////////////////////////////////////////////////////////////
class Socket {
public:
  Socket(const ip_v _IPVersion, const proto_t _Protocol);
  ~Socket();
  void close(void);
  void bind(const port_t _Port) const;
  void listen(const std::size_t _LinkCount) const;
  Connection* accept(void);
  Connection* connect(const addr_t &_HostAddr);
  bool setub() const;

private:
  void makeSocket();
  Socket_impl* impl;
};
////////////////////////////////////////////////////////////////////////////////
class Connection
{
public:
  Connection(const Socket_impl* socket_impl);
  ~Connection();
  std::size_t send(const byte* _Buffer, const std::size_t _Length);
  std::size_t recv(byte* _Buffer, const std::size_t _Size);
  const addr_t &getAddr() const;
  bool isConnecting() const;
  bool avaliable(const sock_t _SockDesc) const;
  void close(void);

private:
  Connection_impl* impl;
};

} // namespace cppsocket

#endif