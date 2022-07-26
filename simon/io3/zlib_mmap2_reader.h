#pragma once

#include "mmap_reader.h"

#include <zlib.h>
#include <ranges>

namespace io3 {

struct mmap_queue {
    // this mmap_queue only sufficient for 4 Terabytes of data
    size_t filesize = 1'024ul*1'024ul*1'024ul*1'024ul*4ul;
    char* buffer {[&]() {
        auto ptr = (char*)mmap(nullptr, filesize, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_NORESERVE, -1, 0);
        return ptr;
    }()};

    mmap_queue() = default;
    mmap_queue(mmap_queue&& _other) noexcept
        : filesize{_other.filesize}
        , buffer  {_other.buffer}
    {
        _other.buffer = nullptr;
    }

    ~mmap_queue() {
        if (buffer == nullptr) return;
        munmap((void*)buffer, filesize);
    }


    auto dropUntil(size_t i) -> size_t {
        if (i < 1'024ul * 1'024ul) return i;
        auto mask = std::numeric_limits<size_t>::max() - 4095;
        auto diff = i & mask;
        munmap((void*)buffer, diff);
        buffer = buffer + diff;
        filesize -= diff;
        return i - diff;
    }

};


struct zlib_mmap2_reader : protected mmap_reader {
    z_stream stream {
        .next_in = Z_NULL,
        .avail_in = 0,
        .total_out = 0,
        .zalloc = Z_NULL,
        .zfree = Z_NULL,
        .opaque = Z_NULL,
    };

    mmap_queue queue;

    zlib_mmap2_reader(char const* name)
        : mmap_reader(name)
    {
        if (inflateInit2(&stream, 16 + MAX_WBITS) != Z_OK) {
            throw "error";
        }
        stream.avail_in = size();
        stream.next_in  = (unsigned char*)buffer;
        stream.next_out = (unsigned char*)queue.buffer;
    }
    zlib_mmap2_reader(zlib_mmap2_reader&& _other)
        : mmap_reader{std::move(_other)}
    {
        if (inflateInit2(&stream, 16 + MAX_WBITS) != Z_OK) {
            throw "error";
        }
        stream.avail_in = size();
        stream.next_in  = (unsigned char*)buffer;
        stream.next_out = (unsigned char*)queue.buffer;


    }
    zlib_mmap2_reader(zlib_mmap2_reader const&) = delete;
    ~zlib_mmap2_reader() {
        inflateEnd(&stream);
    }

    auto operator=(zlib_mmap2_reader const&) -> zlib_mmap2_reader& = delete;
    auto operator=(zlib_mmap2_reader&&) -> zlib_mmap2_reader& = delete;

    size_t filesize{};
    size_t largestSize{1'024ul*1'024ul*4};

    auto readMore() -> bool {
        while(true) {
            if (filesize < largestSize) {
                stream.avail_out = largestSize-filesize;
            } else {
                stream.avail_out += largestSize*0.25;
            }
            auto available = stream.avail_out;
            auto ret = inflate(&stream, Z_NO_FLUSH);
            mmap_reader::dropUntil((char*)stream.next_in - buffer);

            auto producedBytes = available - stream.avail_out;
            filesize += producedBytes;
            largestSize = std::max(largestSize, filesize);
            if (producedBytes > 0) {
                return true;
            }
            if (ret == Z_STREAM_END) {
                return false;
            }
        }
    }

    size_t readUntil(char c, size_t lastUsed) {
        while (true) {
            auto ptr = (char const*)memchr(queue.buffer + lastUsed, c, filesize - lastUsed);
            if (ptr != nullptr) {
                return ptr - queue.buffer;
            }
            if (!readMore()) {
                return filesize;
            }
        }
    }

    size_t dropUntil(size_t i) {
        auto diff = i - queue.dropUntil(i);
        filesize -= diff;
        return i - diff;
    }

    bool eof(size_t i) const {
        return filesize == i;
    }

    auto string_view(size_t start, size_t end) -> std::string_view {
        return std::string_view{queue.buffer+start, queue.buffer+end};
    }
};

}