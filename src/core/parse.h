#pragma once

#include <charconv>
#include <concepts>
#include <optional>
#include <string_view>

namespace heddle {
template <std::integral T>
[[nodiscard]] std::optional<T> parse_number(std::string_view text) noexcept {
    T res{};
    const char *last = text.data() + text.length();
    const auto [end, error] = std::from_chars(text.data(), last, res);
    if (error != std::errc{} || end != last) {
        return std::nullopt;
    }
    return res;
}
}  // namespace heddle