#pragma once

#include "../prelude.hh"

#include "../io.hh"
#include "../networking/simple_buffer.hh"
#include "../serialization/deserializer.hh"
#include "../serialization/serializer.hh"
#include "../serialization/buffer_writer.hh"

template<typename S>
requires io::Readable<S> && io::Writable<S>
class PacketStream {
    S inner;
    SimpleBuffer read_buffer;
    DynBytes write_buffer;

public:
    PacketStream(S&& s) :
        inner(obj::move(s)),
        read_buffer(SimpleBuffer::create(512)),
        write_buffer() {}

    template<typename P>
    expected<P, monostate> read_packet() {
        // todo: overflow checks

        // todo: remove this ---
        if (this->read_buffer.is_empty()) {
            this->replenish().value();
        }

        {
            auto buf = this->read_buffer.readable();
            if (buf[0] == u8(0xfe) && buf[1] == u8(0x1) && buf[2] == u8(0xfa)) {
                return unexpected(monostate{});
            }
        }
        // ---

        size_t packet_size = 0;
        {
            auto res = this->read_packet_size();
            if (!res) {
                return unexpected(monostate{});
            }
            packet_size = static_cast<i32>(*res); // todo: better cast
        }

        size_t bytes_read = this->read_buffer.readable().size();
        if (bytes_read < packet_size) {
            size_t needed = packet_size - bytes_read;
            if (needed > 1024 * 1024 * 16) {
                return unexpected(monostate{});
            }
            this->read_buffer.occupy(needed);
            i32 a = 0;
            while (needed != 0) {
                a++;
                size_t read = this->replenish().value();
                if (read >= needed) {
                    break;
                }
                needed -= read;
                if (a > 100) {
                    throw i32(-1);
                }
            }
        }

        SpanCursor cursor(this->read_buffer.readable());
        this->read_buffer.advance_start(packet_size);

        auto packet_id = serialization::read_var_int(cursor);
        if (!packet_id) {
            return unexpected(monostate{});
        }
        if (*packet_id != P::ID) {
            return unexpected(monostate{});
        }
        return P::read_from(cursor);
    }

    template<typename P>
    expected<monostate, monostate> write_packet(const P& packet) {
        auto packet_writer = BufferWriter<P>::create(packet);
        i32 packet_size = packet_writer.size();
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
        auto writable = this->read_buffer.writable();
        auto res = this->inner.read(writable);
        if (!res) {
            return unexpected(monostate{});
        }
        size_t read = *res;
        this->read_buffer.advance_read(read);
        return read;
    }

    // todo: return size_t?
    expected<i32, monostate> read_packet_size() {
        for (;;) {
            SpanCursor cursor(this->read_buffer.readable());
            auto size = serialization::read_var_int(cursor);
            if (!size) {
                if (size.error() == VarIntReadError::Partial) {
                    if (auto res = this->replenish(); !res) {
                        return unexpected(monostate{});
                    }
                    continue;
                } else if (size.error() == VarIntReadError::Overflow) {
                    return unexpected(monostate{});
                }
            }
            this->read_buffer.advance_start(cursor.pos());
            return *size;
        }
    }

    void occupy_write_buf(size_t n) {
        if (this->write_buffer.size() < n) {
            this->write_buffer = DynBytes::make(n);
        }
    }
};
