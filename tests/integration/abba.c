#include <pthread.h>

static pthread_mutex_t a = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t b = PTHREAD_MUTEX_INITIALIZER;

static void *lock_in_order(void *arg) {
    pthread_mutex_t **order = arg;
    pthread_mutex_lock(order[0]);
    pthread_mutex_lock(order[1]);
    pthread_mutex_unlock(order[1]);
    pthread_mutex_unlock(order[0]);
    return NULL;
}

int main(void) {
    pthread_mutex_t *ab[] = {&a, &b};
    pthread_mutex_t *ba[] = {&b, &a};
    pthread_t t1;
    pthread_t t2;
    pthread_create(&t1, NULL, lock_in_order, ab);
    pthread_create(&t2, NULL, lock_in_order, ba);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    return 0;
}