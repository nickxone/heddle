#pragma once

#include "rng.h"
#include "strategy.h"

namespace heddle {
    class RandomStrategy : public Strategy {
    public:
        explicit RandomStrategy(std::uint64_t seed) noexcept : rng_(seed) {
        }

        [[nodiscard]] ThreadId choose(const SchedulingContext &ctx) override;

    private:
        Rng rng_;
    };
}
