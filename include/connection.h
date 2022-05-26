#ifndef CONNECTION_H
#define CONNECTION_H

#include "socket.h"
#include "uni_socketIO.h"

namespace socket {

class Connection
{
public:
  Connection(sockd_t _sockd, ip_v _ipv, proto_t _protocal, addr_t _haddr, addr_t _oaddr);
  ~Connection();
  std::size_t send(const byte* _Buffer, const std::size_t _Length) const;
  std::size_t recv(byte* _Buffer, const std::size_t _Size) const;
  const addr_t &getAddr() const { return oaddr; }
  bool isConnecting() const;
  bool avaliable(const sock_t _SockDesc) const;
  void close(void);

private:
  sockd_t sockd = NULL_SOCKD;
  ip_v ipv;
  proto_t protocol;
  addr_t haddr;
  addr_t oaddr;
};

}

#endif