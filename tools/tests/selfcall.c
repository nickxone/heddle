// Has recursion on purpose; `check-interposition` doesn't check for that

#include <pthread.h>

int pthread_mutex_lock(pthread_mutex_t* m) {
    return pthread_mutex_lock(m);
}
