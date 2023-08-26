#pragma once

#include "../prelude.hh"

#include "../networking/cursor.hh"

inline size_t count_var_int_bytes(i32 input) {
    if ((input & (0xFFFFFFFF << 7)) == 0) {
        return 1;
    } else if ((input & (0xFFFFFFFF << 14)) == 0) {
        return 2;
    } else if ((input & (0xFFFFFFFF << 21)) == 0) {
        return 3;
    } else if ((input & (0xFFFFFFFF << 28)) == 0) {
        return 4;
    } else {
        return 5;
    }
}

class VarIntWriter {
    i32 data;
    size_t _size;

public:
    VarIntWriter(i32 v) : data(v), _size(count_var_int_bytes(v)) {}

    VarIntWriter(const VarIntWriter&) = delete;
    VarIntWriter(VarIntWriter&&) = default;

    size_t size() const {
        return this->_size;
    }

    void write(u8* buf) {
        const i32 CONTINUE_BIT = (i32(1) << 7);
        const i32 SEGMENT_BITS = ~CONTINUE_BIT;

        i32 value = this->data;
        size_t pos = 0;
        for (;;) {
            if ((value & CONTINUE_BIT) == 0) {
                buf[pos] = value;
                return;
            }
            buf[pos] = (value & SEGMENT_BITS) | CONTINUE_BIT;
            pos++;
            value >>= 7;
        }
    }
};

class BytesWriter {
    span<const u8> data;

public:
    BytesWriter(span<const u8> v) : data(v) {}

    BytesWriter(const BytesWriter&) = delete;
    BytesWriter(BytesWriter&&) = default;

    size_t size() const {
        return this->data.size();
    }

    void write(u8* buf) {
        mem::memcpy(buf, this->data.data(), this->data.size());
    }
};

class U16Writer {
    u16 data;

    size_t size() const {
        return 2;
    }

    void write(u8* buf) {
        #if BYTE_ORDER == BIG_ENDIAN
        u16 val = this->data;
        return v;
        #elif BYTE_ORDER == LITTLE_ENDIAN
        u16 val = __bswap_16(this->data);
        #else
        # error "What kind of system is this?"
        #endif
        buf[0] = static_cast<u8>(val);
        buf[1] = static_cast<u8>(val >> 8);
    }
};

namespace serialization {

    template<typename T>
    expected<monostate, monostate> write_var_int(CursorWrite<T>& cursor, i32 v) {
        auto w = VarIntWriter(v);
        auto bytes = cursor.occupy_bytes(w.size());
        if (!bytes) {
            return unexpected(monostate{});
        }
        w.write(bytes->data());
        return monostate{};
    }
};
