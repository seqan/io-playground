#pragma once

#include "file_reader.h"

#include <zlib.h>
#include <span>
#include <vector>
#include <ranges>

struct zlib_reader {
    file_reader file;

    z_stream stream {
        .next_in = Z_NULL,
        .avail_in = 0,
        .total_out = 0,
        .zalloc = Z_NULL,
        .zfree = Z_NULL,
        .opaque = Z_NULL,
    };

    std::vector<char> inBuf;

    zlib_reader(char const* name)
        : file(name)
    {
        if (inflateInit2(&stream, 16 + MAX_WBITS) != Z_OK) {
            throw "error";
        }
    }
    zlib_reader(zlib_reader&& _other)
        : file{std::move(_other.file)}
    {
        if (inflateInit2(&stream, 16 + MAX_WBITS) != Z_OK) {
            throw "error";
        }
    }

    ~zlib_reader() {
        inflateEnd(&stream);
    }

    size_t read(std::ranges::sized_range auto&& range) {
        while(true) {
            if (stream.avail_in == 0) {
                inBuf.resize(inBuf.capacity());
                if (inBuf.size() < range.size()/4) {
                    inBuf.resize(range.size()/4);
                }
                auto readBytes = file.read(inBuf);
                stream.avail_in = readBytes;
                stream.next_in  = (unsigned char*)inBuf.data();
            }

            stream.avail_out = range.size();
            stream.next_out  = (unsigned char*)&*std::begin(range);
            auto ret = inflate(&stream, Z_NO_FLUSH);
            auto producedBytes = (size_t)(stream.next_out - (unsigned char*)&*std::begin(range));
            if (producedBytes > 0) {
                return producedBytes;
            }
            if (ret == Z_STREAM_END) {
                return 0;
            }
        }
    }
};

