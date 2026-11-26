#pragma once

#include <chrono>
#include <filesystem>
#include <string>
#include <vector>

#include "outcome.h"

namespace heddle::driver {
enum class Output : std::uint8_t { Silent, Inherit };

class Launcher {
  public:
    Launcher(std::vector<std::string> command, const std::filesystem::path &shim,
             std::chrono::milliseconds timeout);

    [[nodiscard]] RunResult run(std::uint64_t seed, Output output) const;

  private:
    std::vector<std::string> command_;
    std::vector<std::string> environment_;
    std::chrono::milliseconds timeout_;
};
}  // namespace heddle::driver