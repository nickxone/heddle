#include <stdexcept>
#include <thread>

int main(void) {
    std::jthread worker{[] { throw std::runtime_error("worker failed"); }};
    return 0;
}