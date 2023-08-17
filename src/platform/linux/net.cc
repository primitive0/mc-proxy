#include "net.hh"

#include <sys/socket.h>
#include <netinet/in.h>

namespace platform_linux::net {
    RawFd socket_create(SocketType type) {
        return socket(AF_INET, type, 0);
    }

    i32 socket_bind(RawFd fd, u32 ip, u16 port) {
        sockaddr_in addr_in{};
        addr_in.sin_family = AF_INET;
        addr_in.sin_addr.s_addr = ip;
        addr_in.sin_port = htons(port);
        auto addr = std::bit_cast<sockaddr>(addr_in);
        return bind(fd, &addr, sizeof(addr));
    }

    i32 socket_listen(RawFd fd) {
        // TODO: change?
        return listen(fd, 128);
    }

    expected<i32, LinuxError> socket_accept(RawFd listener_fd) {
        sockaddr addr{};
        socklen_t addr_len = sizeof(addr);
        RawFd fd = accept(listener_fd, &addr, &addr_len);
        // todo: remove
        if (fd == -1) {
            LinuxError err = LinuxError::from_errno();
            return unexpected(err);
        }
        return fd;
    }

    expected<monostate, LinuxError> socket_setopt(RawFd socket_fd, SocketOption option, bool enable) {
        int opt = 0;
        if (option == SocketOption::ReuseAddr) {
            opt = SO_REUSEADDR;
        } else {
            opt = -1;
        }
        int v = enable ? 1 : 0;
        if (setsockopt(socket_fd, SOL_SOCKET, opt, &v, sizeof(int)) == -1) {
            LinuxError err = LinuxError::from_errno();
            return unexpected(err);
        }
        return monostate{};
    }
}
