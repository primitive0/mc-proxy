#pragma once

#include "../prelude.hh"
#include "../serialization/deserializer.hh"

struct C2S_StatusRequest {

    static const i32 ID = 0;

    template<typename T>
    static expected<C2S_StatusRequest, monostate> read_from(CursorRead<T>& c) {
        return C2S_StatusRequest{};
    }
};
