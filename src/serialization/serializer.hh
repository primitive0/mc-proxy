#pragma once

#include "../prelude.hh"

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

class VarIntSer {
    i32 data;

public:
    static constexpr bool DELEGATED = false;

    VarIntSer(i32 v) : data(v) {}

    VarIntSer(const VarIntSer&) = delete;
    VarIntSer(VarIntSer&&) = default;

    VarIntWriter writer() {
        return VarIntWriter(this->data);
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

class BytesSer {
    span<const u8> data;

public:
    static constexpr bool DELEGATED = false;

    BytesSer(span<const u8> v) : data(v) {}

    BytesSer(const BytesSer&) = delete;
    BytesSer(BytesSer&&) = default;

    BytesWriter writer() {
        return BytesWriter(this->data);
    }
};

struct StringSer {
    static constexpr bool DELEGATED = true;

    const string* data;

    StringSer(const string& v) : data(&v) {}

    StringSer(const StringSer&) = delete;
    StringSer(StringSer&&) = default;

    template<typename Q>
    auto on_push(Q&& queue) {
        auto str_data = reinterpret_cast<const u8*>(this->data->data());
        auto str_size = this->data->size();
        return queue
            .push(VarIntSer(this->data->size()))
            .push(BytesSer(span(str_data, str_size)));
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

class U16Ser {
    u16 data;

public:
    static constexpr bool DELEGATED = false;

    U16Ser(u16 v) : data(v) {}

    U16Ser(const U16Ser&) = delete;
    U16Ser(U16Ser&&) = default;

    VarIntWriter writer() {
        return VarIntWriter(this->data);
    }
};
