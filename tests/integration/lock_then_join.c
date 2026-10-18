#include <pthread.h>

static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

static void *lock(void *arg) {
    (void)arg;
    pthread_mutex_lock(&m);
    pthread_mutex_unlock(&m);
    return NULL;
}

int main(void) {
    pthread_mutex_lock(&m);

    pthread_t t;
    pthread_create(&t, NULL, lock, NULL);
    pthread_join(t, NULL);
    return 0;
}