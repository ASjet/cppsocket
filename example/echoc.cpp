#include <iostream>
#include <string>
#include <cstdio>
#include <cstring>
#include "Socket.h"
////////////////////////////////////////////////////////////////////////////////
#define BUF_SIZE 2048
#define HOSTNAME "localhost"
char buf[BUF_SIZE];
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char ** argv)
{
    size_t cnt;
    addr_t peer;
    port_t port;
    std::string msg;

    if(argc != 2)
    {
        printf("Usage %s <bind_port>", argv[0]);
        return 0;
    }

    port = std::stoi(argv[1]);

    Socket s(IPv4, TCP);
    if(!s.avaliable())
        return -1;

    if(-1 == s.connectTo(HOSTNAME, port))
        return -1;

    printf("Connected to %s:%hu\n", HOSTNAME, port);

    peer = s.addr();

    while(s.isConnecting() && std::getline(std::cin, msg))
    {
        cnt = s.sendData(msg.c_str(), msg.length());
        printf("send %zd byte(s) to %s:%hu\n", cnt, peer.addr.c_str(), port);
        if(0 == (cnt = s.recvData(buf, BUF_SIZE)))
            break;
        printf("Receive %zd Byte(s) from %s:%hu\n%s\n", cnt, peer.addr.c_str(), peer.port, buf);
    }
    s.closeSocket();
    return 0;
}