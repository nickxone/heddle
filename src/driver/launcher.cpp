#include "driver/launcher.h"

#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <sys/personality.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <format>
#include <iostream>
#include <system_error>

extern char **environ;

namespace heddle::driver {
namespace {
constexpr int kExecFailed = 127;

class UniqueFd {
  public:
    explicit UniqueFd(int fd) : fd_(fd) {}
    UniqueFd(const UniqueFd &) = delete;
    UniqueFd &operator=(const UniqueFd &) = delete;
    ~UniqueFd() {
        if (fd_ >= 0) {
            ::close(fd_);
        }
    }

    [[nodiscard]] int get() const noexcept { return fd_; }

  private:
    int fd_;
};

[[noreturn]] void throw_errno(const char *what) {
    throw std::system_error{errno, std::generic_category(), what};
}

std::vector<char *> c_strings(const std::vector<std::string> &strings) {
    std::vector<char *> ptrs;
    ptrs.reserve(strings.size() + 1);
    for (const std::string &s : strings) {
        ptrs.push_back(const_cast<char *>(s.c_str()));
    }
    ptrs.push_back(nullptr);
    return ptrs;
}

std::vector<std::string> child_env(const std::filesystem::path &shim) {
    constexpr std::string_view kPreload = "LD_PRELOAD=";
    constexpr std::string_view kSeed = "HEDDLE_SEED=";

    std::vector<std::string> env;
    std::string preload = std::string{kPreload} + shim.string();
    for (char **entry = environ; *entry != nullptr; ++entry) {
        const std::string_view variable = *entry;
        if (variable.starts_with(kPreload)) {
            preload += ':';
            preload += variable.substr(kPreload.length());
        } else if (!variable.starts_with(kSeed)) {
            env.emplace_back(variable);
        }
    }
    env.push_back(std::move(preload));
    return env;
}

void disable_aslr() {
    const int curr = ::personality(0xffffffff);  // read current flags
    if (curr == -1 || ::personality(curr | ADDR_NO_RANDOMIZE) == -1) {
        std::cerr << "heddle: warning: can't disable ASLR\n";
    }
}

[[noreturn]] void exec_child(char *const argv[], char *const envp[], Output output) {
    if (const int input = ::open("/dev/null", O_RDONLY | O_CLOEXEC); input >= 0) {
        ::dup2(input, STDIN_FILENO);
    }
    if (output == Output::Silent) {
        const int null = ::open("/dev/null", O_WRONLY | O_CLOEXEC);
        ::dup2(null, STDOUT_FILENO);
        ::dup2(null, STDERR_FILENO);
    }
    ::execvpe(argv[0], argv, envp);
    std::fprintf(stderr, "heddle: can't execute %s: %s\n", argv[0], std::strerror(errno));
    ::_exit(kExecFailed);
}

RunResult wait_for(const pid_t pid, const std::chrono::milliseconds timeout) {
    const UniqueFd pidfd{static_cast<int>(::syscall(SYS_pidfd_open, pid, 0))};
    if (pidfd.get() < 0) {
        throw_errno("pidfd_open");
    }
    pollfd exited{.fd = pidfd.get(), .events = POLLIN, .revents = 0};
    int ready = 0;
    do {
        ready = ::poll(&exited, 1, static_cast<int>(timeout.count()));
    } while (ready < 0 && errno == EINTR);

    const bool timed_out = ready == 0;
    if (timed_out) {
        ::kill(pid, SIGKILL);
    }

    int status = 0;
    while (::waitpid(pid, &status, 0) < 0) {
        if (errno != EINTR) {
            throw_errno("waitpid");
        }
    }
    return classify(status, timed_out);
}
}  // namespace

Launcher::Launcher(std::vector<std::string> command, const std::filesystem::path &shim,
                   const std::chrono::milliseconds timeout)
    : command_{std::move(command)}, environment_{child_env(shim)}, timeout_{timeout} {
    disable_aslr();
}

RunResult Launcher::run(const std::uint64_t seed, const Output output) const {
    std::vector<std::string> env = environment_;
    env.push_back(std::format("HEDDLE_SEED={}", seed));
    const std::vector<char *> argv = c_strings(command_);
    const std::vector<char *> envp = c_strings(env);

    std::cout.flush();
    const pid_t pid = ::fork();
    if (pid < 0) {
        throw_errno("fork");
    }
    if (pid == 0) {
        exec_child(argv.data(), envp.data(), output);
    }
    return wait_for(pid, timeout_);
}
}  // namespace heddle::driver