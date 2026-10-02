#pragma once

#include <cstdint>
#include <variant>

#include "../core/ids.h"

namespace heddle {
    enum class Op : std::uint8_t {
        ThreadStart = 0,
        ThreadCreate = 1,
        ThreadJoin = 2,
        ThreadExit = 3,
        MutexLock = 4,
        MutexUnlock = 5,
    };

    struct ThreadStart {
        static constexpr auto kOp = Op::ThreadStart;
    };

    struct ThreadCreate {
        static constexpr auto kOp = Op::ThreadCreate;
    };

    struct ThreadJoin {
        static constexpr auto kOp = Op::ThreadJoin;
        ThreadId target;
    };

    struct ThreadExit {
        static constexpr auto kOp = Op::ThreadExit;
    };

    struct MutexLock {
        static constexpr auto kOp = Op::MutexLock;
        const void *mutex;
    };

    struct MutexUnlock {
        static constexpr auto kOp = Op::MutexUnlock;
        const void *mutex;
    };

    // Job that thread is about to do
    using Event = std::variant<ThreadStart, ThreadCreate, ThreadJoin,
        ThreadExit, MutexLock, MutexUnlock>;

    void initialise();
}
