#include "../prelude.hh"

#include "../networking/deserializer.hh"

struct C2S_LoginStart {
    using UuidType = array<u8, 16>;

    string username;
    optional<UuidType> uuid;

    template<typename T>
    static expected<C2S_LoginStart, monostate> read_from(Deserializer<T>& de) {
        C2S_LoginStart packet{};

        auto username = de.read_string();
        if (!username) {
            return unexpected(monostate{});
        }
        packet.username = std::move(*username);

        auto has_player_uuid = de.read_bool();
        if (!has_player_uuid) {
            return unexpected(monostate{});
        }

        optional<UuidType> uuid = nullopt;
        if (*has_player_uuid) {
            auto data = de.read_bytes(8);
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
