find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(unit_tests rng_test.cpp random_strategy_test.cpp model_test.cpp)
target_link_libraries(unit_tests PRIVATE heddle_core GTest::gtest_main)
gtest_discover_tests(unit_tests TEST_PREFIX "unit.")
