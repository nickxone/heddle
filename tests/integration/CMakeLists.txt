set(RUN_SEEDS ${CMAKE_SOURCE_DIR}/tools/run-seeds.sh)

function(heddle_integration_test name seeds codes)
    add_executable(it_${name} ${name}.c)
    target_link_libraries(it_${name} PRIVATE Threads::Threads)
    add_test(NAME integration.${name}
            COMMAND ${RUN_SEEDS} $<TARGET_FILE:heddle_shim> ${seeds} ${codes} $<TARGET_FILE:it_${name}>)
endfunction()

heddle_integration_test(relock 20 120)
heddle_integration_test(unlock_not_owned 20 121)
heddle_integration_test(lock_then_join 20 120)
heddle_integration_test(counter 50 0)
heddle_integration_test(thread_exit 50 0)
heddle_integration_test(abba 100 0,120)

add_test(NAME integration.passthrough_without_seed
        COMMAND ${CMAKE_COMMAND} -E env LD_PRELOAD=$<TARGET_FILE:heddle_shim> $<TARGET_FILE:it_counter>)