#include "../prelude.hh"

#include "../serialization/write_queue.hh"
#include "../serialization/serializer.hh"
#include "../serialization/codec.hh"
#include "../serialization/field_codec.hh"

struct S2C_Disconnect {
    string json;

    i32 test; // todo: remove

    auto write_queue() const {
        return WriteQueue<>{}
            .push(StringSer(this->json));
    }

    static constexpr auto codec() {
        return Codec<S2C_Disconnect, fc::VarInt<&S2C_Disconnect::test>>{};
    }
};
