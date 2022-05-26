#include "socket.h"
#include "uni_socketIO.h"
#include <atomic>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <list>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <unordered_map>
#include <utility>
using socket::addr_t;
using socket::byte;
using socket::ip_v;
using socket::port_t;
using socket::proto_t;
using std::string;
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
namespace socket {
////////////////////////////////////////////////////////////////////////////////
/* Error messages */
const string connect_errmsg("\nUnable to recreate socket after\
                            failed to establish connection\n");
////////////////////////////////////////////////////////////////////////////////

const addr_t NULL_ADDR;
/////////////////////////////////////////////////////////////////////////////////

Socket::Socket(const ip_v ipv, const proto_t proto)
  : ipv(ipv)
  , protocol(proto)
{
  makeSocket();
}

void Socket::makeSocket() {
  close();
  try {
    sockd = uni_socket(ipv, protocol);
  } catch (std::system_error& rec_e) {
    close();
    throw std::system_error(rec_e.code(),
                            string(rec_e.what()) + connect_errmsg);
  }
}

Socket::~Socket()
{
  close();
}

void
Socket::bind(const port_t port) const
{
  uni_bind(sockd, port, ipv, protocol);
}

void
Socket::close()
{
  if (sockd != NULL_SOCKD)
    uni_close(sockd);
  sockd = NULL_SOCKD;
}

void
Socket::listen(const std::size_t cnt) const
{
  uni_listen(sockd, cnt);
}

Connection*
Socket::accept(void)
{
  addr_t addr;
  Connection* conn = nullptr;
  sockd_t conn_fd = uni_accept(sockd, addr, ipv);
  if(conn_fd != NULL_SOCKD)
    conn = new Connection(conn_fd, ipv, protocol, addr, addr);
  return conn;
}

Connection*
Socket::connect(const addr_t& host)
{
  haddr = host;
  Connection* conn = nullptr;
  switch(protocol) {
    case proto_t::TCP:
      if(uni_connect(sockd, haddr, oaddr, ipv, protocol))
        conn = new Connection(sockd, ipv, protocol, haddr, oaddr);
      break;
    case proto_t::UDP:
      conn = new Connection(sockd, ipv, protocol, haddr, oaddr);
      break;
  }
  makeSocket();
  return conn;
}

bool
Socket::setub() const
{
  return uni_setub(sockd);
}

const char* strerr(const int errcode)
{
  return uni_strerr(errcode);
}

} // namespace socket