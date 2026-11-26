#pragma once

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace heddle::driver {
struct Options {
    std::uint64_t first_seed = 1;
    std::uint64_t seed_count = 100;
    std::chrono::seconds timeout{10};
    bool carry_on = false;
    std::optional<std::filesystem::path> shim_path;
    std::vector<std::string> command;
};

struct ExitRequest : std::exception {
    bool success;
    explicit ExitRequest(const bool success) : success{success} {}
};

[[nodiscard]] Options parse_options(int argc, const char *const *argv);
}  // namespace heddle::driver