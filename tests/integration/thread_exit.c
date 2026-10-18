#include <pthread.h>

static void nested_function() { pthread_exit((void *)12); }

static void *worker(void *arg) {
    (void)arg;
    nested_function();
    return NULL;
}

int main(void) {
    pthread_t t;
    pthread_create(&t, NULL, worker, NULL);
    void *result = NULL;
    pthread_join(t, &result);
    return !(result == (void *)12);
}