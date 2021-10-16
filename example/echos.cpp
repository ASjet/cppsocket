#include <string>
#include <cstdio>
#include <cstring>
#include "Socket.h"
////////////////////////////////////////////////////////////////////////////////
#define BUF_SIZE 2048
char buf[BUF_SIZE];
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char ** argv)
{
    size_t cnt;
    port_t port;
    addr_t peer;
    sockd_t connd;

    if(argc != 2)
    {
        printf("Usage: %s <bind_port>", argv[0]);
        return 0;
    }
    port = std::stoi(argv[1]);

    Socket s(IPv4, TCP);
    if(!s.avaliable())
        return -1;

    if(-1 == s.bindTo(port))
        return -1;

    if(-1 == s.listenOn(1))
        return -1;

    printf("Listening on %hu\n", port);

    while(true)
    {
        if(SOCKD_ERR == (connd = s.acceptFrom()))
            break;

        peer = s.addr(connd);
        printf("Connection with %s:%hu established\n", peer.addr.c_str(), peer.port);

        while(s.isConnecting(connd))
        {
            if(0 == (cnt = s.recvData(buf, BUF_SIZE, connd)))
                break;
            printf("Receved %zd Byte(s)\n%s\n", cnt, buf);
            s.sendData(buf, cnt, connd);
        }
        printf("Connection closed.\n");
    }
    s.closeSocket();
    printf("Echo server shutdown.\n");
    return 0;
}