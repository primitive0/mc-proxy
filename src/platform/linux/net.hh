#pragma once

#include "../../prelude.hh"

#include "linux.hh"

namespace platform_linux::net {
    enum SocketType : i32 {
        Stream = 1,
        Datagram = 2,
    };

    enum SocketOption : i32 {
        ReuseAddr,
    };

    RawFd socket_create(SocketType type);

    i32 socket_bind(RawFd fd, u32 ip, u16 port);

    i32 socket_listen(RawFd fd);

    expected<i32, LinuxError> socket_accept(RawFd listener_fd);

    expected<monostate, LinuxError> socket_setopt(RawFd socket_fd, SocketOption option, bool enable);
}
