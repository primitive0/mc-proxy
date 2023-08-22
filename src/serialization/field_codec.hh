#pragma once

#include "../prelude.hh"

#include "../util/member_ptr_traits.hh"

#include "serializer.hh"

namespace fc {
    template<
        typename T,
        auto M,
        std::enable_if_t<std::is_same_v<typename MemberPtrTraits<decltype(M)>::Type, T>, bool> = true>
    struct FieldTypeBase {
        using FieldType = T;
        using MemberPtr = decltype(M);
        static constexpr MemberPtr PTR = M;
    };

    class VarIntWriter {
        i32 data;
        size_t _size;

    public:
        VarIntWriter() : data(0), _size(0) {}

        VarIntWriter(const i32* v) : data(*v), _size(count_var_int_bytes(*v)) {}

        // VarIntWriter(const VarIntWriter&) = delete;
        // VarIntWriter(VarIntWriter&&) = default;

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

    // struct VarIntWriter {
    //     const i32* data = nullptr;
    //     size_t _size = 0;

    //     VarIntWriter() {}
    //     VarIntWriter(const i32* v) : data(v), _size(2) {}

    //     size_t size() const {
    //         return this->_size;
    //     }
    // };

    class StringWriter {
        const string* data;
        size_t _varint_size;
        size_t _size;

    public:
        StringWriter() : data(nullptr), _size(0) {}

        StringWriter(const string* v) : data(v) {
            this->_varint_size = count_var_int_bytes(data->size());
            this->_size = this->_varint_size + data->size();
        }

        size_t size() const {
            return this->_size;
        }

        void write(u8* buf) {
            auto str_size = static_cast<i32>(this->data->size());
            // todo: use varint api
            VarIntWriter(&str_size).write(buf);
            mem::memcpy(buf + _varint_size, this->data->data(), this->data->size());
        }
    };

    template<auto M>
    struct VarInt : public FieldTypeBase<i32, M> {
        using Writer = fc::VarIntWriter;
    };

    template<auto M>
    struct String : public FieldTypeBase<string, M> {
        using Writer  = fc::StringWriter;
    };
}
