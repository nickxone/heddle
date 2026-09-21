#pragma once

#include <cstdint>

namespace heddle {
    enum class ThreadID: std::uint8_t {};

    inline constexpr ThreadID kMainThread{0};

    constexpr std::uint32_t index(ThreadID id) noexcept {
        return static_cast<std::uint32_t>(id);
    }
}