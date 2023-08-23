#pragma once

#include "../prelude.hh"
#include "../serialization/codec.hh"
#include "../serialization/field_codec.hh"

struct C2S_SetCompression {
    i32 threshold;

    static constexpr auto codec() {
        return Codec<C2S_SetCompression,
            fc::VarInt<&C2S_SetCompression::threshold>>{};
    }
};
