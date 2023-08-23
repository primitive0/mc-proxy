#pragma once

#include "../prelude.hh"

#include "../serialization/codec.hh"
#include "../serialization/field_codec.hh"

struct S2C_Disconnect {
    string json;

    static constexpr auto codec() {
        return Codec<S2C_Disconnect,
            fc::String<&S2C_Disconnect::json>>{};
    }
};
