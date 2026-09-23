#include <array>
#include <cstdint>
#include <set>

#include <gtest/gtest.h>

#include "shim/rng.h"

using heddle::SplitMix64;

namespace {
    constexpr std::uint64_t kReferenceSeed = 1234567;
    constexpr std::array<std::uint64_t, 5> kReferenceOutputs = {
        6457827717110365317ULL,
        3203168211198807973ULL,
        9817491932198370423ULL,
        4593380528125082431ULL,
        16408922859458223821ULL
    };
}

static_assert([] {
    SplitMix64 rng{kReferenceSeed};
    return rng.next();
}() == kReferenceOutputs[0]);

TEST(SplitMix64, MatchesReferenceSequence) {
    SplitMix64 rng{kReferenceSeed};
    for (std::uint64_t expected : kReferenceOutputs) {
        EXPECT_EQ(expected, rng.next());
    }
}

TEST(SplitMix64, SameSeedGivesSameSequence) {
    SplitMix64 a{42}, b{42};
    for (int i = 0; i < 1000; ++i) {
        ASSERT_EQ(a.next(), b.next());
    }
}

TEST(SplitMix64, BoundedStaysInRange) {
    SplitMix64 rng{7};
    for (std::uint64_t n :{1ULL, 2ULL, 3ULL, 7ULL, 1000ULL}) {
        for (int i = 0; i < 1000; ++i) {
            ASSERT_LT(rng.bounded(n), n);
        }
    }
}

TEST(SplitMix64, BoundedHandlesWorstCaseRejection) {
    SplitMix64 rng{7};
    const std::uint64_t n = (1ULL << 63) + 1;
    for (int i = 0; i < 1000; ++i) {
        ASSERT_LT(rng.bounded(n), n);
    }
}

TEST(SplitMix64, BoundedREachesEveryValue) {
    SplitMix64 rng{7};
    std::set<std::uint64_t> values;
    for (int i = 0; i < 1000; ++i) {
        values.insert(rng.bounded(5));
    }
    EXPECT_EQ(values.size(), 5);
}
