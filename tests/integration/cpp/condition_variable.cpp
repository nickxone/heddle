#include <condition_variable>
#include <mutex>
#include <thread>

int main(void) {
    std::mutex m;
    std::condition_variable ready_changed;
    bool ready = false;

    std::jthread worker{[&] {
        std::lock_guard lock{m};
        ready = true;
        ready_changed.notify_one();
    }};
    std::unique_lock lock{m};
    ready_changed.wait(lock, [&] { return ready; });
    return 0;
}
