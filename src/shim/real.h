#pragma once

#include <pthread.h>

namespace heddle {
struct RealFns {
    decltype(&::pthread_create) create;
    decltype(&::pthread_join) join;
    decltype(&::pthread_exit) exit;
    decltype(&::pthread_mutex_lock) mutex_lock;
    decltype(&::pthread_mutex_unlock) mutex_unlock;
};

[[nodiscard]] const RealFns &real() noexcept;
}