#pragma once

#include "prelude.hh"

namespace io {
    template<typename T>
    concept Readable = requires(T r, span<u8> buf) {
        { r.read(buf) } -> std::same_as<expected<size_t, monostate>>;
    };

    template<typename T>
    concept Writable = requires(T w, span<const u8> buf) {
        { w.write(buf) } -> std::same_as<expected<size_t, monostate>>;
    };

    expected<monostate, monostate> read_all(Readable auto r, span<u8> buf) {
    }
}
