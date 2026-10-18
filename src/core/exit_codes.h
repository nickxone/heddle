#pragma once

#include <cstdint>

namespace heddle {
enum class ExitCode : std::uint8_t {
    Deadlock = 120,
    Violation = 121,  // illegal operation
    InternalError = 122
};
}  // namespace heddle