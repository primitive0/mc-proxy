#pragma once

#include "../prelude.hh"

#include "codec.hh"
#include "../util/rtype_access.hh"

template<typename T, typename CodecType = decltype(T::codec()), std::size_t CodecSize = CodecType::SIZE>
struct BufferWriter : BufferWriter<T, typename CodecType::PrevType> {

    using PrevType = BufferWriter<T, typename CodecType::PrevType>;

    using FieldCodec = typename CodecType::FieldInfoType::FieldCodec;
    using ThisWriter = typename FieldCodec::Writer;

    ThisWriter this_writer{};
    size_t size = 0;

    template <std::size_t... Is>
    static void create_impl(BufferWriter<T>& buffer_writer, const T& t, std::index_sequence<Is...>) {
        size_t size_ = 0;
        (([&]() {
            using BufferWriterT = typename RTypeAccess<BufferWriter<T>, Is>::It;
            using WriterType = BufferWriterT::ThisWriter;

            auto writer_member_ptr = &BufferWriterT::this_writer;
            auto t_member_ptr = RTypeAccess<CodecType, Is>::It::FieldInfoType::FieldCodec::PTR;

            auto field_writer = WriterType(&(t.*(t_member_ptr)));
            size_ += field_writer.size();
            buffer_writer.*(writer_member_ptr) = field_writer;
        })(), ...);
        buffer_writer.size = size_;
    }

    static auto create(const T& t) {
        BufferWriter<T> buffer_writer{};
        create_impl(buffer_writer, t, std::make_index_sequence<CodecSize>{});
        return buffer_writer;
    }

    template <std::size_t... Is>
    void write_impl(span<u8> buf, std::index_sequence<Is...>) {
        size_t cursor = 0;
        (([&]() {
            using BufferWriterT = typename RTypeAccess<BufferWriter<T>, Is>::It;
            auto writer_member_ptr = &BufferWriterT::this_writer;
            auto& field_writer = this->*(writer_member_ptr);
            field_writer.write(buf.data() + cursor);
            cursor += field_writer.size();
        })(), ...);
    }

    void write(span<u8> buf) {
        this->write_impl(buf, std::make_index_sequence<CodecSize>{});
    }
};

template<typename T>
struct BufferWriter<T, Codec<T, void>, 0> {};
