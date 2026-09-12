#include <stdio.h>
#include <pthread.h>

static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
static long cnt = 0;

static void *worker(void *arg) {
    (void)arg;
    for (int i = 0; i < 3; ++i) {
        pthread_mutex_lock(&m);
        ++cnt;
        pthread_mutex_unlock(&m);
    }
    return NULL;
}

int main(void) {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, worker, NULL);
    pthread_create(&t2, NULL, worker, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("counter = %ld\n", cnt);
    return 0;
}