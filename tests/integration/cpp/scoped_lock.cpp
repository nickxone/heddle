#include <mutex>
#include <thread>

int main(void) {
    std::mutex a;
    std::mutex b;
    std::jthread worker{[&] { std::scoped_lock lock{a, b}; }};
    std::scoped_lock lock{b, a};
    return 0;
}