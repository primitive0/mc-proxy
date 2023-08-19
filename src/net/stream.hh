#pragma once

#include "../prelude.hh"

#include "ip_addr.hh"

#include "../platform/linux/io.hh"
#include "../platform/linux/net.hh"

namespace net {
    namespace impl = platform_linux;

    class TcpStream {
        impl::RawFd socket_fd = -1;

        TcpStream() {}

        explicit TcpStream(impl::RawFd fd) : socket_fd(fd) {}

        TcpStream(const TcpStream&) = delete;

    public:
        TcpStream(TcpStream&& other) {
            this->socket_fd = other.socket_fd;
            other.socket_fd = -1;

            std::cout << "moved TcpStream(" << this->socket_fd << ")\n";
        }

        expected<size_t, monostate> read(span<u8> buf) {
            size_t read = impl::io::fd_read(this->socket_fd, buf.data(), buf.size());
            if (read == -1) {
                return unexpected(monostate{});
            }
            return read;
        }

        expected<size_t, monostate> write(span<const u8> buf) {
            size_t written = impl::io::fd_write(this->socket_fd, buf.data(), buf.size());
            if (written == -1) {
                return unexpected(monostate{});
            }
            return written;
        }

        void close() {
            if (impl::is_valid_fd(this->socket_fd)) {
                std::cout << "closed TcpStream(" << this->socket_fd << ")\n";
                impl::io::fd_close(this->socket_fd);
                this->socket_fd = -1;
            }
        }

        ~TcpStream() {
            this->close();
        }

        static TcpStream from_raw_fd(impl::RawFd fd) {
            std::cout << "opened TcpStream(" << fd << ")\n";
            return TcpStream(fd);
        }
    };

    class TcpListener {
        impl::RawFd socket_fd = -1;

        TcpListener() {}

        explicit TcpListener(impl::RawFd fd) : socket_fd(fd) {}

        TcpListener(const TcpListener&) = delete;

    public:
        TcpListener(TcpListener&& other) {
            this->socket_fd = other.socket_fd;
            other.socket_fd = -1;
        }

        expected<TcpStream, impl::LinuxError> accept() {
            auto client_socket_fd = impl::net::socket_accept(this->socket_fd);
            if (!client_socket_fd) {
                return unexpected(client_socket_fd.error());
            }
            return TcpStream::from_raw_fd(*client_socket_fd);
        }

        static expected<TcpListener, monostate> bind(IpAddrV4 ip, u16 port) {
            impl::RawFd socket_fd = impl::net::socket_create(impl::net::SocketType::Stream);
            if (socket_fd == -1) {
                return unexpected(monostate{});
            }

            {
                auto res = impl::net::socket_setopt(socket_fd, impl::net::SocketOption::ReuseAddr, true);
                if (!res) {
                    return unexpected(monostate{});
                }
            }

            if (impl::net::socket_bind(socket_fd, ip.as_u32(), port) == -1) {
                impl::io::fd_close(socket_fd);
                return unexpected(monostate{});
            }

            if (impl::net::socket_listen(socket_fd) == -1) {
                impl::io::fd_close(socket_fd);
                return unexpected(monostate{});
            }

            std::cout << "opened TcpListener(" << socket_fd << ")\n";

            return TcpListener(socket_fd);
        }

        void close() {
            if (impl::is_valid_fd(this->socket_fd)) {
                std::cout << "closed TcpListener(" << this->socket_fd << ")\n";
                impl::io::fd_close(this->socket_fd);
                this->socket_fd = -1;
            }
        }

        ~TcpListener() {
            this->close();
        }
    };
}
