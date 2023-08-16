#pragma once

#include "prelude.hh"

// todo: replace with abstraction
struct VectoredBytes {
    span<u8> first{};
    span<u8> second{};
};

template<typename T>
concept BytesRead = requires(T a) {
    { a.next_bytes(size_t{}) } -> std::same_as<expected<VectoredBytes, monostate>>;
};

template<BytesRead R>
class Deserializer {
    R& inner;
    // TODO: add bytes_read

public:
    Deserializer(R& r) : inner(r) {}

    Deserializer(Deserializer& other) = default;

    Deserializer(const Deserializer& other) = delete;

    inline expected<span<const u8>, monostate> next_bytes(size_t n) {
        return this->inner.next_bytes(n);
    }

    inline expected<u8, monostate> next_u8() {
        auto byte = this->next_bytes(1);
        if (!byte) {
            return unexpected(monostate{});
        }
        return byte->front();
    }

    inline expected<u16, monostate> next_u16() {
        auto bytes = this->next_bytes(2);
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

    inline expected<i32, monostate> next_var_int() {
        const u8 CONTINUE_BIT = (u8(1) << 7);
        const u8 SEGMENT_BITS = static_cast<u8>(~CONTINUE_BIT);

        i32 value = 0;
        u8 pos = 0;
        for (;;) {
            auto byte = this->next_u8();
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

    inline expected<bool, monostate> next_bool() {
        auto v = this->next_u8();
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

    inline expected<string, monostate> next_string() {
        // todo: better casts
        auto size = this->next_var_int();
        if (!size) {
            return unexpected(monostate{});
        }
        auto bytes = this->next_bytes(static_cast<size_t>(*size));
        if (!bytes) {
            return unexpected(monostate{});
        }
        // todo: safety?
        return string(bytes->begin(), bytes->end());
    }
};
