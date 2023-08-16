#include "shutdown_handler.hh"

#include "platform/linux/signal.hh"

using namespace platform_linux;

std::atomic<bool> shutdown_requested(false);

static void signal_handler(i32 sig) {
    if (sig != signal::SignalType::Interrupt && sig != signal::SignalType::Terminate) {
        return;
    }
    shutdown_requested.store(true);
}

void setup_signal_handler() {
    signal::set_signal_handler(signal::SignalType::Interrupt, signal_handler);
    signal::set_signal_handler(signal::SignalType::Terminate, signal_handler);
}
