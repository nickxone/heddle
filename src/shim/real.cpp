#include "shim/real.h"

#include <dlfcn.h>

#include <atomic>

#include "shim/fatal.h"

namespace heddle {
namespace {
constinit RealFns g_real{};
constinit std::atomic<bool> g_resolved{false};
constinit thread_local bool t_resolving = false;

template <typename F>
F resolve(const char *name) noexcept {
    void *symbol = ::dlsym(RTLD_NEXT, name);
    if (symbol == nullptr) {
        fatal({"cannot resolve ", name});
    }

    return reinterpret_cast<F>(symbol);
}

void resolve_all() noexcept {
    if (t_resolving) {
        fatal({"symbol resolution re-entered itself"});
    }

    t_resolving = true;
    g_real = {
        .create = resolve<decltype(RealFns::create)>("pthread_create"),
        .join = resolve<decltype(RealFns::join)>("pthread_join"),
        .exit = resolve<decltype(RealFns::exit)>("pthread_exit"),
        .mutex_lock = resolve<decltype(RealFns::mutex_lock)>("pthread_mutex_lock"),
        .mutex_unlock = resolve<decltype(RealFns::mutex_unlock)>("pthread_mutex_unlock"),
    };
    t_resolving = false;
    g_resolved.store(true, std::memory_order_release);
}

// indicate to the compiler that the pointers to functions need to be looked up early
[[gnu::constructor(101)]] void resolve_at_load() noexcept { static_cast<void>(real()); }

}  // namespace

const RealFns &real() noexcept {
    if (!g_resolved.load(std::memory_order_acquire)) {
        resolve_all();
    }
    return g_real;
}
}  // namespace heddle