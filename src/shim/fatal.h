#pragma once

#include <initializer_list>
#include <string_view>

namespace heddle {
[[noreturn]] void fatal(std::initializer_list<std::string_view> parts) noexcept;
}
