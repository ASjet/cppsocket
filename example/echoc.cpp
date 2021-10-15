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
    int cnt;
    addr_t info;
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

    info = s.peerAddr();

    while(s.isConnecting() && std::getline(std::cin, msg))
    {
        cnt = s.sendData(msg.c_str(), msg.length());
        printf("send %d byte(s) to %s:%hu\n", cnt, info.addr.c_str(), port);
        if(0 == (cnt = s.recvData(buf, BUF_SIZE)))
            break;
        printf("Receive %d Byte(s) from %s:%hu\n%s\n", cnt, info.addr.c_str(), info.port, buf);
    }
    s.closeSocket();
    return 0;
}