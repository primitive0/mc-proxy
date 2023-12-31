#include "prelude.hh"

#include "net/stream.hh"

#include "packet/handshake.hh"
#include "packet/status_request.hh"
#include "packet/status_response.hh"
#include "packet/packet_stream.hh"
#include "packet/disconnect.hh"

#include "shutdown_handler.hh"

#include "serialization/buffer_writer.hh"

using namespace net;

void handle_connection(TcpStream&&);

i32 main() {
    setup_shutdown_handler();

    auto listener = TcpListener::bind(IpAddrV4::UNSPECIFIED, 8080).value();

    for (;;) {
        auto client = listener.accept();
        if (!client) {
            auto& err = client.error();
            if (err.kind == platform_linux::ErrorKind::Interrupted && shutdown_requested.load()) {
                std::cout << "shutting down...\n";
                break;
            }
        }
        handle_connection(obj::move(*client));
        // std::thread t(handle_connection, obj::move(client));
    }

    return 0;
}

// i32 main() {
//     S2C_Disconnect packet{};
//     packet.json = "";

//     auto buffer_writer = BufferWriter<S2C_Disconnect>::create(packet);
//     DynBytes buf = DynBytes::make(buffer_writer.size());
//     buffer_writer.write(buf.as_span());
//     print_array(buf.as_span());

//     return 0;
// }

void handle_connection(TcpStream&& client) {
    std::cout << "new connection\n";

    PacketStream packet_stream(obj::move(client));

    auto handshake = packet_stream.read_packet<C2S_Handshake>();
    if (!handshake) {
        std::cout << "failed to read packet\n";
        return;
    }

    std::cout << "protocol version = " << handshake->protocol_version << "\n";
    std::cout << "server address = " << handshake->server_address << "\n";
    std::cout << "port = " << handshake->port << "\n";

    if (handshake->next_state == HandshakeNextState::Status) {
        packet_stream.read_packet<C2S_StatusRequest>().value();
        packet_stream.write_packet(S2C_StatusResponse{"{\"version\":{\"name\":\"1.19.4\",\"protocol\":762},\"players\":{\"max\":100,\"online\":5,\"sample\":[{\"name\":\"thinkofdeath\",\"id\":\"4566e69f-c907-48ee-8d71-d7ba5aa00d20\"}]},\"description\":{\"text\":\"Hello world\"},\"enforcesSecureChat\":true,\"previewsChat\":true}"});
        return;
    }

    packet_stream.write_packet(S2C_Disconnect{"{\"text\":\"foo\"}"});

    // auto size = cursor.read_var_int();
    // auto id = cursor.read_var_int();
    // if (!size || !id) {
    //     std::cout << "failed to read frame" << "\n";
    //     return;
    // }
}
