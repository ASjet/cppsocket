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

const string gzip_encoding = string("gzip");

encoding_set encoding_type(string encoding)
{
    if (encoding == string("gzip"))
        return GZIP;
    else if (encoding == string("inflate"))
        return INFLATE;
    else
        return OTHERS;
}

int decompress(byte *in, size_t in_size, byte *out, size_t out_size)
{
    int ret;
    unsigned have;
    z_stream strm;
    memset(out, 0, out_size);
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
    (void)inflateEnd(&strm);
    return have;
}

http_msg::http_msg(byte *buf, size_t size)
{
    parse(buf, size);
}

int http_msg::parse(byte *buf, size_t size)
{
    int cnt = size, chunk_size;
    byte *start = nullptr, *p = buf, *decmpr_buf = nullptr;

    // find header boundary
    while (spl != *((int32_t *)p) && cnt--)
        ++p;
    // convert header from bytes to string
    std::smatch desc, hds;
    string header_str((char *)buf, p - buf);

    // get http status
    if (!std::regex_search(header_str, desc, method_ptn))
    {
        fprintf(stderr, "http_msg: parse: regex_search: http message not found.\n");
        clear();
        return -1;
    }
    type = desc[1];
    ver = desc[2];
    stat_code = std::stoi(string(desc[3]));

    // save http message and header length
    if (nullptr == (start = (byte *)strstr((char *)buf, desc.str(0).c_str())))
    {
        fprintf(stderr, "http_msg: parse: strstr: unknown error.\n");
        clear();
        return -1;
    }
    msg_len = size - (start - buf);
    header_len = p - start;

    // parse http headers
    headers.clear();
    while (std::regex_search(header_str, hds, header_ptn))
    {
        int header_cnt = hds.size();
        if (header_cnt = 3)
            headers[hds.str(1)] = hds.str(2);
        header_str = hds.suffix().str();
    }

    // allocate memory for http message
    if (nullptr == (header = new byte[msg_len]))
    {
        fprintf(stderr, "http_msg: parse: allocation for msg(%d) failed\n", msg_len);
        clear();
        return -1;
    }
    // copy http message
    p = header;
    for (size_t i = 0; i < msg_len; ++i)
        *p++ = *start++;

    // compute body index;
    body = header + header_len + strlen(HTTP_BOUNDARY);
    body_len = p - body;

    start = body;
    p = body;
    cnt = body_len;
    while (true)
    {
        while (crlf != *((int16_t *)p) && cnt--)
            ++p;
        chunk_size = std::stoi(string((char *)start, p - start), nullptr, 16);
        if (chunk_size == 0)
            break;

        p += strlen(CRLF);
        chunk_t chunk;
        chunk.raw_data = p;
        chunk.size = chunk_size;

        switch (encoding_type(headers[CONTENT_ENCODING]))
        {
        case GZIP:
            cnt = chunk_size * GZIP_MAX_COMPRESS_RATE;
            decmpr_buf = new byte[cnt];
            if (nullptr == decmpr_buf)
            {
                fprintf(stderr, "http_msg: parse: allocation for chunk(%d) failed\n", chunk_size);
                clear();
                return -1;
            }
            decompress(p, chunk_size, decmpr_buf, cnt);
            chunk.data = decmpr_buf;
            break;
        case INFLATE:
        default:
            chunk.data = p;
        }

        chunks.push_back(chunk);

        p += chunk_size + strlen(CRLF);
        start = p;
    }
    return 0;
}

http_msg::~http_msg()
{
    clear();
}

void http_msg::clear(void)
{
    headers.clear();
    for (auto chunk = chunks.begin(); chunk != chunks.end(); ++chunk)
        if (chunk->data != chunk->raw_data)
            delete[] chunk->data;
    chunks.clear();
    if (header != nullptr)
        delete[] header;
    header = nullptr;

    type.clear();
    ver.clear();
    stat_code = 0;
    msg_len = 0;
    encoding = OTHERS;
    header_len = 0;
    body_len = 0;
}

std::string http_msg::httpType(void)
{
    return type;
}

std::string http_msg::httpVersion(void)
{
    return ver;
}

int http_msg::statusCode(void)
{
    return stat_code;
}

size_t http_msg::headerSize(void)
{
    return header_len;
}

size_t http_msg::bodySize(void)
{
    return body_len;
}

std::vector<chunk_t> &http_msg::chunk(void)
{
    return chunks;
}