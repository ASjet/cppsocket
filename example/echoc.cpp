#include "Socket.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <system_error>
using namespace CppSocket;
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
  string msg;
  std::size_t cnt;

  try {
    Socket s(ip_v::IPv4, proto_t::TCP);
    if (!s.connect(addr)) {
      fprintf(stderr, "Cannot connect to %s:%hu\n", addr.ipaddr.c_str(),
              addr.port);
      return 0;
    }

    addr_t peer = s.addr();
    printf("Connected to %s:%hu\n", peer.ipaddr.c_str(), peer.port);

    while (s.isConnecting() && std::getline(std::cin, msg)) {
      auto cnt =
          s.send(reinterpret_cast<const byte *>(msg.c_str()), msg.length());
      printf("send %zd byte(s)\n", cnt);
      memset(buf, 0, BUF_SIZE);
      if (0 == (cnt = s.recv(buf, BUF_SIZE)))
        break;
      printf("Receive %zd byte(s)\n%s\n", cnt, buf);
    }

    if (!s.isConnecting())
      printf("Connection closed.\n");

    s.close();
  } catch (std::system_error &e) {
    auto errcode = e.code().value();
    fprintf(stderr, "error: %s(%d): %s\n", e.what(), errcode,
            Socket::strerr(errcode));
    return -1;
  }
  return 0;
}