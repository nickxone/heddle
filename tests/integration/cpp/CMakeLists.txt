heddle_integration_test(abba.cpp SEEDS 100 SOMETIMES deadlock)
heddle_integration_test(counter.cpp SEEDS 50)
heddle_integration_test(uncaught_exception.cpp SEEDS 10 ALWAYS crash)

# Need pthread_mutex_trylock, pthread_cond_*, and mutex types in the model
heddle_integration_test(scoped_lock.cpp SEEDS 50 PENDING)
heddle_integration_test(condition_variable.cpp SEEDS 50 PENDING)
heddle_integration_test(recursive_mutex.cpp SEEDS 20 PENDING)
