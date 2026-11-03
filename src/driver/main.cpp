#include <iostream>

#include "driver/launcher.h"
#include "driver/options.h"
#include "driver/outcome.h"

namespace {
using namespace heddle::driver;
namespace fs = std::filesystem;

enum class DriverExit : std::uint8_t { NoFailures = 0, FailureFound = 1, Error = 2 };

fs::path find_shim(const Options &options) {
    const fs::path shim = options.shim_path.value_or(
        fs::read_symlink("/proc/self/exe").parent_path() / "libheddle_shim.so");
    if (!fs::exists(shim)) {
        throw std::runtime_error{std::format("shim not found: {}", shim.string())};
    }
    return fs::absolute(shim);
}

std::string join(const std::vector<std::string> &words) {
    std::string joined;
    for (const std::string &word : words) {
        joined += (joined.empty() ? "" : " ");
        joined += word;
    }
    return joined;
}

DriverExit run(const Options &options) {
    const Launcher launcher{options.command, find_shim(options), options.timeout};
    const bool single_seed = options.seed_count == 1;

    std::uint64_t runs = 0;
    std::uint64_t failures = 0;

    for (std::uint64_t seed = options.first_seed; seed < options.first_seed + options.seed_count;
         ++seed) {
        ++runs;
        const RunResult result = launcher.run(seed, single_seed ? Output::Inherit : Output::Silent);
        if (result.outcome == Outcome::Pass) {
            continue;
        }

        std::cout << std::format("seed {}: {}\n", seed, describe(result));
        if (!single_seed && launcher.run(seed, Output::Inherit) != result) {
            std::cout << "warning: replaying the seed gave a different result\n";
        }

        if (result.outcome == Outcome::InternalError) {
            return DriverExit::Error;
        }
        std::cout << std::format("reproduce: heddle run --seed {} -- {}\n", seed,
                                 join(options.command));
        ++failures;
        if (!options.carry_on) {
            break;
        }
    }
    std::cout << std::format("{} failing seed(s) in {} run(s)\n", failures, runs);
    return failures == 0 ? DriverExit::NoFailures : DriverExit::FailureFound;
}
}  // namespace

int main(int argc, char **argv) {
    try {
        return static_cast<int>(run(parse_options(argc, argv)));
    } catch (const ExitRequest &request) {
        return static_cast<int>(request.success ? DriverExit::NoFailures : DriverExit::Error);
    } catch (const std::exception &error) {
        std::cerr << "heddle: " << error.what() << '\n';
    }
    return static_cast<int>(DriverExit::Error);
}