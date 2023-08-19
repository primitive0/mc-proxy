#pragma once

#include "../prelude.hh"

template<typename S = void, typename P = void>
struct WriteQueue {
    S ser;
    P prev;

    WriteQueue(S&& v, P&& prev) : ser(std::move(v)), prev(std::move(prev)) {}

    WriteQueue(const WriteQueue&) = delete;
    WriteQueue(WriteQueue&&) = default;

    template<typename N>
    auto push(N&& next_ser) {
        if constexpr (N::DELEGATED) {
            return next_ser.on_push(std::move(*this));
        } else {
            WriteQueue<N, WriteQueue<S, P>> next(std::move(next_ser), std::move(*this));
            return next;
        }
    }

    template<typename A>
    auto apply(A&& accum) {
        return this->prev.apply(std::move(accum)).consume(std::move(this->ser));
    }

    template<typename A>
    auto apply() {
        return this->apply(A::make());
    }
};

template<typename S>
struct WriteQueue<S, void> {
    S ser;

    WriteQueue(S&& v) : ser(std::move(v)) {}

    WriteQueue(const WriteQueue&) = delete;
    WriteQueue(WriteQueue&&) = default;

    template<typename N>
    auto push(N&& next_ser) {
        if constexpr (N::DELEGATED) {
            return next_ser.on_push(std::move(*this));
        } else {
            WriteQueue<N, WriteQueue<S, void>> next(std::move(next_ser), std::move(*this));
            return next;
        }
    }

    template<typename A>
    auto apply(A&& accum) {
        return accum.consume(std::move(this->ser));
    }

    template<typename A>
    auto apply() {
        return this->apply(A::make());
    }
};

template<>
struct WriteQueue<void, void> {
    WriteQueue() = default;

    WriteQueue(const WriteQueue&) = delete;
    WriteQueue(WriteQueue&&) = default;

    template<typename N>
    auto push(N&& next_ser) {
        if constexpr (N::DELEGATED) {
            return next_ser.on_push(std::move(*this));
        } else {
            WriteQueue<N, void> next(std::move(next_ser));
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
    size_t size;

    BufferAccum(P&& prev, W&& writer) : prev(std::move(prev)), writer(std::move(writer)), size(prev.size) {}

    BufferAccum(const BufferAccum&) = delete;
    BufferAccum(BufferAccum&&) = default;

    template<typename S>
    auto consume(S&& ser) {
        auto writer = ser.writer();
        auto size = writer.size();
        BufferAccum<decltype(writer), BufferAccum<W, P>> next(std::move(*this), std::move(writer));
        next.size += size;
        return next;
    }

    size_t write(span<u8> buf, size_t cursor) {
        cursor = this->prev.write(buf, cursor);
        // todo: overflow check
        u8* ptr = buf.data() + cursor;
        writer.write(ptr);
        return cursor + this->writer.size();
    }

    void write(span<u8> buf) {
        this->write(buf, 0);
    }
};

template<typename W>
struct BufferAccum<W, void> {
    W writer;
    size_t size;

    BufferAccum(W&& w, size_t size) : writer(std::move(w)), size(size) {}

    BufferAccum(const BufferAccum&) = delete;
    BufferAccum(BufferAccum&&) = default;

    template<typename S>
    auto consume(S&& ser) {
        auto writer = ser.writer();
        auto size = writer.size();
        BufferAccum<decltype(writer), BufferAccum<W, void>> next(std::move(*this), std::move(writer));
        next.size += size;
        return next;
    }

    size_t write(span<u8> buf, size_t cursor) {
        // todo: overflow check
        u8* ptr = buf.data() + cursor;
        writer.write(ptr);
        return cursor + this->writer.size();
    }

    void write(span<u8> buf) {
        this->write(buf, 0);
    }
};

template<>
struct BufferAccum<void, void> {
    size_t size = 0;

    BufferAccum() {}

    BufferAccum(const BufferAccum&) = delete;
    BufferAccum(BufferAccum&&) = default;

    static BufferAccum<> make() {
        return BufferAccum<>{};
    }

    template<typename S>
    auto consume(S&& ser) {
        auto writer = ser.writer();
        size_t size = writer.size();
        BufferAccum<decltype(writer), void> next(std::move(writer), size);
        return next;
    }

    void write(span<u8> buf) {
        return;
    }
};
