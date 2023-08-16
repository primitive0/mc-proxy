#include "../prelude.hh"

#include "../serialization.hh"

class RingBuffer {
    vector<u8> inner;
    // todo: rename
    size_t head;
    size_t tail;

    RingBuffer(vector<u8>&& v) : inner(obj::move(v)), head(0), tail(0) {}

public:
    RingBuffer() : inner() {}

    static inline RingBuffer create(size_t n) {
        return RingBuffer(vector<u8>(n, u8(0)));
    }

    size_t size() const {
        return this->inner.size();
    }

    inline size_t occupied() const {
        //todo: overflow?
        if (this->head <= this->tail) {
            return this->tail - this->head;
        }
        // head > tail
        return this->size() - this->head + this->tail;
    }

    inline size_t free() const {
        return this->size() - this->occupied();
    }

    inline VectoredBytes get_free() {
        VectoredBytes bytes{};
        // todo: overflow?
        // todo: iterators?
        if (this->head <= this->tail) {
            bytes.first = span(&this->inner[this->head], &this->inner[this->tail-1]);
        } else {
            bytes.first = span(&this->inner[this->head], &this->inner.back());
            bytes.second = span(&this->inner.front(), &this->inner[this->tail-1]);
        }
        return bytes;
    }

    inline bool advance_tail(size_t n) {
        if (n > this->free() - 1) {
            return false;
        }
        this->tail = (this->tail + n) % this->size();
        return true;
    }

    inline VectoredBytes occupy(size_t n) {
        size_t available = this->size() - this->occupied();
        if (n > available) {
            size_t needed = n - available;
            //todo: overflow?
            this->grow(this->size() + needed);
        }
    }

    inline bool is_empty() {
        return this->head == this->tail;
    }

    inline expected<u8, monostate> next_u8() {
        if (this->is_empty()) {
            return unexpected(monostate{});
        }
        u8 byte = this->inner[this->head];
        // todo: overflow?
        this->head++;
        if (this->head == this->inner.size()) {
            this->head = 0;
        }
        return byte;
    }

    // todo: fix
    inline expected<VectoredBytes, monostate> next_bytes(size_t n) {
        VectoredBytes bytes{};
        if (this->head < this->tail) {
            //todo: overflow?
            size_t available = this->tail - this->head;
            if (n > available) {
                return unexpected(monostate{});
            }
            // todo: iterators?
            bytes.first = span(&this->inner[this->head], &this->inner[this->tail-1]);
        } else if (this->head > this->tail) {
            size_t available = this->size() - this->head + this->tail;
            if (n > available) {
                return unexpected(monostate{});
            }
            bytes.first = span(&this->inner[this->head], &this->inner.back());
            bytes.second = span(&this->inner.front(), &this->inner[this->tail-1]);
        }

        this->head = (this->head + n) % this->size();

        return bytes;
    }

private:
    inline void grow(size_t new_size) {
        //todo: overflow?
        vector<u8> new_inner(new_size, u8(0));
        if (this->head == this->tail) {
            return;
        } else if (this->head < this->tail) {
            mem::memcpy(new_inner.data(), &this->inner[this->head], this->tail - this->head);
        } else if (this->head > this->tail) {
            mem::memcpy(new_inner.data(), &this->inner[this->head], this->tail - this->head);
            mem::memcpy(new_inner.data(), &this->inner[this->head], this->tail - this->head);
        }
    }
};
