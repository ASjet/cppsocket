#include "Socket.h"
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
using CppSocket::addr_t;
using CppSocket::byte;
using CppSocket::ip_v;
using CppSocket::port_t;
using CppSocket::proto_t;
using std::string;
////////////////////////////////////////////////////////////////////////////////
sockd_t uni_socket(const ip_v _IPVersion, const proto_t _Protocol);
void uni_close(const sockd_t _Socket);
void uni_bind(const sockd_t _Socket, const port_t _Port, const ip_v _IPVersion,
              const proto_t _Protocol);
void uni_listen(const sockd_t _Socket, const std::size_t _ListenCount);
sockd_t uni_accept(const sockd_t _Socket, addr_t &_OppoAddr,
                   const ip_v _IPversion);
bool uni_connect(const sockd_t _Socket, const addr_t &_HostAddr,
                 addr_t &_OppoAddr, const ip_v _IPVersion,
                 const proto_t _Protocol);
std::size_t uni_send(const sockd_t _Socket, const byte *_DataBuffer,
                     const std::size_t _Length);
std::size_t uni_sendto(const sockd_t _Socket, const byte *_DataBuffer,
                       const std::size_t _Length, const addr_t &_HostAddr,
                       addr_t &_OppoAddr, const ip_v _IPVersion,
                       const proto_t _Protocol);
std::size_t uni_recv(const sockd_t _Socket, byte *_DataBuffer,
                     const std::size_t _BufferSize);
std::size_t uni_recvfrom(const sockd_t _Socket, byte *_DataBuffer,
                         const std::size_t _BufferSize, const addr_t &_HostAddr,
                         addr_t &_OppoAddr, const ip_v _IPVersion,
                         const proto_t _Protocol);
bool uni_isConnecting(const sockd_t sd,
                      std::unordered_map<sockd_t, std::unique_ptr<tcb_t>> &tcbs,
                      std::mutex &mutex);
bool uni_setub(const sockd_t _Socket);
const char *uni_strerr(const int _ErrorCode);
////////////////////////////////////////////////////////////////////////////////
namespace CppSocket {
////////////////////////////////////////////////////////////////////////////////
/* Error messages */
const string connect_errmsg("\nUnable to recreate socket after\
                            failed to establish connection\n");
////////////////////////////////////////////////////////////////////////////////
struct Socket::conn_t {
  conn_t() : sock(NULL_SOCKD) {}
  conn_t(sockd_t _Socket) : sock(_Socket) {}
  conn_t(sockd_t _Socket, addr_t _Addr) : sock(_Socket), addr(_Addr) {}
  sockd_t sock;
  addr_t addr;
};
////////////////////////////////////////////////////////////////////////////////
struct Socket::Impl {
  Impl(ip_v _IPVersion, proto_t _Protocol)
      : hostname(NULL_ADDRESS), port(NULL_PORT), ipaddr(NULL_ADDRESS),
        ipv(_IPVersion), proto(_Protocol), cur_sd(NULL_SOCK) {}
  ~Impl() {
    for (const auto &s : socks)
      uni_close(s.second.sock);
  }
  string hostname;
  port_t port;
  string ipaddr;
  ip_v ipv;
  proto_t proto;
  std::atomic<sock_t> cur_sd;
  std::mutex mutex_socks, mutex_conns;
  std::list<sock_t> conns;
  std::unordered_map<sock_t, conn_t> socks;
  std::unordered_map<sockd_t, std::unique_ptr<tcb_t>> tcbs;
};

const addr_t NULL_ADDR;
/////////////////////////////////////////////////////////////////////////////////
void Socket::disconnect(const sock_t sd) {
  if (pImpl_->socks.contains(sd)) {
    sockd_t sock(pImpl_->socks[sd].sock);
    uni_close(sock);
    std::unique_lock<std::mutex> lock(pImpl_->mutex_socks);
    pImpl_->socks.erase(sd);
    pImpl_->tcbs.erase(sock);
    lock.unlock();

    for (auto i = pImpl_->conns.cbegin(); i != pImpl_->conns.cend(); ++i)
      if (*i == sd) {
        std::scoped_lock<std::mutex> lock(pImpl_->mutex_conns);
        pImpl_->conns.erase(i);
        break;
      }
  }
}

void Socket::close() {
  for (const auto &s : pImpl_->socks)
    uni_close(s.second.sock);
}

Socket::Socket(const ip_v ipv, const proto_t proto)
    : pImpl_(std::make_shared<Impl>(ipv, proto)) {
  pImpl_->ipv = ipv;
  pImpl_->proto = proto;
  sockd_t sockd = uni_socket(ipv, proto);
  std::scoped_lock<std::mutex> lock(pImpl_->mutex_conns);
  pImpl_->socks[MAIN_SOCK] = conn_t(sockd);
  pImpl_->cur_sd = MAIN_SOCK;
}

Socket::~Socket() { close(); }

void Socket::bind(const port_t port) const {
  uni_bind(pImpl_->socks[MAIN_SOCK].sock, port, pImpl_->ipv, pImpl_->proto);
}

void Socket::listen(const std::size_t cnt) const {
  uni_listen(pImpl_->socks[MAIN_SOCK].sock, cnt);
}

sock_t Socket::accept(void) {
  addr_t addr;
  sockd_t conn_fd =
      uni_accept(pImpl_->socks[MAIN_SOCK].sock, addr, pImpl_->ipv);
  if (NULL_SOCKD == conn_fd)
    return NULL_SOCK;

  sock_t sd(++pImpl_->cur_sd);
  std::unique_lock<std::mutex> slock(pImpl_->mutex_socks);
  pImpl_->socks[sd] = conn_t(conn_fd, addr);
  slock.unlock();
  std::scoped_lock<std::mutex> clock(pImpl_->mutex_conns);
  pImpl_->conns.emplace_back(sd);
  return sd;
}

bool Socket::connect(const addr_t &host) {
  conn_t &ms = pImpl_->socks[MAIN_SOCK];
  if (uni_connect(ms.sock, host, ms.addr, pImpl_->ipv, pImpl_->proto)) {
    return true;
  } else {
    try {
      uni_close(ms.sock);
      ms.sock = uni_socket(pImpl_->ipv, pImpl_->proto);
    } catch (std::system_error &rec_e) {
      close();
      throw std::system_error(rec_e.code(),
                              string(rec_e.what()) + connect_errmsg);
    }
  }
  return false;
}

std::size_t Socket::send(const byte *buf, const std::size_t len,
                         const sock_t sd) const {
  return uni_send(pImpl_->socks[sd].sock, buf, len);
}

std::size_t Socket::sendto(const byte *buf, const std::size_t size,
                           const addr_t &haddr) const {
  conn_t &ms = pImpl_->socks[MAIN_SOCK];
  return uni_sendto(ms.sock, buf, size, haddr, ms.addr, pImpl_->ipv,
                    pImpl_->proto);
}

std::size_t Socket::recv(byte *buf, const std::size_t size,
                         const sock_t sd) const {
  return uni_recv(pImpl_->socks[sd].sock, buf, size);
}

std::size_t Socket::recvfrom(byte *buf, const std::size_t size,
                             const addr_t &haddr) const {
  conn_t &ms = pImpl_->socks[MAIN_SOCK];
  return uni_recvfrom(ms.sock, buf, size, haddr, ms.addr, pImpl_->ipv,
                      pImpl_->proto);
}

addr_t &Socket::addr(const sock_t sd) const { return pImpl_->socks[sd].addr; }

bool Socket::isConnecting(const sock_t sd) const {
  return (pImpl_->socks.contains(sd))
             ? uni_isConnecting(pImpl_->socks[sd].sock, pImpl_->tcbs,
                                pImpl_->mutex_socks)
             : false;
}

bool Socket::avaliable(const sock_t sd) const {
  return (pImpl_->socks.contains(sd)) ? (NULL_SOCKD != pImpl_->socks[sd].sock)
                                      : false;
}

std::size_t Socket::connCnt(void) const { return pImpl_->conns.size(); }

bool Socket::setub(const sock_t sd) const {
  return (pImpl_->socks.contains(sd)) ? uni_setub(pImpl_->socks[sd].sock)
                                      : false;
}

const char *Socket::strerr(const int errcode) { return uni_strerr(errcode); }
} // namespace CppSocket