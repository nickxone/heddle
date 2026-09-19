#include <semaphore>
#include <thread>

static std::counting_semaphore<1> sem{0};

int main() {
    std::thread t([] { sem.release(); });
    sem.acquire();
    t.join();
}
