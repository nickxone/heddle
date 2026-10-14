function(heddle_add_core name)
    add_library(${name} STATIC
            core/random_strategy.cpp
            core/model.cpp
            core/scheduler.cpp)
    target_include_directories(${name} PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR})
    set_target_properties(${name} PROPERTIES
            POSITION_INDEPENDENT_CODE ON)
    target_compile_options(${name} PRIVATE
            -fvisibility=hidden -ftls-model=initial-exec)
endfunction()

heddle_add_core(heddle_core)

if (HEDDLE_SANITIZE)
    heddle_add_core(heddle_core_sanitized)
    target_compile_options(heddle_core_sanitized PUBLIC ${HEDDLE_SANITIZER_FLAGS})
    target_link_options(heddle_core_sanitized PUBLIC ${HEDDLE_SANITIZER_FLAGS})
endif ()

add_library(heddle_shim SHARED
        shim/fatal.cpp
        shim/real.cpp
        shim/runtime.cpp
        shim/interpose.cpp)
target_link_options(heddle_shim PRIVATE
        "LINKER:--version-script=${CMAKE_CURRENT_SOURCE_DIR}/shim/exports.ver")
target_compile_options(heddle_shim PRIVATE
        -fvisibility=hidden -ftls-model=initial-exec)
set_target_properties(heddle_shim PROPERTIES
        LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shim/exports.ver)
target_link_libraries(heddle_shim PRIVATE
        heddle_core Threads::Threads ${CMAKE_DL_LIBS})
