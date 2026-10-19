#pragma once

#include <cstdint>
#include <string>

namespace heddle::driver {
enum class Outcome : std::uint8_t {
    Pass,
    Deadlock,
    Violation,
    Crash,
    Failure,
    Hang,
    InternalError
};

struct RunResult {
    Outcome outcome = Outcome::Pass;
    int detail = 0;  // exit code or signal number

    bool operator==(const RunResult &) const = default;
};

[[nodiscard]] RunResult classify(int wait_status, bool timed_out) noexcept;
[[nodiscard]] std::string describe(const RunResult &result);
}  // namespace heddle::driver