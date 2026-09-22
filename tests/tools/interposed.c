// Deliberately broken fixture: defines an interceptor (see `caller.c`)

#include <pthread.h>

int pthread_mutex_lock(pthread_mutex_t* m) {
    (void)m;
    return 0;
}
