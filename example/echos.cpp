#include "Socket.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <system_error>
using namespace CppSocket;
////////////////////////////////////////////////////////////////////////////////
#define BUF_SIZE 2048
byte buf[BUF_SIZE];
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: %s <bind_port>", argv[0]);
    return 0;
  }

  port_t port = std::stoi(argv[1]);
  addr_t peer;

  try {
    Socket s(ip_v::IPv4, proto_t::TCP);
    s.bind(port);
    s.listen(1);
    printf("Listening on %hu\n", port);

    while (true) {
      sock_t cs;
      if (NULL_SOCK == (cs = s.accept()))
        break;

      peer = s.addr(cs);
      printf("Connection with %s:%hu established\n", peer.ipaddr.c_str(),
             peer.port);

      while (s.isConnecting(cs)) {
        memset(buf, 0, BUF_SIZE);
        auto cnt = s.recv(buf, BUF_SIZE, cs);
        if (0 == cnt)
          break;
        printf("Receved %zd byte(s) from %s:%hu\n%s\n", cnt,
               peer.ipaddr.c_str(), peer.port, buf);
        s.send(buf, cnt, cs);
      }
      printf("Connection closed.\n");
    }

    s.close();
  } catch (std::system_error &e) {
    auto errcode = e.code().value();
    fprintf(stderr, "error: %s(%d): %s\n", e.what(), errcode,
            Socket::strerr(errcode));
    return -1;
  }
  printf("Echo server shutdown.\n");
  return 0;
}