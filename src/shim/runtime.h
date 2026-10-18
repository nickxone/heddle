#pragma once

#include <pthread.h>

#include <array>
#include <cstdint>
#include <optional>
#include <semaphore>

#include "core/model.h"
#include "core/random_strategy.h"
#include "core/scheduler.h"

namespace heddle {
using StartRoutine = void *(*)(void *);

class Runtime {
  public:
    explicit Runtime(std::uint64_t seed) noexcept;

    void schedule(const Event &next) noexcept;

    [[nodiscard]] int create_thread(pthread_t *thread, const pthread_attr_t *attr,
                                         StartRoutine start, void *arg) noexcept;
    [[nodiscard]] std::optional<ThreadId> find(pthread_t handle) const noexcept;
    void exit_current() noexcept;

  private:
    using Token = std::binary_semaphore;

    struct Slot {
        Token token{0};
        pthread_t handle{};
        StartRoutine start = nullptr;
        void *arg = nullptr;
    };

    static void *run_child(void *encoded_id);

    [[nodiscard]] Slot &slot(ThreadId id) noexcept;
    void hand_off(const Decision &decision) noexcept;
    void check(Violation violation) noexcept;

    Model model_;
    RandomStrategy strategy_;
    std::uint64_t step_ = 0;
    std::array<Slot, kMaxThreads> slots_;
};

[[nodiscard]] Runtime *current_runtime() noexcept;
}  // namespace heddle