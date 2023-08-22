#pragma once

#include "../prelude.hh"

#include "codec.hh"
#include "../util/rtype_access.hh"

template<typename T, typename CodecType = decltype(T::codec()), std::size_t CodecSize = CodecType::SIZE>
class BufferWriterData : public BufferWriterData<T, typename CodecType::PrevType> {
public:
    using PrevType = BufferWriterData<T, typename CodecType::PrevType>;

    using FieldCodec = typename CodecType::FieldInfoType::FieldCodec;
    using ThisWriter = typename FieldCodec::Writer;

    ThisWriter this_writer{};

    template<typename Callable>
    constexpr void for_each(Callable fn) {
        this->for_each_impl(fn, std::make_index_sequence<CodecSize>{});
    }

private:
    template<typename Callable, std::size_t... Is>
    constexpr void for_each_impl(Callable fn, std::index_sequence<Is...>) {
        (([&]() {
            using NodeType = typename RTypeAccess<BufferWriterData<T>, Is>::It;
            auto& node = static_cast<NodeType&>(*this);
            fn(node);
        })(), ...);
    }
};

template<typename T>
struct BufferWriterData<T, Codec<T, void>, 0> {};

template<typename T>
class BufferWriter {
    using DataType = BufferWriterData<T>;

    DataType data;
    size_t _size;

    // todo: move
    BufferWriter(DataType data, size_t _size) : data(data), _size(_size) {}

public:
    static BufferWriter<T> create(const T& t) {
        DataType data{};
        size_t size = 0;
        data.for_each([&](auto& node) {
            using NodeType = std::remove_reference_t<decltype(node)>;
            using WriterType = typename NodeType::ThisWriter;
            auto t_member = NodeType::FieldCodec::PTR;
            node.this_writer = WriterType(&(t.*(t_member)));
            size += node.this_writer.size();
        });
        return BufferWriter(data, size);
    }

    void write(span<u8> buf) {
        size_t cursor = 0;
        this->data.for_each([&](auto& node){
            node.this_writer.write(buf.data() + cursor);
            cursor += node.this_writer.size();
        });
    }

    size_t size() const {
        return this->_size;
    }
};
