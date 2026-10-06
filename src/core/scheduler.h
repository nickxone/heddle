#pragma once

#include "core/model.h"
#include "core/strategy.h"

namespace heddle {
    struct Deadlock {
    };

    struct AllFinished {
    };

    using Decision = std::variant<ThreadId, Deadlock, AllFinished>;

    [[nodiscard]] Decision decide(const Model &model, Strategy &strategy,
                                  ThreadId current, std::uint64_t step);
}
