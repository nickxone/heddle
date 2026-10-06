#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <optional>
#include <span>

#include "core/event.h"

namespace heddle {
    inline constexpr std::size_t kMaxHeldMutexes = 256;

    enum class ThreadStatus : std::uint8_t { Unused, Live, Finished };

    enum class Violation : std::uint8_t { None, UnlockNotOwned, TooManyHeldMutexes };

    class ThreadSet {
    public:
        void push_back(const ThreadId id) noexcept {
            assert(size_ < ids_.size());
            ids_[size_++] = id;
        }

        [[nodiscard]] std::span<const ThreadId> view() const noexcept {
            return {ids_.data(), size_};
        }

        [[nodiscard]] bool empty() const noexcept {
            return size_ == 0;
        }

    private:
        std::array<ThreadId, kMaxThreads> ids_{};
        std::size_t size_ = 0;
    };

    class HeldMutexes {
    public:
        [[nodiscard]] std::optional<ThreadId> owner(const void *mutex) const noexcept;

        [[nodiscard]] bool acquire(const void *mutex, ThreadId thread) noexcept;

        [[nodiscard]] bool release(const void *mutex, ThreadId thread) noexcept;

    private:
        struct Entry {
            const void *mutex;
            ThreadId owner;
        };

        std::array<Entry, kMaxHeldMutexes> entries_{};
        std::size_t size_ = 0;
    };

    class Model {
    public:
        Model() noexcept;

        [[nodiscard]] std::optional<ThreadId> spawn() noexcept;
        // announce what thread wants to do next
        void announce(ThreadId id, const Event &next) noexcept;
        // indicates whether thread can finish without blocking (given that is has the token)
        [[nodiscard]] bool enabled(ThreadId id) const noexcept;

        [[nodiscard]] Violation commit(ThreadId id) noexcept;

        [[nodiscard]] ThreadSet runnable() const noexcept;

        [[nodiscard]] bool any_live() const noexcept;

        [[nodiscard]] ThreadStatus status(ThreadId id) const noexcept;

    private:
        struct Thread {
            ThreadStatus status = ThreadStatus::Unused;
            std::optional<Event> pending = ThreadStart{};
        };

        std::array<Thread, kMaxThreads> threads_{};
        std::uint32_t spawned_ = 0;
        HeldMutexes held_;
    };
}
