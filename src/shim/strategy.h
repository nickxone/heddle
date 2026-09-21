#pragma once

#include <cstdint>
#include <span>

#include "ids.h"

namespace heddle {
    struct SchedulingContext {
        std::span<const ThreadId> runnable;
        ThreadId current;
        std::uint64_t step;
    };

    class Strategy {
    public:
        Strategy() = default;

        Strategy(const Strategy &) = delete;

        Strategy &operator=(const Strategy &) = delete;

        virtual ~Strategy() = default;

        [[nodiscard]] virtual ThreadId choose(const SchedulingContext &ctx) = 0;

        virtual void on_thread_created(ThreadId) {
        }
    };
}
