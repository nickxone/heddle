add_library(heddle_core STATIC
        core/random_strategy.cpp
        core/model.cpp)
target_include_directories(heddle_core PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(heddle_core PROPERTIES
        POSITION_INDEPENDENT_CODE ON)
target_compile_options(heddle_core PRIVATE
        -fvisibility=hidden -ftls-model=initial-exec)

add_library(heddle_shim SHARED
        shim/runtime.cpp shim/interpose.cpp)
target_link_options(heddle_shim PRIVATE
        "LINKER:--version-script=${CMAKE_CURRENT_SOURCE_DIR}/shim/exports.ver")
target_compile_options(heddle_shim PRIVATE
        -fvisibility=hidden -ftls-model=initial-exec)
set_target_properties(heddle_shim PROPERTIES
        LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shim/exports.ver)
target_link_libraries(heddle_shim PRIVATE
        heddle_core Threads::Threads ${CMAKE_DL_LIBS})
