#include <pthread.h>

#include "shim/real.h"
#include "shim/runtime.h"

using heddle::current_runtime;
using heddle::real;

extern "C" {
[[gnu::visibility("default")]] int pthread_mutex_lock(pthread_mutex_t *mutex) noexcept {
    if (auto *runtime = current_runtime()) {
        runtime->schedule(heddle::MutexLock{mutex});
    }
    return real().mutex_lock(mutex);
}

[[gnu::visibility("default")]] int pthread_mutex_unlock(pthread_mutex_t *mutex) noexcept {
    if (auto *runtime = current_runtime()) {
        runtime->schedule(heddle::MutexUnlock{mutex});
    }
    return real().mutex_unlock(mutex);
}

[[gnu::visibility("default")]] int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                                                  heddle::StartRoutine start, void *arg) noexcept {
    if (auto *runtime = current_runtime()) {
        return runtime->create_thread(thread, attr, start, arg);
    }
    return real().create(thread, attr, start, arg);
}

[[gnu::visibility("default")]] int pthread_join(pthread_t thread, void **result) {
    if (auto *runtime = current_runtime()) {
        if (const auto target = runtime->find(thread)) {
            runtime->schedule(heddle::ThreadJoin{*target});
        }
    }
    return real().join(thread, result);
}

[[gnu::visibility("default")]] void pthread_exit(void *result) {
    if (auto *runtime = current_runtime()) {
        runtime->exit_current();
    }
    real().exit(result);
    __builtin_unreachable(); // `noreturn` function
}
}