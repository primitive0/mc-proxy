#pragma once

#include "../prelude.hh"

#include "../serialization/deserializer.hh"
#include "../serialization/serializer.hh"

enum class HandshakeNextState {
    Status,
    Login
};

struct C2S_Handshake {
    i32 protocol_version;
    string server_address;
    u16 port;
    HandshakeNextState next_state;

    template<typename T>
    static expected<C2S_Handshake, monostate> read_from(Deserializer<T>& de) {
        C2S_Handshake packet{};

        auto protocol_version = de.read_var_int();
        if (!protocol_version) {
            return unexpected(monostate{});
        }
        packet.protocol_version = *protocol_version;

        auto server_address = de.read_string();
        if (!server_address) {
            return unexpected(monostate{});
        }
        packet.server_address = std::move(*server_address);

        auto port = de.read_u16();
        if (!port) {
            return unexpected(monostate{});
        }
        packet.port = *port;

        auto next_state_raw = de.read_var_int();
        if (!next_state_raw) {
            return unexpected(monostate{});
        }
        if (next_state_raw == 1) {
            packet.next_state = HandshakeNextState::Status;
        } else if (next_state_raw == 2) {
            packet.next_state = HandshakeNextState::Login;
        }

        return packet;
    }

    // auto write_queue() const {
    //     i32 next_status_byte;
    //     switch (this->next_state) {
    //     case HandshakeNextState::Status:
    //         next_status_byte = 1;
    //         break;
    //     case HandshakeNextState::Login:
    //         next_status_byte = 2;
    //         break;
    //     }

    //     return WriteQueue<>{}
    //         .push(VarIntSer(this->protocol_version))
    //         .push(StringSer(this->server_address))
    //         .push(U16Ser(this->port))
    //         .push(VarIntSer(next_status_byte));
    // }
};

// struct C2S_SA_Handshake {
//     i32 protocol_version;
//     string server_address;
//     bool is_server_auth_handshake;
//     u16 port;
//     HandshakeNextState next_state;

//     static expected<C2S_SA_Handshake, monostate> read_from(Cursor& cursor) {
//         auto mc_handshake = C2S_Handshake::read_from(cursor);
//         if (!mc_handshake) {
//             return unexpected(monostate{});
//         }

//         C2S_SA_Handshake packet{};
//         packet.protocol_version = mc_handshake->protocol_version;
//         packet.port = mc_handshake->port;
//         packet.next_state = mc_handshake->next_state;

//         if (auto index = mc_handshake->server_address.find('\0'); index != string::npos) {
//             auto server_address = mc_handshake->server_address.substr(0, index);
//             auto data = mc_handshake->server_address.substr(index + 1); // TODO: overflow
//             packet.server_address = server_address;
//             packet.is_server_auth_handshake = data == "SA";
//         } else {
//             packet.server_address = std::move(mc_handshake->server_address);
//             packet.is_server_auth_handshake = false;
//         }

//         return packet;
//     }
// };
