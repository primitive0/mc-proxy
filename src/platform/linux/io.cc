#include "io.hh"

#include <unistd.h>

namespace platform_linux::io {
    i32 fd_close(RawFd fd) {
        return close(fd);
    }

    i32 fd_read(RawFd fd, u8* buf, size_t len) {
        return read(fd, buf, len);
    }

    i32 fd_write(RawFd fd, const u8* buf, size_t len) {
        return write(fd, buf, len);
    }
}
