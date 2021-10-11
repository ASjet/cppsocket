#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <cstring>
#include "socket.h"
////////////////////////////////////////////////////////////////////////////////
using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;
#define BUF_SIZE 2048
int port = 2333;
const char * stop = "stop";
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char ** argv)
{
    Socket s(IPv4, TCP);
    char buf[BUF_SIZE];
    sock_info info;
    int cnt;
    if(-1 == s.bindTo(std::stoi(argv[1])))
        exit(-1);
    if(-1 == s.listenOn(128))
        exit(-1);
    printf("Listening on %d\n", port);
    s.acceptFrom();
    info = s.peerAddr();
    printf("Connection with %s:%d established\n", info.address.c_str(), info.port);
    while(s.isConnecting())
    {
        bzero(buf, BUF_SIZE);
        if(0 <= (cnt = s.recvData(buf, BUF_SIZE)))
            break;
        printf("Receved %d Byte(s)\n%s\n", cnt, buf);
        s.sendData(buf, strlen(buf));
    }
    s.closeSocket();
    return 0;
}