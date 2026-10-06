#pragma once

#include <cstdint>
#include <span>

#include "ids.h"

namespace heddle {
struct SchedulingContext {
    std::span<const ThreadId> runnable;  // sorted ascending, never empty
    ThreadId current;
    std::uint64_t step;
};

class Strategy {
  public:
    Strategy() = default;

    Strategy(const Strategy &) = delete;

    Strategy &operator=(const Strategy &) = delete;

    virtual ~Strategy() = default;

    // Must return an element of `ctx.runnable` and be a pure function of the seed
    [[nodiscard]] virtual ThreadId choose(const SchedulingContext &ctx) = 0;

    virtual void on_thread_created(ThreadId) {}
};
}  // namespace heddle
