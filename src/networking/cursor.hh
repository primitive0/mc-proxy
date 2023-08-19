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

template<typename D>
class Bytes {
    D& impl() {
        return static_cast<D&>(*this);
    }

public:
    expected<monostate, monostate> copy(span<u8> buf) {
        return this->impl().copy_impl(buf);
    }

    size_t size() const {
        return this->impl().size_impl();
    }
};

template<typename T>
struct CursorReadTypeTraits;

template<typename D>
class CursorRead {
    D& impl() {
        return static_cast<D&>(*this);
    }

public:
    using BytesType = CursorReadTypeTraits<D>::Bytes;
    static_assert(std::is_base_of_v<Bytes<BytesType>, BytesType>);

    expected<BytesType, monostate> read_bytes(size_t n) {
        return this->impl().read_bytes_impl(n);
    }
};
