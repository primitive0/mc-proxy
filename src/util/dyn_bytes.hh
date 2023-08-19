#pragma once

#include "../prelude.hh"

class DynBytes {
    u8* ptr = nullptr;
    size_t _size = 0;

    explicit DynBytes(u8* ptr, size_t size) : ptr(ptr), _size(size) {}

public:
    constexpr DynBytes() = default;

    DynBytes& operator=(DynBytes&& other) {
        this->ptr = other.ptr;
        this->_size = other._size;

        other.ptr = nullptr;
        other._size = 0;

        return *this;
    }

    DynBytes(DynBytes&& other) {
        this->operator=(obj::move(other));
    }

    DynBytes(const DynBytes&) = delete;

    DynBytes& operator=(const DynBytes&) = delete;

    ~DynBytes() {
        if (this->ptr != nullptr) {
            std::free(this->ptr);
        }
    }

    static DynBytes make(size_t size) {
        auto ptr = static_cast<u8*>(std::malloc(size));
        if (ptr == nullptr) {
            throw std::bad_alloc{};
        }
        return DynBytes(ptr, size);
    }

public:
    size_t size() const {
        return this->_size;
    }

    span<u8> as_span() {
        return span(this->ptr, this->size());
    }

    span<const u8> as_span() const {
        return span(this->ptr, this->size());
    }
};
