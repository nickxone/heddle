// Deliberately broken fixture: calls a defined interceptor (see `interposed.c`)

#include <pthread.h>

int heddle_internal_helper(pthread_mutex_t* m) {
    return pthread_mutex_lock(m);
}
