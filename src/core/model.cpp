#include "core/model.h"

#include <algorithm>

namespace heddle {
// Model
Model::Model() noexcept {
    [[maybe_unused]] const auto main = spawn();
    assert(main == kMainThread);
}

std::optional<ThreadId> Model::spawn() noexcept {
    if (spawned_ == kMaxThreads) {
        return std::nullopt;
    }
    const ThreadId id{spawned_++};
    threads_[index(id)] = {.status = ThreadStatus::Live, .pending = ThreadStart{}};
    return id;
}

void Model::announce(ThreadId id, const Event &next) noexcept {
    assert(status(id) == ThreadStatus::Live);
    threads_[index(id)].pending = next;
}

bool Model::any_live() const noexcept {
    return std::ranges::any_of(threads_,
                               [](const Thread &t) { return t.status == ThreadStatus::Live; });
}

ThreadStatus Model::status(ThreadId id) const noexcept {
    assert(index(id) < kMaxThreads);
    return threads_[index(id)].status;
}

bool Model::enabled(const ThreadId id) const noexcept {
    const Thread &thread = threads_[index(id)];
    if (thread.status != ThreadStatus::Live) {
        return false;
    }
    if (!thread.pending) {
        return true;
    }

    return std::visit(
        Overloaded{[&](const MutexLock &e) { return !held_.owner(e.mutex).has_value(); },
                   [&](const ThreadJoin &e) { return status(e.target) == ThreadStatus::Finished; },
                   [](const auto &) { return true; }},
        *thread.pending);
}

Violation Model::commit(const ThreadId id) noexcept {
    assert(enabled(id));

    Thread &thread = threads_[index(id)];

    assert(thread.pending.has_value());

    auto overloaded = Overloaded{[&](const MutexLock &e) {
                                     const bool res = held_.acquire(e.mutex, id);
                                     return res ? Violation::None : Violation::TooManyHeldMutexes;
                                 },
                                 [&](const MutexUnlock &e) {
                                     const bool res = held_.release(e.mutex, id);
                                     return res ? Violation::None : Violation::UnlockNotOwned;
                                 },
                                 [&](const ThreadExit &) {
                                     thread.status = ThreadStatus::Finished;
                                     return Violation::None;
                                 },
                                 [](const ThreadStart &) { return Violation::None; },
                                 [](const ThreadCreate &) { return Violation::None; },
                                 [](const ThreadJoin &) { return Violation::None; }};
    const Violation violation = std::visit(overloaded, *thread.pending);
    thread.pending.reset();
    return violation;
}

ThreadSet Model::runnable() const noexcept {
    ThreadSet out;
    for (std::uint32_t i = 0; i < spawned_; i++) {
        if (const ThreadId id{i}; enabled(id)) {
            out.push_back(id);
        }
    }
    return out;
}

// HeldMutexes
std::optional<ThreadId> HeldMutexes::owner(const void *mutex) const noexcept {
    const auto held = std::span{entries_}.first(size_);
    const auto it = std::ranges::find(held, mutex, &Entry::mutex);
    if (it == held.end()) {
        return std::nullopt;
    }
    return it->owner;
}

bool HeldMutexes::acquire(const void *mutex, const ThreadId thread) noexcept {
    assert(!owner(mutex).has_value());
    if (size_ == entries_.size()) {
        return false;
    }
    entries_[size_++] = {.mutex = mutex, .owner = thread};
    return true;
}

bool HeldMutexes::release(const void *mutex, const ThreadId thread) noexcept {
    const auto held = std::span(entries_).first(size_);
    const auto it = std::ranges::find(held, mutex, &Entry::mutex);
    if (it == held.end() || it->owner != thread) {
        return false;
    }
    *it = held.back();  // last entry overwrites the removed one
    --size_;
    return true;
}
}  // namespace heddle
