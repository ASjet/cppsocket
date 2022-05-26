#include "socket.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <system_error>
using namespace cppsocket;
using std::byte;
////////////////////////////////////////////////////////////////////////////////
constexpr std::size_t BUF_SIZE(2048);
const std::string HOSTNAME("localhost");
byte buf[BUF_SIZE];
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage %s <bind_port>", argv[0]);
    return 0;
  }

  port_t port = std::stoi(argv[1]);
  addr_t addr(HOSTNAME, port);
  std::string msg;

  try {
    Socket s(ip_v::IPv4, proto_t::TCP);
    Connection* conn = s.connect(addr);
    if (conn == nullptr) {
      fprintf(stderr, "Cannot connect to %s:%hu\n", addr.ipaddr.c_str(),
              addr.port);
      return 0;
    }

    addr_t peer = conn->getAddr();
    printf("Connected to %s:%hu\n", peer.ipaddr.c_str(), peer.port);

    while (conn->isConnecting() && std::getline(std::cin, msg)) {
      auto cnt =
          conn->send(msg.c_str(), msg.length());
      printf("send %zd byte(s)\n", cnt);
      memset(buf, 0, BUF_SIZE);
      if (0 == (cnt = conn->recv(buf, BUF_SIZE)))
        break;
      printf("Receive %zd byte(s)\n%s\n", cnt, buf);
    }

    if (!conn->isConnecting())
      printf("Connection closed\n");

    conn->close();
  } catch (std::system_error &e) {
    auto errcode = e.code().value();
    fprintf(stderr, "%s(%d): %s\n", e.what(), errcode, strerr(errcode));
    return -1;
  }
  return 0;
}