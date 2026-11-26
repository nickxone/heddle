#include <pthread.h>

static pthread_mutex_t a = PTHREAD_MUTEX_INITIALIZER;

static void *unlock() {
    pthread_mutex_unlock(&a);
    return NULL;
}

int main(void) {
    pthread_t t;
    pthread_create(&t, NULL, unlock, NULL);
    pthread_join(t, NULL);
    return 0;
}