#define _GNU_SOURCE

#include <dlfcn.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

static int (*real_mutex_lock)(pthread_mutex_t*);
static int (*real_mutex_unlock)(pthread_mutex_t*);
static int (*real_create)(pthread_t*, const pthread_attr_t*, void* (*)(void*),
                          void*);
static int (*real_join)(pthread_t thread, void** retval);

static void log_line(const char* s) { write(STDERR_FILENO, s, strlen(s)); }

int pthread_mutex_lock(pthread_mutex_t* m) {
    if (!real_mutex_lock) {
        real_mutex_lock = dlsym(RTLD_NEXT, "pthread_mutex_lock");
        if (!real_mutex_lock) {
            log_line("resolve failed: lock\n");
            _exit(127);
        }
    }

    log_line("lock\n");
    return real_mutex_lock(m);
}

int pthread_mutex_unlock(pthread_mutex_t* m) {
    if (!real_mutex_unlock) {
        real_mutex_unlock = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
        if (!real_mutex_unlock) {
            log_line("resolve failed: unlock\n");
            _exit(127);
        }
    }

    log_line("unlock\n");
    return real_mutex_unlock(m);
}

int pthread_create(pthread_t* thread, const pthread_attr_t* attr,
                   void* (*start)(void*), void* arg) {
    if (!real_create) {
        real_create = dlsym(RTLD_NEXT, "pthread_create");
        if (!real_create) {
            log_line("resolve failed: create\n");
            _exit(127);
        }
    }

    log_line("create thread\n");
    return real_create(thread, attr, start, arg);
}

int pthread_join(pthread_t thread, void** retval) { 
    if (!real_join) {
        real_join = dlsym(RTLD_NEXT, "pthread_join");
        if (!real_join) {
            log_line("resolve failed: join\n");
            _exit(127);
        }
    } 
    return real_join(thread, retval);
}
