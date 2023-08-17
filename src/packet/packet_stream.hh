#include "../prelude.hh"

#include "../io.hh"
#include "../networking/simple_buffer.hh"

template<typename S>
requires io::Readable<S> && io::Writable<S>
class PacketStream {
    S inner;
    SimpleBuffer buffer{};

public:
    PacketStream(S&& s) : inner(obj::move(s)) {
        this->buffer = SimpleBuffer::create(512);
    }

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
};
