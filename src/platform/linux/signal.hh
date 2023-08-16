#pragma once

#include "../../prelude.hh"

#include "linux.hh"

namespace platform_linux::signal {
    using SignalHandler = void(*)(i32);

    enum SignalType : i32 {
        Interrupt = 2,
        Terminate = 15,
    };

    void set_signal_handler(SignalType, SignalHandler);
}
