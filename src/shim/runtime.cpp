#include "shim/runtime.h"

#include <atomic>
#include <charconv>
#include <cstdlib>
#include <ranges>
#include <string_view>
#include <variant>

#include "core/parse.h"
#include "shim/fatal.h"
#include "shim/real.h"

namespace heddle {
namespace {
constinit std::atomic<Runtime *> g_runtime{nullptr};
constinit thread_local std::optional<ThreadId> t_self{};

class Decimal {
  public:
    explicit Decimal(std::uint64_t value) noexcept
        : end_{std::to_chars(digits_.begin(), digits_.end(), value).ptr} {}

    [[nodiscard]] std::string_view view() const noexcept { return {digits_.data(), end_}; }

  private:
    std::array<char, 20> digits_{};
    const char *end_;
};

void *encode(ThreadId id) noexcept {
    return reinterpret_cast<void *>(static_cast<std::uintptr_t>(index(id)));
}

ThreadId decode(void *encoded) noexcept {
    return static_cast<ThreadId>(reinterpret_cast<std::uintptr_t>(encoded));
}

[[gnu::constructor]] void start_runtime() noexcept {
    const char *env = std::getenv("HEDDLE_SEED");
    if (env == nullptr) {
        return;
    }
    const std::optional<std::uint64_t> seed = parse_number<std::uint64_t>(env);
    if (!seed) {
        fatal({"HEDDLE_SEED is not a valid seed: ", env});
    }
    t_self = kMainThread;
    g_runtime.store(new Runtime(*seed), std::memory_order_release);
}
}  // namespace

Runtime::Runtime(const std::uint64_t seed) noexcept : strategy_{seed} {
    slot(kMainThread).handle = ::pthread_self();
    check(model_.commit(kMainThread));
}

void Runtime::schedule(const Event &next) noexcept {
    const ThreadId self = *t_self;
    model_.announce(self, next);
    const Decision decision = decide(model_, strategy_, self, step_++);
    if (const auto *chosen = std::get_if<ThreadId>(&decision);
        chosen == nullptr || *chosen != self) {
        hand_off(decision);
        slot(self).token.acquire();
    }
    check(model_.commit(self));
}

int Runtime::create_thread(pthread_t *thread, const pthread_attr_t *attr, StartRoutine start,
                           void *arg) noexcept {
    schedule(ThreadCreate{});

    const std::optional<ThreadId> child = model_.spawn();
    if (!child) {
        fatal({"Thread creation failed; thread limit reached: ", Decimal{kMaxThreads}.view()});
    }
    strategy_.on_thread_created(*child);

    Slot &child_slot = slot(*child);
    child_slot.start = start;
    child_slot.arg = arg;

    if (const int error = real().create(thread, attr, &Runtime::run_child, encode(*child));
        error != 0) {
        model_.announce(*child, ThreadExit{});
        check(model_.commit(*child));
        return error;
    }
    child_slot.handle = *thread;
    return 0;
}

std::optional<ThreadId> Runtime::find(const pthread_t handle) const noexcept {
    for (const std::uint32_t i : std::views::iota(0U, kMaxThreads) | std::views::reverse) {
        const ThreadId id{i};
        if (model_.status(id) != ThreadStatus::Unused &&
            ::pthread_equal(slots_[i].handle, handle) != 0) {
            return id;
        }
    }
    return std::nullopt;
}

void Runtime::exit_current() noexcept {
    const ThreadId self = *t_self;
    t_self.reset();
    model_.announce(self, ThreadExit{});
    check(model_.commit(self));
    hand_off(decide(model_, strategy_, self, step_++));
}

void *Runtime::run_child(void *encoded_id) {
    const ThreadId self = decode(encoded_id);
    t_self = self;
    Runtime &runtime = *g_runtime.load(std::memory_order_acquire);

    Slot &own = runtime.slot(self);
    own.token.acquire();
    runtime.check(runtime.model_.commit(self));

    void *result = own.start(own.arg);
    runtime.exit_current();
    return result;
}

Runtime::Slot &Runtime::slot(const ThreadId id) noexcept { return slots_[index(id)]; }

void Runtime::hand_off(const Decision &decision) noexcept {
    std::visit(Overloaded{
                   [this](const ThreadId next) { slot(next).token.release(); },
                   [this](Deadlock) {
                       exit_with(ExitCode::Deadlock, {"deadlock at step ", Decimal{step_}.view()});
                   },
                   [](AllFinished) {},
               },
               decision);
}

void Runtime::check(const Violation violation) noexcept {
    switch (violation) {
        case Violation::None:
            return;
        case Violation::UnlockNotOwned:
            exit_with(ExitCode::Violation, {"unlock of a mutex the thread doesn't own"});
        case Violation::TooManyHeldMutexes:
            fatal({"held mutex limit reached: ", Decimal{kMaxHeldMutexes}.view()});
    }
}

Runtime *current_runtime() noexcept {
    return t_self ? g_runtime.load(std::memory_order_acquire) : nullptr;
}
}  // namespace heddle