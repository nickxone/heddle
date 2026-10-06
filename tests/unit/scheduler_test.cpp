#include "core/scheduler.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <vector>

#include "core/random_strategy.h"

using namespace heddle;
using ::testing::ElementsAre;

namespace {
int mutex_a;
int mutex_b;

class FirstRunnable final : public Strategy {
  public:
    ThreadId choose(const SchedulingContext &ctx) override {
        seen.assign(ctx.runnable.begin(), ctx.runnable.end());
        return ctx.runnable.front();
    }

    std::vector<ThreadId> seen;
};

Violation run(Model &model, const ThreadId id, const Event &event) {
    model.announce(id, event);
    return model.commit(id);
}
}  // namespace

TEST(Decide, ChoosesTheOnlyRunnableThread) {
    const Model model;
    FirstRunnable strategy;

    EXPECT_EQ(std::get<ThreadId>(decide(model, strategy, kMainThread, 0)), kMainThread);
}

TEST(Decide, StrategySeesOnlyEnabledThreadsInOrder) {
    Model model;
    const ThreadId t1 = model.spawn().value();
    const ThreadId t2 = model.spawn().value();

    ASSERT_EQ(run(model, kMainThread, MutexLock{&mutex_a}), Violation::None);
    model.announce(t1, MutexLock{&mutex_a});

    FirstRunnable strategy;
    static_cast<void>(decide(model, strategy, kMainThread, 0));

    EXPECT_THAT(strategy.seen, ElementsAre(kMainThread, t2));
}

TEST(Decide, ReportsDeadlockWithoutConsultingStrategy) {
    Model model;
    const ThreadId other = model.spawn().value();

    ASSERT_EQ(run(model, kMainThread, MutexLock{&mutex_a}), Violation::None);
    ASSERT_EQ(run(model, other, MutexLock{&mutex_b}), Violation::None);

    model.announce(kMainThread, MutexLock{&mutex_b});
    model.announce(other, MutexLock{&mutex_a});

    FirstRunnable strategy;
    EXPECT_TRUE(std::holds_alternative<Deadlock>(decide(model, strategy, kMainThread, 0)));
    EXPECT_TRUE(strategy.seen.empty());
}

TEST(Decide, ReportsAllFinishedWhenNoThreadIsLive) {
    Model model;
    ASSERT_EQ(run(model, kMainThread, ThreadExit{}), Violation::None);

    FirstRunnable strategy;
    EXPECT_TRUE(std::holds_alternative<AllFinished>(decide(model, strategy, kMainThread, 0)));
}

TEST(Decide, RandomStrategyOnlyChoosesRunnableThreads) {
    Model model;
    const ThreadId blocked = model.spawn().value();

    ASSERT_EQ(run(model, kMainThread, MutexLock{&mutex_a}), Violation::None);
    model.announce(blocked, MutexLock{&mutex_a});

    RandomStrategy strategy{42};
    for (std::uint64_t step = 0; step < 100; ++step) {
        EXPECT_EQ(std::get<ThreadId>(decide(model, strategy, kMainThread, step)), kMainThread);
    }
}