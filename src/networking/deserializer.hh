#pragma once

#include "../prelude.hh"
#include <type_traits>

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

template<typename T>
class Deserializer {
    CursorRead<T>& reader;

public:
    using BytesType = Bytes<typename CursorRead<T>::BytesType>;

    Deserializer(CursorRead<T>& r) : reader(r) {};

    inline expected<vector<u8>, monostate> read_bytes(size_t n) {
        auto bytes = this->reader.read_bytes(n);
        if (!bytes) {
            return unexpected(monostate{});
        }
        vector<u8> buf(n, 0);
        bytes->copy(buf);
        return buf;
    }

    inline expected<u8, monostate> read_u8() {
        auto byte = this->read_bytes(1);
        if (!byte) {
            return unexpected(monostate{});
        }
        return byte->front();
    }

    inline expected<u16, monostate> read_u16() {
        auto bytes = this->read_bytes(2);
        if (!bytes) {
            return unexpected(monostate{});
        }
        u16 v = ((*bytes)[0] << 8) | ((*bytes)[1]);
        #if BYTE_ORDER == BIG_ENDIAN
        return v;
        #elif BYTE_ORDER == LITTLE_ENDIAN
        return __bswap_16(v);
        #else
        # error "What kind of system is this?"
        #endif
    }

    inline expected<i32, monostate> read_var_int() {
        const u8 CONTINUE_BIT = (u8(1) << 7);
        const u8 SEGMENT_BITS = static_cast<u8>(~CONTINUE_BIT);

        i32 value = 0;
        u8 pos = 0;
        for (;;) {
            auto byte = this->read_u8();
            if (!byte) {
                return unexpected(monostate{});
            }

            value |= (*byte & SEGMENT_BITS) << pos;
            if ((*byte & CONTINUE_BIT) == 0) {
                break;
            }
            pos = pos + 7;

            // todo: change
            if (pos >= 32) {
                // invalid varint
                return unexpected(monostate{});
            }
        }
        return value;
    }

    inline expected<bool, monostate> read_bool() {
        auto v = this->read_u8();
        if (!v) {
            return unexpected(monostate{});
        }
        switch (*v) {
            case u8(1):
                return true;
            case u8(0):
                return false;
            default:
                return unexpected(monostate{});
        }
    }

    inline expected<string, monostate> read_string() {
        // todo: better casts
        auto size = this->read_var_int();
        if (!size) {
            return unexpected(monostate{});
        }
        auto bytes = this->read_bytes(static_cast<size_t>(*size));
        if (!bytes) {
            return unexpected(monostate{});
        }
        // todo: safety?
        return string(bytes->begin(), bytes->end());
    }
};
