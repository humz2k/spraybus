#pragma once

/**
 * @file interrupts.hpp
 * @brief Simple SIGINT-aware run-loop helpers.
 */

#include <signal.h>
#include <thread>

namespace spraybus::common {

/// @cond INTERNAL
namespace interrupts_impl {
inline volatile sig_atomic_t should_run = 1;
inline void sigint_handler(int signal) { should_run = 0; }
inline void setup() { signal(SIGINT, sigint_handler); }
} // namespace interrupts_impl
/// @endcond

/**
 * @brief Run a callback repeatedly until SIGINT is received.
 *
 * The callback is invoked in a tight loop. Callers that poll non-blocking APIs
 * should include their own sleep or blocking wait inside @p lambda.
 *
 * @tparam F Callable type.
 * @param lambda Callback invoked once per loop iteration.
 */
template <typename F> inline void run_forever(F&& lambda) {
    interrupts_impl::setup();
    while (interrupts_impl::should_run) {
        lambda();
    }
}

/**
 * @brief Sleep in a loop until SIGINT is received.
 */
inline void run_forever() {
    interrupts_impl::setup();
    while (interrupts_impl::should_run) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        volatile int x = 0; // to not optimize out this loop
    }
}

} // namespace spraybus::common
