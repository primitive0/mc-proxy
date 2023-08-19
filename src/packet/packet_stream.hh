#pragma once

#include "../prelude.hh"

#include "../io.hh"
#include "../networking/simple_buffer.hh"
#include "../serialization/deserializer.hh"
#include "../serialization/write_queue.hh"
#include "../serialization/serializer.hh"

template<typename S>
requires io::Readable<S> && io::Writable<S>
class PacketStream {
    S inner;
    SimpleBuffer buffer{};
    DynBytes write_buffer{};

public:
    PacketStream(S&& s) : inner(obj::move(s)) {
        this->buffer = SimpleBuffer::create(512);
    }

    template<typename P>
    expected<P, monostate> read_packet() {
        // todo: overflow checks
        if (this->buffer.is_empty()) {
            this->replenish().value();
        }

        {
            auto buf = this->buffer.readable();
            if (buf[0] == u8(0xfe) && buf[1] == u8(0x1) && buf[2] == u8(0xfa)) {
                return unexpected(monostate{});
            }
        }

        size_t packet_size = 0;
        {
            Deserializer de(this->buffer);
            auto res = de.read_var_int();
            auto ignored = de.read_var_int(); // temporary id
            if (!res) {
                throw -1;
            }
            packet_size = static_cast<size_t>(*res);
        }

        size_t bytes_read = this->buffer.readable().size();
        if (bytes_read < packet_size) {
            size_t needed = packet_size - bytes_read;
            this->buffer.occupy(needed);
            while (needed != 0) {
                size_t read1 = this->replenish().value();
                if (read1 >= needed) {
                    break;
                }
                needed -= read1;
            }
        }

        Deserializer de(this->buffer);
        return P::read_from(de);
    }

    template<typename P>
    expected<monostate, monostate> write_packet(const P& packet) {
        auto packet_writer = packet.write_queue().apply(BufferAccum<>::make());
        i32 packet_size = packet_writer.size;
        i32 packet_id = 0; // todo: replace
        i32 framed_packet_size = count_var_int_bytes(packet_id) + packet_size;
        i32 buf_size = count_var_int_bytes(framed_packet_size) + framed_packet_size;

        this->occupy_write_buf(buf_size);

        size_t cursor = 0;
        {
            auto w = VarIntSer(framed_packet_size).writer();
            w.write(this->write_buffer.as_span().data() + cursor);
            cursor += w.size();
        }
        {
            auto w = VarIntSer(packet_id).writer();
            w.write(this->write_buffer.as_span().data() + cursor);
            cursor += w.size();
        }

        auto packet_buf = this->write_buffer.as_span().subspan(cursor);
        packet_writer.write(packet_buf);

        auto buf = this->write_buffer.as_span().subspan(0, buf_size);
        io::write_all(this->inner, buf).value();
        return monostate{};
    }

private:
    expected<size_t, monostate> replenish() {
        auto writable = this->buffer.writable();
        auto res = this->inner.read(writable);
        if (!res) {
            return unexpected(monostate{});
        }
        size_t read = *res;
        this->buffer.advance_read(read);
        return read;
    }

    void occupy_write_buf(size_t n) {
        if (this->write_buffer.size() < n) {
            this->write_buffer = DynBytes::make(n);
        }
    }
};
