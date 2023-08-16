#pragma once

#include "../../prelude.hh"

#include "linux.hh"

namespace platform_linux::io {
    i32 fd_close(RawFd fd);

    i32 fd_read(RawFd fd, u8* buf, size_t len);

    i32 fd_write(RawFd fd, const u8* buf, size_t len);
}
