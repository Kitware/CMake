enable_language(C)

set(CMAKE_TEST_BUILD_DEPENDS ON)
enable_testing()

add_executable(testexe main.c)
add_test(NAME prep COMMAND testexe)

add_library(installlib STATIC simplelib.c)
install(TARGETS installlib DESTINATION lib/$<CONFIG> COMPONENT Dev)

include(${CMAKE_CURRENT_LIST_DIR}/Common.cmake)
generate_output_files(testexe installlib)
