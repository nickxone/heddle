#pragma once

#include <cstdint>

namespace heddle {
    enum class ThreadId: std::uint32_t {};

    inline constexpr ThreadId kMainThread{0};
    inline constexpr std::uint32_t kMaxThreads = 64;

    constexpr std::uint32_t index(ThreadId id) noexcept {
        return static_cast<std::uint32_t>(id);
    }
}