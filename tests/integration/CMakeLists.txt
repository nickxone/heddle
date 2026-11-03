set(HEDDLE_FAILURES "deadlock" "wrong use" "crash" "failure" "hang" "heddle internal error")

function(heddle_integration_test name)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "SEEDS;ALWAYS;SOMETIMES" "")
    add_executable(it_${name} ${name}.c)
    target_link_libraries(it_${name} PRIVATE Threads::Threads)
    add_test(NAME integration.${name}
            COMMAND $<TARGET_FILE:heddle> run --seeds ${arg_SEEDS} --carry-on -- $<TARGET_FILE:it_${name}>)

    set(expected ${arg_ALWAYS} ${arg_SOMETIMES})
    if (NOT expected)
        return()
    endif ()

    set(unexpected ${HEDDLE_FAILURES})
    list(REMOVE_ITEM unexpected ${expected})
    list(TRANSFORM unexpected PREPEND "seed [0-9]+: ")
    list(APPEND unexpected "replaying the seed")
    set(every_seed_failed "${arg_SEEDS} failing seed\\(s\\) in ${arg_SEEDS} run")

    if (arg_ALWAYS)
        set(pass ${every_seed_failed})
    else ()
        set(pass "seed [0-9]+: ${expected}")
        list(APPEND unexpected ${every_seed_failed})
    endif ()
    set_tests_properties(integration.${name} PROPERTIES
            PASS_REGULAR_EXPRESSION "${pass}"
            FAIL_REGULAR_EXPRESSION "${unexpected}")
endfunction()

heddle_integration_test(relock SEEDS 20 ALWAYS deadlock)
heddle_integration_test(unlock_not_owned SEEDS 20 ALWAYS "wrong use")
heddle_integration_test(lock_then_join SEEDS 20 ALWAYS deadlock)
heddle_integration_test(counter SEEDS 50)
heddle_integration_test(thread_exit SEEDS 50)
heddle_integration_test(abba SEEDS 100 SOMETIMES deadlock)

add_test(NAME integration.passthrough_without_seed
        COMMAND ${CMAKE_COMMAND} -E env LD_PRELOAD=$<TARGET_FILE:heddle_shim> $<TARGET_FILE:it_counter>)

add_test(NAME integration.driver_reports_exec_failure
        COMMAND $<TARGET_FILE:heddle> run --seed 1 -- /nonexistent)
set_tests_properties(integration.driver_reports_exec_failure PROPERTIES
        PASS_REGULAR_EXPRESSION "seed 1: failure \\(exit 127\\)")

add_test(NAME integration.driver_rejects_missing_program COMMAND $<TARGET_FILE:heddle> run)
set_tests_properties(integration.driver_rejects_missing_program PROPERTIES WILL_FAIL TRUE)