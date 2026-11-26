#include <mutex>
#include <thread>

int main(void) {
    std::mutex a;
    std::mutex b;
    std::jthread worker{[&] {
        std::lock_guard first{a};
        std::lock_guard second{b};
    }};
    std::lock_guard first{b};
    std::lock_guard second{a};
    return 0;
}
