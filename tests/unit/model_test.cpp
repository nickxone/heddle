#include <vector>
#include <array>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "core/model.h"

using namespace heddle;
using ::testing::ElementsAre;

namespace {
    int mutex_a;
    int mutex_b;

    Violation run(Model &model, const ThreadId id, const Event &event) {
        model.announce(id, event);
        return model.commit(id);
    }

    std::vector<ThreadId> runnable(const Model &model) {
        const ThreadSet set = model.runnable();
        const auto ids = set.view();
        return {ids.begin(), ids.end()};
    }
}

TEST(Model, NewModelHasOnlyMainThreadRunnable) {
    const Model model;
    EXPECT_EQ(runnable(model), std::vector { kMainThread });
}

TEST(Model, HeldMutexBlocksOtherThreadsUntilUnlocked) {
    Model model;
    const ThreadId other = model.spawn().value();

    ASSERT_EQ(run(model, kMainThread, MutexLock { &mutex_a }), Violation::None);
    model.announce(other, MutexLock{&mutex_a});
    EXPECT_FALSE(model.enabled(other));

    ASSERT_EQ(run(model, kMainThread, MutexUnlock { &mutex_a }), Violation::None);
    EXPECT_TRUE(model.enabled(other));
}

TEST(Model, OpposingLockOrderDeadlocks) {
    Model model;
    const ThreadId other = model.spawn().value();

    ASSERT_EQ(run(model, kMainThread, MutexLock { &mutex_a }), Violation::None);
    ASSERT_EQ(run(model, other, MutexLock { &mutex_b }), Violation::None);
    model.announce(kMainThread, MutexLock{&mutex_b});
    model.announce(other, MutexLock{&mutex_a});

    EXPECT_TRUE(runnable(model).empty());
    EXPECT_TRUE(model.any_live());
}

TEST(Model, HasSpawnLimit) {
    Model model;
    for (std::uint32_t i = 1; i < kMaxThreads; ++i) {
        ASSERT_EQ(model.spawn().value(), ThreadId { i });
    }
    EXPECT_EQ(model.spawn(), std::nullopt);
}

TEST(Model, HeldMutexDoesNotBlockADifferentMutex) {
    Model model;
    const ThreadId other = model.spawn().value();

    ASSERT_EQ(run(model, kMainThread, MutexLock{&mutex_a}), Violation::None);
    model.announce(other, MutexLock{&mutex_b});

    EXPECT_TRUE(model.enabled(other));
}

TEST(Model, RelockingHeldMutexSelfDeadlocks) {
    Model model;

    ASSERT_EQ(run(model, kMainThread, MutexLock{&mutex_a}), Violation::None);
    model.announce(kMainThread, MutexLock{&mutex_a});

    EXPECT_TRUE(runnable(model).empty());
    EXPECT_TRUE(model.any_live());
}

TEST(Model, JoinIsEnabledOnlyAfterTargetExits) {
    Model model;
    const ThreadId child = model.spawn().value();

    model.announce(kMainThread, ThreadJoin{child});
    EXPECT_FALSE(model.enabled(kMainThread));

    ASSERT_EQ(run(model, child, ThreadExit{}), Violation::None);
    EXPECT_TRUE(model.enabled(kMainThread));
}

TEST(Model, FinishedThreadsAreNeitherRunnableNorLive) {
    Model model;
    const ThreadId child = model.spawn().value();

    ASSERT_EQ(run(model, child, ThreadExit{}), Violation::None);
    EXPECT_EQ(model.status(child), ThreadStatus::Finished);
    EXPECT_THAT(runnable(model), ElementsAre(kMainThread));

    ASSERT_EQ(run(model, kMainThread, ThreadExit{}), Violation::None);
    EXPECT_TRUE(runnable(model).empty());
    EXPECT_FALSE(model.any_live());
}

TEST(Model, UnlockingUnheldMutexIsViolation) {
    Model model;
    EXPECT_EQ(run(model, kMainThread, MutexUnlock{&mutex_a}), Violation::UnlockNotOwned);
}

TEST(Model, UnlockingAnotherThreadsMutexIsViolationAndKeepsOwnership) {
    Model model;
    const ThreadId other = model.spawn().value();

    ASSERT_EQ(run(model, kMainThread, MutexLock{&mutex_a}), Violation::None);
    EXPECT_EQ(run(model, other, MutexUnlock{&mutex_a}), Violation::UnlockNotOwned);

    model.announce(other, MutexLock{&mutex_a});
    EXPECT_FALSE(model.enabled(other));
}

TEST(Model, ReleasingOneMutexKeepsTheOthersHeld) {
    Model model;
    const ThreadId other = model.spawn().value();
    int mutex_c;

    ASSERT_EQ(run(model, kMainThread, MutexLock{&mutex_a}), Violation::None);
    ASSERT_EQ(run(model, kMainThread, MutexLock{&mutex_b}), Violation::None);
    ASSERT_EQ(run(model, kMainThread, MutexLock{&mutex_c}), Violation::None);
    ASSERT_EQ(run(model, kMainThread, MutexUnlock{&mutex_a}), Violation::None);

    model.announce(other, MutexLock{&mutex_a});
    EXPECT_TRUE(model.enabled(other));
    model.announce(other, MutexLock{&mutex_b});
    EXPECT_FALSE(model.enabled(other));
    model.announce(other, MutexLock{&mutex_c});
    EXPECT_FALSE(model.enabled(other));
}

TEST(Model, ExceedingHeldMutexCapacityIsViolation) {
    Model model;
    std::array<int, kMaxHeldMutexes + 1> mutexes{};

    for (std::size_t i = 0; i < kMaxHeldMutexes; ++i) {
        ASSERT_EQ(run(model, kMainThread, MutexLock{&mutexes[i]}), Violation::None);
    }
    EXPECT_EQ(run(model, kMainThread, MutexLock{&mutexes.back()}), Violation::TooManyHeldMutexes);
}

TEST(Model, RunnableIsAscendingAndSkipsBlockedThreads) {
    Model model;
    const ThreadId t1 = model.spawn().value();
    const ThreadId t2 = model.spawn().value();
    const ThreadId t3 = model.spawn().value();

    ASSERT_EQ(run(model, t3, MutexLock{&mutex_a}), Violation::None);
    model.announce(t1, MutexLock{&mutex_a});

    EXPECT_THAT(runnable(model), ElementsAre(kMainThread, t2, t3));
}

TEST(ModelDeathTest, CommittingDisabledEventAsserts) {
    Model model;
    const ThreadId other = model.spawn().value();
    ASSERT_EQ(run(model, kMainThread, MutexLock{&mutex_a}), Violation::None);
    model.announce(other, MutexLock{&mutex_a});

    EXPECT_DEBUG_DEATH(static_cast<void>(model.commit(other)), "");
}

TEST(Model, ThreadIsRunnableAfterCommittingItsLock) {
    Model model;
    ASSERT_EQ(run(model, kMainThread, MutexLock{&mutex_a}), Violation::None);
    EXPECT_TRUE(model.enabled(kMainThread));
}
