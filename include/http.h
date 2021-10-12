#ifndef HTTP_H
#define HTTP_H

#include "socket.h"

#define CHUNK 0x4000;
#define HEX_BUF_SIZE 16

int getHTTPbody(byte * buf, size_t size);
int getChunkSize(byte *buf, size_t size);
int getGzip(byte * buf, size_t size);
int decompress(byte * in, size_t in_size, byte * out, size_t out_size);


#endif