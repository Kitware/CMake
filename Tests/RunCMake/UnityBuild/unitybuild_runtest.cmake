project(unitybuild_runtest C)

set(CMAKE_UNITY_BUILD ON) # This tests that the variable works in addition to the property

add_library(lib main.c func.c)
add_executable(main main.c func.c)

enable_testing()
add_test(NAME main COMMAND main)
