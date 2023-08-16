#pragma once

#include "prelude.hh"

extern std::atomic<bool> shutdown_requested;
static_assert(std::atomic<bool>::is_always_lock_free);

void setup_signal_handler();
