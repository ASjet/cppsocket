#include "uni_socketIO.h"
#include <cstring>
#include <memory>
#include <system_error>
#include <utility>

using std::string;
////////////////////////////////////////////////////////////////////////////////
namespace cppsocket {
/* Error messages */
const string connect_errmsg("\nUnable to recreate socket after\
                            failed to establish connection\n");
const addr_t NULL_ADDR;
/////////////////////////////////////////////////////////////////////////////////
struct Socket_impl
{
  Socket_impl(ip_v _ipv, proto_t proto)
    : ipv(_ipv)
    , protocol(proto)
  {}
  sockd_t sockd = NULL_SOCKD;
  ip_v ipv;
  proto_t protocol;
  addr_t haddr;
  addr_t oaddr;
};
struct Connection_impl
{
  Connection_impl(const std::unique_ptr<Socket_impl, void(*)(Socket_impl*)>& si)
    : sockd(si->sockd)
    , ipv(si->ipv)
    , protocol(si->protocol)
    , haddr(si->haddr)
    , oaddr(si->oaddr)
  {}
  sockd_t sockd = NULL_SOCKD;
  ip_v ipv;
  proto_t protocol;
  addr_t haddr;
  addr_t oaddr;
};

void socket_deleter(Socket_impl* impl) {
  if (impl->sockd != NULL_SOCKD)
    uni_close(impl->sockd);
  delete impl;
}

void conn_deleter(Connection_impl* impl) {
  if (impl->sockd != NULL_SOCKD)
    uni_close(impl->sockd);
  delete impl;
}

////////////////////////////////////////////////////////////////////////////////
Socket::Socket(const ip_v ipv, const proto_t proto)
: impl(new Socket_impl(ipv, proto), &socket_deleter)
{ }

void
Socket::makeSocket()
{
  try {
    impl->sockd = uni_socket(impl->ipv, impl->protocol);
  } catch (std::system_error& rec_e) {
    close();
    throw std::system_error(rec_e.code(),
                            string(rec_e.what()) + connect_errmsg);
  }
}

Socket::~Socket()
{
  impl.release();
}

void
Socket::close()
{
  if (impl->sockd != NULL_SOCKD)
    uni_close(impl->sockd);
  impl->sockd = NULL_SOCKD;
}

void
Socket::listen(const port_t port, const std::size_t cnt)
{
  if (impl->sockd == NULL_SOCKD) {
    makeSocket();
  }
  uni_bind(impl->sockd, port, impl->ipv, impl->protocol);
  uni_listen(impl->sockd, cnt);
}

Connection*
Socket::accept(void)
{
  if (impl->sockd == NULL_SOCKD) {
    makeSocket();
  }
  addr_t addr;
  Connection* conn = nullptr;
  sockd_t conn_fd = uni_accept(impl->sockd, addr, impl->ipv);
  impl->oaddr = addr;
  if (conn_fd != NULL_SOCKD) {
    sockd_t tmp = impl->sockd;
    impl->sockd = conn_fd;
    conn = new Connection(impl);
    impl->sockd = tmp;
  }
  return conn;
}

Connection*
Socket::connect(const addr_t& host)
{
  if (impl->sockd == NULL_SOCKD) {
    makeSocket();
  }
  impl->haddr = host;
  Connection* conn = nullptr;
  switch (impl->protocol) {
    case proto_t::TCP:
      if (uni_connect(
            impl->sockd, impl->haddr, impl->oaddr, impl->ipv, impl->protocol))
        conn = new Connection(impl);
      break;
    case proto_t::UDP:
      conn = new Connection(impl);
      break;
  }
  makeSocket();
  return conn;
}

bool
Socket::setub()
{
  if (impl->sockd == NULL_SOCKD) {
    makeSocket();
  }
  return uni_setub(impl->sockd);
}

const char*
strerr(const int errcode)
{
  return uni_strerr(errcode);
}
////////////////////////////////////////////////////////////////////////////////
Connection::Connection(const std::unique_ptr<Socket_impl, void(*)(Socket_impl*)>& si)
: impl(new Connection_impl(si), &conn_deleter)
{ }

Connection::~Connection()
{
  impl.release();
}

std::size_t
Connection::send(const void* _Buffer, const std::size_t _Size)
{
  switch (impl->protocol) {
    case proto_t::TCP:
      return uni_send(
        impl->sockd, reinterpret_cast<const byte*>(_Buffer), _Size);
    case proto_t::UDP:
      return uni_sendto(impl->sockd,
                        reinterpret_cast<const byte*>(_Buffer),
                        _Size,
                        impl->haddr,
                        impl->oaddr,
                        impl->ipv,
                        impl->protocol);
    default:
      return 0;
  }
}

std::size_t
Connection::recv(void* _Buffer, const std::size_t _Size)
{
  switch (impl->protocol) {
    case proto_t::TCP:
      return uni_recv(impl->sockd, reinterpret_cast<byte*>(_Buffer), _Size);
    case proto_t::UDP:
      return uni_recvfrom(impl->sockd,
                          reinterpret_cast<byte*>(_Buffer),
                          _Size,
                          impl->haddr,
                          impl->oaddr,
                          impl->ipv,
                          impl->protocol);
    default:
      return 0;
  }
}

const addr_t&
Connection::getAddr() const
{
  return impl->oaddr;
}

bool
Connection::isConnecting() const
{
  return uni_isConnecting(impl->sockd);
}

bool
Connection::avaliable() const
{
  return (impl->sockd == NULL_SOCKD);
}

void
Connection::close()
{
  if (impl->sockd != NULL_SOCKD)
    uni_close(impl->sockd);
  impl->sockd = NULL_SOCKD;
}

} // namespace cppsocket