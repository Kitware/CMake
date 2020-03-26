project(unitybuild_object_library C)

set(CMAKE_UNITY_BUILD ON) # This tests that the variable works in addition to the property

add_library(lib OBJECT func.c)

add_library(other-lib STATIC func.c)

add_executable(main main.c)
target_link_libraries(main PRIVATE lib)

enable_testing()
add_test(NAME main COMMAND main)
