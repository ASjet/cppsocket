#ifndef HTTP_H
#define HTTP_H

#include <string>
#include <vector>
#include <map>
#include "socket.h"

#define CHUNK 0x4000;

int decompress(byte * in, size_t in_size, byte * out, size_t out_size);

struct chunk_t{
    byte * raw_data;
    byte * data;
    size_t size;
};

class http_msg {
    public:
    http_msg() = default;
    http_msg(byte * _HTTPMessageBuffer, size_t _BufferSize);
    ~http_msg();
    size_t headerSize(void);
    size_t bodySize(void);
    size_t chunkSize(void);

    std::string type;
    std::string ver;
    int stat_code;
    size_t msg_len;

    std::map<std::string, std::string> headers;
    byte * header;
    size_t header_len;

    byte * body;
    size_t body_len;

    std::vector<chunk_t> chunks;
};



#endif