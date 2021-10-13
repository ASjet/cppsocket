#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include "Socket.h"
////////////////////////////////////////////////////////////////////////////////
using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;
#define BUF_SIZE 2048
string hostname = "vpshk.itfs127.com";
int port = 2333;
const char * stop = "stop";
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char ** argv)
{
    string disc("disconnect");
    string msg;
    sock_info info;
    Socket s(IPv4, TCP);
    char buf[BUF_SIZE];
    s.connectTo(hostname, std::stoi(argv[1]));
    int cnt;
    info = s.peerAddr();
    while(s.isConnecting() && std::getline(cin, msg))
    {
        s.sendData(msg.c_str(), msg.length());
        // s.sendTo(msg.c_str(), msg.length(), hostname, port);
        printf("send %s to %s:%d\n", msg.c_str(), hostname.c_str(), port);
        if(msg == string("stop") || msg == string("disconnect"))
            break;
        msg.clear();
        memset(buf, 0, BUF_SIZE);
        if(-1 == (cnt = s.recvData(buf, BUF_SIZE)))
            break;
        printf("Receive %d Byte(s) from %s:%d\n%s\n", cnt, info.address.c_str(), info.port, buf);
    }
    s.closeSocket();
    printf("Connection closed.\n");
    return 0;
}