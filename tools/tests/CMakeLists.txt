set(CHECK_INTERPOSITION ${CMAKE_SOURCE_DIR}/tools/check-interposition.sh)

add_library(fixture_plt_selfcall SHARED interposed.c caller.c)
add_test(NAME interposition_rejects_plt_selfcall
        COMMAND ${CHECK_INTERPOSITION} $<TARGET_FILE:fixture_plt_selfcall>)

# Correctly consider the failing selfcall test
set_tests_properties(interposition_rejects_plt_selfcall PROPERTIES WILL_FAIL TRUE)

add_library(fixture_same_tu_selfcall SHARED selfcall.c)
target_compile_options(fixture_same_tu_selfcall PRIVATE -Wno-infinite-recursion)
add_test(NAME interposition_blind_to_same_tu_selfcall
        COMMAND ${CHECK_INTERPOSITION} $<TARGET_FILE:fixture_same_tu_selfcall>)

add_executable(semaphore_probe semaphore_probe.cpp)
target_link_libraries(semaphore_probe PRIVATE Threads::Threads)
add_test(NAME semaphore_probe_avoids_interposed_symbols
        COMMAND readelf -rW $<TARGET_FILE:semaphore_probe>)
set_tests_properties(semaphore_probe_avoids_interposed_symbols
        PROPERTIES FAIL_REGULAR_EXPRESSION " sem_| pthread_(mutex|cond)")