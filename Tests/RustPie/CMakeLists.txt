cmake_minimum_required(VERSION 4.2)

set(CMAKE_EXPERIMENTAL_RUST "6b6e613b-f6cb-402d-8aea-59034fa8c65b")

function(setup PREFIX)
    add_library(${PREFIX}_static STATIC ../static.rs)
    add_library(${PREFIX}_shared SHARED ../shared.rs)
    add_library(${PREFIX}_object OBJECT ../object.rs)

    add_executable(${PREFIX}_RustPie ../main.rs)
    target_link_libraries(
        ${PREFIX}_RustPie PRIVATE
        ${PREFIX}_static
        ${PREFIX}_shared
        ${PREFIX}_object
    )
endfunction()

project(RustPie LANGUAGES Rust)

add_subdirectory(Default)
add_subdirectory(Disabled)
add_subdirectory(Enabled)

file(
    GENERATE OUTPUT ${CMAKE_BINARY_DIR}/runner_$<CONFIG>.rs
    INPUT runner.rs
)

add_executable(RustPie ${CMAKE_BINARY_DIR}/runner_$<CONFIG>.rs)
add_dependencies(RustPie default_RustPie disabled_RustPie enabled_RustPie)
