#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#define WIN32_LEAN_AND_MEAN
#include "socket.h"
#include "uni_socketIO.h"
#include <cassert>
#include <cstring>
#include <iphlpapi.h>
#include <string>
#include <system_error>
#include <vector>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

using namespace socket;
using std::string;
////////////////////////////////////////////////////////////////////////////////
constexpr std::size_t IPADDR_BUFSIZE(32);
constexpr std::size_t SOCKADDR_BUFSIZE(128);
const string wsa_em("WSAStartup");
const string getaddrinfo_em("getaddrinfo");
const string inet_ntop_em("inet_ntop");
const string socket_em("socket");
const string bind_em("bind");
const string listen_em("listen");
const string accept_em("accept");
////////////////////////////////////////////////////////////////////////////////
static void throw_em(const int code, const string em) {
  throw std::system_error(code, std::system_category(), em);
}
static int AF(const ip_v ipv) noexcept {
  return ((ip_v::IPv4 == (ipv)) ? AF_INET : AF_INET6);
}
static int PROTO(proto_t proto) noexcept {
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
static int SOCK_TYPE(const proto_t proto) noexcept {
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
////////////////////////////////////////////////////////////////////////////////
static bool ns(const char *host, const char *port, addrinfo **result, const ip_v ipv,
        const proto_t proto) {
  addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF(ipv);
  hints.ai_socktype = SOCK_TYPE(proto);
  hints.ai_protocol = PROTO(proto);

  if (nullptr == host)
    hints.ai_flags = AI_PASSIVE;

  return (0 == getaddrinfo(host, port, &hints, result));
}

static void getPeerAddr(const sockaddr_in *addr, addr_t &peer, const ip_v ipv) {
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
sockd_t uni_socket(const ip_v ipv, const proto_t proto) {
  int iResult;

  // Initialize Winsock
  WSADATA wsaData;
  if (0 != (iResult = WSAStartup(MAKEWORD(2, 2), &wsaData)))
    throw_em(iResult, wsa_em);

  // Create a SOCKET
  sockd_t sd = socket(AF(ipv), SOCK_TYPE(proto), PROTO(proto));
  if (INVALID_SOCKET == sd)
    throw_em(WSAGetLastError(), socket_em);

  BOOL bOptVal(FALSE);
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR,
             reinterpret_cast<const char *>(&bOptVal), sizeof(bOptVal));

  return sd;
}

void uni_close(const sockd_t sd) {
  if (NULL_SOCKD != sd)
    closesocket(sd);
}

void uni_bind(const sockd_t sd, const port_t port, const ip_v ipv,
              const proto_t proto) {
  addrinfo *result = nullptr;
  ns(nullptr, std::to_string(port).c_str(), &result, ipv, proto);
  int ret = bind(sd, result->ai_addr, (int)result->ai_addrlen);
  freeaddrinfo(result);

  if (SOCKET_ERROR == ret)
    throw(WSAGetLastError(), bind_em);
}

void uni_listen(const sockd_t sd, const std::size_t cnt) {
  if (SOCKET_ERROR == listen(sd, cnt))
    throw_em(WSAGetLastError(), listen_em);
}

sockd_t uni_accept(const sockd_t sd, addr_t &peer, const ip_v ipv) {
  byte addr[SOCKADDR_BUFSIZE];
  memset(addr, 0, SOCKADDR_BUFSIZE);
  int len(SOCKADDR_BUFSIZE);
  sockd_t cd = accept(sd, reinterpret_cast<sockaddr *>(addr), &len);

  if (NULL_SOCK == cd) {
    switch (cd) {
    case WSAEINTR:
      break;
    default:
      throw_em(WSAGetLastError(), accept_em);
    }
  }

  getPeerAddr(reinterpret_cast<sockaddr_in *>(addr), peer, ipv);
  return cd;
}

bool uni_connect(const sockd_t sd, const addr_t &host, addr_t &peer,
                 const ip_v ipv, proto_t proto) {
  addrinfo *result = nullptr, *p = nullptr;
  if (ns(host.ipaddr.c_str(), std::to_string(host.port).c_str(), &result, ipv,
         proto)) {
    for (p = result; nullptr != p; p = p->ai_next)
      if (SOCKET_ERROR != connect(sd, p->ai_addr, p->ai_addrlen))
        break;

    freeaddrinfo(result);
    if (nullptr != p) {
      getPeerAddr(reinterpret_cast<sockaddr_in *>(p->ai_addr), peer, ipv);
      return true;
    }
  }
  if (nullptr != result)
    freeaddrinfo(result);
  return false;
}

std::size_t uni_send(const sockd_t sd, const byte *buf,
                     const std::size_t length) {
  auto cnt = send(sd, reinterpret_cast<const char *>(buf), length, 0);
  return (SOCKET_ERROR == cnt) ? 0 : static_cast<std::size_t>(cnt);
}

std::size_t uni_sendto(const sockd_t sd, const byte *buf,
                       const std::size_t length, const addr_t &host,
                       addr_t &peer, const ip_v ipv, const proto_t proto) {
  int cnt;
  addrinfo *result = nullptr, *p = nullptr;
  if (ns(host.ipaddr.c_str(), std::to_string(host.port).c_str(), &result, ipv,
         proto)) {
    for (p = result; nullptr != p; p = p->ai_next)
      if (SOCKET_ERROR != (cnt = sendto(sd, reinterpret_cast<const char *>(buf),
                                        length, 0, p->ai_addr, p->ai_addrlen)))
        break;

    freeaddrinfo(result);
    if (nullptr != p) {
      getPeerAddr(reinterpret_cast<sockaddr_in *>(p->ai_addr), peer, ipv);
      return static_cast<std::size_t>(cnt);
    }
  }
  if (nullptr != result)
    freeaddrinfo(result);
  return 0;
}

std::size_t uni_recv(const sockd_t sd, byte *buf, const std::size_t size) {
  auto cnt = recv(sd, reinterpret_cast<char *>(buf), size, 0);
  return (SOCKET_ERROR == cnt) ? 0 : static_cast<std::size_t>(cnt);
}

std::size_t uni_recvfrom(const sockd_t sd, byte *buf,
                               const std::size_t size, const addr_t &host,
                               addr_t &peer, ip_v ipv, const proto_t proto) {
  int cnt;
  addrinfo *result = nullptr, *p = nullptr;
  if (ns(host.ipaddr.c_str(), std::to_string(host.port).c_str(), &result, ipv,
         proto)) {
    for (p = result; nullptr != p; p = p->ai_next)
      if (SOCKET_ERROR ==
          (cnt = recvfrom(sd, reinterpret_cast<char *>(buf), size, 0,
                          p->ai_addr, reinterpret_cast<int *>(&p->ai_addrlen))))
        break;

    freeaddrinfo(result);
    if (nullptr != p) {
      getPeerAddr(reinterpret_cast<sockaddr_in *>(p->ai_addr), peer, ipv);
      return static_cast<std::size_t>(cnt);
    }
  }
  if (nullptr != result)
    freeaddrinfo(result);
  return 0;
}

bool uni_isConnecting(const sockd_t sd) { return (NULL_SOCKD != sd); }

bool uni_setub(const sockd_t sd) { return false; }

const char *uni_strerr(const int ec) {
  return "https://docs.microsoft.com/en-us/windows/win32/winsock/"
         "windows-sockets-error-codes-2";
}

#endif