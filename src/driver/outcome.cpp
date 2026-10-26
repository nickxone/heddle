#include "driver/outcome.h"

#include <sys/wait.h>

#include <cstring>
#include <format>

#include "core/exit_codes.h"

namespace heddle::driver {
namespace {
constexpr int code(ExitCode exit_code) noexcept { return static_cast<int>(exit_code); }
}  // namespace

RunResult classify(const int wait_status, const bool timed_out) noexcept {
    if (timed_out) return {.outcome = Outcome::Hang};
    if (WIFSIGNALED(wait_status)) {
        return {.outcome = Outcome::Crash,
                .detail = WTERMSIG(wait_status)};  // check if child process was terminated
    }

    const int exit_code = WEXITSTATUS(wait_status);
    switch (exit_code) {
        case 0:
            return {.outcome = Outcome::Pass};
        case code(ExitCode::Deadlock):
            return {.outcome = Outcome::Deadlock, .detail = exit_code};
        case code(ExitCode::Violation):
            return {.outcome = Outcome::Violation, .detail = exit_code};
        case code(ExitCode::InternalError):
            return {.outcome = Outcome::InternalError, .detail = exit_code};
        default:
            return {.outcome = Outcome::Failure, .detail = exit_code};
    }
}

std::string describe(const RunResult &result) {
    switch (result.outcome) {
        case Outcome::Pass:
            return "pass";
        case Outcome::Deadlock:
            return "deadlock";
        case Outcome::Violation:
            return "wrong use of a synchronisation primitive";
        case Outcome::Crash:
            return std::format("crash (SIG{})", ::sigabbrev_np(result.detail));
        case Outcome::Failure:
            return std::format("failure (exit {})", result.detail);
        case Outcome::Hang:
            return "hang (time out)";
        case Outcome::InternalError:
            return "heddle internal error";
        default:
            return "unknown";
    }
}
}  // namespace heddle::driver