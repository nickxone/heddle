set(HEDDLE_FAILURES "deadlock" "wrong use" "crash" "failure" "hang" "heddle internal error")

function(heddle_integration_test source)
    cmake_parse_arguments(PARSE_ARGV 1 arg "PENDING" "SEEDS;ALWAYS;SOMETIMES" "")
    cmake_path(GET source STEM name)
    cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME suite)
    set(target it_${suite}_${name})
    set(test integration.${suite}.${name})

    add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE Threads::Threads)
    add_test(NAME ${test}
            COMMAND $<TARGET_FILE:heddle> run --seeds ${arg_SEEDS} --carry-on -- $<TARGET_FILE:${target}>)
    set_tests_properties(${test} PROPERTIES DISABLED ${arg_PENDING})

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
    set_tests_properties(${test} PROPERTIES
            PASS_REGULAR_EXPRESSION "${pass}"
            FAIL_REGULAR_EXPRESSION "${unexpected}")
endfunction()

add_subdirectory(c)
add_subdirectory(cpp)

add_test(NAME integration.passthrough_without_seed
        COMMAND ${CMAKE_COMMAND} -E env LD_PRELOAD=$<TARGET_FILE:heddle_shim> $<TARGET_FILE:it_c_counter>)

add_test(NAME integration.driver_reports_exec_failure
        COMMAND $<TARGET_FILE:heddle> run --seed 1 -- /nonexistent)
set_tests_properties(integration.driver_reports_exec_failure PROPERTIES
        PASS_REGULAR_EXPRESSION "seed 1: failure \\(exit 127\\)")

add_test(NAME integration.driver_rejects_missing_program COMMAND $<TARGET_FILE:heddle> run)
set_tests_properties(integration.driver_rejects_missing_program PROPERTIES WILL_FAIL TRUE)