enable_language(CXX)

foreach(i 1 2 3)
    file(WRITE ${CMAKE_BINARY_DIR}/empty${i}.cpp "void nothing${i}() {}\n")
endforeach()

add_library(base_lib_static STATIC ${CMAKE_BINARY_DIR}/empty1.cpp)
target_precompile_headers(base_lib_static PRIVATE <vector>)

add_library(object_lib OBJECT ${CMAKE_BINARY_DIR}/empty2.cpp)
target_precompile_headers(object_lib REUSE_FROM base_lib_static)

add_library(mid_lib_static STATIC ${CMAKE_BINARY_DIR}/empty3.cpp)
target_link_libraries(mid_lib_static PRIVATE object_lib)

add_executable(exec main.cpp)
target_link_libraries(exec PRIVATE mid_lib_static)
set_target_properties(exec PROPERTIES MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>)

target_precompile_headers(exec PRIVATE <string>)

enable_testing()
add_test(NAME exec COMMAND exec)
