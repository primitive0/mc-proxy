#pragma once

#include "../prelude.hh"

// template<typename T>
// concept BytesImpl = true;

// template<typename T, typename B>
// concept CursorReadImpl = BytesImpl<B> && requires(T a, size_t n) {
//     { a.read_bytes_impl(n) } -> std::same_as<expected<B, monostate>>;
// };

// template<typename D, typename BytesType>
// class CursorRead {
//     inline CursorReadImpl<BytesType> auto& impl() {
//         return static_cast<D&>(*this);
//     }

// public:
//     inline expected<BytesType, monostate> read_bytes(size_t n) {
//         return this->impl().read_bytes_impl(n);
//     }
// };

// template<typename D>
// class Bytes {
//     D& impl() {
//         return static_cast<D&>(*this);
//     }

// public:
//     expected<monostate, monostate> copy(span<u8> buf) {
//         return this->impl().copy_impl(buf);
//     }

//     size_t size() const {
//         return this->impl().size_impl();
//     }
// };

// template<typename T>
// struct CursorReadTypeTraits;

// template<typename D>
// class CursorRead {
//     D& impl() {
//         return static_cast<D&>(*this);
//     }

// public:
//     using BytesType = CursorReadTypeTraits<D>::Bytes;
//     static_assert(std::is_base_of_v<Bytes<BytesType>, BytesType>);

//     expected<BytesType, monostate> read_bytes(size_t n) {
//         return this->impl().read_bytes_impl(n);
//     }
// };

template<typename D>
class CursorRead {
    D& impl() {
        return static_cast<D&>(*this);
    }

public:
    // interface

    expected<span<const u8>, monostate> read_bytes(size_t n) {
        return this->impl().read_bytes_impl(n);
    }

    expected<u8, monostate> read_u8() {
        return this->impl().read_u8_impl();
    }

    // default implementation

    expected<u8, monostate> read_u8_impl() {
        return this->read_bytes(1).map([](auto s) {
            return s[0];
        });
    }
};

class SpanCursor : public CursorRead<SpanCursor> {
    span<const u8> buf;
    size_t _pos;

public:
    SpanCursor(span<const u8> buf) : buf(buf), _pos(0) {}

    expected<span<const u8>, monostate> read_bytes_impl(size_t n) {
        if (n > buf.size() - this->_pos) {
            return unexpected(monostate{});
        }
        size_t pos = this->_pos;
        this->_pos += n;
        return this->buf.subspan(pos, n);
    }

    size_t pos() const {
        return this->_pos;
    }
};
