#include "../prelude.hh"

#include "../serialization/write_queue.hh"
#include "../serialization/serializer.hh"

struct S2C_Disconnect {
    string json;

    auto write_queue() const {
        return WriteQueue<>{}
            .push(StringSer(this->json));
    }
};
