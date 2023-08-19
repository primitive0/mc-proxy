#pragma once

#include "../prelude.hh"

template<typename T = void, typename P = void>
struct WriteQueue {
    T _val;
    P _prev;

    WriteQueue(T val, P prev) : _val(val), _prev(prev) {}

    template<typename X>
    auto push(X val) {
        if constexpr (X::DELEGATED) {
            return val.on_push(*this);
        } else {
            WriteQueue<X, WriteQueue<T, P>> next(val, *this);
            return next;
        }
    }

    template<typename A>
    auto apply(A accum) {
        auto it = this->_prev.apply(accum);
        return it.consume(_val);
    }

    template<typename A>
    auto apply() {
        return this->apply(A::make());
    }
};

template<typename T>
struct WriteQueue<T, void> {
    T _val;

    WriteQueue(T val) : _val(val) {}

    template<typename X>
    auto push(X val) {
        if constexpr (X::DELEGATED) {
            return val.on_push(*this);
        } else {
            WriteQueue<X, WriteQueue<T, void>> next(val, *this);
            return next;
        }
    }

    template<typename A>
    auto apply(A accum) {
        return accum.consume(_val);
    }

    template<typename A>
    auto apply() {
        return this->apply(A::make());
    }
};

template<>
struct WriteQueue<void, void> {
    WriteQueue() = default;

    template<typename T>
    auto push(T val) {
        if constexpr (T::DELEGATED) {
            return val.on_push(*this);
        } else {
            WriteQueue<T, void> next(val);
            return next;
        }
    }

    template<typename A>
    auto apply() {
        return A::make();
    }
};

template<typename W = void, typename P = void>
struct BufferAccum {
    P prev;
    W writer;
    size_t max_size = 0;

    BufferAccum(const P& prev, W writer) : prev(prev), writer(writer), max_size(prev.max_size) {}

    template<typename S>
    auto consume(S ser) {
        auto [writer, max_size] = ser.writer();
        BufferAccum<decltype(writer), BufferAccum<W, P>> next(*this, writer);
        next.max_size += max_size;
        return next;
    }
};

template<typename W>
struct BufferAccum<W, void> {
    W writer;
    size_t max_size;

    BufferAccum(W w, size_t ms) : writer(w), max_size(ms) {}

    template<typename S>
    auto consume(S ser) {
        auto [writer, max_size] = ser.writer();
        BufferAccum<decltype(writer), BufferAccum<W, void>> next(*this, writer);
        next.max_size += max_size;
        return next;
    }
};

template<>
struct BufferAccum<void, void> {
    size_t max_size = 0;

    static BufferAccum<> make() {
        return BufferAccum<>{};
    }

    template<typename S>
    auto consume(S ser) {
        auto [writer, max_size] = ser.writer();
        BufferAccum<decltype(writer)> next(writer, max_size);
        return next;
    }
};

constexpr size_t var_int_bytes(int32_t input) {
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

struct VarIntSer {
    static constexpr bool DELEGATED = false;

    int32_t data;

    VarIntSer(int32_t v) : data(v) {}

    VarIntSer& operator=(const VarIntSer& rhs) {
        this->data = rhs.data;
        return *this;
    }

    std::pair<VarIntSer, size_t> writer() {
        size_t size = var_int_bytes(this->data);
        return std::make_pair(*this, size);
    }
};

struct BytesSer {
    static constexpr bool DELEGATED = false;

    std::span<const std::byte> data;

    BytesSer(std::span<const std::byte> v) : data(v) {}

    std::pair<BytesSer, size_t> writer() {
        size_t size = this->data.size();
        return std::make_pair(*this, size);
    }
};

struct StringSer {
    static constexpr bool DELEGATED = true;

    const std::string& data;

    StringSer(const std::string& v) : data(v) {}

    template<typename Q>
    auto on_push(Q queue) {
        std::span str_span(reinterpret_cast<const std::byte*>(this->data.data()), this->data.size());
        return queue
            .push(VarIntSer(this->data.size()))
            .push(BytesSer(str_span));
    }
};

struct MyPacket {
    int32_t protocol = 700;
    std::string address = "localhost";
    int32_t port = 1010;

    auto write_queue() const {
        return WriteQueue<>{}
            .push(VarIntSer(this->protocol))
            .push(StringSer(this->address))
            .push(VarIntSer(this->port));
    }
};
