#if defined(__linux__) || defined(__unix__)

#include "socket.h"
#include "uni_socketIO.h"
#include <arpa/inet.h>
#include <cassert>
#include <cstring>
#include <fcntl.h>
#include <memory>
#include <mutex>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <system_error>
#include <unordered_map>

using namespace cppsocket;
using std::string;
////////////////////////////////////////////////////////////////////////////////
constexpr std::size_t IPADDR_BUFSIZE(32);
constexpr std::size_t SOCKADDR_BUFSIZE(128);
const string getaddrinfo_em("getaddrinfo");
const string inet_ntop_em("inet_ntop");
const string socket_em("socket");
const string bind_em("bind");
const string listen_em("listen");
const string accept_em("accept");
////////////////////////////////////////////////////////////////////////////////
static void
throw_em(const int code, const string em)
{
  throw std::system_error(code, std::system_category(), em);
}
static int
AF(const ip_v ipv) noexcept
{
  return ((ip_v::IPv4 == (ipv)) ? AF_INET : AF_INET6);
}
static int
PROTO(const proto_t proto) noexcept
{
  switch (proto) {
    case proto_t::TCP:
      return IPPROTO_TCP;
    case proto_t::UDP:
      return IPPROTO_UDP;
    case proto_t::SCTP:
      return IPPROTO_SCTP;
    default:
      assert(false);
  }
}
static int
SOCK_TYPE(const proto_t proto) noexcept
{
  switch (proto) {
    case proto_t::TCP:
      return SOCK_STREAM;
    case proto_t::UDP:
      return SOCK_DGRAM;
    case proto_t::SCTP:
      return SOCK_SEQPACKET;
    case proto_t::RAW:
      return SOCK_RAW;
    default:
      assert(false);
  }
}
//////////////////////////////////////////////////////////////////////////////////
static bool
ns(const char* host,
   const char* port,
   addrinfo** result,
   const ip_v ipv,
   const proto_t proto)
{
  addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF(ipv);
  hints.ai_socktype = SOCK_TYPE(proto);
  hints.ai_protocol = PROTO(proto);

  if (nullptr == host)
    hints.ai_flags = AI_PASSIVE;

  return (0 == getaddrinfo(host, port, &hints, result));
}

static void
getPeerAddr(const sockaddr_in* addr, addr_t& peer, const ip_v ipv)
{
  char addr_buf[IPADDR_BUFSIZE];
  memset(addr_buf, 0, IPADDR_BUFSIZE);

  if (nullptr ==
      inet_ntop(AF(ipv), &(addr->sin_addr), addr_buf, IPADDR_BUFSIZE)) {

    peer.ipaddr = NULL_ADDRESS;
    peer.port = NULL_PORT;
  }

  peer.ipaddr = std::string(addr_buf);
  peer.port = ntohs(addr->sin_port);
}
////////////////////////////////////////////////////////////////////////////////
sockd_t
uni_socket(const ip_v ipv, const proto_t proto)
{
  int reuse = 1;
  sockd_t sd = socket(AF(ipv), SOCK_TYPE(proto), PROTO(proto));

  if (NULL_SOCK == sd)
    throw_em(errno, socket_em);

  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
  return sd;
}

void
uni_close(const sockd_t sd)
{
  if (NULL_SOCKD != sd)
    close(sd);
}

void
uni_bind(const sockd_t sd,
         const port_t port,
         const ip_v ipv,
         const proto_t proto)
{
  addrinfo* result = nullptr;
  ns(nullptr, std::to_string(port).c_str(), &result, ipv, proto);
  int ret = bind(sd, result->ai_addr, result->ai_addrlen);
  freeaddrinfo(result);

  if (-1 == ret)
    throw_em(errno, bind_em);
}

void
uni_listen(const sockd_t sd, const std::size_t cnt)
{
  if (-1 == listen(sd, cnt))
    throw_em(errno, listen_em);
}

sockd_t
uni_accept(const sockd_t sd, addr_t& peer, const ip_v ipv)
{
  byte addr[SOCKADDR_BUFSIZE];
  memset(addr, 0, SOCKADDR_BUFSIZE);
  socklen_t len(SOCKADDR_BUFSIZE);
  sockd_t cd = accept(sd, reinterpret_cast<sockaddr*>(addr), &len);

  if (NULL_SOCK == cd) {
    switch (errno) {
      case EAGAIN: // Return with no block
      case EINTR:  // Return from interrupt
      case EPERM:  // Return due to firewall
        break;
      default:
        throw_em(errno, accept_em);
    }
  }

  getPeerAddr(reinterpret_cast<sockaddr_in*>(addr), peer, ipv);
  return cd;
}

bool
uni_connect(const sockd_t sd,
            const addr_t& host,
            addr_t& peer,
            const ip_v ipv,
            const proto_t proto)
{
  addrinfo *result = nullptr, *p = nullptr;
  if (ns(host.ipaddr.c_str(),
         std::to_string(host.port).c_str(),
         &result,
         ipv,
         proto)) {
    for (p = result; nullptr != p; p = p->ai_next)
      if (0 == connect(sd, p->ai_addr, p->ai_addrlen))
        break;

    freeaddrinfo(result);
    if (nullptr != p) {
      getPeerAddr(reinterpret_cast<sockaddr_in*>(p->ai_addr), peer, ipv);
      return true;
    }
  }
  if (nullptr != result)
    freeaddrinfo(result);
  return false;
}

std::size_t
uni_send(const sockd_t sd, const byte* buf, const std::size_t length)
{
  auto cnt = send(sd, buf, length, 0);
  return (-1 == cnt) ? 0 : static_cast<std::size_t>(cnt);
}

std::size_t
uni_sendto(const sockd_t sd,
           const byte* buf,
           const std::size_t length,
           const addr_t& host,
           addr_t& peer,
           const ip_v ipv,
           const proto_t proto)
{
  ssize_t cnt;
  addrinfo *result = nullptr, *p = nullptr;
  if (ns(host.ipaddr.c_str(),
         std::to_string(host.port).c_str(),
         &result,
         ipv,
         proto)) {
    for (p = result; nullptr != p; p = p->ai_next)
      if (0 < (cnt = sendto(sd, buf, length, 0, p->ai_addr, p->ai_addrlen)))
        break;

    freeaddrinfo(result);
    if (nullptr != p) {
      getPeerAddr(reinterpret_cast<sockaddr_in*>(p->ai_addr), peer, ipv);
      return static_cast<std::size_t>(cnt);
    }
  }
  if (nullptr != result)
    freeaddrinfo(result);
  return 0;
}

std::size_t
uni_recv(const sockd_t sd, byte* buf, const std::size_t size)
{
  auto cnt = recv(sd, buf, size, 0);
  return (-1 == cnt) ? 0 : static_cast<std::size_t>(cnt);
}

const std::size_t
uni_recvfrom(const sockd_t sd,
             byte* buf,
             const std::size_t size,
             const addr_t& host,
             addr_t& peer,
             ip_v ipv,
             const proto_t proto)
{
  ssize_t cnt;
  addrinfo *result = nullptr, *p = nullptr;
  if (ns(host.ipaddr.c_str(),
         std::to_string(host.port).c_str(),
         &result,
         ipv,
         proto)) {
    for (p = result; nullptr != p; p = p->ai_next)
      if (0 < (cnt = recvfrom(sd, buf, size, 0, p->ai_addr, &p->ai_addrlen)))
        break;

    freeaddrinfo(result);
    if (nullptr != p) {
      getPeerAddr(reinterpret_cast<sockaddr_in*>(p->ai_addr), peer, ipv);
      return static_cast<std::size_t>(cnt);
    }
  }
  if (nullptr != result)
    freeaddrinfo(result);
  return 0;
}

bool
uni_isConnecting(const sockd_t sd)
{
  tcb_t* tcbp(nullptr);
  socklen_t len(sizeof(tcb_t));
  getsockopt(sd, IPPROTO_TCP, TCP_INFO, tcbp, &len);
  return (tcbp->tcpi_state == TCP_ESTABLISHED);
}

bool
uni_setub(const sockd_t sd)
{
  return (-1 == fcntl(sd, F_SETFL, fcntl(sd, F_GETFL, 0) | O_NONBLOCK)) ? false
                                                                        : true;
}

const char*
uni_strerr(const int ec)
{
  return strerror(ec);
}

#endif
