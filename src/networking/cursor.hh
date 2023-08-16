#pragma once

#include "../prelude.hh"

struct Cursor {
    const u8* begin;
    const u8* end;

    Cursor(const vector<u8>& vec) {
        // TODO: check overflow
        this->begin = vec.data();
        this->end = vec.data() + vec.size();
    }

    Cursor(const Cursor&) = delete;

    inline expected<span<const u8>, monostate> next_bytes(size_t n) {
        // todo: overflow
        if (n == 0) {
            return span<const u8>();
        }

        const u8* new_begin = this->begin + n;
        if (new_begin > this->end) {
            return unexpected(monostate{});
        }
        span<const u8> res(this->begin, n);
        this->begin = new_begin;

        return res;
    }
};
