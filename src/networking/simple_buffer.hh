#pragma once

#include "../prelude.hh"

#include "../util/dyn_bytes.hh"
#include "cursor.hh"

template<typename T>
inline void print_array(span<const T> array) {
    std::cout << "[";
    if (array.empty()) {
        std::cout << " ]\n";
        return;
    }

    for (size_t i = 0; i < array.size() - 1; i++) {
        std::cout << i32(array[i]) << ", ";
    }
    std::cout << i32(array[array.size() - 1]) << "]\n";
}

template<typename T>
inline void print_array(span<T> array) {
    print_array(span<const T>(array));
}

struct SimpleBytes : public Bytes<SimpleBytes> {
    span<const u8> inner;

    expected<monostate, monostate> copy_impl(span<u8> buf) {
        print_array(inner);
        if (buf.size() > this->inner.size()) {
            return unexpected(monostate{});
        }
        mem::memcpy(buf.data(), this->inner.data(), buf.size());
        return monostate{};
    }

    size_t size_impl() const {
        return this->inner.size();
    }
};

template<>
struct CursorReadTypeTraits<class SimpleBuffer> {
    using Bytes = SimpleBytes;
};

class SimpleBuffer : public CursorRead<SimpleBuffer> {
    DynBytes inner{};
    size_t start = 0;
    size_t read = 0;

    SimpleBuffer(DynBytes&& v) : inner(obj::move(v)), start(0), read(0) {}

public:
    SimpleBuffer() = default;

    static SimpleBuffer create(size_t n) {
        return SimpleBuffer(DynBytes::make(n));
    }

    bool is_empty() const {
        return this->start == this->read;
    }

    size_t size() const {
        return this->inner.size();
    }

    span<u8> writable() {
        return this->inner.as_span().subspan(this->read, this->size() - this->read);
    }

    span<const u8> readable() {
        return this->inner.as_span().subspan(this->start, this->read - this->start);
    }

    bool advance_read(size_t n) {
        if (n > this->size() - this->read) {
            return false;
        }
        this->read += n;
        return true;
    }

    void occupy(size_t n) {
        if (n > this->writable().size()) {
            throw i32(-1);
        }
        return;
    }

    expected<SimpleBytes, monostate> read_bytes_impl(size_t n) {
        // todo: overflow
        size_t available = this->read - this->start;
        if (n > available) {
            return unexpected(monostate{});
        }

        SimpleBytes bytes{};
        bytes.inner = this->inner.as_span().subspan(this->start, n);

        this->start += n;

        return bytes;
    }
};
