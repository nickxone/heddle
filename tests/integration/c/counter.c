#include <assert.h>
#include <pthread.h>

static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

static int counter = 0;

enum { kThreads = 4, kIterations = 100 };

static void *increment(void *arg) {
    (void)arg;
    for (int i = 0; i < kIterations; ++i) {
        pthread_mutex_lock(&m);
        ++counter;
        pthread_mutex_unlock(&m);
    }
    return NULL;
}

int main(void) {
    pthread_t threads[kThreads];
    for (int i = 0; i < 4; ++i) {
        pthread_create(&threads[i], NULL, increment, NULL);
    }
    for (int i = 0; i < 4; ++i) {
        pthread_join(threads[i], NULL);
    }
    assert(counter == kThreads * kIterations);
    return !(counter == kThreads * kIterations);
}