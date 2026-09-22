add_subdirectory(unit)
add_subdirectory(tools)
add_test(NAME shim_interposition_hygiene
        COMMAND ${CMAKE_SOURCE_DIR}/tools/check-interposition.sh
        $<TARGET_FILE:heddle_shim>)
