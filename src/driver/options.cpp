#include "driver/options.h"

#include <CLI/CLI.hpp>

namespace heddle::driver {
namespace {
const CLI::Validator kPositive{
    [](const std::string &value) {
        return value.find_first_not_of('0') == std::string::npos ? "must be positive" : "";
    },
    "POSITIVE"};
}  // namespace
Options parse_options(int argc, const char *const *argv) {
    Options opts;

    CLI::App app{"Concurrency testing for C/C++ programs", "heddle"};
    app.require_subcommand(1);

    CLI::App *run = app.add_subcommand("run", "Run a program under seeded schedules");
    run->usage("heddle run [OPTIONS] [--] PROGRAM [ARGS...]");
    CLI::Option *seeds =
        run->add_option("--seeds", opts.seed_count, "Number of seeds to try")->check(kPositive);
    unsigned timeout_seconds = 10;
    run->add_option("--timeout", timeout_seconds, "Seconds before the run is hanged");
    opts.timeout = std::chrono::seconds{timeout_seconds};
    run->add_flag("--carry-on", opts.carry_on, "Carry on after the first failure");
    run->add_option("--shim", opts.shim_path, "Use shim different to the one bundled")
        ->check(CLI::ExistingFile);
    run->add_option("program", opts.command, "Program to test")->required();
    run->positionals_at_end();  // the rest is meant for the program specified

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &error) {
        throw ExitRequest(app.exit(error) == 0);
    }

    if (run->count("--seed") != 0) {
        opts.seed_count = 1;
    }
    return opts;
}
}  // namespace heddle::driver