#pragma once

#include "../prelude.hh"

#include "../util/dyn_bytes.hh"
#include "cursor.hh"

// template<typename T>
// inline void print_array(span<const T> array) {
//     std::cout << "[";
//     if (array.empty()) {
//         std::cout << " ]\n";
//         return;
//     }

//     for (size_t i = 0; i < array.size() - 1; i++) {
//         std::cout << i32(array[i]) << ", ";
//     }
//     std::cout << i32(array[array.size() - 1]) << "]\n";
// }

// template<typename T>
// inline void print_array(span<T> array) {
//     print_array(span<const T>(array));
// }

// struct SimpleBytes : public Bytes<SimpleBytes> {
//     span<const u8> inner;

//     expected<monostate, monostate> copy_impl(span<u8> buf) {
//         print_array(inner);
//         if (buf.size() > this->inner.size()) {
//             return unexpected(monostate{});
//         }
//         mem::memcpy(buf.data(), this->inner.data(), buf.size());
//         return monostate{};
//     }

//     size_t size_impl() const {
//         return this->inner.size();
//     }
// };

// template<>
// struct CursorReadTypeTraits<class SimpleBuffer> {
//     using Bytes = SimpleBytes;
// };

// class SimpleBuffer : public CursorRead<SimpleBuffer> {
//     DynBytes inner{};
//     size_t start = 0;
//     size_t read = 0;

//     SimpleBuffer(DynBytes&& v) : inner(obj::move(v)), start(0), read(0) {}

// public:
//     SimpleBuffer() = default;

//     static SimpleBuffer create(size_t n) {
//         return SimpleBuffer(DynBytes::make(n));
//     }

//     bool is_empty() const {
//         return this->start == this->read;
//     }

//     size_t size() const {
//         return this->inner.size();
//     }

//     span<u8> writable() {
//         return this->inner.as_span().subspan(this->read, this->size() - this->read);
//     }

//     span<const u8> readable() {
//         return this->inner.as_span().subspan(this->start, this->read - this->start);
//     }

//     bool advance_read(size_t n) {
//         if (n > this->size() - this->read) {
//             return false;
//         }
//         this->read += n;
//         return true;
//     }

//     void occupy(size_t n) {
//         if (n > this->writable().size()) {
//             throw i32(-1);
//         }
//         return;
//     }

//     expected<SimpleBytes, monostate> read_bytes_impl(size_t n) {
//         // todo: overflow
//         size_t available = this->read - this->start;
//         if (n > available) {
//             return unexpected(monostate{});
//         }

//         SimpleBytes bytes{};
//         bytes.inner = this->inner.as_span().subspan(this->start, n);

//         this->start += n;

//         return bytes;
//     }
// };

class SimpleBuffer {
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

    span<u8> readable() {
        return this->inner.as_span().subspan(this->start, this->read - this->start);
    }

    bool advance_read(size_t n) {
        if (n > this->size() - this->read) {
            return false;
        }
        this->read += n;
        return true;
    }

    bool advance_start(size_t n) {
        if (n > this->read - this->start) {
            return false;
        }
        this->start += n;
        return true;
    }

    void reset() {
        if (this->is_empty()) {
            this->start = 0;
            this->read = 0;
        }
    }

    void occupy(size_t n) {
        if (n <= this->writable().size()) {
            return;
        }

        size_t readable = this->readable().size();

        DynBytes new_buf{};
        DynBytes* out;
        if (n <= this->size() - readable) {
            out = &this->inner;
        } else {
            // todo: allocate more bytes?
            new_buf = DynBytes::make(n + readable + 0);
            out = &new_buf;
        }

        for (size_t i = 0; i < readable; i++) {
            out->at(i) = this->inner.at(this->start + i);
        }

        if (&this->inner != out) {
            this->inner = obj::move(*out);
        }
        this->start = 0;
        this->read = readable;
    }

    // expected<SimpleBytes, monostate> read_bytes_impl(size_t n) {
    //     // todo: overflow
    //     size_t available = this->read - this->start;
    //     if (n > available) {
    //         return unexpected(monostate{});
    //     }

    //     SimpleBytes bytes{};
    //     bytes.inner = this->inner.as_span().subspan(this->start, n);

    //     this->start += n;

    //     return bytes;
    // }
};
