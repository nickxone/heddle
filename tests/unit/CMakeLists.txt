find_package(GTest REQUIRED)
include(GoogleTest)

if(HEDDLE_SANITIZE)
    set(core heddle_core_sanitized)
else()
    set(core heddle_core)
endif()

add_executable(unit_tests
        rng_test.cpp
        random_strategy_test.cpp
        model_test.cpp
        scheduler_test.cpp)
target_link_libraries(unit_tests PRIVATE ${core} GTest::gmock GTest::gtest_main)
gtest_discover_tests(unit_tests TEST_PREFIX "unit.")
