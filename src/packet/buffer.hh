#pragma once

#include "../prelude.hh"

class ByteBuffer {
    vector<u8> inner;

    ByteBuffer(vector<u8>&& v) : inner(obj::move(v)) {}

public:
    ByteBuffer() : inner() {}

    span<u8> as_span() {
        return span(this->inner.begin(), this->inner.end());
    }

    size_t size() const {
        return this->inner.size();
    }

    static ByteBuffer create(size_t n) {
        return ByteBuffer(vector<u8>(n, u8(0)));
    }
};
