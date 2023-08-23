#pragma once

#include "../prelude.hh"

class DynBytes {
    u8* ptr = nullptr;
    size_t _size = 0;

    explicit DynBytes(u8* ptr, size_t size) : ptr(ptr), _size(size) {}

public:
    constexpr DynBytes() = default;

    DynBytes& operator=(DynBytes&& other) {
        this->free_unsafe();

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
        this->free_unsafe();
    }

    static DynBytes make(size_t size) {
        auto ptr = static_cast<u8*>(std::malloc(size));
        if (ptr == nullptr) {
            throw std::bad_alloc{};
        }
        return DynBytes(ptr, size);
    }

private:
    void free_unsafe() {
        if (this->ptr != nullptr) {
            std::free(this->ptr);
        }
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

    u8& at(size_t i) {
        return this->as_span()[i];
    }

    const u8& at(size_t i) const {
        return this->as_span()[i];
    }
};
