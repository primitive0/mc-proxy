#pragma once

#include "../prelude.hh"
#include "../networking/cursor.hh"

enum class VarIntReadError {
    Partial,
    Overflow,
};

namespace serialization {
    template<typename T>
    inline expected<i32, VarIntReadError> read_var_int(CursorRead<T>& c) {
        const u8 CONTINUE_BIT = (u8(1) << 7);
        const u8 SEGMENT_BITS = static_cast<u8>(~CONTINUE_BIT);

        i32 value = 0;
        u8 pos = 0;
        for (;;) {
            auto byte = c.read_u8();
            if (!byte) {
                return unexpected(VarIntReadError::Partial);
            }

            value |= (*byte & SEGMENT_BITS) << pos;
            if ((*byte & CONTINUE_BIT) == 0) {
                break;
            }
            pos = pos + 7;

            if (pos >= 32) {
                return unexpected(VarIntReadError::Overflow);
            }
        }
        return value;
    }

    template<typename T>
    inline expected<string, monostate> read_string(CursorRead<T>& c) {
        // todo: better casts
        auto size = read_var_int(c);
        if (!size) {
            return unexpected(monostate{});
        }
        auto bytes = c.read_bytes(static_cast<size_t>(*size));
        if (!bytes) {
            return unexpected(monostate{});
        }
        // todo: safety?
        return string(bytes->begin(), bytes->end());
    }

    template<typename T>
    inline expected<u16, monostate> read_u16(CursorRead<T>& c) {
        auto bytes = c.read_bytes(2);
        if (!bytes) {
            return unexpected(monostate{});
        }
        u16 v = ((*bytes)[0]) | ((*bytes)[1] << 8);
        #if BYTE_ORDER == BIG_ENDIAN
        return v;
        #elif BYTE_ORDER == LITTLE_ENDIAN
        return __bswap_16(v);
        #else
        # error "What kind of system is this?"
        #endif
    }

    template<typename T>
    inline expected<bool, monostate> read_bool(CursorRead<T>& c) {
        auto v = c.read_u8();
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
}
