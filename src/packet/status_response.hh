#pragma once

#include "../prelude.hh"

#include "../serialization/codec.hh"
#include "../serialization/field_codec.hh"

struct S2C_StatusResponse {
    string json;

    static constexpr auto codec() {
        return Codec<S2C_StatusResponse,
            fc::String<&S2C_StatusResponse::json>>{};
    }
};
