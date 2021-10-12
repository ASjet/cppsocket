#include <string>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <arpa/inet.h>
#include "zlib.h"
#include "http.h"

const int32_t spl = htonl(0x0D0A0D0A);
const int16_t crlf = htons(0x0D0A);
const int16_t gzip_head = htons(0x1F8B);

int getHTTPbody(byte *buf, size_t size)
{
    byte *p = buf;
    while (spl != *((int32_t *)p) && size--)
        ++p;
    p += 4;
    return p - buf;
}

int getChunkSize(byte *buf, size_t size)
{
    char hex_buf[HEX_BUF_SIZE];
    bzero(hex_buf, HEX_BUF_SIZE);
    byte *p = buf;
    while (crlf != *((int16_t *)p) && size--)
        ++p;
    strncpy(hex_buf, (char *)buf, p - buf);
    return std::stoi(hex_buf, nullptr, 16);
}

int getGzip(byte *buf, size_t size)
{
    byte *p = buf;
    while (gzip_head != *((int16_t *)p) && size--)
        ++p;
    return p - buf;
}

int decompress(byte *in, size_t in_size, byte *out, size_t out_size)
{
    int ret;
    unsigned have;
    z_stream strm;
    bzero(out, out_size);
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    if (Z_OK != (ret = inflateInit2(&strm, 47)))
    {
        fprintf(stderr, "main: decompress: inflateInit(%d): zlib initialization failed.\n", ret);
        return 0;
    }
    strm.avail_in = in_size;
    strm.next_in = in;
    strm.avail_out = out_size;
    strm.next_out = out;
    ret = inflate(&strm, Z_NO_FLUSH);
    assert(ret != Z_STREAM_ERROR);
    switch (ret)
    {
    case Z_NEED_DICT:
        ret = Z_DATA_ERROR;
    case Z_DATA_ERROR:
    case Z_MEM_ERROR:
        (void)inflateEnd(&strm);
        fprintf(stderr, "main: decompress: inflate(%d): zlib inflate error.\n", ret);
        return 0;
    default:
        break;
    }
    have = out_size - strm.avail_out;
    if (ret != Z_STREAM_END)
    {
        fprintf(stderr, "main: decompress: out buffer is not enough to hold output.\n");
    }
    return have;
}