#include "shim/fatal.h"

#include <unistd.h>

#include "core/exit_codes.h"

namespace heddle {
namespace {
void write_stderr(std::string_view text) noexcept {
    while (!text.empty()) {
        const ssize_t written = ::write(STDERR_FILENO, text.data(), text.size());
        if (written <= 0) {
            return;
        }
        text.remove_prefix(static_cast<std::size_t>(written));
    }
}
}  // namespace

void fatal(const std::initializer_list<std::string_view> parts) noexcept {
    write_stderr("heddle: fatal: ");
    for (const std::string_view part : parts) {
        write_stderr(part);
    }
    write_stderr("\n");
    ::_exit(kExitInternalError);
}
}  // namespace heddle