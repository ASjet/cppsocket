#include "connection.h"

namespace socket {

Connection::Connection(sockd_t _sockd,
                       ip_v _ipv,
                       proto_t _protocal,
                       addr_t _haddr,
                       addr_t _oaddr)
  : sockd(_sockd)
  , ipv(_ipv)
  , protocol(_protocal)
  , haddr(_haddr)
  , oaddr(_oaddr)
{}

Connection::~Connection()
{
  close();
}

std::size_t
Connection::send(const byte* _Buffer, const std::size_t _Size) const
{
  switch (protocol) {
    case proto_t::TCP:
      return uni_send(sockd, _Buffer, _Size);
    case proto_t::UDP:
      return uni_sendto(sockd, _Buffer, _Size, haddr, oaddr, ipv, protocol);
    default:
      return 0;
  }
}

std::size_t
Connection::recv(byte* _Buffer, const std::size_t _Size) const
{
  switch (protocol) {
    case proto_t::TCP:
      return uni_recv(sockd, _Buffer, _Size);
    case proto_t::UDP:
      return uni_recvfrom(sockd, _Buffer, _Size, haddr, oaddr, ipv, protocol);
    default:
      return 0;
  }
}

bool
Connection::isConnecting() const
{
  return uni_isConnecting(sockd);
}


bool Connection::avaliable(const sock_t _SockDesc) const {
  return (sockd == NULL_SOCKD);
}

void
Connection::close()
{
  if (sockd != NULL_SOCKD)
    uni_close(sockd);
  sockd = NULL_SOCKD;
}

}