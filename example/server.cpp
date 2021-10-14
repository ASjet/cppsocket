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
    if(argc != 2)
    {
        printf("Usage: echos <bind_port>");
        return 0;
    }
    int cnt;
    sock_info info;
    port_t port = std::stoi(argv[1]);

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
        if(-1 == s.acceptFrom())
            break;

        info = s.peerAddr();
        printf("Connection with %s:%hu established\n", info.addr.c_str(), info.port);

        while(s.isConnecting())
        {
            cnt = s.recvData(buf, BUF_SIZE);
            printf("Receved %d Byte(s)\n%s\n", cnt, buf);
            s.sendData(buf, strlen(buf));
        }
        printf("Connection closed.\n");
    }
    printf("Echo server shutdown.\n");
    return 0;
}