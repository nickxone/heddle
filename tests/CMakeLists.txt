add_subdirectory(unit)
add_subdirectory(tools)

# Test that the shim doesn't call any functions it defines
add_test(NAME shim.interposition_hygiene
        COMMAND ${CMAKE_SOURCE_DIR}/tools/check-interposition.sh
        $<TARGET_FILE:heddle_shim>)
