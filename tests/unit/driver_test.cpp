#include <gtest/gtest.h>

#include <csignal>
#include <initializer_list>
#include <string>
#include <vector>

#include "driver/options.h"
#include "driver/outcome.h"

namespace heddle::driver {
namespace {
constexpr int exited(int code) { return code << 8; }

Options parse(std::initializer_list<const char *> args) {
    std::vector<const char *> argv{"heddle"};
    argv.insert(argv.end(), args);
    return parse_options(static_cast<int>(argv.size()), argv.data());
}

TEST(Classify, MapsShimExitCodes) {
    EXPECT_EQ(classify(exited(0), false).outcome, Outcome::Pass);
    EXPECT_EQ(classify(exited(120), false).outcome, Outcome::Deadlock);
    EXPECT_EQ(classify(exited(121), false).outcome, Outcome::Violation);
    EXPECT_EQ(classify(exited(122), false).outcome, Outcome::InternalError);
    EXPECT_EQ(classify(exited(3), false), (RunResult{Outcome::Failure, 3}));
}

TEST(Classify, SignalIsCrashAndTimeoutIsHang) {
    EXPECT_EQ(classify(SIGABRT, false), (RunResult{Outcome::Crash, SIGABRT}));
    EXPECT_EQ(classify(SIGKILL, true).outcome, Outcome::Hang);
}

TEST(ParseOptions, ReadsFlagsAndCommand) {
    const Options options =
        parse({"run", "--seeds", "5", "--timeout", "3", "--carry-on", "--", "./prog", "-x"});
    EXPECT_EQ(options.seed_count, 5U);
    EXPECT_EQ(options.timeout, std::chrono::seconds{3});
    EXPECT_TRUE(options.carry_on);
    EXPECT_EQ(options.command, (std::vector<std::string>{"./prog", "-x"}));
}

TEST(ParseOptions, ArgumentsAfterTheProgramBelongToIt) {
    const Options options = parse({"run", "./prog", "--seeds", "7"});
    EXPECT_EQ(options.seed_count, 100U);
    EXPECT_EQ(options.command, (std::vector<std::string>{"./prog", "--seeds", "7"}));
}

TEST(ParseOptions, SingleSeed) {
    const Options options = parse({"run", "--seed", "42", "./prog"});
    EXPECT_EQ(options.first_seed, 42U);
    EXPECT_EQ(options.seed_count, 1U);
}

TEST(ParseOptions, RejectsBadInput) {
    EXPECT_THROW(parse({"run"}), ExitRequest);
    EXPECT_THROW(parse({"run", "--seeds", "x", "./prog"}), ExitRequest);
    EXPECT_THROW(parse({"run", "--seeds", "0", "./prog"}), ExitRequest);
    EXPECT_THROW(parse({"run", "--timeout", "0", "./prog"}), ExitRequest);
    EXPECT_THROW(parse({"run", "--bogus", "./prog"}), ExitRequest);
    EXPECT_THROW(parse({"run", "--seed", "1", "--seeds", "3", "./prog"}), ExitRequest);
}
}  // namespace
}  // namespace heddle::driver