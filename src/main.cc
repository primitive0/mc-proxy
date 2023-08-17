#include "prelude.hh"

#include "net/stream.hh"

#include "packet/handshake.hh"
#include "packet/packet_stream.hh"

#include "shutdown_handler.hh"

using namespace net;

void handle_connection(TcpStream&&);

i32 main() {
    setup_signal_handler();

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
    // auto res = IpAddrV4::parse("127.0.0.1");
    // if (!res) {
    //     std::cout << "failed to parse\n";
    // }
    // print_array<const u8>(res.value().data);

    return 0;
}

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

    // auto size = cursor.read_var_int();
    // auto id = cursor.read_var_int();
    // if (!size || !id) {
    //     std::cout << "failed to read frame" << "\n";
    //     return;
    // }
}
