add_library(heddle_shim SHARED shim/runtime.cpp shim/interpose.cpp shim/random_strategy.cpp)
target_compile_options(heddle_shim PRIVATE -fvisibility=hidden -ftls-model=initial-exec)
target_link_libraries(heddle_shim PRIVATE Threads::Threads ${CMAKE_DL_LIBS})
target_link_options(heddle_shim PRIVATE
    "LINKER:--version-script=${CMAKE_CURRENT_SOURCE_DIR}/shim/exports.ver")
set_target_properties(heddle_shim PROPERTIES
    LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shim/exports.ver)

add_test(NAME shim_interposition_hygiene
        COMMAND ${CMAKE_SOURCE_DIR}/tools/check-interposition.sh
        $<TARGET_FILE:heddle_shim>)