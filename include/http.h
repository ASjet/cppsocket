#ifndef HTTP_H
#define HTTP_H

#include <string>
#include <vector>
#include <map>
#include "Socket.h"

#define CHUNK 0x4000;
#define GZIP_MAX_COMPRESS_RATE 100
#define HTTP_BOUNDARY "\r\n\r\n"
#define CRLF "\r\n"
#define CONTENT_ENCODING "Content-Encoding"

int decompress(byte * in, size_t in_size, byte * out, size_t out_size);

enum encoding_set
{
    OTHERS,
    GZIP,
    INFLATE
};
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
    void clear(void);
    int parse(byte * _HTTPMessageBuffer, size_t _BufferSize);
    std::string httpType(void);
    std::string httpVersion(void);
    int statusCode(void);
    size_t headerSize(void);
    size_t bodySize(void);
    std::vector<chunk_t>& chunk(void);

    private:
    std::string type;
    std::string ver;
    int stat_code;
    size_t msg_len;
    encoding_set encoding;

    std::map<std::string, std::string> headers;
    byte * header;
    size_t header_len;

    byte * body;
    size_t body_len;

    std::vector<chunk_t> chunks;
};



#endif