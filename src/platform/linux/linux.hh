#pragma once

#include "../../prelude.hh"

namespace platform_linux {
    using RawFd = i32;

    enum class ErrorKind {
        Interrupted,
        Unknown,
    };

    struct LinuxError {
        ErrorKind kind;

        static LinuxError from_errno();
    };

    inline bool is_valid_fd(RawFd fd) {
        return fd != -1;
    }
}
