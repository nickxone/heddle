add_library(heddle_shim SHARED shim/random.cpp shim/runtime.cpp shim/interpose.cpp)
target_compile_options(heddle_shim PRIVATE -fvisibility=hidden -ftls-model=initial-exec)
target_link_libraries(heddle_shim PRIVATE Threads::Threads ${CMAKE_DL_LIBS})

add_test(NAME shim_interposition_hygiene
        COMMAND ${CMAKE_SOURCE_DIR}/tools/check-interposition.sh
        $<TARGET_FILE:heddle_shim>)