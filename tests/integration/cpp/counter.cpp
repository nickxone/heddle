#include <mutex>
#include <thread>
#include <vector>

int main(void) {
    constexpr int kThreads = 4;
    constexpr int kIncrements = 100;

    std::mutex m;
    int cnt = 0;
    {
        std::vector<std::jthread> workers;
        for (int i = 0; i < kThreads; ++i) {
            workers.emplace_back([&] {
                for (int j = 0; j < kIncrements; ++j) {
                    std::lock_guard lock{m};
                    ++cnt;
                }
            });
        }
    }
    return cnt == kThreads * kIncrements ? 0 : 1;
}
