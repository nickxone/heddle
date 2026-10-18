#include <pthread.h>

static pthread_mutex_t a = PTHREAD_MUTEX_INITIALIZER;

static void *lock() {
    pthread_mutex_lock(&a);
    pthread_mutex_lock(&a);
    pthread_mutex_unlock(&a);
    return NULL;
}

int main(void) {
    pthread_t t;
    pthread_create(&t, NULL, lock, NULL);
    pthread_join(t, NULL);
    return 0;
}