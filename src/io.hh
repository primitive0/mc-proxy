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

    expected<monostate, monostate> write_all(Writable auto& w, span<const u8> buf) {
        size_t to_write = buf.size();
        for (;;) {
            auto res = w.write(buf);
            if (!res) {
                return unexpected(monostate{});
            }
            if (*res >= to_write) {
                break;
            }
            to_write -= *res;
        }
        return monostate{};
    }

    // expected<monostate, monostate> read_all(Readable auto r, span<u8> buf) {
    // }
}
