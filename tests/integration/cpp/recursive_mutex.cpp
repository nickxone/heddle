#include <mutex>
#include <thread>

int main(void) {
    std::recursive_mutex m;
    std::jthread worker{[&] {
        std::lock_guard outer{m};
        std::lock_guard inner{m};
    }};
    return 0;
}
