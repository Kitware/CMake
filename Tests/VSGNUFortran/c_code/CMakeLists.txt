add_executable(c_using_fortran main.c)
target_link_libraries(c_using_fortran hello)
target_compile_definitions(c_using_fortran PRIVATE "EXPECTED_BUILD_TYPE=\"$<CONFIG>\"")
