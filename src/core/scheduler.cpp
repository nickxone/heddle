#include "core/scheduler.h"

namespace heddle {
    Decision decide(const Model &model, Strategy &strategy,
                    const ThreadId current, const std::uint64_t step) {
        const ThreadSet runnable = model.runnable();
        if (runnable.empty()) {
            return model.any_live()
                       ? Decision{Deadlock{}}
                       : Decision{AllFinished{}};
        }

        const SchedulingContext context = {
            .runnable = runnable.view(),
            .current = current,
            .step = step
        };
        return strategy.choose(context);
    }
}
