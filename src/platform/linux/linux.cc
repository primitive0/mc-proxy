#include "linux.hh"

#include <errno.h>

namespace platform_linux {
    LinuxError LinuxError::from_errno() {
        ErrorKind kind{};
        switch (errno) {
            case EINTR:
                kind = ErrorKind::Interrupted;
                break;
            default:
                kind = ErrorKind::Unknown;
                break;
        }

        LinuxError error{};
        error.kind = kind;
        return error;
    }
}
