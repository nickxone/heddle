#include <array>
#include <vector>
#include <ranges>

#include <gtest/gtest.h>

#include "shim/random_strategy.h"

using heddle::RandomStrategy;
using heddle::SchedulingContext;
using heddle::ThreadId;

namespace {
    constexpr std::array kRunnable = {ThreadId{0}, ThreadId{2}, ThreadId{13}};

    SchedulingContext context(const std::span<const ThreadId> runnable, const std::uint64_t step = 0) {
        return {.runnable = runnable, .current = runnable.front(), .step = step};
    }

    std::vector<ThreadId> choices(const std::uint64_t seed, const int count) {
        RandomStrategy strategy{seed};
        std::vector<ThreadId> out;
        for (int i = 0; i < count; ++i) {
            out.push_back(strategy.choose(context(kRunnable, i)));
        }
        return out;
    }
}

TEST(RandomStrategy, AlwaysChoosesARunnableThread) {
    for (ThreadId chosen: choices(1, 1000)) {
        EXPECT_NE(std::ranges::find(kRunnable, chosen), kRunnable.end());
    }
}

TEST(RandomStratey, SameSeedGivesSameChoices) {
    EXPECT_EQ(choices(99, 200), choices(99, 200));
}

TEST(RandomStratey, DifferentSeedsGiveDifferentChoices) {
    EXPECT_NE(choices(1, 200), choices(2, 200));
}

TEST(RandomStrategy, SingleRunnableThreadIsAlwaysChosen) {
    constexpr std::array singleThread = {ThreadId{3}};
    RandomStrategy strategy{0};
    for (int i = 0; i < 100; ++i) {
        EXPECT_EQ(strategy.choose(context(singleThread)), ThreadId{3});
    }
}
