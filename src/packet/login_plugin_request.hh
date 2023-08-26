#pragma once

#include "../prelude.hh"

#include "../util/dyn_bytes.hh"
#include "../serialization/codec.hh"
#include "../serialization/field_codec.hh"

struct S2C_LoginPluginRequest {
    i32 message_id;
    string identifier;
    DynBytes data;

    static constexpr auto codec() {
        return Codec<S2C_LoginPluginRequest,
            fc::VarInt<&S2C_LoginPluginRequest::message_id>,
            fc::String<&S2C_LoginPluginRequest::identifier>>{};
    }
};
