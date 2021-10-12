#include <string>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <arpa/inet.h>
#include <regex>
#include "zlib.h"
#include "http.h"

using std::string;
const int32_t spl = htonl(0x0D0A0D0A);
const int16_t crlf = htons(0x0D0A);
const int16_t gzip_head = htons(0x1F8B);

const std::regex method_ptn("([A-Z]+)/([\\d|\\.]+)\\s+(\\d+)");
const std::regex header_ptn("([\\w|-]+):\\s*(.*)");

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

http_msg::http_msg(byte *buf, size_t size)
{
    int cnt = size, chunk_size;
    byte * start = nullptr, *p = buf;

    while(spl != *((int32_t *)p) && cnt--)
        ++p;

    std::smatch desc, hds;
    string header_str((char*)buf, p - buf);

    if (!std::regex_search(header_str, desc, method_ptn))
    {
        fprintf(stderr, "http_msg: regex_search: http message not found.\n");
        return;
    }
    type = desc[1];
    ver = desc[2];
    stat_code = std::stoi(string(desc[3]));

    if (nullptr == (start = (byte*)strstr((char *)buf, desc.str(0).c_str())))
    {
        fprintf(stderr, "http_msg: strstr: unknown error.\n");
        return;
    }
    msg_len = size - (start - buf);

    headers.clear();
    while(std::regex_search(header_str, hds, header_ptn))
    {
        int header_cnt = hds.size();
        if (header_cnt = 3)
            headers[hds.str(1)] = hds.str(2);
        header_str = hds.suffix().str();
    }

    header = new byte[msg_len];
    for (size_t i = 0; i < msg_len; ++i)
        *header++ = *start++;

    p = header;
    cnt = msg_len;
    while (spl != *((int32_t *)p) && cnt--)
        ++p;
    header_len = p - header;

    body = p + 4;
    body_len = msg_len - (body - header);

    start = body;
    p = body;
    cnt = body_len;
    while(true)
    {
        while(crlf != *((int16_t *)p) && cnt--)
            ++p;
        chunk_size = std::stoi(string((char*)start, p - start), nullptr, 16);
        if(chunk_size == 0)
            break;
        p += 2;

        chunk_t chunk;
        chunk.raw_data = p;
        chunk.size = chunk_size;
        if(gzip_head == *((int16_t *)p))
        {
            byte * decmpr_buf = new byte[chunk_size];
            bzero(decmpr_buf, chunk_size);
            decompress(p, chunk_size, decmpr_buf, chunk_size);
            chunk.data = decmpr_buf;
        }
        else
            chunk.data = p;

        chunks.push_back(chunk);
        p += chunk_size;
        start = p;
    }
}

http_msg::~http_msg()
{
    headers.clear();
    for(auto chunk = chunks.begin(); chunk != chunks.end(); ++chunk)
        if(chunk->data == chunk->raw_data)
            delete [] chunk->data;
    delete []header;
    chunks.clear();
}