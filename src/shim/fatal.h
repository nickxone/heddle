#pragma once

#include <initializer_list>
#include <string_view>

#include "core/exit_codes.h"

namespace heddle {
[[noreturn]] void exit_with(ExitCode code, std::initializer_list<std::string_view> parts) noexcept;
[[noreturn]] void fatal(std::initializer_list<std::string_view> parts) noexcept;
}  // namespace heddle
