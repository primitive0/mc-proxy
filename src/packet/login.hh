#pragma once

#include "../prelude.hh"

#include "../serialization/deserializer.hh"

struct C2S_LoginStart {
    using UuidType = array<u8, 16>;

    string username;
    optional<UuidType> uuid;

    template<typename T>
    static expected<C2S_LoginStart, monostate> read_from(CursorRead<T>& c) {
        C2S_LoginStart packet{};

        auto username = serialization::read_string(c);
        if (!username) {
            return unexpected(monostate{});
        }
        packet.username = std::move(*username);

        auto has_player_uuid = serialization::read_bool(c);
        if (!has_player_uuid) {
            return unexpected(monostate{});
        }

        optional<UuidType> uuid = nullopt;
        if (*has_player_uuid) {
            auto data = c.read_bytes(8);
            if (!data) {
                return unexpected(monostate{});
            }
            uuid = UuidType();
            mem::memcpy(uuid->data(), data->data(), 8);
        }
        packet.uuid = obj::move(uuid);

        return packet;
    }
};
