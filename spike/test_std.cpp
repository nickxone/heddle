#include <thread>
#include <mutex>
#include <cstdio>

static std::mutex m;
static long cnt = 0;

static void worker() {
    for (int i = 0; i < 3; ++i) {
        std::lock_guard<std::mutex> g(m);
        ++cnt;
    }
}

int main(void) {
    std::thread t1(worker), t2(worker);
    t1.join();
    t2.join();
    std::printf("counter = %ld\n", cnt);
    return 0;
}