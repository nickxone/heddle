#include "random_strategy.h"

namespace heddle {
    ThreadId RandomStrategy::choose(const SchedulingContext &ctx) {
        return ctx.runnable[rng_.bounded(ctx.runnable.size())];
    }
}
