#include "../prelude.hh"

#include "../io.hh"
#include "../serialization.hh"
#include "ring_buffer.hh"

template<typename S>
requires io::Readable<S> && io::Writable<S>
class PacketStream {
    S inner;
    RingBuffer buffer{};

public:
    PacketStream(S&& s) : inner(obj::move(s)) {
        this->buffer = RingBuffer::create(512);
    }

    template<typename P>
    expected<P, monostate> read_packet() {
        // todo: overflow checks

        size_t bytes_read;

        if (this->buffer.is_empty()) {
            VectoredBytes buf = this->buffer.get_free();
            bytes_read = this->inner.read(buf.first);
            if (bytes_read == 0) {
                //todo:???
            }
            if (!this->buffer.advance_tail(bytes_read)) {
                // should always work; throw?
            }
        }

        size_t varint_size = 0;
        size_t packet_size = 0;
        {
            const u8 CONTINUE_BIT = (u8(1) << 7);
            const u8 SEGMENT_BITS = static_cast<u8>(~CONTINUE_BIT);

            i32 value = 0;
            u8 pos = 0;
            for (;;) {
                auto byte = this->next_u8();
                varint_size++;
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
            packet_size = static_cast<size_t>(value);
        }

        bytes_read -= varint_size;
        if (bytes_read < packet_size) {
            size_t needed = packet_size - bytes_read;
            this->buffer.occupy(needed);
            while (needed != 0) {
                VectoredBytes buf = this->buffer.get_free();
                size_t bytes_read1 = this->inner.read(buf.first);
                if (bytes_read == 0) {
                    //todo:???
                }
                if (!this->buffer.advance_tail(bytes_read)) {
                    // should always work; throw?
                }

                needed -= bytes_read1; // todo: FIX!
            }
        }

        Deserializer de(this->buffer);
        return P::read_from(de);
    }
};
