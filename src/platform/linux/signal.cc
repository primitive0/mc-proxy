#include "signal.hh"

#include <csignal>

namespace platform_linux::signal {
    void set_signal_handler(SignalType signal, SignalHandler handler) {
        struct sigaction action;
        action.sa_handler = handler;
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;
        sigaction(i32(signal), &action, nullptr);
    }
}
